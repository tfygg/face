#pragma once
#include "../../common.h"
#include "../../utils/landmarks.h"
#include "shader_image.h"
#include <queue>
NAMESPACE_BEGIN

struct LabelData {
    static const int TypeNone  = 0;
    static const int TypeImage = 1;
    static const int TypeVideo = 2;
    int                          mType;
    std::string                  mLabelPath;
    std::map<int64_t, Landmarks> mTimeLmsMap;
    int                          mLmsGenre;

    LabelData(int type, std::string path);
    ~LabelData();

    Landmarks *fetch(int64_t ms);
    bool loadFromFile_Legacy(const std::string &oldFormatPath);
    bool loadFromFile();
    bool saveIntoFile();
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

/* for operation */
struct Modification {
    static const int MaxOperations = 5;
    enum OP { None = 0, MovePoint = 1, ReplaceManual = 2, DiscardManual = 3 };
    OP      mOperation;
    int64_t mTimestamp;
    void *  mDataPtr;

    Modification() : mOperation(OP::None), mTimestamp(0), mDataPtr(nullptr) {}
    ~Modification() {
        if (mOperation == MovePoint || mOperation == ReplaceManual)
            delete (std::pair<std::vector<snow::float2>, std::vector<snow::float2>> *)mDataPtr;
        else
            delete (std::vector<snow::float2> *)mDataPtr;
        mDataPtr = nullptr;
    }
};

/** LabelWindow: labelling image or video
 *  - image 
 *  - video
 *  - list of images, videos
 * */
class LabelWindow : public snow::AbstractWindow {
    // data source
    snow::Image *             mImagePtr;
    snow::MediaReader *       mVideoPtr;
    VideoHelper *             mVideoHelperPtr;
    // labelling data
    LabelData *               mDataPtr;
    // image shader
    ImageShader *             mShaderPtr;
    // for selecting
    std::string               mSelectedPath;
    int                       mCurFileIndex;
    std::vector<std::string>  mFileList;
    // forcing overwrite
    bool                      mForcingOverwrite;
    // for auto gui layout
    float                     mGuiHeight;
    std::string               mLmsMessage;

    // for manually adjusting
    int                       mSelectPointIndex;
    std::vector<snow::float2> mTmpPtsList;
    std::list<Modification*>  mOpList;
    std::list<Modification*>::iterator mOpIter;

    // for easy video read
    bool _getFrame();
    void _drawTools();
    void _drawVideoController();
    void _drawLandmarks();

    // for operations
    void _redoOperation(const Modification &modification);
    void _undoOperation(const Modification &modification);
    void _pushOperation(Modification *ptr);
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
        , mGuiHeight(0.f), mLmsMessage("")
        , mSelectPointIndex(-1)
        , mTmpPtsList(0)
        , mOpList(0)
        , mOpIter(mOpList.end())
    {}
    ~LabelWindow() {
        closeSource();
        mCurFileIndex     = -1;
        mSelectPointIndex = -1;
        mTmpPtsList.clear();
        mFileList.clear();
        while (mOpList.size() > 0) {
            delete mOpList.front();
            mOpList.pop_front();
        }
    }

    bool openVideo(std::string);
    bool openImage(std::string);
    void closeSource(); // close image or video, save and release data

    /*overwrite virtual functions*/
    void processEvent(SDL_Event &event);
    bool onQuit(SDL_Event &event);
    void draw();

    /* for manually adjusting */
    void selectPoint(int x, int y);
    void movePoint(int x, int y);
    void releasePoint();
    void discardThisManualFrame();
};

NAMESPACE_END