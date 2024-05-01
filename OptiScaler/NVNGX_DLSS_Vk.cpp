#pragma once
#include "pch.h"

#include <ankerl/unordered_dense.h>
#include <vulkan/vulkan.hpp>

#include "Config.h"
#include "backends/fsr2/FSR2Feature_Vk.h"
#include "backends/fsr2_212/FSR2Feature_Vk_212.h"
#include "NVNGX_Parameter.h"

VkInstance vkInstance;
VkPhysicalDevice vkPD;
VkDevice vkDevice;
PFN_vkGetInstanceProcAddr vkGIPA;
PFN_vkGetDeviceProcAddr vkGDPA;

static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Vk>> VkContexts;
static inline NVSDK_NGX_Parameter* createParams = nullptr;
static inline int changeBackendCounter = 0;

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD,
	VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_VULKAN_Init InApplicationId: {0}", InApplicationId);
	std::wstring string(InApplicationDataPath);
	std::string str(string.begin(), string.end());
	spdlog::debug("NVSDK_NGX_VULKAN_Init InApplicationDataPath {0}", str);
	spdlog::info("NVSDK_NGX_VULKAN_Init InSDKVersion: {0:x}", (int)InSDKVersion);

	Config::Instance()->NVNGX_ApplicationId = InApplicationId;
	Config::Instance()->NVNGX_ApplicationDataPath = InApplicationDataPath;
	Config::Instance()->NVNGX_Version = InSDKVersion;
	Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;

	if (InInstance)
	{
		spdlog::info("NVSDK_NGX_VULKAN_Init InInstance exist!");
		vkInstance = InInstance;
	}

	if (InPD)
	{
		spdlog::info("NVSDK_NGX_VULKAN_Init InPD exist!");
		vkPD = InPD;
	}

	if (InDevice)
	{
		spdlog::info("NVSDK_NGX_VULKAN_Init InDevice exist!");
		vkDevice = InDevice;
	}

	if (InGDPA)
	{
		spdlog::info("NVSDK_NGX_VULKAN_Init InGDPA exist!");
		vkGDPA = InGDPA;
	}

	if (InGIPA)
	{
		spdlog::info("NVSDK_NGX_VULKAN_Init InGIPA exist!");
		vkGIPA = InGIPA;
	}

	if (InFeatureInfo)
		Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

	Config::Instance()->Api = NVNGX_VULKAN;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA,
	NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InEngineType: {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = InEngineVersion;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InEngineType {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = InEngineVersion;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_GetParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_VULKAN_GetParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_AllocateParameters");

	try
	{
		*OutParameters = new NVNGX_Parameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_VULKAN_AllocateParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_GetCapabilityParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_VULKAN_GetCapabilityParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_PopulateParameters_Impl");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	InitNGXParameters(InParameters);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_DestroyParameters");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	delete InParameters;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::debug("NVSDK_NGX_VULKAN_GetScratchBufferSize -> 52428800");

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{

	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_VULKAN_CreateFeature1 Can't create this feature ({0})!", (int)InFeatureID);
		return NVSDK_NGX_Result_Fail;
	}

	// Create feature
	auto handleId = IFeature::GetNextHandleId();
	spdlog::info("NVSDK_NGX_VULKAN_CreateFeature1 HandleId: {0}", handleId);

	if (Config::Instance()->VulkanUpscaler.value_or("fsr21") == "fsr22")
		VkContexts[handleId] = std::make_unique<FSR2FeatureVk>(handleId, InParameters);
	else
		VkContexts[handleId] = std::make_unique<FSR2FeatureVk212>(handleId, InParameters);

	auto deviceContext = VkContexts[handleId].get();
	*OutHandle = deviceContext->Handle();

	if (deviceContext->Init(vkInstance, vkPD, InDevice, InCmdList, vkGIPA, vkGDPA, InParameters))
	{
		Config::Instance()->ActiveFeatureCount++;
		return NVSDK_NGX_Result_Success;
	}

	spdlog::error("NVSDK_NGX_VULKAN_CreateFeature1 CreateFeature failed");
	return NVSDK_NGX_Result_FAIL_PlatformError;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_VULKAN_CreateFeature");

	return NVSDK_NGX_VULKAN_CreateFeature1(vkDevice, InCmdBuffer, InFeatureID, InParameters, OutHandle);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (!InHandle)
		return NVSDK_NGX_Result_Success;

	auto handleId = InHandle->Id;
	spdlog::info("NVSDK_NGX_VULKAN_ReleaseFeature releasing feature with id {0}", handleId);

	if (auto deviceContext = VkContexts[handleId].get(); deviceContext)
	{
		VkContexts[handleId].reset();
		auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
		VkContexts.erase(it);
	}

	Config::Instance()->ActiveFeatureCount--;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_EvaluateFeature(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_VULKAN_EvaluateFeature");

	if (!InCmdList)
	{
		spdlog::error("NVSDK_NGX_VULKAN_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		spdlog::warn("NVSDK_NGX_VULKAN_EvaluateFeature callback exist");

	IFeature_Vk* deviceContext = nullptr;
	auto handleId = InFeatureHandle->Id;

	if (Config::Instance()->changeBackend)
	{
		if (Config::Instance()->newBackend == "")
			Config::Instance()->newBackend = Config::Instance()->Dx12Upscaler.value_or("xess");

		spdlog::info("NVSDK_NGX_VULKAN_EvaluateFeature changing backend to {0}", Config::Instance()->newBackend);

		// first release everything
		if (VkContexts.contains(handleId) && changeBackendCounter == 0)
		{
			auto dc = VkContexts[handleId].get();

			createParams = GetNGXParameters();
			createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
			createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
			createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
			createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
			createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
			createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

			dc = nullptr;

			spdlog::trace("NVSDK_NGX_VULKAN_EvaluateFeature sleeping before reset of current feature for 1000ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			VkContexts[handleId].reset();
			auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			VkContexts.erase(it);

			return NVSDK_NGX_Result_Success;
		}

		changeBackendCounter++;

		if (changeBackendCounter == 1)
		{
			// prepare new upscaler
			if (Config::Instance()->newBackend == "fsr22")
			{
				Config::Instance()->Dx12Upscaler = "fsr22";
				spdlog::info("NVSDK_NGX_VULKAN_EvaluateFeature creating new FSR 2.2.1 feature");
				VkContexts[handleId] = std::make_unique<FSR2FeatureVk>(handleId, createParams);
			}
			else
			{
				Config::Instance()->Dx12Upscaler = "fsr21";
				spdlog::info("NVSDK_NGX_VULKAN_EvaluateFeature creating new FSR 2.1.2 feature");
				VkContexts[handleId] = std::make_unique<FSR2FeatureVk212>(handleId, createParams);
			}

			return NVSDK_NGX_Result_Success;
		}

		if (changeBackendCounter == 2)
		{
			// next frame create context
			auto initResult = VkContexts[handleId]->Init(vkInstance, vkPD, vkDevice, InCmdList, vkGIPA, vkGDPA, createParams);

			Config::Instance()->newBackend = "";
			Config::Instance()->changeBackend = false;
			free(createParams);
			createParams = nullptr;
			changeBackendCounter = 0;

			if (!initResult)
				return NVSDK_NGX_Result_Fail;
		}

		return NVSDK_NGX_Result_Success;
	}

	deviceContext = VkContexts[handleId].get();
	Config::Instance()->CurrentFeature = deviceContext;

	if (deviceContext->Evaluate(InCmdList, InParameters))
		return NVSDK_NGX_Result_Success;
	else
		return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Shutdown(void)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown");

	for (auto const& [key, val] : VkContexts)
		NVSDK_NGX_VULKAN_ReleaseFeature(val->Handle());

	VkContexts.clear();

	vkInstance = nullptr;
	vkPD = nullptr;
	vkDevice = nullptr;

	Config::Instance()->ActiveFeatureCount = 0;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Shutdown1(VkDevice InDevice)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown1");
	return NVSDK_NGX_VULKAN_Shutdown();
}
