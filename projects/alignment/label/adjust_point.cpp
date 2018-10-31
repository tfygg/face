#include "window.h"
NAMESPACE_BEGIN

static const float Threshold = 0.01;

void LabelWindow::_pushOperation(Modification *ptr) {
    // remove after
    for (auto iter = mOpIter; iter != mOpList.end(); ++iter) {
        delete (*iter);
    }
    mOpList.erase(mOpIter, mOpList.end());
    // push
    mOpList.push_back(ptr);
    while (mOpList.size() > Modification::MaxOperations) {
        delete mOpList.front();
        mOpList.pop_front();
    }
    mOpIter = mOpList.end();
}
void LabelWindow::_redoOperation(const Modification &modification) {
    Landmarks *lmsPtr = nullptr;
    if (mImagePtr)      { lmsPtr = mDataPtr->fetch(0); }
    else if (mVideoPtr) { 
        mVideoHelperPtr->mTimestamp = modification.mTimestamp;
        lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp);
    }
    if (lmsPtr == nullptr) return;

    mLmsMessage = "[do]: ";
    if (modification.mOperation == Modification::MovePoint) {
        mLmsMessage += "move point";
        const auto &ptsList = ((std::pair<std::vector<snow::float2>, std::vector<snow::float2>>*)modification.mDataPtr)->second;
        lmsPtr->setLandmarks("manual", ptsList);
        auto iter = std::find(Landmarks::GenreList().begin(), Landmarks::GenreList().end(), std::string("manual"));
        mDataPtr->mLmsGenre = std::distance(Landmarks::GenreList().begin(), iter);
    }
    if (modification.mOperation == Modification::ReplaceManual) {
        mLmsMessage += "replace manual";
        const auto &ptsList = ((std::pair<std::vector<snow::float2>, std::vector<snow::float2>>*)modification.mDataPtr)->second;
        lmsPtr->setLandmarks("manual", ptsList);
        auto iter = std::find(Landmarks::GenreList().begin(), Landmarks::GenreList().end(), std::string("manual"));
        mDataPtr->mLmsGenre = std::distance(Landmarks::GenreList().begin(), iter);
    }
    if (modification.mOperation == Modification::DiscardManual) {
        mLmsMessage += "discard manual";
        lmsPtr->removeLandmarks("manual");
        mDataPtr->mLmsGenre = 0;
    }
}

void LabelWindow::_undoOperation(const Modification &modification) {
    Landmarks *lmsPtr = nullptr;
    if (mImagePtr)      { lmsPtr = mDataPtr->fetch(0); }
    else if (mVideoPtr) { 
        mVideoHelperPtr->mTimestamp = modification.mTimestamp;
        lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp);
    }
    if (lmsPtr == nullptr) return;

    mLmsMessage = "[undo]: ";
    if (modification.mOperation == Modification::MovePoint) {
        mLmsMessage += "move point";
        const auto &ptsList = ((std::pair<std::vector<snow::float2>, std::vector<snow::float2>>*)modification.mDataPtr)->first;
        lmsPtr->setLandmarks("manual", ptsList);
        auto iter = std::find(Landmarks::GenreList().begin(), Landmarks::GenreList().end(), std::string("manual"));
        mDataPtr->mLmsGenre = std::distance(Landmarks::GenreList().begin(), iter);
    }
    if (modification.mOperation == Modification::ReplaceManual) {
        mLmsMessage += "replace manual";
        const auto &ptsList = ((std::pair<std::vector<snow::float2>, std::vector<snow::float2>>*)modification.mDataPtr)->first;
        if (ptsList.size()) lmsPtr->setLandmarks("manual", ptsList);
        else                lmsPtr->removeLandmarks("manual");
        mDataPtr->mLmsGenre = 0;
    }
    if (modification.mOperation == Modification::DiscardManual) {
        mLmsMessage += "discard manual";
        lmsPtr->setLandmarks("manual", *(std::vector<snow::float2>*)modification.mDataPtr);
        auto iter = std::find(Landmarks::GenreList().begin(), Landmarks::GenreList().end(), std::string("manual"));
        mDataPtr->mLmsGenre = std::distance(Landmarks::GenreList().begin(), iter);
    }
}

