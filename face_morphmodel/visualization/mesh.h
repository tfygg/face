#pragma once
#include <snow.h>
#include <vector>

struct FaceVertex {
    snow::float3 mPosition;  // position of vertice
    snow::float3 mNormal;    // normal of vertice
    snow::float2 mTexCoords; // tex coordinate of vertice
    snow::float3 mColor;     // rgb color of vertice
    // texture and color will be mixtured
};

class FaceMesh {
    std::vector<FaceVertex>  mVertexList;
    std::vector<int32_t>     mIndiceList;
    snow::shader             mShader;
    // [TODO]: textures
    /* gl object */
    uint32_t mVAO, mVBO, mEBO;
public:
    FaceMesh(int numVertices, int numTriangles);
    ~FaceMesh();

    /* get */
    int                            numVertices()   const { return (int)mVertexList.size();      }
    int                            numTriangles()  const { return (int)mIndiceList.size() / 3;  }
    std::vector<FaceVertex> &      verticeList()         { return mVertexList;                  }
    std::vector<int32_t> &         indiceList()          { return mIndiceList;                  }
    const std::vector<FaceVertex> &verticeList()   const { return mVertexList;                  }
    const std::vector<int32_t> &   indiceList()    const { return mIndiceList;                  }

    void draw();
    void update();
};
