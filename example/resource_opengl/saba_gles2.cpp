#include "saba_gles2.h"
#include <Saba/Base/Path.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void NewCamera(
	glm::vec3 &eyes,
	glm::vec3 &center,
	glm::vec2 angles = glm::vec2(0),
	glm::vec3 shift = glm::vec3(0))
{
	angles = angles * glm::pi<float>() / 180.0f;
	glm::vec3 direction = eyes - center;
	float toEyeLen = glm::length(direction);
	glm::vec3 toEyeNormal = direction / toEyeLen;
	float phi = std::atan2(toEyeNormal.x, toEyeNormal.z) + glm::pi<float>() + angles.x;
	float theta = std::acos(toEyeNormal.y) + angles.y;

	const float st = std::sin(theta);
	const float sp = std::sin(phi);
	const float ct = std::cos(theta);
	const float cp = std::cos(phi);
	toEyeNormal = glm::vec3(-st * sp, ct, -st * cp);

	eyes = center + toEyeNormal * (toEyeLen + shift.z);
	center = center + glm::vec3(shift.x, shift.y, 0) / 15.f;
}

saba_gles2::~saba_gles2(void)
{
	appContext.Clear();
}

void saba_gles2::SetScreenSize(
	int width,
	int height)
{
	appContext.m_screenWidth = width;
	appContext.m_screenHeight = height;
}

void saba_gles2::Draw(void)
{
	for (auto &model : models)
	{
		// Update Animation
		model.UpdateAnimation(appContext);

		// Update Vertices
		model.Update(appContext);

		// Draw
		model.Draw(appContext);
	}
}

void saba_gles2::Evaluate(
	float ElapsedTime,
	float fps)
{
	appContext.m_elapsed = ElapsedTime;
	appContext.m_animTime += ElapsedTime;

	// Setup camera
	if (appContext.m_vmdCameraAnim)
	{
		appContext.m_vmdCameraAnim->Evaluate(appContext.m_animTime * fps);
		const auto mmdCam = appContext.m_vmdCameraAnim->GetCamera();
		saba::MMDLookAtCamera lookAtCam(mmdCam);
		eyes = lookAtCam.m_eye;
		center = lookAtCam.m_center;
		up = lookAtCam.m_up;
		fov = mmdCam.m_fov;
	}
	else
		NewCamera(eyes, center, newCamera_angles, newCamera_shift);

	appContext.m_viewMat = glm::lookAt(eyes, center, up);
	appContext.m_projMat = glm::perspectiveFovRH(fov, (float)appContext.m_screenWidth, (float)appContext.m_screenHeight, 1.0f, 10000.0f);
}

bool saba_gles2::Parse(std::vector<std::string> &args)
{
	Input currentInput;
	for (auto argIt = args.begin(); argIt != args.end(); ++argIt)
	{
		const auto &arg = (*argIt);
		if (arg == "-model")
		{
			if (!currentInput.m_modelPath.empty())
				inputModels.emplace_back(currentInput);

			++argIt;
			if (argIt == args.end())
				return false;

			currentInput = Input();
			currentInput.m_modelPath = (*argIt);
		}
		else if (arg == "-vmd")
		{
			if (currentInput.m_modelPath.empty())
				return false;

			++argIt;
			if (argIt == args.end())
				return false;

			currentInput.m_vmdPaths.push_back((*argIt));
		}
		else if (arg == "-transparent")
		{
			appContext.m_enableTransparentWindow = true;
		}
	}
	if (!currentInput.m_modelPath.empty())
		inputModels.emplace_back(currentInput);

	return true;
}

bool saba_gles2::Setup(void)
{
	if (inputModels.empty())
		return false;

	if (!appContext.Setup())
	{
		std::cout << "Failed to setup AppContext.\n";
		return false;
	}

	// Load MMD model
	for (const auto &input : inputModels)
	{
		// Load MMD model
		Model model;
		auto ext = saba::PathUtil::GetExt(input.m_modelPath);
		if (ext == "pmd")
		{
			auto pmdModel = std::unique_ptr<saba::PMDModel>(new saba::PMDModel);
			// auto pmdModel = std::make_unique<saba::PMDModel>();
			if (!pmdModel->Load(input.m_modelPath, appContext.m_mmdDir))
			{
				std::cout << "Failed to load pmd file.\n";
				return false;
			}
			model.m_mmdModel = std::move(pmdModel);
		}
		else if (ext == "pmx")
		{
			auto pmxModel = std::unique_ptr<saba::PMXModel>(new saba::PMXModel);
			// auto pmxModel = std::make_unique<saba::PMXModel>();
			if (!pmxModel->Load(input.m_modelPath, appContext.m_mmdDir))
			{
				std::cout << "Failed to load pmx file.\n";
				return false;
			}
			model.m_mmdModel = std::move(pmxModel);
		}
		else
		{
			std::cout << "Unknown file type. [" << ext << "]\n";
			return false;
		}
		model.m_mmdModel->InitializeAnimation();

		// Load VMD animation
		auto vmdAnim = std::unique_ptr<saba::VMDAnimation>(new saba::VMDAnimation);
		// auto vmdAnim = std::make_unique<saba::VMDAnimation>();
		if (!vmdAnim->Create(model.m_mmdModel))
		{
			std::cout << "Failed to create VMDAnimation.\n";
			return false;
		}
		for (const auto &vmdPath : input.m_vmdPaths)
		{
			saba::VMDFile vmdFile;
			if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str()))
			{
				std::cout << "Failed to read VMD file.\n";
				return false;
			}
			if (!vmdAnim->Add(vmdFile))
			{
				std::cout << "Failed to add VMDAnimation.\n";
				return false;
			}
			if (!vmdFile.m_cameras.empty())
			{
				auto vmdCamAnim = std::unique_ptr<saba::VMDCameraAnimation>(new saba::VMDCameraAnimation);
				// auto vmdCamAnim = std::make_unique<saba::VMDCameraAnimation>();
				if (!vmdCamAnim->Create(vmdFile))
					std::cout << "Failed to create VMDCameraAnimation.\n";

				appContext.m_vmdCameraAnim = std::move(vmdCamAnim);
			}
		}
		vmdAnim->SyncPhysics(0.0f);
		model.m_vmdAnim = std::move(vmdAnim);
		model.Setup(appContext);
		models.emplace_back(std::move(model));
	}
	return true;
}

int saba_gles2::GetMsaaSamples(void)
{
	return appContext.m_msaaSamples;
}

bool saba_gles2::IsEnableTransparent(void)
{
	return appContext.m_enableTransparentWindow;
}

void saba_gles2::SetupTransparent(void)
{
	if (appContext.m_enableTransparentWindow)
		appContext.SetupTransparentFBO();
}

void saba_gles2::UpdateTransparent(void)
{
	if (appContext.m_enableTransparentWindow)
		appContext.UpdateTransparentFBO();
}
