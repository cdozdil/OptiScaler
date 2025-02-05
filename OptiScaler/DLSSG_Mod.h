#pragma once

#include "proxies/NVNGX_Proxy.h"

#define DLSSG_MOD_ID_OFFSET 2000000

typedef void(*PFN_EnableDebugView)(bool enable);

class DLSSGMod
{
private:
    inline static HMODULE _dll = nullptr;
    
    inline static PFN_EnableDebugView _fsrDebugView = nullptr;

    inline static PFN_D3D12_Init _DLSSG_D3D12_Init = nullptr;
    inline static PFN_D3D12_Init_Ext _DLSSG_D3D12_Init_Ext = nullptr;
    inline static PFN_D3D12_Shutdown _DLSSG_D3D12_Shutdown = nullptr;
    inline static PFN_D3D12_Shutdown1 _DLSSG_D3D12_Shutdown1 = nullptr;
    inline static PFN_D3D12_GetScratchBufferSize _DLSSG_D3D12_GetScratchBufferSize = nullptr;
    inline static PFN_D3D12_CreateFeature _DLSSG_D3D12_CreateFeature = nullptr;
    inline static PFN_D3D12_ReleaseFeature _DLSSG_D3D12_ReleaseFeature = nullptr;
    inline static PFN_D3D12_GetFeatureRequirements _DLSSG_D3D12_GetFeatureRequirements = nullptr; // unused
    inline static PFN_D3D12_EvaluateFeature _DLSSG_D3D12_EvaluateFeature = nullptr;
    inline static PFN_D3D12_PopulateParameters_Impl _DLSSG_D3D12_PopulateParameters_Impl = nullptr;

    inline static PFN_VULKAN_Init _DLSSG_VULKAN_Init = nullptr;
    inline static PFN_VULKAN_Init_Ext _DLSSG_VULKAN_Init_Ext = nullptr;
    inline static PFN_VULKAN_Init_Ext2 _DLSSG_VULKAN_Init_Ext2 = nullptr;
    inline static PFN_VULKAN_Shutdown _DLSSG_VULKAN_Shutdown = nullptr;
    inline static PFN_VULKAN_Shutdown1 _DLSSG_VULKAN_Shutdown1 = nullptr;
    inline static PFN_VULKAN_GetScratchBufferSize _DLSSG_VULKAN_GetScratchBufferSize = nullptr;
    inline static PFN_VULKAN_CreateFeature _DLSSG_VULKAN_CreateFeature = nullptr;
    inline static PFN_VULKAN_CreateFeature1 _DLSSG_VULKAN_CreateFeature1 = nullptr;
    inline static PFN_VULKAN_ReleaseFeature _DLSSG_VULKAN_ReleaseFeature = nullptr;
    inline static PFN_VULKAN_GetFeatureRequirements _DLSSG_VULKAN_GetFeatureRequirements = nullptr; // unused
    inline static PFN_VULKAN_EvaluateFeature _DLSSG_VULKAN_EvaluateFeature = nullptr;
    inline static PFN_VULKAN_PopulateParameters_Impl _DLSSG_VULKAN_PopulateParameters_Impl = nullptr;

