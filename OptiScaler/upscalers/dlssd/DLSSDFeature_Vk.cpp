#include <pch.h>
#include <Config.h>
#include <Logger.h>

#include "DLSSDFeature_Vk.h"

bool DLSSDFeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		LOG_ERROR("nvngx.dll not loaded!");

		SetInit(false);
		return false;
	}

	HRESULT result;
	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	Instance = InInstance;
	PhysicalDevice = InPD;
	Device = InDevice;

	do
	{
		if (!_dlssdInited)
		{
			_dlssdInited = NVNGXProxy::InitVulkan(InInstance, InPD, InDevice, InGIPA, InGDPA);

			if (!_dlssdInited)
				return false;

			_moduleLoaded = (NVNGXProxy::VULKAN_Init_ProjectID() != nullptr || NVNGXProxy::VULKAN_Init_Ext() != nullptr) &&
				(NVNGXProxy::VULKAN_Shutdown() != nullptr || NVNGXProxy::VULKAN_Shutdown1() != nullptr) &&
				(NVNGXProxy::VULKAN_GetParameters() != nullptr || NVNGXProxy::VULKAN_AllocateParameters() != nullptr) &&
				NVNGXProxy::VULKAN_DestroyParameters() != nullptr && NVNGXProxy::VULKAN_CreateFeature() != nullptr &&
				NVNGXProxy::VULKAN_ReleaseFeature() != nullptr && NVNGXProxy::VULKAN_EvaluateFeature() != nullptr;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		LOG_INFO("Creating DLSSD feature");

		if (NVNGXProxy::VULKAN_CreateFeature() != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssdHandle = &_dlssdHandle;
			nvResult = NVNGXProxy::VULKAN_CreateFeature()(InCmdList, NVSDK_NGX_Feature_RayReconstruction, InParameters, &_p_dlssdHandle);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				LOG_ERROR("_CreateFeature result: {0:X}", (unsigned int)nvResult);
				break;
			}
			else
			{
				LOG_INFO("_CreateFeature result: NVSDK_NGX_Result_Success, HandleId: {0}", _p_dlssdHandle->Id);
			}
		}
		else
		{
			LOG_ERROR("_CreateFeature is nullptr");
			break;
		}

		ReadVersion();

		initResult = true;

	} while (false);

	SetInit(initResult);

	return initResult;
}

bool DLSSDFeatureVk::Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		LOG_ERROR("nvngx.dll or _nvngx.dll is not loaded!");
			return false;
	}

	NVSDK_NGX_Result nvResult;

	if (NVNGXProxy::VULKAN_EvaluateFeature() != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = NVNGXProxy::VULKAN_EvaluateFeature()(InCmdBuffer, _p_dlssdHandle, InParameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			LOG_ERROR("_EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}
	}
	else
	{
		LOG_ERROR("_EvaluateFeature is nullptr");
		return false;
	}

	_frameCount++;

	return true;
}

void DLSSDFeatureVk::Shutdown(VkDevice InDevice)
{
}

DLSSDFeatureVk::DLSSDFeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), DLSSDFeature(InHandleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		LOG_INFO("nvngx.dll not loaded, now loading");
		NVNGXProxy::InitNVNGX();
	}

	LOG_INFO("binding complete!");
}

DLSSDFeatureVk::~DLSSDFeatureVk()
{
	if (State::Instance().isShuttingDown)
		return;

	if (NVNGXProxy::VULKAN_ReleaseFeature() != nullptr && _p_dlssdHandle != nullptr)
		NVNGXProxy::VULKAN_ReleaseFeature()(_p_dlssdHandle);
}
