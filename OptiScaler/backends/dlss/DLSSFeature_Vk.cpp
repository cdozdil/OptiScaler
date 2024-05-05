#include "DLSSFeature_Vk.h"
#include <dxgi1_4.h>
#include "../../Config.h"
#include "../../pch.h"

bool DLSSFeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureVk::Init nvngx.dll not loaded!");

		SetInit(false);
		return false;
	}

	HRESULT result;
	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	do
	{
		if (!_dlssInited)
		{
			NVSDK_NGX_FeatureCommonInfo fcInfo{};

			GetFeatureCommonInfo(&fcInfo);

			if (Config::Instance()->NVNGX_ProjectId != "" && _Init_with_ProjectID != nullptr)
			{
				spdlog::debug("DLSSFeatureVk::Init _Init_with_ProjectID!");

				nvResult = _Init_with_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
					Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InInstance, InPD, InDevice, InGIPA, InGDPA, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else if (_Init_Ext != nullptr)
			{
				spdlog::debug("DLSSFeatureVk::Init _Init_Ext!");

				nvResult = _Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(),
					InInstance, InPD, InDevice, InGIPA, InGDPA, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else
			{
				spdlog::error("DLSSFeatureVk::Init _Init_with_ProjectID and  is null");
				break;
			}

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Init _Init_with_ProjectID result: {0:X}", (unsigned int)nvResult);
				break;
			}

			_dlssInited = true;
		}

		if (_AllocateParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureVk::Init _AllocateParameters will be used");

			nvResult = _AllocateParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else if (_GetParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureVk::Init _GetParameters will be used");

			nvResult = _GetParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else
		{
			spdlog::error("DLSSFeatureVk::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureVk::Evaluate Creating DLSS feature");

		if (_CreateFeature != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = _CreateFeature(InCmdList, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Evaluate _CreateFeature result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else
		{
			spdlog::error("DLSSFeatureVk::Evaluate _CreateFeature is nullptr");
			break;
		}

		initResult = true;

	} while (false);

	SetInit(initResult);

	return initResult;
}

bool DLSSFeatureVk::Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureVk::Evaluate nvngx.dll or _nvngx.dll is not loaded!");
			return false;
	}

	NVSDK_NGX_Result nvResult;

	if (_EvaluateFeature != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = _EvaluateFeature(InCmdBuffer, _p_dlssHandle, Parameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			spdlog::error("DLSSFeatureVk::Evaluate _EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}
	}
	else
	{
		spdlog::error("DLSSFeatureVk::Evaluate _EvaluateFeature is nullptr");
		return false;
	}

	return true;
}

void DLSSFeatureVk::Shutdown(VkDevice InDevice)
{
	if (_dlssInited)
	{
		if (_Shutdown != nullptr)
			_Shutdown();
		else if (_Shutdown1 != nullptr)
			_Shutdown1(InDevice);
	}

	DLSSFeature::Shutdown();
}

DLSSFeatureVk::DLSSFeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (!_moduleLoaded)
		return;

	if (_Init_with_ProjectID == nullptr)
		_Init_with_ProjectID = (PFN_NVSDK_NGX_VULKAN_Init_ProjectID)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_Init_ProjectID");

	if (_Init_Ext == nullptr)
		_Init_Ext = (PFN_NVSDK_NGX_VULKAN_Init)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_Init");

	if (_Shutdown == nullptr)
		_Shutdown = (PFN_NVSDK_NGX_VULKAN_Shutdown)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_Shutdown");

	if (_Shutdown1 == nullptr)
		_Shutdown1 = (PFN_NVSDK_NGX_VULKAN_Shutdown1)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_Shutdown1");

	if (_GetParameters == nullptr)
		_GetParameters = (PFN_NVSDK_NGX_VULKAN_GetParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_GetParameters");

	if (_AllocateParameters == nullptr)
		_AllocateParameters = (PFN_NVSDK_NGX_VULKAN_AllocateParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_AllocateParameters");

	if (_DestroyParameters == nullptr)
		_DestroyParameters = (PFN_NVSDK_NGX_VULKAN_DestroyParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_DestroyParameters");

	if (_CreateFeature == nullptr)
		_CreateFeature = (PFN_NVSDK_NGX_VULKAN_CreateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_CreateFeature");

	if (_ReleaseFeature == nullptr)
		_ReleaseFeature = (PFN_NVSDK_NGX_VULKAN_ReleaseFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_ReleaseFeature");

	if (_EvaluateFeature == nullptr)
		_EvaluateFeature = (PFN_NVSDK_NGX_VULKAN_EvaluateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_VULKAN_EvaluateFeature");

	_moduleLoaded = (_Init_with_ProjectID != nullptr || _Init_Ext != nullptr) && (_Shutdown != nullptr || _Shutdown1 != nullptr) &&
		(_GetParameters != nullptr || _AllocateParameters != nullptr) && _DestroyParameters != nullptr && _CreateFeature != nullptr &&
		_ReleaseFeature != nullptr && _EvaluateFeature != nullptr;

	spdlog::info("DLSSFeatureVk::DLSSFeatureVk binding complete!");
}

DLSSFeatureVk::~DLSSFeatureVk()
{
	if (Parameters != nullptr && _DestroyParameters != nullptr)
		_DestroyParameters(Parameters);

	if (_ReleaseFeature != nullptr)
		_ReleaseFeature(_p_dlssHandle);
}
