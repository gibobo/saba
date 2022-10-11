#pragma once

#include "Model.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class saba_gles2
{
private:
	std::vector<Model> models;
	glm::vec3 center = glm::vec3(0, 10, 0);
	glm::vec3 eyes = glm::vec3(0, 10, 50);
	glm::vec3 up = glm::vec3(0, 1, 0);

public:
	~saba_gles2();
	bool Setup(std::vector<std::string> &args);
	void Draw();
	void SetScreenSize(int width, int height);
	void Evaluate(float ElapsedTime = 0);

public:
	AppContext appContext;
	bool enableTransparentWindow = false;
	float fov = glm::radians(30.f);
	glm::vec2 newCamera_angles = glm::vec2(0.f);
	glm::vec3 newCamera_shift = glm::vec3(0.f);
};
