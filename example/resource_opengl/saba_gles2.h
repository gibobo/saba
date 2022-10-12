#pragma once

#include "Model.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class saba_gles2
{
private:
	struct Input
	{
		std::string m_modelPath;
		std::vector<std::string> m_vmdPaths;
	};

private:
	std::vector<Model> models;
	std::vector<Input> inputModels;
	AppContext appContext;
	glm::vec3 center = glm::vec3(0, 10, 0);
	glm::vec3 eyes = glm::vec3(0, 10, 50);
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec2 newCamera_angles = glm::vec2(0.f);
	glm::vec3 newCamera_shift = glm::vec3(0.f);
	float fov = glm::radians(30.f);

public:
	~saba_gles2(void);
	bool Parse(std::vector<std::string> &args);
	bool Setup(void);
	void Draw(void);
	void SetScreenSize(int width, int height);
	void Evaluate(float ElapsedTime = 0, float fps = 30.0);
	int GetMsaaSamples(void);
	bool IsEnableTransparent(void);
	void SetupTransparent(void);
	void UpdateTransparent(void);
};
