#pragma once

#include <string>
#if _WIN32
#include <GL/gl3w.h>
#else
#include <GLES2/gl2.h>
#ifdef __linux__
#include <GLES2/gl2ext.h>
#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#endif
#endif

GLuint CreateShader(GLenum shaderType, const std::string code);
GLuint CreateShaderProgram(const std::string vsFile, const std::string fsFile);

struct MMDEdgeShader
{
    GLuint m_prog = 0;

    // attribute
    GLint m_inPos = -1;
    GLint m_inNor = -1;

    // uniform
    GLint m_uWV = -1;
    GLint m_uWVP = -1;
    GLint m_uScreenSize = -1;
    GLint m_uEdgeSize = -1;

    GLint m_uEdgeColor = -1;

    bool Setup(const std::string m_shaderDir);
    ~MMDEdgeShader();
    void Clear();
};

struct MMDGroundShadowShader
{
    unsigned int m_prog = 0;

    // attribute
    int m_inPos = -1;

    // uniform
    int m_uWVP = -1;
    int m_uShadowColor = -1;

    bool Setup(const std::string m_shaderDir);
    ~MMDGroundShadowShader();
    void Clear();
};

struct MMDShader
{
    GLuint m_prog = 0;

    // attribute
    GLint m_inPos = -1;
    GLint m_inNor = -1;
    GLint m_inUV = -1;

    // uniform
    GLint m_uWV = -1;
    GLint m_uWVP = -1;

    GLint m_uAmbinet = -1;
    GLint m_uDiffuse = -1;
    GLint m_uSpecular = -1;
    GLint m_uSpecularPower = -1;
    GLint m_uAlpha = -1;

    GLint m_uTexMode = -1;
    GLint m_uTex = -1;
    GLint m_uTexMulFactor = -1;
    GLint m_uTexAddFactor = -1;

    GLint m_uSphereTexMode = -1;
    GLint m_uSphereTex = -1;
    GLint m_uSphereTexMulFactor = -1;
    GLint m_uSphereTexAddFactor = -1;

    GLint m_uToonTexMode = -1;
    GLint m_uToonTex = -1;
    GLint m_uToonTexMulFactor = -1;
    GLint m_uToonTexAddFactor = -1;

    GLint m_uLightColor = -1;
    GLint m_uLightDir = -1;

    GLint m_uLightVP = -1;
    GLint m_uShadowMapSplitPositions = -1;
    GLint m_uShadowMap0 = -1;
    GLint m_uShadowMap1 = -1;
    GLint m_uShadowMap2 = -1;
    GLint m_uShadowMap3 = -1;
    GLint m_uShadowMapEnabled = -1;

    bool Setup(const std::string m_shaderDir);
    ~MMDShader();
    void Clear();
};
