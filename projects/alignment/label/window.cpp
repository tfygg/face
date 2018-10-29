#include "window.h"
#include <fstream>

NAMESPACE_BEGIN

bool LabelWindow::openVideo(std::string filename) {

}
bool LabelWindow::openImage(std::string filename) {
    // read source
    mImagePtr = new snow::Image();
    *mImagePtr = snow::Image::Read(filename);
    snow::assertion(mImagePtr->bpp() == 4 || mImagePtr->bpp() == 3,
                    "[LabelWindow]: image {} is not 24bits or 32bits", filename);
    // alloc data
    mDataPtr = new LabelData(LabelData::TypeImage, filename);
    // alloc shader
    mShaderPtr = new ImageShader(Landmarks::NumPoints());
    mShaderPtr->uploadImage(mImagePtr->data(), mImagePtr->width(), mImagePtr->height(),
                            (mImagePtr->bpp() == 4)? GL_RGBA: GL_RGB);
    // adjust ratio for draw()
    this->setRatio((float)mImagePtr->width() / (float) mImagePtr->height());
}
void LabelWindow::closeSource() {
    if (mShaderPtr) { delete mShaderPtr; mShaderPtr = nullptr; }
    if (mImagePtr)  { delete mImagePtr;  mImagePtr  = nullptr; }
    if (mVideoPtr)  { delete mVideoPtr;  mVideoPtr  = nullptr; }
    if (mDataPtr) {
        // save or not?
        bool save = true;
        if (snow::path::Exists(mDataPtr->mLabelPath))
            save = snow::App::Query("Save", "replace existed result or not?");
        if (save) { // saveing
            // rearange data
#define TIME_LMS std::pair<int64_t, const Landmarks *>
            std::vector<TIME_LMS> lmsList;
            for (auto iter = mDataPtr->mTimeLmsMap.begin(); iter != mDataPtr->mTimeLmsMap.end(); ++iter)
                lmsList.push_back({iter->first, &(iter->second)});
            std::sort(lmsList.begin(), lmsList.end(), [](const TIME_LMS &a, const TIME_LMS &b) -> bool {
                return a.first < b.first;
            });
#undef TIME_LMS
            std::ofstream fout(mDataPtr->mLabelPath);
            if (fout.is_open()) {
                fout << lmsList.size() << std::endl;
                for (auto pair : lmsList)
                    fout << pair.first << " " << *(pair.second);
                fout.close();
            }
            else snow::fatal("[LabelWindow]: failed to save result.");
            lmsList.clear();
        }
        // finally delete and set null
        delete mDataPtr; mDataPtr = nullptr;
    }
}

/* drawing */
void LabelWindow::draw() {
    if (mShaderPtr) {
        mShaderPtr->use();
        mShaderPtr->draw();
    }
    ImGui::Begin("debug");
    ImGui::End();
}

NAMESPACE_END