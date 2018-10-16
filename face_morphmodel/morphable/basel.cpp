#include "basel.h"
#include <stdlib.h>
#include <snow.h>

BaselModel *BaselModel::gInstance = nullptr;


ProbabilisticModel::ProbabilisticModel()
    : mMeanPtr(nullptr)
    , mPCABasisPtr(nullptr)
    , mPCAVariancePtr(nullptr)
    , mNoiseVariancePtr(nullptr) {}
ProbabilisticModel::ProbabilisticModel(const ProbabilisticModel &other)
    : mMeanPtr(mMeanPtr)
    , mPCABasisPtr(mPCABasisPtr)
    , mPCAVariancePtr(mPCAVariancePtr)
    , mNoiseVariancePtr(mNoiseVariancePtr) {}

void ProbabilisticModel::loadFrom(std::ifstream &fin) {
    auto read = [&](double **ptr, std::vector<int32_t> &shape) -> void {
        int32_t dims = 0;
        fin.read((char *)(&dims), sizeof(int32_t));
        shape.resize(dims);
        int32_t size = 1;
        for (int32_t i = 0; i < dims; ++i) {
            fin.read((char *)(&shape[i]), sizeof(int32_t));
            size *= shape[i];
        }
        (*ptr) = new double[size];
        float val;
        for (int32_t i = 0; i < size; ++i) {
            fin.read((char*)(&val), sizeof(float));
            (*ptr)[i] = val;
        }
        std::cout << "shape: " << shape << std::endl;
    };

    read(&mMeanPtr,          mMeanShape);
    read(&mPCABasisPtr,      mPCABasisShape);
    read(&mPCAVariancePtr,   mPCAVarianceShape);
    read(&mNoiseVariancePtr, mNoiseVarianceShape);
}

void ProbabilisticModel::clear() {
    delete[] mMeanPtr; mMeanPtr = nullptr;
    delete[] mPCABasisPtr; mPCABasisPtr = nullptr;
    delete[] mPCAVariancePtr; mPCAVariancePtr = nullptr;
    delete[] mNoiseVariancePtr; mNoiseVariancePtr = nullptr;
}


void BaselModel::Clear() {
    delete gInstance;
    gInstance = nullptr;
}

BaselModel *BaselModel::LoadBaselModel(std::string root, std::string type, std::string version) {
    if (type != "face" && type != "head") snow::fatal("[Basel]: type should be `face` or `head`");
    auto dir = snow::path::Join(root, type);
    auto colorPath = snow::path::Join(dir, "color_" + version + ".bin");
    auto shapePath = snow::path::Join(dir, "shape_" + version + ".bin");
    auto expressionPath = snow::path::Join(dir, "expression_" + version + ".bin");
    if (!snow::path::AllExists({colorPath, shapePath, expressionPath}))
        snow::fatal("[Basel]: failed to find all of `{0}, {1}, {2}`", colorPath, shapePath, expressionPath);
    if (gInstance == nullptr) std::atexit(BaselModel::Clear);
    gInstance = new BaselModel(type, colorPath, shapePath, expressionPath);
    return gInstance;
}

BaselModel::BaselModel(std::string type, std::string colorPath, std::string shapePath, std::string expressionPath)
    : mType(type) {
    {
        std::ifstream fin(colorPath, std::ios::binary);
        mColor.loadFrom(fin);
    }
    {
        std::ifstream fin(shapePath, std::ios::binary);
        mShape.loadFrom(fin);
    }
    {
        std::ifstream fin(expressionPath, std::ios::binary);
        mExpression.loadFrom(fin);
    }
}
BaselModel::~BaselModel() {
    mColor.clear();
    mShape.clear();
    mExpression.clear();
}
