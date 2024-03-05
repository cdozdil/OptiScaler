#include "pch.h"

#include <ankerl/unordered_dense.h>
#include <vulkan/vulkan.hpp>

#include "Config.h"
#include "FSR2Feature_Vk.h"
#include "NVNGX_Parameter.h"

VkInstance vkInstance;
VkPhysicalDevice vkPD;
VkDevice vkDevice;
PFN_vkGetInstanceProcAddr vkGIPA;
PFN_vkGetDeviceProcAddr vkGDPA;

static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Vk>> VkContexts;

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_VULKAN_Init InApplicationId: {0}", InApplicationId);
	std::wstring string(InApplicationDataPath);
	std::string str(string.begin(), string.end());
	spdlog::debug("NVSDK_NGX_VULKAN_Init InApplicationDataPath {0}", str);
	spdlog::info("NVSDK_NGX_VULKAN_Init InSDKVersion: {0:x}", (int)InSDKVersion);

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

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InEngineType: {0}", (int)InEngineType);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_ProjectID InEngineVersion: {0}", InEngineVersion);

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InEngineType {0}", (int)InEngineType);
	spdlog::debug("NVSDK_NGX_VULKAN_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);

	return NVSDK_NGX_VULKAN_Init(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown(void)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown");

	for (auto const& [key, val] : VkContexts)
		NVSDK_NGX_D3D12_ReleaseFeature(val->Handle());

	VkContexts.clear();

	vkInstance = nullptr;
	vkPD = nullptr;
	vkDevice = nullptr;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_Shutdown1(VkDevice InDevice)
{
	spdlog::debug("NVSDK_NGX_VULKAN_Shutdown1");
	return NVSDK_NGX_VULKAN_Shutdown();
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

	delete InParameters;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::debug("NVSDK_NGX_VULKAN_GetScratchBufferSize -> 52428800");

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_VULKAN_CreateFeature1");

	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_VULKAN_CreateFeature1 Can't create this feature ({0})!", (int)InFeatureID);
		return NVSDK_NGX_Result_Fail;
	}

	// Create feature
	auto handleId = IFeature::GetNextHandleId();

	VkContexts[handleId] = std::make_unique<FSR2FeatureVk>(handleId, InParameters);

	auto deviceContext = VkContexts[handleId].get();
	*OutHandle = deviceContext->Handle();

	if (deviceContext->Init(vkInstance, vkPD, InDevice, InCmdList, vkGIPA, vkGDPA, InParameters))
		return NVSDK_NGX_Result_Success;

	spdlog::error("NVSDK_NGX_VULKAN_CreateFeature1 CreateFeature failed");
	return NVSDK_NGX_Result_FAIL_PlatformError;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_VULKAN_CreateFeature");

	return NVSDK_NGX_VULKAN_CreateFeature1(vkDevice, InCmdBuffer, InFeatureID, InParameters, OutHandle);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	spdlog::debug("NVSDK_NGX_VULKAN_ReleaseFeature");

	if (!InHandle)
		return NVSDK_NGX_Result_Success;

	auto handleId = InHandle->Id;

	if (auto deviceContext = VkContexts[handleId].get(); deviceContext)
	{
		auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
		VkContexts.erase(it);
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_VULKAN_EvaluateFeature(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_VULKAN_EvaluateFeature");

	if (!InCmdList)
	{
		spdlog::error("NVSDK_NGX_VULKAN_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature callback exist");

	const auto deviceContext = VkContexts[InFeatureHandle->Id].get();

	if (deviceContext->Evaluate(InCmdList, InParameters))
		return NVSDK_NGX_Result_Success;
	else
		return NVSDK_NGX_Result_Fail;
}