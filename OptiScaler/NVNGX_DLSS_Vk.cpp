#pragma once
#include "pch.h"
#include "Util.h"
#include "resource.h"

#include <ankerl/unordered_dense.h>
#include <vulkan/vulkan.hpp>

#include "Config.h"
#include "backends/fsr2/FSR2Feature_Vk.h"
#include "backends/dlss/DLSSFeature_Vk.h"
#include "backends/dlssd/DLSSDFeature_Vk.h"
#include "backends/fsr2_212/FSR2Feature_Vk_212.h"
#include "backends/fsr31/FSR31Feature_Vk.h"

#include "NVNGX_Parameter.h"
#include "NVNGX_Proxy.h"

#include "imgui/imgui_overlay_vk.h"
#include "imgui/imgui_overlay_dx12.h"

VkInstance vkInstance;
VkPhysicalDevice vkPD;
VkDevice vkDevice;
PFN_vkGetInstanceProcAddr vkGIPA;
PFN_vkGetDeviceProcAddr vkGDPA;

static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Vk>> VkContexts;
static inline NVSDK_NGX_Parameter* createParams = nullptr;
static inline int changeBackendCounter = 0;
static inline int evalCounter = 0;

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsVulkanInited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_Init_Ext() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::VULKAN_Init_Ext");
            auto result = NVNGXProxy::VULKAN_Init_Ext()(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InSDKVersion, InFeatureInfo);
            LOG_INFO("NVNGXProxy::VULKAN_Init_Ext result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetVulkanInited(true);
        }
    }

    return NVSDK_NGX_VULKAN_Init_Ext2(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, vkGetInstanceProcAddr, vkGetDeviceProcAddr, InSDKVersion, InFeatureInfo);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_Ext2(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsVulkanInited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_Init_Ext2() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::VULKAN_Init_Ext2");
            auto result = NVNGXProxy::VULKAN_Init_Ext2()(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
            LOG_INFO("NVNGXProxy::VULKAN_Init_Ext2 result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetVulkanInited(true);
        }
    }

    Config::Instance()->NVNGX_ApplicationId = InApplicationId;
    Config::Instance()->NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
    Config::Instance()->NVNGX_Version = InSDKVersion;
    Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;
    Config::Instance()->NVNGX_Version = InSDKVersion;

    Config::Instance()->NVNGX_FeatureInfo_Paths.clear();

    if (InFeatureInfo != nullptr)
    {
        if (InSDKVersion > 0x0000013)
            Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

        // Doom Ethernal is sending junk data
        if (InFeatureInfo->PathListInfo.Length < 10)
        {
            for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
            {
                const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];
                Config::Instance()->NVNGX_FeatureInfo_Paths.push_back(std::wstring(path));
            }
        }
    }

    LOG_INFO("InApplicationId: {0}", InApplicationId);
    LOG_INFO("InSDKVersion: {0:x}", (UINT)InSDKVersion);
    std::wstring string(InApplicationDataPath);

    LOG_DEBUG("InApplicationDataPath {0}", wstring_to_string(string));

    if (Config::Instance()->NVNGX_FeatureInfo_Paths.size() > 0)
    {
        for (size_t i = 0; i < Config::Instance()->NVNGX_FeatureInfo_Paths.size(); ++i)
        {
            LOG_DEBUG("PathListInfo[{0}]: {1}", i, wstring_to_string(Config::Instance()->NVNGX_FeatureInfo_Paths[i]));
        }
    }

    if (InInstance)
    {
        LOG_INFO("InInstance exist!");
        vkInstance = InInstance;
    }

    if (InPD)
    {
        LOG_INFO("InPD exist!");
        vkPD = InPD;
    }

    if (InDevice)
    {
        LOG_INFO("InDevice exist!");
        vkDevice = InDevice;
    }

    if (InGDPA)
    {
        LOG_INFO("InGDPA exist!");
        vkGDPA = InGDPA;
    }
    else
    {
        LOG_INFO("InGDPA does not exist!");
        vkGDPA = vkGetDeviceProcAddr;
    }

    if (InGIPA)
    {
        LOG_INFO("InGIPA exist!");
        vkGIPA = InGIPA;
    }
    else
    {
        LOG_INFO("InGIPA does not exist!");
        vkGIPA = vkGetInstanceProcAddr;
    }

    Config::Instance()->Api = NVNGX_VULKAN;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_ProjectID_Ext(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsVulkanInited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_Init_ProjectID_Ext() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::VULKAN_Init_ProjectID_Ext");
            auto result = NVNGXProxy::VULKAN_Init_ProjectID_Ext()(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
            LOG_INFO("NVNGXProxy::VULKAN_Init_ProjectID_Ext result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetVulkanInited(true);
        }
    }

    auto result = NVSDK_NGX_VULKAN_Init_Ext2(0x1337, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);

    LOG_DEBUG("InProjectId: {0}", InProjectId);
    LOG_DEBUG("InEngineType: {0}", (int)InEngineType);
    LOG_DEBUG("InEngineVersion: {0}", InEngineVersion);

    Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
    Config::Instance()->NVNGX_Engine = InEngineType;
    Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsVulkanInited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_Init() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::VULKAN_Init");
            auto result = NVNGXProxy::VULKAN_Init()(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
            LOG_INFO("NVNGXProxy::VULKAN_Init result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetVulkanInited(true);
        }
    }

    return NVSDK_NGX_VULKAN_Init_Ext2(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);

}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsVulkanInited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_Init_ProjectID() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::VULKAN_Init_ProjectID");
            auto result = NVNGXProxy::VULKAN_Init_ProjectID()(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
            LOG_INFO("NVNGXProxy::VULKAN_Init_ProjectID result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetVulkanInited(true);
        }
    }

    return NVSDK_NGX_VULKAN_Init_ProjectID_Ext(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_GetParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_GetParameters");
        auto result = NVNGXProxy::VULKAN_GetParameters()(OutParameters);
        LOG_INFO("NVNGXProxy::VULKAN_GetParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return result;
        }
    }

    *OutParameters = GetNGXParameters("OptiVk");
    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements(const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties)
{
    LOG_DEBUG("FeatureID: {0}", (UINT)FeatureDiscoveryInfo->FeatureID);

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_GetFeatureInstanceExtensionRequirements() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_GetFeatureInstanceExtensionRequirements");
        auto result = NVNGXProxy::VULKAN_GetFeatureInstanceExtensionRequirements()(FeatureDiscoveryInfo, OutExtensionCount, OutExtensionProperties);
        LOG_INFO("NVNGXProxy::VULKAN_GetFeatureInstanceExtensionRequirements result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            if (*OutExtensionCount > 0)
                LOG_DEBUG("required extensions: {0}", *OutExtensionCount);

            return result;
        }
    }

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        if (NVNGXProxy::NVNGXModule() != nullptr)
        {
            LOG_INFO("returning no extensions are needed");
            *OutExtensionCount = 0;
        }

        // Only Nvidia hardware should reach here, so in any case returning known extensions for DLSS
        //if (OutExtensionProperties == nullptr)
        //{

        //    if (NVNGXProxy::NVNGXModule() != nullptr)
        //    {
        //        LOG_INFO("returning 3 extensions are needed");
        //        *OutExtensionCount = 3;
        //    }
        //    else
        //    {
        //        LOG_INFO("returning no extensions are needed");
        //        *OutExtensionCount = 0;
        //    }
        //}
        //else if (*OutExtensionCount == 3)
        //{
        //    LOG_INFO("returning extension infos");

        //    strncpy_s(OutExtensionProperties[0]->extensionName, (VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME + '\0'), 39);
        //    OutExtensionProperties[0]->specVersion = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION;

        //    strncpy_s(OutExtensionProperties[1]->extensionName, (VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME + '\0'), 36);
        //    OutExtensionProperties[1]->specVersion = VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_SPEC_VERSION;

        //    strncpy_s(OutExtensionProperties[2]->extensionName, (VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME + '\0'), 39);
        //    OutExtensionProperties[2]->specVersion = VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_SPEC_VERSION;
        //}

        return NVSDK_NGX_Result_Success;
    }

    return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(VkInstance Instance, VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties)
{
    LOG_DEBUG("FeatureID: {0}", (UINT)FeatureDiscoveryInfo->FeatureID);

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_GetFeatureDeviceExtensionRequirements() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_GetFeatureDeviceExtensionRequirements");
        auto result = NVNGXProxy::VULKAN_GetFeatureDeviceExtensionRequirements()(Instance, PhysicalDevice, FeatureDiscoveryInfo, OutExtensionCount, OutExtensionProperties);
        LOG_INFO("NVNGXProxy::VULKAN_GetFeatureDeviceExtensionRequirements result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            if (*OutExtensionCount > 0)
                LOG_DEBUG("required extensions: {0}", *OutExtensionCount);

            return result;
        }
    }

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        if (OutExtensionProperties == nullptr)
        {
            LOG_INFO("returning no extensions are needed!");
            *OutExtensionCount = 0;
        }
        
        // Only Nvidia hardware should reach here, so in any case returning known extensions for DLSS
        //if (OutExtensionProperties == nullptr)
        //{

        //    if (NVNGXProxy::NVNGXModule() != nullptr)
        //    {

        //        LOG_INFO("returning 4 extensions are needed!");
        //        *OutExtensionCount = 4;
        //    }
        //    else
        //    {
        //        LOG_INFO("returning no extensions are needed!");
        //        *OutExtensionCount = 0;
        //    }
        //}
        //else if (*OutExtensionCount == 4)
        //{
        //    LOG_INFO("returning extension infos");

        //    strncpy_s(OutExtensionProperties[0]->extensionName, (VK_NVX_BINARY_IMPORT_EXTENSION_NAME + '\0'), 21);
        //    OutExtensionProperties[0]->specVersion = VK_NVX_BINARY_IMPORT_SPEC_VERSION;

        //    strncpy_s(OutExtensionProperties[1]->extensionName, (VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME + '\0'), 25);
        //    OutExtensionProperties[1]->specVersion = VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION;

        //    strncpy_s(OutExtensionProperties[2]->extensionName, (VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME + '\0'), 29);
        //    OutExtensionProperties[2]->specVersion = VK_EXT_BUFFER_DEVICE_ADDRESS_SPEC_VERSION;

        //    strncpy_s(OutExtensionProperties[3]->extensionName, (VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME + '\0'), 23);
        //    OutExtensionProperties[3]->specVersion = VK_KHR_PUSH_DESCRIPTOR_SPEC_VERSION;
        //}

        return NVSDK_NGX_Result_Success;
    }

    return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_AllocateParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_AllocateParameters");
        auto result = NVNGXProxy::VULKAN_AllocateParameters()(OutParameters);
        LOG_INFO("NVNGXProxy::VULKAN_AllocateParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
            return result;
    }

    auto params = new NVNGX_Parameters();
    params->Name = "OptiVk";
    *OutParameters = params;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetFeatureRequirements(VkInstance VulkanInstance, VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_DEBUG("for FeatureID: {0}", (int)FeatureDiscoveryInfo->FeatureID);

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        if (OutSupported == nullptr)
            OutSupported = new NVSDK_NGX_FeatureRequirement();

        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;

        //Some old windows 10 os version
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D12_GetFeatureRequirements() != nullptr)
    {
        LOG_DEBUG("calling NVNGXProxy::VULKAN_GetFeatureRequirements");
        auto result = NVNGXProxy::VULKAN_GetFeatureRequirements()(VulkanInstance, PhysicalDevice, FeatureDiscoveryInfo, OutSupported);
        LOG_DEBUG("NVNGXProxy::VULKAN_GetFeatureRequirements result {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
            LOG_DEBUG("FeatureSupported: {0}", (UINT)OutSupported->FeatureSupported);

        return result;
    }
    else
    {
        LOG_DEBUG("VULKAN_GetFeatureRequirements not available for FeatureID: {0}", (int)FeatureDiscoveryInfo->FeatureID);
    }

    OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
    return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::IsVulkanInited() && NVNGXProxy::VULKAN_GetCapabilityParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_GetCapabilityParameters");
        auto result = NVNGXProxy::VULKAN_GetCapabilityParameters()(OutParameters);
        LOG_INFO("calling NVNGXProxy::VULKAN_GetCapabilityParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return result;
        }
    }

    *OutParameters = GetNGXParameters("OptiVk");

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    InitNGXParameters(InParameters);

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::VULKAN_DestroyParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::VULKAN_DestroyParameters");
        auto result = NVNGXProxy::VULKAN_DestroyParameters()(InParameters);
        LOG_INFO("calling NVNGXProxy::VULKAN_DestroyParameters result: {0:X}", (UINT)result);

        return result;
    }

    delete InParameters;
    InParameters = nullptr;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
    LOG_WARN("-> 52428800");

    *OutSizeInBytes = 52428800;
    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
    if (InFeatureID != NVSDK_NGX_Feature_SuperSampling && InFeatureID != NVSDK_NGX_Feature_RayReconstruction)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::InitVulkan(vkInstance, vkPD, vkDevice, vkGIPA, vkGDPA) && NVNGXProxy::VULKAN_CreateFeature1() != nullptr)
        {
            auto result = NVNGXProxy::VULKAN_CreateFeature1()(InDevice, InCmdList, InFeatureID, InParameters, OutHandle);
            LOG_INFO("VULKAN_CreateFeature1 result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
            return result;
        }
        else
        {
            LOG_ERROR("Can't create this feature ({0})!", (int)InFeatureID);
            return NVSDK_NGX_Result_Fail;
        }
    }

    // Create feature
    auto handleId = IFeature::GetNextHandleId();
    LOG_INFO("HandleId: {0}", handleId);

    if (InFeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        std::string defaultUpscaler = "fsr21";

        // If original NVNGX available use DLSS as base upscaler
        if (NVNGXProxy::IsVulkanInited())
            defaultUpscaler = "dlss";

        if (Config::Instance()->VulkanUpscaler.value_or(defaultUpscaler) == "dlss")
        {
            if (Config::Instance()->DLSSEnabled.value_or(true))
            {
                VkContexts[handleId] = std::make_unique<DLSSFeatureVk>(handleId, InParameters);

                if (!VkContexts[handleId]->ModuleLoaded())
                {
                    LOG_ERROR("can't create new DLSS feature, Fallback to FSR2.1!");

                    VkContexts[handleId].reset();
                    auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                    VkContexts.erase(it);

                    Config::Instance()->VulkanUpscaler = "fsr21";
                }
                else
                {
                    LOG_INFO("creating new DLSS feature");
                    Config::Instance()->VulkanUpscaler = "dlss";
                }
            }
            else
            {
                Config::Instance()->VulkanUpscaler.reset();
            }
        }

        if (Config::Instance()->VulkanUpscaler.value_or(defaultUpscaler) == "fsr31")
        {
            VkContexts[handleId] = std::make_unique<FSR31FeatureVk>(handleId, InParameters);

            if (!VkContexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new FSR 3.1 feature, Fallback to FSR 2.1!");

                VkContexts[handleId].reset();
                auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                VkContexts.erase(it);

                Config::Instance()->VulkanUpscaler = "fsr21";
            }
            else
            {
                LOG_INFO("creating new FSR 3.1 feature");
                Config::Instance()->VulkanUpscaler = "fsr31";
            }
        }

        if (Config::Instance()->VulkanUpscaler.value_or(defaultUpscaler) == "fsr22")
        {
            VkContexts[handleId] = std::make_unique<FSR2FeatureVk>(handleId, InParameters);
            Config::Instance()->VulkanUpscaler = "fsr22";
        }
        else if (Config::Instance()->VulkanUpscaler.value_or(defaultUpscaler) == "fsr21")
        {
            VkContexts[handleId] = std::make_unique<FSR2FeatureVk212>(handleId, InParameters);
            Config::Instance()->VulkanUpscaler = "fsr21";
        }
    }
    else
    {
        LOG_INFO("creating new DLSSD feature");
        VkContexts[handleId] = std::make_unique<DLSSDFeatureVk>(handleId, InParameters);
    }

    auto deviceContext = VkContexts[handleId].get();
    *OutHandle = deviceContext->Handle();

    if (deviceContext->Init(vkInstance, vkPD, InDevice, InCmdList, vkGIPA, vkGDPA, InParameters))
    {
        Config::Instance()->CurrentFeature = deviceContext;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        evalCounter = 0;

        return NVSDK_NGX_Result_Success;
    }

    LOG_ERROR("CreateFeature failed");
    return NVSDK_NGX_Result_FAIL_PlatformError;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
    LOG_FUNC();

    if (InFeatureID != NVSDK_NGX_Feature_SuperSampling && InFeatureID != NVSDK_NGX_Feature_RayReconstruction)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::InitVulkan(vkInstance, vkPD, vkDevice, vkGIPA, vkGDPA) && NVNGXProxy::VULKAN_CreateFeature() != nullptr)
        {
            auto result = NVNGXProxy::VULKAN_CreateFeature()(InCmdBuffer, InFeatureID, InParameters, OutHandle);
            LOG_INFO("VULKAN_CreateFeature result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
            return result;
        }
    }

    return NVSDK_NGX_VULKAN_CreateFeature1(vkDevice, InCmdBuffer, InFeatureID, InParameters, OutHandle);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
    if (!InHandle)
        return NVSDK_NGX_Result_Success;

    auto handleId = InHandle->Id;
    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::VULKAN_ReleaseFeature() != nullptr)
        {
            auto result = NVNGXProxy::VULKAN_ReleaseFeature()(InHandle);
            LOG_INFO("VULKAN_ReleaseFeature result for ({0}): {1:X}", handleId, (UINT)result);
            return result;
        }
        else
        {
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    LOG_INFO("releasing feature with id {0}", handleId);

    if (auto deviceContext = VkContexts[handleId].get(); deviceContext)
    {
        if (deviceContext == Config::Instance()->CurrentFeature)
        {
            Config::Instance()->CurrentFeature = nullptr;
            deviceContext->Shutdown();
        }

        vkDeviceWaitIdle(vkDevice);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        VkContexts[handleId].reset();
        auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
        VkContexts.erase(it);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_EvaluateFeature(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Handle* InFeatureHandle, NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
    if (InFeatureHandle == nullptr)
    {
        LOG_DEBUG("InFeatureHandle is null");
        return NVSDK_NGX_Result_Fail;
        // returning success to prevent breaking flow of the app
        // return NVSDK_NGX_Result_Success;
    }
    else
    {
        LOG_DEBUG("Handle: {0}", InFeatureHandle->Id);
    }

    if (InCmdBuffer == nullptr)
    {
        LOG_ERROR("InCmdBuffer is null!!!");
        return NVSDK_NGX_Result_Fail;
    }

    auto handleId = InFeatureHandle->Id;
    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::VULKAN_EvaluateFeature() != nullptr)
        {
            auto result = NVNGXProxy::VULKAN_EvaluateFeature()(InCmdBuffer, InFeatureHandle, InParameters, InCallback);
            LOG_INFO("VULKAN_EvaluateFeature result for ({0}): {1:X}", handleId, (UINT)result);
            return result;
        }
        else
        {
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    evalCounter++;
    if (Config::Instance()->SkipFirstFrames.has_value() && evalCounter < Config::Instance()->SkipFirstFrames.value())
        return NVSDK_NGX_Result_Success;

    // DLSS Enabler check
    int deAvail;
    if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
    {
        if (Config::Instance()->DE_Available != (deAvail > 0))
            LOG_INFO("DLSSEnabler.Available: {0}", deAvail);

        Config::Instance()->DE_Available = (deAvail > 0);
    }

    if (Config::Instance()->OverlayMenu.value_or(true) && Config::Instance()->CurrentFeature != nullptr && !ImGuiOverlayVk::IsInitedVk() &&
        Config::Instance()->CurrentFeature->FrameCount() > Config::Instance()->MenuInitDelay.value_or(75))
    {
        ImGuiOverlayDx12::ShutdownDx12();
        auto hwnd = Util::GetProcessWindow();
        ImGuiOverlayVk::InitVk(hwnd, vkDevice, vkInstance, vkPD);
    }

    // Check window recreation
    if (Config::Instance()->OverlayMenu.value_or(true))
    {
        HWND currentHandle = Util::GetProcessWindow();
        if (ImGuiOverlayVk::IsInitedVk() && ImGuiOverlayVk::Handle() != currentHandle)
            ImGuiOverlayVk::ReInitVk(currentHandle);
    }

    if (InCallback)
        LOG_WARN("callback exist");

    IFeature_Vk* deviceContext = nullptr;

    if (Config::Instance()->changeBackend)
    {
        if (Config::Instance()->newBackend == "" || (!Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->newBackend == "dlss"))
            Config::Instance()->newBackend = Config::Instance()->VulkanUpscaler.value_or("fsr21");

        changeBackendCounter++;
        
        LOG_INFO("changeBackend is true, counter: {0}", changeBackendCounter);

        // first release everything
        if (changeBackendCounter == 1)
        {
            if (VkContexts.contains(handleId))
            {
                LOG_INFO("changing backend to {0}", Config::Instance()->newBackend);

                auto dc = VkContexts[handleId].get();

                if (Config::Instance()->newBackend != "dlssd" && Config::Instance()->newBackend != "dlss")
                    createParams = GetNGXParameters("OptiVk");
                else
                    createParams = InParameters;

                createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
                createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
                createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
                createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
                createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
                createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

                dc = nullptr;

                LOG_DEBUG("sleeping before reset of current feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                VkContexts[handleId].reset();
                auto it = std::find_if(VkContexts.begin(), VkContexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                VkContexts.erase(it);

                Config::Instance()->CurrentFeature = nullptr;
            }
            else
            {
                LOG_ERROR("can't find handle {0} in VkContexts!", handleId);

                Config::Instance()->newBackend = "";
                Config::Instance()->changeBackend = false;

                if (createParams != nullptr)
                {
                    free(createParams);
                    createParams = nullptr;
                }

                changeBackendCounter = 0;
            }

            return NVSDK_NGX_Result_Success;
        }

        if (changeBackendCounter == 2)
        {
            // prepare new upscaler
            if (Config::Instance()->newBackend == "fsr22")
            {
                Config::Instance()->VulkanUpscaler = "fsr22";
                LOG_INFO("creating new FSR 2.2.1 feature");
                VkContexts[handleId] = std::make_unique<FSR2FeatureVk>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "dlss")
            {
                Config::Instance()->VulkanUpscaler = "dlss";
                LOG_INFO("creating new DLSS feature");
                VkContexts[handleId] = std::make_unique<DLSSFeatureVk>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr31")
            {
                Config::Instance()->VulkanUpscaler = "fsr31";
                LOG_INFO("creating new FSR 3.1 feature");
                VkContexts[handleId] = std::make_unique<FSR31FeatureVk>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "dlssd")
            {
                LOG_INFO("creating new DLSSD feature");
                VkContexts[handleId] = std::make_unique<DLSSDFeatureVk>(handleId, InParameters);
            }
            else
            {
                Config::Instance()->VulkanUpscaler = "fsr21";
                LOG_INFO("creating new FSR 2.1.2 feature");
                VkContexts[handleId] = std::make_unique<FSR2FeatureVk212>(handleId, createParams);
            }

            return NVSDK_NGX_Result_Success;
        }

        if (changeBackendCounter == 3)
        {
            // next frame create context
            auto initResult = VkContexts[handleId]->Init(vkInstance, vkPD, vkDevice, InCmdBuffer, vkGIPA, vkGDPA, createParams);

            changeBackendCounter = 0;

            if (!initResult || !VkContexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("init failed with {0} feature", Config::Instance()->newBackend);

                if (Config::Instance()->newBackend != "dlssd")
                {
                    Config::Instance()->newBackend = "fsr21";
                    Config::Instance()->changeBackend = true;
                }
                else
                {
                    Config::Instance()->newBackend = "";
                    Config::Instance()->changeBackend = false;
                    return NVSDK_NGX_Result_Success;
                }
            }
            else
            {
                LOG_INFO("init successful for {0}, upscaler changed", Config::Instance()->newBackend);

                Config::Instance()->newBackend = "";
                Config::Instance()->changeBackend = false;
                evalCounter = 0;
            }

            // if opti nvparam release it
            int optiParam = 0;
            if (createParams->Get("OptiScaler", &optiParam) == NVSDK_NGX_Result_Success && optiParam == 1)
            {
                free(createParams);
                createParams = nullptr;
            }
        }

        // if initial feature can't be inited
        Config::Instance()->CurrentFeature = VkContexts[handleId].get();

        return NVSDK_NGX_Result_Success;
    }

    deviceContext = VkContexts[handleId].get();
    Config::Instance()->CurrentFeature = deviceContext;

    if (!deviceContext->IsInited() && Config::Instance()->Dx12Upscaler.value_or("fsr21") != "fsr21")
    {
        Config::Instance()->newBackend = "fsr21";
        Config::Instance()->changeBackend = true;
        return NVSDK_NGX_Result_Success;
    }

    if (deviceContext->Evaluate(InCmdBuffer, InParameters))
        return NVSDK_NGX_Result_Success;
    else
        return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Shutdown(void)
{
    LOG_FUNC();

    for (auto const& [key, val] : VkContexts)
        NVSDK_NGX_VULKAN_ReleaseFeature(val->Handle());

    VkContexts.clear();

    vkInstance = nullptr;
    vkPD = nullptr;
    vkDevice = nullptr;

    Config::Instance()->CurrentFeature = nullptr;

    DLSSFeatureVk::Shutdown(vkDevice);

    if (Config::Instance()->OverlayMenu.value_or(true) && ImGuiOverlayVk::IsInitedVk())
        ImGuiOverlayVk::ShutdownVk();


    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsVulkanInited() && NVNGXProxy::VULKAN_Shutdown() != nullptr)
    {
        auto result = NVNGXProxy::VULKAN_Shutdown()();
        NVNGXProxy::SetVulkanInited(false);
        LOG_INFO("VULKAN_Shutdown result: {0:X}", (UINT)result);
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_VULKAN_Shutdown1(VkDevice InDevice)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsVulkanInited() && NVNGXProxy::VULKAN_Shutdown1() != nullptr)
    {
        auto result = NVNGXProxy::VULKAN_Shutdown1()(InDevice);
        NVNGXProxy::SetVulkanInited(false);
        LOG_INFO("VULKAN_Shutdown1 result: {0:X}", (UINT)result);
    }

    return NVSDK_NGX_VULKAN_Shutdown();
}
