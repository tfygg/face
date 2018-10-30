#include "window.h"
#include <fstream>

NAMESPACE_BEGIN
const std::vector<std::string> LabelWindow::ImageExtensions {"JPG", "JPEG", "PNG", "BMP"};
const std::vector<std::string> LabelWindow::VideoExtensions {"MKV", "AVI", "MP4", "MPEG", "FLV"};

bool LabelWindow::_getFrame() {
    mVideoHelperPtr->mFrame = std::move(mVideoPtr->readFrame(mVideoHelperPtr->mStream.get()));
    if (mVideoHelperPtr->mFrame == nullptr) return false;
    const auto *ptr = (const snow::VideoFrame *)mVideoHelperPtr->mFrame.get();
    if (ptr->isNull()) return false;
    mVideoHelperPtr->mTimestamp = ptr->timestamp();
    if (mShaderPtr == nullptr) {
        // alloc shader
        mShaderPtr = new ImageShader(Landmarks::NumPoints());
        // adjust ratio for draw()
        this->setRatio((float)ptr->mWidth / (float)ptr->mHeight);
    }
    /* update data */ {
        mShaderPtr->uploadImage(ptr->data(), ptr->mWidth, ptr->mHeight, GL_RGBA);
    }
    return true;
}
bool LabelWindow::openVideo(std::string filename) {
    snow::info("open video {}", filename);
    closeSource();
    snow::MediaReader::initializeFFmpeg();
    mVideoPtr = new snow::MediaReader(filename);
    mVideoPtr->setSyncVideoStreams(false); // only use first video track, close video sync.
    if (!mVideoPtr->open()) {
        snow::error("Failed to open video {}", filename);
        closeSource();
        return false;
    }
    mVideoHelperPtr = new VideoHelper();
    auto streams = mVideoPtr->getStreams();
    for (auto &st : streams) {
        if (st->type() == snow::MediaType::Video) {
            mVideoHelperPtr->mStream = st;
            break;
        }
    }
    if (mVideoHelperPtr->mStream == nullptr) {
        snow::error("Failed to find video stream in {}", filename);
        closeSource();
        return false;
    }
    // alloc data
    mDataPtr = new LabelData(LabelData::TypeVideo, filename);
    _getFrame();
    return true;
}
bool LabelWindow::openImage(std::string filename) {
    snow::info("open image {}", filename);
    closeSource();
    // read source
    mImagePtr = new snow::Image();
    *mImagePtr = snow::Image::Read(filename);
    snow::assertion(mImagePtr->bpp() == 4 || mImagePtr->bpp() == 3,
                    "[LabelWindow]: image {} is not 24bits or 32bits", filename);
    // alloc shader
    mShaderPtr = new ImageShader(Landmarks::NumPoints());
    mShaderPtr->uploadImage(mImagePtr->data(), mImagePtr->width(), mImagePtr->height(),
                            (mImagePtr->bpp() == 4)? GL_RGBA: GL_RGB);
    // adjust ratio for draw()
    this->setRatio((float)mImagePtr->width() / (float) mImagePtr->height());
    // alloc data
    mDataPtr = new LabelData(LabelData::TypeImage, filename);
    return true;
}
void LabelWindow::closeSource() {
    if (mShaderPtr)      { delete mShaderPtr;      mShaderPtr      = nullptr; }
    if (mImagePtr)       { delete mImagePtr;       mImagePtr       = nullptr; }
    if (mVideoPtr)       { delete mVideoPtr;       mVideoPtr       = nullptr; }
    if (mVideoHelperPtr) { delete mVideoHelperPtr; mVideoHelperPtr = nullptr; }
    if (mDataPtr) {
        // save or not?
        bool save = true;
        if (!mForcingOverwrite && snow::path::Exists(mDataPtr->mLabelPath))
            save = snow::App::Query("Save", "replace existed result or not?");
        if (save) { // saveing
            snow::info("save landmarks {}", mDataPtr->mLabelPath);
            if (!mDataPtr->saveIntoFile())
                snow::fatal("[LabelWindow]: failed to save result.");
        }
        // finally delete and set null
        delete mDataPtr; mDataPtr = nullptr;
    }
}