void LabelWindow::selectPoint(int x, int y) {
    auto area = this->validArea();
    snow::float2 pos { (x - area[0]) / (float)area[2],
                       (y - area[1]) / (float)area[3] };
    // get current landmarks
    const Landmarks *lmsPtr = nullptr;
    if (mImagePtr)      lmsPtr = mDataPtr->fetch(0);
    else if (mVideoPtr) lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp);
    if (lmsPtr && lmsPtr->hasGenre(Landmarks::GenreList()[mDataPtr->mLmsGenre])) {
        const auto &ptsList = lmsPtr->landmarks(Landmarks::GenreList()[mDataPtr->mLmsGenre]);
        float minDist = 1e10;
        int   index = -1;
        for (size_t i = 0; i < ptsList.size(); ++i) {
            float dist = snow::euclidean_norm(ptsList[i] - pos);
            if (dist < minDist) {
                minDist = dist;
                index = (int)i;
            }
        }
        if (minDist < Threshold) {
            snow::info("select point {}", index);
            mSelectPointIndex = index;
            mLmsMessage = std::string("select point ") + std::to_string(mSelectPointIndex);
            mTmpPtsList = lmsPtr->landmarks(Landmarks::GenreList()[mDataPtr->mLmsGenre]);
        }
        else {
            mSelectPointIndex = -1;
            mLmsMessage = "";
            mTmpPtsList.clear();
        }
    }
}
void LabelWindow::movePoint(int x, int y) {
    if (mSelectPointIndex < 0) return;
    if (mTmpPtsList.size() == 0) return;
    Landmarks *lmsPtr = nullptr;
    if (mImagePtr)      lmsPtr = mDataPtr->fetch(0);
    else if (mVideoPtr) lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp);
    if (lmsPtr == nullptr) return;
    auto &ptsList = lmsPtr->landmarks(Landmarks::GenreList()[mDataPtr->mLmsGenre]);
    auto area = this->validArea();
    ptsList[mSelectPointIndex].x = (float)(x - area[0]) / (float)area[2];
    ptsList[mSelectPointIndex].y = (float)(y - area[1]) / (float)area[3];
}
void LabelWindow::releasePoint() {
    if (mSelectPointIndex < 0) return;
    if (mTmpPtsList.size() == 0) return;
    // [TODO]: redo this operation
    {
        int64_t    ms     = 0;
        Landmarks *lmsPtr = nullptr;
        if (mImagePtr)      { lmsPtr = mDataPtr->fetch(0); ms = 0; }
        else if (mVideoPtr) { lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp); ms = mVideoHelperPtr->mTimestamp; }
        if (lmsPtr == nullptr) return;

        std::string genre = Landmarks::GenreList()[mDataPtr->mLmsGenre];
        if (genre != "manual") {
            auto *pair = new std::pair<std::vector<snow::float2>, std::vector<snow::float2>>();
            if (lmsPtr->hasGenre("manual"))
                pair->first = lmsPtr->landmarks("manual");
            pair->second = lmsPtr->landmarks(genre);
            lmsPtr->setLandmarks(genre, mTmpPtsList);

            auto *ptr       = new Modification();
            ptr->mOperation = Modification::ReplaceManual;
            ptr->mTimestamp = ms;
            ptr->mDataPtr   = (void*)pair;
            _redoOperation(*ptr);
            _pushOperation(ptr);
        }
        else {
            auto *pair = new std::pair<std::vector<snow::float2>, std::vector<snow::float2>>();
            pair->first  = mTmpPtsList;
            pair->second = lmsPtr->landmarks("manual");
            lmsPtr->setLandmarks("manual", mTmpPtsList);

            auto *ptr       = new Modification();
            ptr->mOperation = Modification::MovePoint;
            ptr->mTimestamp = ms;
            ptr->mDataPtr   = (void*)pair;
            _redoOperation(*ptr);
            _pushOperation(ptr);
        }
    }
    // reset
    mSelectPointIndex = -1;
    mLmsMessage = "";
    mTmpPtsList.clear();
}
void LabelWindow::discardThisManualFrame() {
    // [TODO]: redo this operation
    {
        int64_t    ms     = 0;
        Landmarks *lmsPtr = nullptr;
        if (mImagePtr)      { lmsPtr = mDataPtr->fetch(0); ms = 0; }
        else if (mVideoPtr) { lmsPtr = mDataPtr->fetch(mVideoHelperPtr->mTimestamp); ms = mVideoHelperPtr->mTimestamp; }
        if (lmsPtr == nullptr) return;

        if (lmsPtr->hasGenre("manual")) {
            std::vector<snow::float2> *manual = new std::vector<snow::float2>(Landmarks::NumPoints());
            *manual = lmsPtr->landmarks("manual");
            auto *ptr       = new Modification();
            ptr->mOperation = Modification::DiscardManual;
            ptr->mTimestamp = ms;
            ptr->mDataPtr   = (void*)manual;
            _redoOperation(*ptr);
            _pushOperation(ptr);
        }
    }
}
void LabelWindow::processEvent(SDL_Event &event) {
    auto onLeftDown = [this](SDL_Event &event) -> void {
        this->selectPoint(event.button.x, event.button.y);
    };
    auto onLeftMove = [this](SDL_Event &event) -> void {
        this->movePoint(event.motion.x, event.motion.y);
    };
    auto onLeftUp   = [this](SDL_Event &event) -> void {
        this->releasePoint();
    };
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        // mouse dowwn
        if (event.button.button == SDL_BUTTON_LEFT) onLeftDown(event);
    }
    else if (event.type == SDL_MOUSEMOTION) {
        // mouse motion
        if (event.button.button == SDL_BUTTON_LEFT) onLeftMove(event);
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
        // mouse up
        if (event.button.button == SDL_BUTTON_LEFT) onLeftUp(event);
    }
}

NAMESPACE_END