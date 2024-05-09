#pragma once
#include "pch.h"
#include "Config.h"
#include "backends/dlss/nvapi.h"

// Implementation of https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/maindll/NGX/NvNGXExports.cpp

NVSDK_NGX_API uint32_t NVSDK_NGX_GetAPIVersion()
{
	spdlog::debug("NVSDK_NGX_GetAPIVersion");
	return 19;
}

NVSDK_NGX_API uint32_t NVSDK_NGX_GetApplicationId()
{
	spdlog::debug("NVSDK_NGX_GetApplicationId");
	return Config::Instance()->NVNGX_ApplicationId;
}

NVSDK_NGX_API uint32_t NVSDK_NGX_GetDriverVersion()
{
	spdlog::debug("NVSDK_NGX_GetDriverVersion");
	return 0x2080000; // NGXMinimumDriverVersion 520.00
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_GetDriverVersionEx(uint32_t* Versions, uint32_t InputVersionCount, uint32_t* TotalDriverVersionCount)
{
	spdlog::debug("NVSDK_NGX_GetDriverVersionEx");

	if (!Versions && !TotalDriverVersionCount)
	{
		spdlog::error("NVSDK_NGX_GetDriverVersionEx invalid parameter!");
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (TotalDriverVersionCount)
		*TotalDriverVersionCount = 2;

	if (Versions && InputVersionCount >= 1)
	{
		Versions[0] = 0x208; // 520

		if (InputVersionCount >= 2)
			Versions[1] = 0;
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API uint32_t NVSDK_NGX_GetGPUArchitecture()
{
	spdlog::info("NVSDK_NGX_GetGPUArchitecture");
	return NV_GPU_ARCHITECTURE_TU100; // NGXGpuArchitecture NV_GPU_ARCHITECTURE_AD100 (0x190)
}

NVSDK_NGX_API uint32_t NVSDK_NGX_GetSnippetVersion()
{
	spdlog::info("NVSDK_NGX_GetSnippetVersion");
	return 0x30500; // NGXSnippetVersion 3.5.0
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_ProcessCommand(const char* Command, const char* Value, void* Unknown)
{
	spdlog::debug("NVSDK_NGX_ProcessCommand({0}, {1}, void)", std::string(Command), std::string(Value));
	return NVSDK_NGX_Result_Success; // Command gets logged but otherwise does nothing
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_SetInfoCallback(void* Callback)
{
	spdlog::debug("NVSDK_NGX_SetInfoCallback");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_SetTelemetryEvaluateCallback(void* Callback)
{
	spdlog::debug("NVSDK_NGX_SetTelemetryEvaluateCallback");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID)
{
	spdlog::debug("NVSDK_NGX_UpdateFeature");

	if (FeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_UpdateFeature Can't update this feature ({0})!", (int)FeatureID);
		return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
	}

	if (ApplicationId != nullptr)
	{
		if (ApplicationId->IdentifierType == NVSDK_NGX_Application_Identifier_Type_Application_Id)
		{
			spdlog::info("NVSDK_NGX_UpdateFeature Update ApplicationId: {0:X}", ApplicationId->v.ApplicationId);
			Config::Instance()->NVNGX_ApplicationId = ApplicationId->v.ApplicationId;
		}
		else if (ApplicationId->IdentifierType == NVSDK_NGX_Application_Identifier_Type_Project_Id)
		{
			Config::Instance()->NVNGX_ProjectId = std::string(ApplicationId->v.ProjectDesc.ProjectId);
			Config::Instance()->NVNGX_Engine = ApplicationId->v.ProjectDesc.EngineType;
			Config::Instance()->NVNGX_EngineVersion = std::string(ApplicationId->v.ProjectDesc.EngineVersion);

			if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && ApplicationId->v.ProjectDesc.EngineVersion)
				Config::Instance()->NVNGX_EngineVersion5 = ApplicationId->v.ProjectDesc.EngineVersion[0] == '5';
			else
				Config::Instance()->NVNGX_EngineVersion5 = false;

			spdlog::info("NVSDK_NGX_UpdateFeature Update InProjectId: {0}", Config::Instance()->NVNGX_ProjectId);
			spdlog::info("NVSDK_NGX_UpdateFeature Update InEngineType: {0}", (int)Config::Instance()->NVNGX_Engine);
			spdlog::info("NVSDK_NGX_UpdateFeature Update InEngineVersion: {0}", Config::Instance()->NVNGX_EngineVersion);
		}
		else
		{
			spdlog::error("NVSDK_NGX_UpdateFeature Unknown IdentifierType ({0})!", (int)ApplicationId->IdentifierType);
		}
	}

	return NVSDK_NGX_Result_Success;
}