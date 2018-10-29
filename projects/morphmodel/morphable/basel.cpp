#include "basel.h"
#include <stdlib.h>

BaselModel *BaselModel::gInstance = nullptr;


ProbabilisticModel::ProbabilisticModel()
    : mMeanPtr(nullptr)
    , mPCABasisPtr(nullptr)
    , mPCAVariancePtr(nullptr)
    , mNoiseVariancePtr(nullptr) {}
ProbabilisticModel::ProbabilisticModel(const ProbabilisticModel &other)
    : mMeanPtr(other.mMeanPtr)
    , mPCABasisPtr(other.mPCABasisPtr)
    , mPCAVariancePtr(other.mPCAVariancePtr)
    , mNoiseVariancePtr(other.mNoiseVariancePtr) {}

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
    auto colorPath      = snow::path::Join(dir, "color_" + version + ".bin");
    auto shapePath      = snow::path::Join(dir, "shape_" + version + ".bin");
    auto expressionPath = snow::path::Join(dir, "expression_" + version + ".bin");
    auto templatePath   = snow::path::Join(dir, "template.bin");
    if (!snow::path::AllExists({colorPath, shapePath, expressionPath, templatePath}))
        snow::fatal("[Basel]: failed to find all of `{0}, {1}, {2}`", colorPath, shapePath, expressionPath);
    if (gInstance == nullptr) std::atexit(BaselModel::Clear);
    gInstance = new BaselModel(type, colorPath, shapePath, expressionPath, templatePath);
    return gInstance;
}

BaselModel::BaselModel(std::string type, std::string colorPath, std::string shapePath,
                       std::string expressionPath, std::string templatePath)
    : mType(type) {
    {
        std::ifstream colorFin(colorPath, std::ios::binary);
        if (colorFin.is_open()) { mColor.loadFrom(colorFin); colorFin.close();               }
        else                    { snow::fatal("[BaselModel]: failed to open {}", colorPath); }
        std::ifstream shapeFin(shapePath, std::ios::binary);
        if (shapeFin.is_open()) { mShape.loadFrom(shapeFin); shapeFin.close();               }
        else                    { snow::fatal("[BaselModel]: failed to open {}", shapePath); }
        std::ifstream exprsFin(expressionPath, std::ios::binary);
        if (exprsFin.is_open()) { mExpression.loadFrom(exprsFin); exprsFin.close();               }
        else                    { snow::fatal("[BaselModel]: failed to open {}", expressionPath); }
    }
    {
        std::ifstream fin(templatePath, std::ios::binary);
        if (fin.is_open()) {
            int32_t dims = 0;
            std::vector<int32_t> shape;
            auto readShape = [&]() -> void {
                fin.read((char*)(&dims), sizeof(int32_t));
                shape.resize(dims);
                for (int32_t i = 0; i < dims; ++i)
                    fin.read((char*)(&shape[i]), sizeof(int32_t));
                std::cout << "shape " << shape << std::endl;    
            };
            // read triangles
            readShape();
            mTriangleList.resize(shape[0]);
            fin.read((char*)&mTriangleList[0], sizeof(snow::int3) * mTriangleList.size());
            readShape();
            mPointList.resize(shape[0]);
            fin.read((char*)&mPointList[0], sizeof(snow::float3) * mPointList.size());
            fin.close();
        }
        else snow::fatal("[BaselModel]: failed to open {}", templatePath);
    }
}
BaselModel::~BaselModel() {
    mColor.clear();
    mShape.clear();
    mExpression.clear();
}
