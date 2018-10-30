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

struct VideoHelper {
    std::shared_ptr<snow::StreamBase> mStream;
    std::unique_ptr<snow::FrameBase>  mFrame;
    int64_t                           mTimestamp;
    float                             mSeconds;
    bool                              mPlaying;

    VideoHelper(): mStream(nullptr), mFrame(nullptr), mTimestamp(0), mSeconds(0.0), mPlaying(false) {}
    ~VideoHelper() { mStream.reset(); mFrame.reset(); }
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
    VideoHelper *       mVideoHelperPtr;
    // labelling data
    LabelData *         mDataPtr;
    // image shader
    ImageShader *       mShaderPtr;
    // for selecting
    std::string         mSelectedPath;
    int                 mCurFileIndex;
    std::vector<std::string> mFileList;
    // forcing overwrite
    bool                mForcingOverwrite;

    // for easy video read
    bool _getFrame();
    void _drawTools();
    void _drawVideoController();
public:
    static const std::vector<std::string> ImageExtensions;
    static const std::vector<std::string> VideoExtensions;
    LabelWindow(const std::vector<std::string> &fileList,
                const char *title="label")
        : snow::AbstractWindow(title)
        , mImagePtr(nullptr), mVideoPtr(nullptr)
        , mVideoHelperPtr(nullptr)
        , mDataPtr(nullptr) , mShaderPtr(nullptr)
        , mSelectedPath("") , mCurFileIndex(-1) , mFileList(fileList)
        , mForcingOverwrite(false)
    {}
    ~LabelWindow() { closeSource(); mFileList.clear(); mCurFileIndex = -1; }

    bool openVideo(std::string);
    bool openImage(std::string);
    void closeSource(); // close image or video, save and release data

    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event) {}
    bool onQuit(SDL_Event &event);
    void draw();
};

NAMESPACE_END