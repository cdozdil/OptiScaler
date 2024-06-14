#include "../../pch.h"
#include "../../Config.h"
#include "../../Logger.h"

#include "DLSSFeature_Vk.h"

bool DLSSFeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		spdlog::error("DLSSFeatureVk::Init nvngx.dll not loaded!");

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
		if (!_dlssInited)
		{
			_dlssInited = NVNGXProxy::InitVulkan(InInstance, InPD, InDevice, InGIPA, InGDPA);

			if (!_dlssInited)
				return false;

			_moduleLoaded = (NVNGXProxy::VULKAN_Init_ProjectID() != nullptr || NVNGXProxy::VULKAN_Init_Ext() != nullptr) &&
				(NVNGXProxy::VULKAN_Shutdown() != nullptr || NVNGXProxy::VULKAN_Shutdown1() != nullptr) &&
				(NVNGXProxy::VULKAN_GetParameters() != nullptr || NVNGXProxy::VULKAN_AllocateParameters() != nullptr) &&
				NVNGXProxy::VULKAN_DestroyParameters() != nullptr && NVNGXProxy::VULKAN_CreateFeature() != nullptr &&
				NVNGXProxy::VULKAN_ReleaseFeature() != nullptr && NVNGXProxy::VULKAN_EvaluateFeature() != nullptr;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (NVNGXProxy::VULKAN_AllocateParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureVk::Init _AllocateParameters will be used");

			nvResult = NVNGXProxy::VULKAN_AllocateParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else if (NVNGXProxy::VULKAN_GetParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureVk::Init _GetParameters will be used");

			nvResult = NVNGXProxy::VULKAN_GetParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureVk::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else
		{
			spdlog::error("DLSSFeatureVk::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureVk::Evaluate Creating DLSS feature");

		if (NVNGXProxy::VULKAN_CreateFeature() != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = NVNGXProxy::VULKAN_CreateFeature()(InCmdList, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

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

		ReadVersion();

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

	if (NVNGXProxy::VULKAN_EvaluateFeature() != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = NVNGXProxy::VULKAN_EvaluateFeature()(InCmdBuffer, _p_dlssHandle, Parameters, NULL);

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

	_frameCount++;

	return true;
}

void DLSSFeatureVk::Shutdown(VkDevice InDevice)
{
	if (_dlssInited)
	{
		if (NVNGXProxy::VULKAN_Shutdown() != nullptr)
			NVNGXProxy::VULKAN_Shutdown()();
		else if (NVNGXProxy::VULKAN_Shutdown1() != nullptr)
			NVNGXProxy::VULKAN_Shutdown1()(InDevice);
	}

	DLSSFeature::Shutdown();
}

DLSSFeatureVk::DLSSFeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		spdlog::info("DLSSFeatureVk::DLSSFeatureVk nvngx.dll not loaded, now loading");
		NVNGXProxy::InitNVNGX();
	}

	spdlog::info("DLSSFeatureVk::DLSSFeatureVk binding complete!");
}

DLSSFeatureVk::~DLSSFeatureVk()
{
	if (Parameters != nullptr && NVNGXProxy::VULKAN_DestroyParameters() != nullptr)
		NVNGXProxy::VULKAN_DestroyParameters()(Parameters);

	if (NVNGXProxy::VULKAN_ReleaseFeature() != nullptr && _p_dlssHandle != nullptr)
		NVNGXProxy::VULKAN_ReleaseFeature()(_p_dlssHandle);
}
