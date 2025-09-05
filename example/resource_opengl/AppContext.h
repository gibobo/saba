#pragma once

#include "GL_ShaderProgram.h"
#include <Saba/Model/MMD/VMDCameraAnimation.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>

struct Texture
{
	GLuint m_texture;
	bool m_hasAlpha;
};

struct AppContext
{
	~AppContext()
	{
		Clear();
	}

	std::string m_resourceDir;
	std::string m_shaderDir;
	std::string m_mmdDir;

	std::unique_ptr<MMDShader> m_mmdShader;
	std::unique_ptr<MMDEdgeShader> m_mmdEdgeShader;
	std::unique_ptr<MMDGroundShadowShader> m_mmdGroundShadowShader;

	glm::mat4 m_viewMat;
	glm::mat4 m_projMat;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	
	// 遮擋剔除相關
	bool m_enableOcclusionCulling = true; // 暫時禁用以進行調試
	bool m_enableFrustumCulling = true;
	int m_occlusionQueryFrameDelay = 2; // 延遲幾幀再檢查結果
	bool m_showBoundingBoxes = false; // 除錯用
	
	// 著色器效能模式設定
	enum class ShaderPerformanceMode {
		ORIGINAL,     // 原始版本
		OPTIMIZED,    // 優化版本  
		BRANCHLESS    // 無分支版本
	};
	ShaderPerformanceMode m_shaderMode = ShaderPerformanceMode::ORIGINAL;

	glm::vec3 m_lightColor = glm::vec3(1, 1, 1);
	glm::vec3 m_lightDir = glm::vec3(-0.5f, -1.0f, -0.5f);

	std::map<std::string, Texture> m_textures;
	GLuint m_dummyColorTex = 0;
	GLuint m_dummyShadowDepthTex = 0;

	const int m_msaaSamples = 4;

	bool m_enableTransparentWindow = false;
	uint32_t m_transparentFboWidth = 0;
	uint32_t m_transparentFboHeight = 0;
	GLuint m_transparentFboColorTex = 0;
	GLuint m_transparentFbo = 0;
	GLuint m_transparentFboMSAAColorRB = 0;
	GLuint m_transparentFboMSAADepthRB = 0;
	GLuint m_transparentMSAAFbo = 0;
	GLuint m_copyTransparentWindowShader = 0;
	GLint m_copyTransparentWindowShaderTex = -1;
	GLuint m_copyShader = 0;
	GLint m_copyShaderTex = -1;
	GLuint m_copyVAO = 0;

	float m_elapsed = 0.0f;
	float m_animTime = 0.0f;
	std::unique_ptr<saba::VMDCameraAnimation> m_vmdCameraAnim;

	bool Setup();
	void Clear();

	void SetupTransparentFBO();
	void UpdateTransparentFBO();

	Texture GetTexture(const std::string &texturePath);
};
