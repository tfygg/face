#include "window.h"
#include <fstream>
NAMESPACE_BEGIN

LabelData::LabelData(int type, std::string path)
    : mType(type), mLabelPath(path + "_label.lms"), mTimeLmsMap()
    , mLmsGenre(0) {
    if (!loadFromFile()) {
        // try to load from legacy format
        loadFromFile_Legacy(path + "_lmrecord");
    }
}
LabelData::~LabelData() { mTimeLmsMap.clear(); }
Landmarks *LabelData::fetch(int64_t ms) {
    auto iter = mTimeLmsMap.find(ms);
    if (iter == mTimeLmsMap.end()) return nullptr;
    else return &(iter->second);
}
bool LabelData::loadFromFile_Legacy(const std::string &oldFormatPath) {
    std::ifstream fin(oldFormatPath);
    if (!fin.is_open()) return false;
    while (!fin.eof()) {
        int64_t ms;
        int state, num;
        fin >> ms >> state >> num;
        Landmarks lms;
        snow::assertion(num == Landmarks::NumPoints(),
                        "[LabelData]: failed to load, because {} (file) != {} (gNumPoints)",
                        num, Landmarks::NumPoints());
        std::vector<snow::float2> pts(num);
        // actual
        for (int i = 0; i < num; ++i) fin >> pts[i].x >> pts[i].y;
        if (state > 0) lms.setLandmarks("actual", pts);
        // dde
        for (int i = 0; i < num; ++i) fin >> pts[i].x >> pts[i].y;
        if (state & 1) lms.setLandmarks("dde", pts);
        // dlib
        for (int i = 0; i < num; ++i) fin >> pts[i].x >> pts[i].y;
        if (state & 4) lms.setLandmarks("dlib", pts);
        // manual
        for (int i = 0; i < num; ++i) fin >> pts[i].x >> pts[i].y;
        if (state & 2) lms.setLandmarks("manual", pts);
        mTimeLmsMap.insert({ms, lms});
    }
    fin.close();
    return true;
}
bool LabelData::loadFromFile() {
    std::ifstream fin(mLabelPath);
    if (!fin.is_open()) return false;
    int numFrames = 0;
    fin >> numFrames;
    for (int i = 0; i < numFrames; ++i) {
        int64_t ms;
        Landmarks lms;
        fin >> ms >> lms;
        mTimeLmsMap.insert({ms, lms});
    }
    return true;
}
bool LabelData::saveIntoFile() {
    // rearange data
#define TIME_LMS std::pair<int64_t, const Landmarks *>
    std::vector<TIME_LMS> lmsList;
    for (auto iter = mTimeLmsMap.begin(); iter != mTimeLmsMap.end(); ++iter)
        lmsList.push_back({iter->first, &(iter->second)});
    std::sort(lmsList.begin(), lmsList.end(), [](const TIME_LMS &a, const TIME_LMS &b) -> bool {
        return a.first < b.first;
    });
#undef TIME_LMS
    std::ofstream fout(mLabelPath);
    if (fout.is_open()) {
        fout << lmsList.size() << std::endl;
        for (auto pair : lmsList)
            fout << pair.first << " " << *(pair.second);
        fout.close();
    }
    else return false;
    return true;
}

NAMESPACE_END