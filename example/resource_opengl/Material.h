#pragma once

#include <Saba/Model/MMD/PMDModel.h>
struct Material
{
	explicit Material(const saba::MMDMaterial &mat) : m_mmdMat(mat) {}

	const saba::MMDMaterial &m_mmdMat;
	GLuint m_texture = 0;
	bool m_textureHasAlpha = false;
	GLuint m_spTexture = 0;
	GLuint m_toonTexture = 0;
};
