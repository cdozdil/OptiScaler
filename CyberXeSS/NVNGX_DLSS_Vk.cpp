#include "pch.h"

#include <ankerl/unordered_dense.h>
#include <vulkan/vulkan.hpp>

#include "Config.h"
#include "FSR2Feature_Vk.h"
#include "NVNGX_Parameter.h"

VkInstance vkInstance;
VkPhysicalDevice vkPD;
VkDevice vkDevice;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Vk>> Dx12Contexts;

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_VULKAN_Init AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_VULKAN_Init AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_VULKAN_Init SDK: {0}", (int)InSDKVersion);

	spdlog::info("NVSDK_NGX_VULKAN_Init BuildPipelines: {0}", Config::Instance()->BuildPipelines.value_or(true));
	spdlog::info("NVSDK_NGX_VULKAN_Init NetworkModel: {0}", Config::Instance()->NetworkModel.value_or(0));
	spdlog::info("NVSDK_NGX_VULKAN_Init LogLevel: {0}", Config::Instance()->LogLevel.value_or(2));

	if (InInstance)
		vkInstance = InInstance;

	if(InPD)
		vkPD = InPD;

	if (InDevice)
		vkDevice = InDevice;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID Init!");
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID SDK: {0}", (int)InSDKVersion);

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID: {0}", InProjectId);

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown(void)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown");

	//CyberXessContext::instance()->VulkanDevice = nullptr;
	//CyberXessContext::instance()->VulkanInstance = nullptr;
	//CyberXessContext::instance()->VulkanPhysicalDevice = nullptr;
	//CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown1(VkDevice InDevice)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown1");

	//CyberXessContext::instance()->VulkanDevice = nullptr;
	//CyberXessContext::instance()->VulkanInstance = nullptr;
	//CyberXessContext::instance()->VulkanPhysicalDevice = nullptr;
	//CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter** OutParameters)
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

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
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

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
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

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_VULKAN_DestroyParameters");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	InParameters->Reset();
	delete InParameters;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::debug("NVSDK_NGX_VULKAN_GetScratchBufferSize -> 52428800");

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	//return NVSDK_NGX_VULKAN_CreateFeature1(CyberXessContext::instance()->VulkanDevice, InCmdBuffer, InFeatureID, InParameters, OutHandle);
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::debug("NVSDK_NGX_VULKAN_CreateFeature1!");
	return NVSDK_NGX_Result_FAIL_PlatformError;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	spdlog::debug("NVSDK_NGX_VULKAN_ReleaseFeature");

	//auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get();
	//CyberXessContext::instance()->DeleteContext(InHandle);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_EvaluateFeature(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_VULKAN_EvaluateFeature");
	return NVSDK_NGX_Result_FAIL_PlatformError;
}