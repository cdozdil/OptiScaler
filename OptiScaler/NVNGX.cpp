#pragma once
#include "pch.h"
#include "Config.h"
#include "NVNGX_Proxy.h"

#pragma region Out of NVNGX Scope

//#include "backends/dlss/nvapi.h"
//
//// Implementation of https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/maindll/NGX/NvNGXExports.cpp
//
//NVSDK_NGX_API uint32_t NVSDK_NGX_GetAPIVersion()
//{
//	spdlog::debug("NVSDK_NGX_GetAPIVersion");
//	return 19;
//}
//
//NVSDK_NGX_API uint32_t NVSDK_NGX_GetApplicationId()
//{
//	spdlog::debug("NVSDK_NGX_GetApplicationId");
//	return Config::Instance()->NVNGX_ApplicationId;
//}
//
//NVSDK_NGX_API uint32_t NVSDK_NGX_GetDriverVersion()
//{
//	spdlog::debug("NVSDK_NGX_GetDriverVersion");
//	return 0x23A0000; // NGXMinimumDriverVersion 570.00
//}
//
//NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_GetDriverVersionEx(uint32_t* Versions, uint32_t InputVersionCount, uint32_t* TotalDriverVersionCount)
//{
//	spdlog::debug("NVSDK_NGX_GetDriverVersionEx");
//
//	if (!Versions && !TotalDriverVersionCount)
//	{
//		spdlog::error("NVSDK_NGX_GetDriverVersionEx invalid parameter!");
//		return NVSDK_NGX_Result_FAIL_InvalidParameter;
//	}
//
//	if (TotalDriverVersionCount)
//		*TotalDriverVersionCount = 2;
//
//	if (Versions && InputVersionCount >= 1)
//	{
//		Versions[0] = 0x23A; // 570
//
//		if (InputVersionCount >= 2)
//			Versions[1] = 0;
//	}
//
//	return NVSDK_NGX_Result_Success;
//}
//
//NVSDK_NGX_API uint32_t NVSDK_NGX_GetGPUArchitecture()
//{
//	spdlog::info("NVSDK_NGX_GetGPUArchitecture");
//	return NV_GPU_ARCHITECTURE_TU100; 
//}
//
//NVSDK_NGX_API uint32_t NVSDK_NGX_GetSnippetVersion()
//{
//	spdlog::info("NVSDK_NGX_GetSnippetVersion");
//	return 0x30500;
//}
//
//NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_ProcessCommand(const char* Command, const char* Value, void* Unknown)
//{
//	spdlog::debug("NVSDK_NGX_ProcessCommand({0}, {1}, void)", std::string(Command), std::string(Value));
//	return NVSDK_NGX_Result_Success; 
//}
//
//NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_SetInfoCallback(void* Callback)
//{
//	spdlog::debug("NVSDK_NGX_SetInfoCallback");
//	return NVSDK_NGX_Result_Success;
//}
//
//NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_SetTelemetryEvaluateCallback(void* Callback)
//{
//	spdlog::debug("NVSDK_NGX_SetTelemetryEvaluateCallback");
//	return NVSDK_NGX_Result_Success;
//}

#pragma endregion

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID)
{
	spdlog::debug("NVSDK_NGX_UpdateFeature FeatureId: {0}", (UINT)FeatureID);

	// To test with puredark mods
	//if (FeatureID != NVSDK_NGX_Feature_SuperSampling)
	//{
	//	if (NVNGXProxy::UpdateFeature() != nullptr)
	//	{
	//		auto result = NVNGXProxy::UpdateFeature()(ApplicationId, FeatureID);
	//		spdlog::error("NVSDK_NGX_UpdateFeature NVNGXProxy result for feature ({0}): {1:X}", (int)FeatureID, (UINT)result);
	//		return result;
	//	}
	//	else
	//	{
	//		spdlog::error("NVSDK_NGX_UpdateFeature Can't update this feature ({0})!", (int)FeatureID);
	//		return NVSDK_NGX_Result_Fail;
	//	}
	//}

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

			spdlog::info("NVSDK_NGX_UpdateFeature Update InProjectId: {0}", Config::Instance()->NVNGX_ProjectId);
			spdlog::info("NVSDK_NGX_UpdateFeature Update InEngineType: {0}", (int)Config::Instance()->NVNGX_Engine);
			spdlog::info("NVSDK_NGX_UpdateFeature Update InEngineVersion: {0}", Config::Instance()->NVNGX_EngineVersion);
		}
		else
		{
			spdlog::error("NVSDK_NGX_UpdateFeature Unknown IdentifierType ({0})!", (int)ApplicationId->IdentifierType);
		}
	}

	// To test with puredark mods
	return NVSDK_NGX_Result_Fail;
}