/* drawing */
void LabelWindow::draw() {
    mGuiHeight = 0;
    _drawTools();
    _drawLandmarks(); // must be done before shader->draw()
    if (mVideoPtr) {
        _drawVideoController();
    }
    if (mShaderPtr) {
        mShaderPtr->use();
        mShaderPtr->draw();
    }
}
void LabelWindow::_drawLandmarks() {
    if (!mDataPtr) return;
    ImGui::SetNextWindowPos(ImVec2(0, mGuiHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250, 180), ImGuiCond_FirstUseEver);
    ImGui::Begin("Landmarks");
    mGuiHeight += ImGui::GetWindowSize().y;
    ImGui::Text("%s", mLmsMessage.c_str());
    Landmarks *lmsPtr = nullptr;
    if (mImagePtr)      lmsPtr = mDataPtr->fetch(0);
    else if (mVideoPtr) lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp);
    if (lmsPtr) {
        int count = 0;
        for (size_t i = 0; i < Landmarks::GenreList().size(); ++i) {
            if (lmsPtr->hasGenre(Landmarks::GenreList()[i])) {
                if (count % 2 == 1) ImGui::SameLine();
                ImGui::RadioButton(Landmarks::GenreList()[i].c_str(), &(mDataPtr->mLmsGenre), (int)i);
                count += 1;
            }
        }
    }
    if (ImGui::Button("Discard manually modified points"))
        this->discardThisManualFrame();
    if (lmsPtr && lmsPtr->hasGenre(Landmarks::GenreList()[mDataPtr->mLmsGenre]))
        mShaderPtr->updateLandmarks(lmsPtr->landmarks(Landmarks::GenreList()[mDataPtr->mLmsGenre]));
    else
        mShaderPtr->updateLandmarks(std::vector<snow::float2>(Landmarks::NumPoints(), {0, 0}));
    ImGui::End();
}
void LabelWindow::_drawTools() {
    bool open = false;
    std::string focusPath = "";
    ImGui::SetNextWindowPos(ImVec2(0, mGuiHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250, 180), ImGuiCond_FirstUseEver);
    ImGui::Begin("Files");
    mGuiHeight += ImGui::GetWindowSize().y;
    /* list */ {
        ImGui::PushItemWidth(-1);
        // IMGUI_API bool  ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);
        ImGui::ListBox("", &mCurFileIndex, [](void *data, int idx, const char **out_text)->bool {
            auto &list = *(std::vector<std::string> *)data;
            if (idx < 0 || idx >= list.size()) return false;
            *out_text = list[idx].c_str();
            return true;
        }, (void *)&mFileList, (int)mFileList.size(), 5);
        if (mCurFileIndex >= 0 && mCurFileIndex < mFileList.size())
            focusPath = mFileList[mCurFileIndex];
    }
    /* open button */ {
        open = ImGui::Button("Open"); ImGui::SameLine(0.0, 5);
        ImGui::Text("%s", focusPath.c_str());
    }
    /* options */ {
        ImGui::Checkbox("Forcing overwrite labels", &mForcingOverwrite);
    }
    ImGui::End(); // Files

    /* open */ if (open && focusPath.length() && focusPath != mSelectedPath) {
        if (!snow::path::Exists(focusPath)) snow::App::Query("Error", std::string("Failed to find ") + focusPath);
        else {
            /* exist, make sure the type and open */
            auto ext = snow::path::SplitExtension(focusPath).second;
            std::transform(ext.begin(), ext.end(), ext.begin(), [](char in)->char{
                if (in <= 'z' && in >= 'a') return in - ('z' - 'Z');
                return in;
            });
            bool isImage = (std::find(ImageExtensions.begin(), ImageExtensions.end(), ext) != ImageExtensions.end());
            bool isVideo = (std::find(VideoExtensions.begin(), VideoExtensions.end(), ext) != VideoExtensions.end());
            if (!isImage && !isVideo) {
                snow::App::Query("Error", std::string("Unknown extension `") + ext + "`");
            }
            else {
                // open
                if (isImage)      openImage(focusPath);
                else if (isVideo) openVideo(focusPath);
                mSelectedPath = focusPath;
                this->setTitle(mSelectedPath);
            }
        }
    }
}
void LabelWindow::_drawVideoController() {
    if (mVideoPtr == nullptr) return;
    auto _seek = [this](float delta = 0.0) -> void {
        mVideoHelperPtr->mSeconds += delta;
        if (mVideoHelperPtr->mSeconds < 0) mVideoHelperPtr->mSeconds = 0.0f;
        if (mVideoHelperPtr->mSeconds > mVideoPtr->duration() / 1000.0f) mVideoHelperPtr->mSeconds = mVideoPtr->duration() / 1000.0f;
        mVideoPtr->seek(mVideoHelperPtr->mSeconds * 1000.0f);
        _getFrame();
    };
    // snow::info("timestamp {}", mVideoHelperPtr->mTimestamp);
    ImGui::SetNextWindowPos(ImVec2(0, mGuiHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Player");
    mGuiHeight += ImGui::GetWindowSize().y;
    mVideoHelperPtr->mSeconds = (float)mVideoHelperPtr->mTimestamp / 1000.0f;
    if (ImGui::SliderFloat("sec", &mVideoHelperPtr->mSeconds, 0, mVideoPtr->duration() / 1000.0f)) {
        mVideoHelperPtr->mPlaying = false;
        _seek();
    }
    /* frame by frame option */
    ImGui::SameLine();
    if (!mVideoHelperPtr->mPlaying) {
        mVideoHelperPtr->mPlaying = ImGui::Button("Play");
        if (ImGui::Button("<<")) { _seek(-0.5); } ImGui::SameLine();
        if (ImGui::Button(">>")) { _seek(0.5);  } ImGui::SameLine();
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Space))) {
            ImGui::Button(" > (hold) "); if (ImGui::IsItemActive()) { _getFrame(); }
        } else {
            if (ImGui::Button(" > (next frame) ")) { _getFrame(); }
        }
        ImGui::Text("hold `space` to switch mode");
    }
    else {
        if (ImGui::Button("Stop")) {
            mVideoHelperPtr->mPlaying = false;
        }
    }
    ImGui::End();
    /* playing */
    if (mVideoHelperPtr->mPlaying) {
        if (!_getFrame()) mVideoHelperPtr->mPlaying = false;
    }
}

bool LabelWindow::onQuit(SDL_Event &event) {
    closeSource();
    return true;
}

NAMESPACE_END