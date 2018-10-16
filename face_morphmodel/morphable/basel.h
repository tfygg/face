#pragma once
#include <map>
#include <string>
#include <vector>
#include <fstream>

class BaselModel;
class ProbabilisticModel {
    friend class BaselModel;
    double *                mMeanPtr;
    double *                mPCABasisPtr;
    double *                mPCAVariancePtr;
    double *                mNoiseVariancePtr;
    std::vector<int32_t>    mMeanShape;
    std::vector<int32_t>    mPCABasisShape;
    std::vector<int32_t>    mPCAVarianceShape;
    std::vector<int32_t>    mNoiseVarianceShape;
    void clear();
    void loadFrom(std::ifstream &fin);
public:
    ProbabilisticModel();
    ProbabilisticModel(const ProbabilisticModel &other);
};

class BaselModel {
private:
    static BaselModel *gInstance;
    static BaselModel *LoadBaselModel(std::string root, std::string type, std::string version);
    static void Clear();
    
    std::string         mType;
    ProbabilisticModel  mColor;
    ProbabilisticModel  mShape;
    ProbabilisticModel  mExpression;

    BaselModel(std::string type, std::string colorPath, std::string shapePath, std::string expressionPath);
    ~BaselModel();
public:

    static void Initialize(std::string root, std::string type, std::string version) { LoadBaselModel(root, type, version); }

};