    inline static bool dx12_inited = false;
    inline static bool vulkan_inited = false;
public:
    static void InitDLSSGMod_Dx12() {
        LOG_FUNC();

        if (dx12_inited)
            return;

        if (Config::Instance()->FGType.value_or_default() == FGType::Nukems && !State::Instance().enablerAvailable) {
            if (_dll == nullptr)
            {
                auto dllPath = Util::DllPath().parent_path() / "dlssg_to_fsr3_amd_is_better.dll";
                _dll = LoadLibraryW(dllPath.c_str());
            }

            if (_dll != nullptr) {
                _DLSSG_D3D12_Init = (PFN_D3D12_Init)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init");
                _DLSSG_D3D12_Init_Ext = (PFN_D3D12_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init_Ext");
                _DLSSG_D3D12_Shutdown = (PFN_D3D12_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown");
                _DLSSG_D3D12_Shutdown1 = (PFN_D3D12_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown1");
                _DLSSG_D3D12_GetScratchBufferSize = (PFN_D3D12_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetScratchBufferSize");
                _DLSSG_D3D12_CreateFeature = (PFN_D3D12_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_CreateFeature");
                _DLSSG_D3D12_ReleaseFeature = (PFN_D3D12_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_ReleaseFeature");
                _DLSSG_D3D12_GetFeatureRequirements = (PFN_D3D12_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetFeatureRequirements");
                _DLSSG_D3D12_EvaluateFeature = (PFN_D3D12_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_EvaluateFeature");
                _DLSSG_D3D12_PopulateParameters_Impl = (PFN_D3D12_PopulateParameters_Impl)GetProcAddress(_dll, "NVSDK_NGX_D3D12_PopulateParameters_Impl");
                _fsrDebugView = (PFN_EnableDebugView)GetProcAddress(_dll, "FSRDebugView");
                dx12_inited = true;

                LOG_INFO("DLSSG Mod initialized for DX12");
            }
            else {
                LOG_INFO("DLSSG Mod enabled but cannot be loaded");
            }
        }
    }

    static void InitDLSSGMod_Vulkan() {
        LOG_FUNC();

        if (vulkan_inited)
            return;

        if (Config::Instance()->FGType.value_or_default() == FGType::Nukems && !State::Instance().enablerAvailable) {
            if (_dll == nullptr)
            {
                auto dllPath = Util::DllPath().parent_path() / "dlssg_to_fsr3_amd_is_better.dll";
                _dll = LoadLibraryW(dllPath.c_str());
            }

            if (_dll != nullptr) {
                _DLSSG_VULKAN_Init = (PFN_VULKAN_Init)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init");
                _DLSSG_VULKAN_Init_Ext = (PFN_VULKAN_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext");
                _DLSSG_VULKAN_Init_Ext2 = (PFN_VULKAN_Init_Ext2)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext2");
                _DLSSG_VULKAN_Shutdown = (PFN_VULKAN_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown");
                _DLSSG_VULKAN_Shutdown1 = (PFN_VULKAN_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown1");
                _DLSSG_VULKAN_GetScratchBufferSize = (PFN_VULKAN_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetScratchBufferSize");
                _DLSSG_VULKAN_CreateFeature = (PFN_VULKAN_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature");
                _DLSSG_VULKAN_CreateFeature1 = (PFN_VULKAN_CreateFeature1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature1");
                _DLSSG_VULKAN_ReleaseFeature = (PFN_VULKAN_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_ReleaseFeature");
                _DLSSG_VULKAN_GetFeatureRequirements = (PFN_VULKAN_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureRequirements");
                _DLSSG_VULKAN_EvaluateFeature = (PFN_VULKAN_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_EvaluateFeature");
                _DLSSG_VULKAN_PopulateParameters_Impl = (PFN_VULKAN_PopulateParameters_Impl)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_PopulateParameters_Impl");
                _fsrDebugView = (PFN_EnableDebugView)GetProcAddress(_dll, "FSRDebugView");
                vulkan_inited = true;

                LOG_INFO("DLSSG Mod initialized for Vulkan");
            }
            else {
                LOG_INFO("DLSSG Mod enabled but cannot be loaded");
            }
        }
    }

    static inline bool isLoaded() {
        return _dll != nullptr;
    }

    static inline PFN_EnableDebugView FSRDebugView() {
        return _fsrDebugView;
    }

    static inline bool isDx12Available() {
        return isLoaded() && dx12_inited;
    }

    static inline NVSDK_NGX_Result D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_Init(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_Shutdown()
    {
        if (isDx12Available())
            return _DLSSG_D3D12_Shutdown();
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_Shutdown1(ID3D12Device* InDevice)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_Shutdown1(InDevice);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_GetScratchBufferSize(InFeatureId, InParameters, OutSizeInBytes);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
    {
        if (isDx12Available()) {
            auto result = _DLSSG_D3D12_CreateFeature(InCmdList, InFeatureID, InParameters, OutHandle);
            (*OutHandle)->Id += DLSSG_MOD_ID_OFFSET;
            return result;
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
    {
        if (isDx12Available() && InHandle->Id >= DLSSG_MOD_ID_OFFSET) {
            NVSDK_NGX_Handle TempHandle = {
                .Id = InHandle->Id - DLSSG_MOD_ID_OFFSET
            };
            return _DLSSG_D3D12_ReleaseFeature(&TempHandle);
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_GetFeatureRequirements(Adapter, FeatureDiscoveryInfo, OutSupported);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
    {
        if (isDx12Available() && InFeatureHandle->Id >= DLSSG_MOD_ID_OFFSET) {
            NVSDK_NGX_Handle TempHandle = {
                .Id = InFeatureHandle->Id - DLSSG_MOD_ID_OFFSET
            };
            return _DLSSG_D3D12_EvaluateFeature(InCmdList, &TempHandle, InParameters, InCallback);
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result D3D12_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
    {
        if (isDx12Available())
            return _DLSSG_D3D12_PopulateParameters_Impl(InParameters);
        return NVSDK_NGX_Result_Fail;
    }



    static inline bool isVulkanAvailable() {
        return isLoaded() && vulkan_inited;
    }

    static inline NVSDK_NGX_Result VULKAN_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_Init(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InFeatureInfo, InSDKVersion);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_Init_Ext(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InSDKVersion, InFeatureInfo);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_Init_Ext2(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_Init_Ext2(InApplicationId, InApplicationDataPath, InInstance, InPD, InDevice, InGIPA, InGDPA, InSDKVersion, InFeatureInfo);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_Shutdown()
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_Shutdown();
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_Shutdown1(VkDevice InDevice)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_Shutdown1(InDevice);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_GetScratchBufferSize(InFeatureId, InParameters, OutSizeInBytes);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_CreateFeature(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
    {
        if (isVulkanAvailable()) {
            auto result = _DLSSG_VULKAN_CreateFeature(InCmdBuffer, InFeatureID, InParameters, OutHandle);
            (*OutHandle)->Id += DLSSG_MOD_ID_OFFSET;
            return result;
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_CreateFeature1(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
    {
        if (isVulkanAvailable()) {
            auto result = _DLSSG_VULKAN_CreateFeature1(InDevice, InCmdList, InFeatureID, InParameters, OutHandle);
            (*OutHandle)->Id += DLSSG_MOD_ID_OFFSET;
            return result;
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
    {
        if (isVulkanAvailable() && InHandle->Id >= DLSSG_MOD_ID_OFFSET) {
            NVSDK_NGX_Handle TempHandle = {
                .Id = InHandle->Id - DLSSG_MOD_ID_OFFSET
            };
            return _DLSSG_VULKAN_ReleaseFeature(&TempHandle);
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_GetFeatureRequirements(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_GetFeatureRequirements(Instance, PhysicalDevice, FeatureDiscoveryInfo, OutSupported);
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_EvaluateFeature(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
    {
        if (isVulkanAvailable() && InFeatureHandle->Id >= DLSSG_MOD_ID_OFFSET) {
            NVSDK_NGX_Handle TempHandle = {
                .Id = InFeatureHandle->Id - DLSSG_MOD_ID_OFFSET
            };
            return _DLSSG_VULKAN_EvaluateFeature(InCmdList, &TempHandle, InParameters, InCallback);
        }
        return NVSDK_NGX_Result_Fail;
    }

    static inline NVSDK_NGX_Result VULKAN_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
    {
        if (isVulkanAvailable())
            return _DLSSG_VULKAN_PopulateParameters_Impl(InParameters);
        return NVSDK_NGX_Result_Fail;
    }
};