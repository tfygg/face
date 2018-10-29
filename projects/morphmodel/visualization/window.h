#pragma once
#define SNOW_MODULE_OPENGL
#include <snow.h>
#include "mesh.h"

class VisualizerWindow : public snow::AbstractWindow {
    snow::ArcballCamera mCamera;
    FaceMesh            mMesh;
    /* mat */
    glm::mat4           mViewMat;
    glm::mat4           mProjMat;
    glm::mat4           mModelMat;
    /* show */
    bool                mShowModel;

public:
    VisualizerWindow(int numVertices, int numTriangles,
                     glm::vec3 eye   =glm::vec3(0.f, 0.f, 0.f),
                     glm::vec3 up    =glm::vec3(0.f, 1.f, 0.f),
                     glm::vec3 lookAt=glm::vec3(0.f, 0.f, -0.5f),
                     const char *title="show")
        : snow::AbstractWindow(title)
        , mCamera(eye, up, lookAt)  // default camera view is identity
        , mMesh(numVertices, numTriangles)
        , mViewMat(1.0f), mProjMat(1.0f), mModelMat(1.0f)
        , mShowModel(true)
    {}

    void setViewMat(const glm::mat4 &view)   { mViewMat = view;   }
    void setProjMat(const glm::mat4 &proj)   { mProjMat = proj;   }
    void setModelMat(const glm::mat4 &model) { mModelMat = model; }

    FaceMesh &faceMesh() { return mMesh; }
};
