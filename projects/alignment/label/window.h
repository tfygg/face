#pragma once
#include "../../common.h"
#include "../../utils/landmarks.h"
#include "shader_image.h"
NAMESPACE_BEGIN

struct LabelData {
    static const int TypeNone  = 0;
    static const int TypeImage = 1;
    static const int TypeVideo = 2;
    int                          mType;
    std::string                  mLabelPath;
    std::map<int64_t, Landmarks> mTimeLmsMap;

    LabelData(int type, std::string path): mType(type), mLabelPath(path + "_label.lms"), mTimeLmsMap() {}
    ~LabelData () {}
};

/** LabelWindow: labelling image or video
 *  - image 
 *  - video
 *  - list of images, videos
 * */
class LabelWindow : public snow::AbstractWindow {
    // data source
    snow::Image *       mImagePtr;
    snow::MediaReader * mVideoPtr;
    // labelling data
    LabelData *         mDataPtr;
    // image shader
    ImageShader *       mShaderPtr;
public:
    LabelWindow(const char *title="label")
        : snow::AbstractWindow(title)
        , mImagePtr(nullptr), mVideoPtr(nullptr), mDataPtr(nullptr)
        , mShaderPtr(nullptr)
    {}

    bool openVideo(std::string);
    bool openImage(std::string);
    void closeSource(); // close image or video, save and release data

    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event) {}
    void draw();
};

NAMESPACE_END