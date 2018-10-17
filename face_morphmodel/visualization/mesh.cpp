#define SNOW_MODULE_OPENGL
#include "mesh.h"

#define VERT_CODE ""\
    "layout (location = 0) in vec3 aPos;                    "\
    "layout (location = 1) in vec3 aNorm;                   "\
    "layout (location = 2) in vec2 aTexCoord;               "\
    "layout (location = 3) in vec3 aColor;                  "\
    "out vec3 FragPos;                                      "\
    "out vec3 FragNormal;                                   "\
    "out vec2 FragTexCoord;                                 "\
    "out vec3 FragColor;                                    "\
    "uniform mat4 Normal;                                   "\
    "uniform mat4 Model;                                    "\
    "uniform mat4 View;                                     "\
    "uniform mat4 Proj;                                     "\
    "void main() {                                          "\
    "  gl_Position  = Proj * View * Model * vec4(aPos, 1.0);"\
    "  FragPos      = vec3(Model  * vec4(aPos, 1.0));       "\
    "  FragNormal   = vec3(Normal * vec4(aNorm, 0.0));      "\
    "  FragTexCoord = aTexCoord;                            "\
    "  FragColor    = aColor;                               "\
    "}                                                      "
#define FRAG_CODE ""\
    "in vec3  FragPos;                                      "\
    "in vec3  FragNormal;                                   "\
    "in vec2  FragTexCoord;                                 "\
    "in vec3  FragColor;                                    "\
    "out vec4 outFragColor;                                 "\
    "uniform vec3 LightPos;                                 "\
    "uniform vec3 Ambient;                                  "\
    "uniform vec3 Diffuse;                                  "\
    "void main() {                                          "\
    "    /* ambient */                                      "\
    "    vec3 ambientColor = Ambient;                       "\
    "    /* diffuse */                                      "\
    "    vec3 normal = normalize(FragNormal);               "\
    "    vec3 lightDir = normalize(LightPos - FragPos);     "\
    "    float diff = max(dot(normal, lightDir), 0);        "\
    "    vec3 diffuseColor = diff * Diffuse;                "\
    "    /* ligth */                                        "\
    "    vec3 result = (ambientColor + diffuseColor);       "\
    "    outFragColor = vec4(result * FragColor, 1.0);      "\
    "}"


FaceMesh::FaceMesh(int numVertices, int numTriangles)
    : mVertexList(numVertices), mIndiceList(numTriangles * 3)
    , mVAO(0), mVBO(0), mEBO(0) {
    mShader.buildFromCode(VERT_CODE, FRAG_CODE);
    // allocate opengl
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1,      &mVBO);
    glGenBuffers(1,      &mEBO);

    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mVertexList.size() * sizeof(FaceVertex), &mVertexList[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndiceList.size() * sizeof(int32_t), &mIndiceList[0], GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FaceVertex), (void*)0);
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FaceVertex), (void*)offsetof(FaceVertex, mNormal));
    glEnableVertexAttribArray(2); // tex_coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FaceVertex), (void*)offsetof(FaceVertex, mTexCoords));
    glEnableVertexAttribArray(3); // color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FaceVertex), (void*)offsetof(FaceVertex, mColor));

    // unbind VAO
    glBindVertexArray(0);
}
FaceMesh::~FaceMesh() {
    if (mVBO) glDeleteBuffers(1,      &mVBO);
    if (mEBO) glDeleteBuffers(1,      &mEBO);
    if (mVAO) glDeleteVertexArrays(1, &mVAO);
}

void FaceMesh::draw() {
    // draw mesh
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)mIndiceList.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void FaceMesh::update() {
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FaceVertex) * mVertexList.size(), (void *)(&mVertexList[0]));
    glBindVertexArray(0);
}

#undef VERT_CODE
#undef FRAG_CODE