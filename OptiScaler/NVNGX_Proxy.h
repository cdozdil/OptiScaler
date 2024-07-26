#pragma once

#include "pch.h"
#include "Util.h"
#include "Config.h"
#include "Logger.h"
#include <vulkan/vulkan.hpp>

#include "nvapi/nvapi.h"

#include <filesystem>
#include "detours/detours.h"

#pragma region spoofing hooks for 16xx

// NvAPI_GPU_GetArchInfo hooking based on Nukem's spoofing code here
// https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/wrapper_generic/nvapi.cpp#L50

enum class NV_INTERFACE : uint32_t
{
    GPU_GetArchInfo = 0xD8265D24,
    D3D12_SetRawScgPriority = 0x5DB3048A,
};

typedef void* (__stdcall* PFN_NvApi_QueryInterface)(NV_INTERFACE InterfaceId);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements)(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice,
                                                                       const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);

using PfnNvAPI_GPU_GetArchInfo = uint32_t(__stdcall*)(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo);

inline static PFN_NvApi_QueryInterface OriginalNvAPI_QueryInterface = nullptr;
inline static PfnNvAPI_GPU_GetArchInfo OriginalNvAPI_GPU_GetArchInfo = nullptr;
inline static PFN_NVSDK_NGX_GetFeatureRequirements Original_Dx11_GetFeatureRequirements = nullptr;
inline static PFN_NVSDK_NGX_GetFeatureRequirements Original_Dx12_GetFeatureRequirements = nullptr;
inline static PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements Original_Vulkan_GetFeatureRequirements = nullptr;

inline static uint32_t __stdcall HookedNvAPI_GPU_GetArchInfo(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo)
{
    if (OriginalNvAPI_GPU_GetArchInfo)
    {
        const auto status = OriginalNvAPI_GPU_GetArchInfo(GPUHandle, ArchInfo);

        if (status == 0 && ArchInfo)
        {
            LOG_DEBUG("From api arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);

            // for 16xx cards
            if (ArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && ArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106)
            {
                ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;
                ArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_TU106;

                LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);
            }
            //else if (ArchInfo->architecture < NV_GPU_ARCHITECTURE_TU100 && ArchInfo->architecture >= NV_GPU_ARCHITECTURE_GP100)
            //{
            //	LOG_INFO("Spoofing below 16xx arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);

            //	ArchInfo->architecture = NV_GPU_ARCHITECTURE_TU100;
            //	ArchInfo->architecture_id = NV_GPU_ARCHITECTURE_TU100;
            //	ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;
            //	ArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_TU106;

            //	LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);
            //}
        }

        return status;
    }

    return 0xFFFFFFFF;
}

inline static void* __stdcall HookedNvAPI_QueryInterface(NV_INTERFACE InterfaceId)
{
    const auto result = OriginalNvAPI_QueryInterface(InterfaceId);

    if (result)
    {
        if (InterfaceId == NV_INTERFACE::GPU_GetArchInfo)
        {
            OriginalNvAPI_GPU_GetArchInfo = static_cast<PfnNvAPI_GPU_GetArchInfo>(result);
            return &HookedNvAPI_GPU_GetArchInfo;
        }
    }

    return result;
}

inline static NVSDK_NGX_Result __stdcall Hooked_Dx12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_FUNC();

    auto result = Original_Dx12_GetFeatureRequirements(Adapter, FeatureDiscoveryInfo, OutSupported);

    if (result == NVSDK_NGX_Result_Success && FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        LOG_INFO("Spoofing support!");
        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
    }

    return result;
}

inline static NVSDK_NGX_Result __stdcall Hooked_Dx11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_FUNC();

    auto result = Original_Dx11_GetFeatureRequirements(Adapter, FeatureDiscoveryInfo, OutSupported);

    if (result == NVSDK_NGX_Result_Success && FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        LOG_INFO("Spoofing support!");
        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
    }

    return result;
}

inline static NVSDK_NGX_Result __stdcall Hooked_Vulkan_GetFeatureRequirements(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice,
                                                                              const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_FUNC();

    auto result = Original_Vulkan_GetFeatureRequirements(Instance, PhysicalDevice, FeatureDiscoveryInfo, OutSupported);

    if (result == NVSDK_NGX_Result_Success && FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        LOG_INFO("Spoofing support!");
        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
    }

    return result;
}

inline static void HookNvApi()
{
    if (OriginalNvAPI_QueryInterface != nullptr)
        return;

    LOG_DEBUG("Trying to hook NvApi");
    OriginalNvAPI_QueryInterface = (PFN_NvApi_QueryInterface)DetourFindFunction("nvapi64.dll", "nvapi_QueryInterface");
    LOG_DEBUG("OriginalNvAPI_QueryInterface = {0:X}", (unsigned long long)OriginalNvAPI_QueryInterface);

    if (OriginalNvAPI_QueryInterface != nullptr)
    {
        LOG_INFO("NvAPI_QueryInterface found, hooking!");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
        DetourTransactionCommit();
    }
}

inline static void HookNgxApi(HMODULE nvngx)
{
    if (Original_Dx11_GetFeatureRequirements != nullptr || Original_Dx12_GetFeatureRequirements != nullptr)
        return;

    LOG_DEBUG("Trying to hook NgxApi");

    Original_Dx11_GetFeatureRequirements = (PFN_NVSDK_NGX_GetFeatureRequirements)GetProcAddress(nvngx, "NVSDK_NGX_D3D11_GetFeatureRequirements");
    Original_Dx12_GetFeatureRequirements = (PFN_NVSDK_NGX_GetFeatureRequirements)GetProcAddress(nvngx, "NVSDK_NGX_D3D12_GetFeatureRequirements");
    Original_Vulkan_GetFeatureRequirements = (PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements)GetProcAddress(nvngx, "NVSDK_NGX_VULKAN_GetFeatureRequirements");

    if (Original_Dx11_GetFeatureRequirements != nullptr || Original_Dx12_GetFeatureRequirements != nullptr)
    {
        LOG_INFO("NVSDK_NGX_D3D1X_GetFeatureRequirements found, hooking!");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (Original_Dx11_GetFeatureRequirements != nullptr)
            DetourAttach(&(PVOID&)Original_Dx11_GetFeatureRequirements, Hooked_Dx11_GetFeatureRequirements);

        if (Original_Dx12_GetFeatureRequirements != nullptr)
            DetourAttach(&(PVOID&)Original_Dx12_GetFeatureRequirements, Hooked_Dx12_GetFeatureRequirements);

        if (Original_Vulkan_GetFeatureRequirements != nullptr)
            DetourAttach(&(PVOID&)Original_Vulkan_GetFeatureRequirements, Hooked_Vulkan_GetFeatureRequirements);

        DetourTransactionCommit();
    }
}

inline static void UnhookApis()
{
    if (OriginalNvAPI_QueryInterface != nullptr || Original_Dx11_GetFeatureRequirements != nullptr || Original_Dx12_GetFeatureRequirements != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (OriginalNvAPI_QueryInterface != nullptr)
        {
            DetourDetach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
            OriginalNvAPI_QueryInterface = nullptr;
        }

        if (Original_Dx11_GetFeatureRequirements != nullptr)
        {
            DetourDetach(&(PVOID&)Original_Dx11_GetFeatureRequirements, Hooked_Dx11_GetFeatureRequirements);
            Original_Dx11_GetFeatureRequirements = nullptr;
        }

        if (Original_Dx12_GetFeatureRequirements != nullptr)
        {
            DetourDetach(&(PVOID&)Original_Dx12_GetFeatureRequirements, Hooked_Dx12_GetFeatureRequirements);
            Original_Dx12_GetFeatureRequirements = nullptr;
        }

        if (Original_Vulkan_GetFeatureRequirements != nullptr)
        {
            DetourDetach(&(PVOID&)Original_Vulkan_GetFeatureRequirements, Hooked_Vulkan_GetFeatureRequirements);
            Original_Vulkan_GetFeatureRequirements = nullptr;
        }

        DetourTransactionCommit();
    }
}

#pragma endregion


typedef NVSDK_NGX_Result(*PFN_CUDA_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_CUDA_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_CUDA_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_CUDA_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_CUDA_CreateFeature)(NVSDK_NGX_Feature InFeatureID, const NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_CUDA_EvaluateFeature)(const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);
typedef NVSDK_NGX_Result(*PFN_CUDA_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);

typedef NVSDK_NGX_Result(*PFN_D3D11_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_D3D11_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D11_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D11_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_D3D11_Shutdown1)(ID3D11Device* InDevice);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_D3D11_CreateFeature)(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_D3D11_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_D3D11_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_D3D11_EvaluateFeature)(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

typedef NVSDK_NGX_Result(*PFN_D3D12_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_D3D12_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D12_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_D3D12_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_D3D12_Shutdown1)(ID3D12Device* InDevice);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_D3D12_CreateFeature)(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_D3D12_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_D3D12_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_D3D12_EvaluateFeature)(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

typedef NVSDK_NGX_Result(*PFN_UpdateFeature)(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID);

typedef NVSDK_NGX_Result(*PFN_VULKAN_RequiredExtensions)(unsigned int* OutInstanceExtCount, const char*** OutInstanceExts, unsigned int* OutDeviceExtCount, const char*** OutDeviceExts);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_Ext2)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_ProjectID_Ext)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_VULKAN_Shutdown1)(VkDevice InDevice);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetCapabilityParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetScratchBufferSize)(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes);
typedef NVSDK_NGX_Result(*PFN_VULKAN_CreateFeature)(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_CreateFeature1)(VkDevice InDevice, VkCommandBuffer InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureRequirements)(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureInstanceExtensionRequirements)(const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties);
typedef NVSDK_NGX_Result(*PFN_VULKAN_GetFeatureDeviceExtensionRequirements)(VkInstance Instance, VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, uint32_t* OutExtensionCount, VkExtensionProperties** OutExtensionProperties);
typedef NVSDK_NGX_Result(*PFN_VULKAN_EvaluateFeature)(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

class NVNGXProxy
{
private:
    inline static HMODULE _dll = nullptr;
    inline static bool _cudaInited = false;
    inline static bool _dx11Inited = false;
    inline static bool _dx12Inited = false;
    inline static bool _vulkanInited = false;

    inline static PFN_CUDA_Init _CUDA_Init = nullptr;
    inline static PFN_CUDA_Init_ProjectID _CUDA_Init_ProjectID = nullptr;
    inline static PFN_CUDA_Shutdown _CUDA_Shutdown = nullptr;
    inline static PFN_CUDA_GetParameters _CUDA_GetParameters = nullptr;
    inline static PFN_CUDA_AllocateParameters _CUDA_AllocateParameters = nullptr;
    inline static PFN_CUDA_GetCapabilityParameters _CUDA_GetCapabilityParameters = nullptr;
    inline static PFN_CUDA_DestroyParameters _CUDA_DestroyParameters = nullptr;
    inline static PFN_CUDA_GetScratchBufferSize _CUDA_GetScratchBufferSize = nullptr;
    inline static PFN_CUDA_CreateFeature _CUDA_CreateFeature = nullptr;
    inline static PFN_CUDA_EvaluateFeature _CUDA_EvaluateFeature = nullptr;
    inline static PFN_CUDA_ReleaseFeature _CUDA_ReleaseFeature = nullptr;

    inline static PFN_D3D11_Init _D3D11_Init = nullptr;
    inline static PFN_D3D11_Init_ProjectID _D3D11_Init_ProjectID = nullptr;
    inline static PFN_D3D11_Init_Ext _D3D11_Init_Ext = nullptr;
    inline static PFN_D3D11_Shutdown _D3D11_Shutdown = nullptr;
    inline static PFN_D3D11_Shutdown1 _D3D11_Shutdown1 = nullptr;
    inline static PFN_D3D11_GetParameters _D3D11_GetParameters = nullptr;
    inline static PFN_D3D11_AllocateParameters _D3D11_AllocateParameters = nullptr;
    inline static PFN_D3D11_GetCapabilityParameters _D3D11_GetCapabilityParameters = nullptr;
    inline static PFN_D3D11_DestroyParameters _D3D11_DestroyParameters = nullptr;
    inline static PFN_D3D11_GetScratchBufferSize _D3D11_GetScratchBufferSize = nullptr;
    inline static PFN_D3D11_CreateFeature _D3D11_CreateFeature = nullptr;
    inline static PFN_D3D11_ReleaseFeature _D3D11_ReleaseFeature = nullptr;
    inline static PFN_D3D11_GetFeatureRequirements _D3D11_GetFeatureRequirements = nullptr;
    inline static PFN_D3D11_EvaluateFeature _D3D11_EvaluateFeature = nullptr;

    inline static PFN_D3D12_Init _D3D12_Init = nullptr;
    inline static PFN_D3D12_Init_ProjectID _D3D12_Init_ProjectID = nullptr;
    inline static PFN_D3D12_Init_Ext _D3D12_Init_Ext = nullptr;
    inline static PFN_D3D12_Shutdown _D3D12_Shutdown = nullptr;
    inline static PFN_D3D12_Shutdown1 _D3D12_Shutdown1 = nullptr;
    inline static PFN_D3D12_GetParameters _D3D12_GetParameters = nullptr;
    inline static PFN_D3D12_AllocateParameters _D3D12_AllocateParameters = nullptr;
    inline static PFN_D3D12_GetCapabilityParameters _D3D12_GetCapabilityParameters = nullptr;
    inline static PFN_D3D12_DestroyParameters _D3D12_DestroyParameters = nullptr;
    inline static PFN_D3D12_GetScratchBufferSize _D3D12_GetScratchBufferSize = nullptr;
    inline static PFN_D3D12_CreateFeature _D3D12_CreateFeature = nullptr;
    inline static PFN_D3D12_ReleaseFeature _D3D12_ReleaseFeature = nullptr;
    inline static PFN_D3D12_GetFeatureRequirements _D3D12_GetFeatureRequirements = nullptr;
    inline static PFN_D3D12_EvaluateFeature _D3D12_EvaluateFeature = nullptr;

    inline static PFN_VULKAN_RequiredExtensions _VULKAN_RequiredExtensions = nullptr;
    inline static PFN_VULKAN_Init _VULKAN_Init = nullptr;
    inline static PFN_VULKAN_Init_ProjectID _VULKAN_Init_ProjectID = nullptr;
    inline static PFN_VULKAN_Init_Ext _VULKAN_Init_Ext = nullptr;
    inline static PFN_VULKAN_Init_Ext2 _VULKAN_Init_Ext2 = nullptr;
    inline static PFN_VULKAN_Init_ProjectID_Ext _VULKAN_Init_ProjectID_Ext = nullptr;
    inline static PFN_VULKAN_Shutdown _VULKAN_Shutdown = nullptr;
    inline static PFN_VULKAN_Shutdown1 _VULKAN_Shutdown1 = nullptr;
    inline static PFN_VULKAN_GetParameters _VULKAN_GetParameters = nullptr;
    inline static PFN_VULKAN_AllocateParameters _VULKAN_AllocateParameters = nullptr;
    inline static PFN_VULKAN_GetCapabilityParameters _VULKAN_GetCapabilityParameters = nullptr;
    inline static PFN_VULKAN_DestroyParameters _VULKAN_DestroyParameters = nullptr;
    inline static PFN_VULKAN_GetScratchBufferSize _VULKAN_GetScratchBufferSize = nullptr;
    inline static PFN_VULKAN_CreateFeature _VULKAN_CreateFeature = nullptr;
    inline static PFN_VULKAN_CreateFeature1 _VULKAN_CreateFeature1 = nullptr;
    inline static PFN_VULKAN_ReleaseFeature _VULKAN_ReleaseFeature = nullptr;
    inline static PFN_VULKAN_GetFeatureRequirements _VULKAN_GetFeatureRequirements = nullptr;
    inline static PFN_VULKAN_GetFeatureInstanceExtensionRequirements _VULKAN_GetFeatureInstanceExtensionRequirements = nullptr;
    inline static PFN_VULKAN_GetFeatureDeviceExtensionRequirements _VULKAN_GetFeatureDeviceExtensionRequirements = nullptr;
    inline static PFN_VULKAN_EvaluateFeature _VULKAN_EvaluateFeature = nullptr;

    inline static PFN_UpdateFeature _UpdateFeature = nullptr;

public:
    static void InitNVNGX()
    {
        // if dll already loaded
        if (_dll != nullptr)
            return;

        LOG_FUNC();

        Config::Instance()->dlssDisableHook = true;

        do
        {
            // From DLSS Enabler
            _dll = LoadLibrary(L"dlss-enabler-ngx.dll");
            LOG_INFO("trying to load dlss-enabler-ngx.dll");

            if (_dll)
            {
                Config::Instance()->DE_Available = true;
                LOG_INFO("dlss-enabler-ngx.dll loaded from DLSS Enabler, ptr: {0:X}", (ULONG64)_dll);
                break;
            }

            // From nvngx.ini path
            if (Config::Instance()->DLSSLibrary.has_value())
            {
                LOG_INFO("trying to load nvngx from ini path!");

                std::filesystem::path cfgPath(Config::Instance()->DLSSLibrary.value().c_str());

                if (cfgPath.has_filename())
                {
                    _dll = LoadLibraryW(cfgPath.c_str());

                    if (_dll)
                    {
                        LOG_INFO("_nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(cfgPath.wstring()), (ULONG64)_dll);
                        break;
                    }
                }
                else
                {
                    auto path = cfgPath / L"_nvngx.dll";

                    LOG_INFO("trying to load _nvngx.dll path: {0}", wstring_to_string(cfgPath.wstring()));
                    _dll = LoadLibraryW(path.c_str());

                    if (_dll)
                    {
                        LOG_INFO("_nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(cfgPath.wstring()), (ULONG64)_dll);
                        break;
                    }

                    path = cfgPath / L"nvngx.dll";
                    LOG_INFO("trying to load nvngx.dll path: {0}", wstring_to_string(cfgPath.wstring()));
                    _dll = LoadLibraryW(path.c_str());

                    if (_dll)
                    {
                        LOG_INFO("nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(cfgPath.wstring()), (ULONG64)_dll);
                        break;
                    }
                }
            }

            // From registry
            auto regNGXCorePath = Util::NvngxPath();
            if (regNGXCorePath.has_value())
            {
                auto nvngxPath = regNGXCorePath.value() / L"_nvngx.dll";
                LOG_INFO("trying to load _nvngx.dll path: {0}", wstring_to_string(nvngxPath.wstring()));

                _dll = LoadLibraryW(nvngxPath.wstring().c_str());
                if (_dll)
                {
                    LOG_INFO("_nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(nvngxPath.wstring()), (ULONG64)_dll);
                    break;
                }

                nvngxPath = regNGXCorePath.value() / L"nvngx.dll";
                LOG_INFO("trying to load nvngx.dll path: {0}", wstring_to_string(nvngxPath.wstring()));

                _dll = LoadLibraryW(nvngxPath.wstring().c_str());
                if (_dll)
                {
                    LOG_INFO("nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(nvngxPath.wstring()), (ULONG64)_dll);
                    break;
                }
            }

            // From System32
            wchar_t sysFolder[MAX_PATH];
            GetSystemDirectory(sysFolder, MAX_PATH);
            std::filesystem::path sysPath(sysFolder);

            auto nvngxPath = sysPath / L"_nvngx.dll";
            LOG_INFO("trying to load _nvngx.dll path: {0}", wstring_to_string(nvngxPath.wstring()));

            _dll = LoadLibraryW(nvngxPath.wstring().c_str());
            if (_dll)
            {
                LOG_INFO("_nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(nvngxPath.wstring()), (ULONG64)_dll);
                break;
            }

            nvngxPath = sysPath / L"nvngx.dll";
            LOG_INFO("trying to load nvngx.dll path: {0}", wstring_to_string(nvngxPath.wstring()));

            _dll = LoadLibraryW(nvngxPath.wstring().c_str());
            if (_dll)
                LOG_INFO("nvngx.dll loaded from {0}, ptr: {1:X}", wstring_to_string(nvngxPath.wstring()), (ULONG64)_dll);

        } while (false);

        if (_dll != nullptr)
        {
            HookNvApi();
            HookNgxApi(_dll);

            LOG_INFO("getting nvngx method addresses");

            _D3D11_Init = (PFN_D3D11_Init)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init");
            _D3D11_Init_ProjectID = (PFN_D3D11_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init_ProjectID");
            _D3D11_Init_Ext = (PFN_D3D11_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Init_Ext");
            _D3D11_Shutdown = (PFN_D3D11_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Shutdown");
            _D3D11_Shutdown1 = (PFN_D3D11_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_D3D11_Shutdown1");
            _D3D11_GetParameters = (PFN_D3D11_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetParameters");
            _D3D11_AllocateParameters = (PFN_D3D11_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_AllocateParameters");
            _D3D11_GetCapabilityParameters = (PFN_D3D11_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetCapabilityParameters");
            _D3D11_DestroyParameters = (PFN_D3D11_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D11_DestroyParameters");
            _D3D11_GetScratchBufferSize = (PFN_D3D11_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetScratchBufferSize");
            _D3D11_CreateFeature = (PFN_D3D11_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_CreateFeature");
            _D3D11_ReleaseFeature = (PFN_D3D11_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_ReleaseFeature");
            _D3D11_GetFeatureRequirements = (PFN_D3D11_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_D3D11_GetFeatureRequirements");
            _D3D11_EvaluateFeature = (PFN_D3D11_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D11_EvaluateFeature");

            _D3D12_Init = (PFN_D3D12_Init)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init");
            _D3D12_Init_ProjectID = (PFN_D3D12_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init_ProjectID");
            _D3D12_Init_Ext = (PFN_D3D12_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Init_Ext");
            _D3D12_Shutdown = (PFN_D3D12_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown");
            _D3D12_Shutdown1 = (PFN_D3D12_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_D3D12_Shutdown1");
            _D3D12_GetParameters = (PFN_D3D12_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetParameters");
            _D3D12_AllocateParameters = (PFN_D3D12_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_AllocateParameters");
            _D3D12_GetCapabilityParameters = (PFN_D3D12_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetCapabilityParameters");
            _D3D12_DestroyParameters = (PFN_D3D12_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_D3D12_DestroyParameters");
            _D3D12_GetScratchBufferSize = (PFN_D3D12_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetScratchBufferSize");
            _D3D12_CreateFeature = (PFN_D3D12_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_CreateFeature");
            _D3D12_ReleaseFeature = (PFN_D3D12_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_ReleaseFeature");
            _D3D12_GetFeatureRequirements = (PFN_D3D12_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_D3D12_GetFeatureRequirements");
            _D3D12_EvaluateFeature = (PFN_D3D12_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_D3D12_EvaluateFeature");

            _VULKAN_RequiredExtensions = (PFN_VULKAN_RequiredExtensions)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_RequiredExtensions");
            _VULKAN_Init = (PFN_VULKAN_Init)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init");
            _VULKAN_Init_Ext = (PFN_VULKAN_Init_Ext)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext");
            _VULKAN_Init_Ext2 = (PFN_VULKAN_Init_Ext2)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_Ext2");
            _VULKAN_Init_ProjectID_Ext = (PFN_VULKAN_Init_ProjectID_Ext)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_ProjectID_Ext");
            _VULKAN_Init_ProjectID = (PFN_VULKAN_Init_ProjectID)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Init_ProjectID");
            _VULKAN_Shutdown = (PFN_VULKAN_Shutdown)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown");
            _VULKAN_Shutdown1 = (PFN_VULKAN_Shutdown1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_Shutdown1");
            _VULKAN_GetParameters = (PFN_VULKAN_GetParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetParameters");
            _VULKAN_AllocateParameters = (PFN_VULKAN_AllocateParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_AllocateParameters");
            _VULKAN_GetCapabilityParameters = (PFN_VULKAN_GetCapabilityParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetCapabilityParameters");
            _VULKAN_DestroyParameters = (PFN_VULKAN_DestroyParameters)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_DestroyParameters");
            _VULKAN_GetScratchBufferSize = (PFN_VULKAN_GetScratchBufferSize)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetScratchBufferSize");
            _VULKAN_CreateFeature = (PFN_VULKAN_CreateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature");
            _VULKAN_CreateFeature1 = (PFN_VULKAN_CreateFeature1)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_CreateFeature1");
            _VULKAN_ReleaseFeature = (PFN_VULKAN_ReleaseFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_ReleaseFeature");
            _VULKAN_GetFeatureRequirements = (PFN_VULKAN_GetFeatureRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureRequirements");
            _VULKAN_GetFeatureInstanceExtensionRequirements = (PFN_VULKAN_GetFeatureInstanceExtensionRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements");
            _VULKAN_GetFeatureDeviceExtensionRequirements = (PFN_VULKAN_GetFeatureDeviceExtensionRequirements)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements");
            _VULKAN_EvaluateFeature = (PFN_VULKAN_EvaluateFeature)GetProcAddress(_dll, "NVSDK_NGX_VULKAN_EvaluateFeature");

            _UpdateFeature = (PFN_UpdateFeature)GetProcAddress(_dll, "NVSDK_NGX_UpdateFeature");
        }

        Config::Instance()->dlssDisableHook = false;
    }

    static void GetFeatureCommonInfo(NVSDK_NGX_FeatureCommonInfo* fcInfo)
    {
        // Allocate memory for the array of const wchar_t*
        wchar_t const** paths = new const wchar_t* [Config::Instance()->NVNGX_FeatureInfo_Paths.size()];

        // Copy the strings from the vector to the array
        for (size_t i = 0; i < Config::Instance()->NVNGX_FeatureInfo_Paths.size(); ++i)
        {
            paths[i] = Config::Instance()->NVNGX_FeatureInfo_Paths[i].c_str();
            LOG_DEBUG("paths[{0}]: {1}", i, wstring_to_string(Config::Instance()->NVNGX_FeatureInfo_Paths[i]));
        }

        fcInfo->PathListInfo.Path = paths;
        fcInfo->PathListInfo.Length = static_cast<unsigned int>(Config::Instance()->NVNGX_FeatureInfo_Paths.size());
    }

    static HMODULE NVNGXModule()
    {
        return _dll;
    }

    static bool IsNVNGXInited()
    {
        return _dll != nullptr && (_dx11Inited || _dx12Inited || _vulkanInited) && Config::Instance()->DLSSEnabled.value_or(true);
    }

    // DirectX11
    static bool InitDx11(ID3D11Device* InDevice)
    {
        if (_dx11Inited)
            return true;

        InitNVNGX();

        if (_dll == nullptr)
            return false;

        NVSDK_NGX_FeatureCommonInfo fcInfo{};
        GetFeatureCommonInfo(&fcInfo);
        NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

        if (Config::Instance()->NVNGX_ProjectId != "" && _D3D11_Init_ProjectID != nullptr)
        {
            LOG_DEBUG("_D3D11_Init_ProjectID!");

            nvResult = _D3D11_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
                                             Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
        }
        else if (_D3D11_Init_Ext != nullptr)
        {
            LOG_DEBUG("_D3D11_Init_Ext!");
            nvResult = _D3D11_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
        }

        LOG_DEBUG("result: {0:X}", (UINT)nvResult);

        _dx11Inited = (nvResult == NVSDK_NGX_Result_Success);
        return _dx11Inited;
    }

    static void SetDx11Inited(bool value)
    {
        _dx11Inited = value;
    }

    static bool IsDx11Inited()
    {
        return _dx11Inited;
    }

    static PFN_D3D11_Init_ProjectID D3D11_Init_ProjectID()
    {
        return _D3D11_Init_ProjectID;
    }

    static PFN_D3D11_Init D3D11_Init()
    {
        return _D3D11_Init;
    }

    static PFN_D3D11_Init_Ext D3D11_Init_Ext()
    {
        return _D3D11_Init_Ext;
    }

    static PFN_D3D11_GetFeatureRequirements D3D11_GetFeatureRequirements()
    {
        return _D3D11_GetFeatureRequirements;
    }

    static PFN_D3D11_GetCapabilityParameters D3D11_GetCapabilityParameters()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_GetCapabilityParameters;
    }

    static PFN_D3D11_AllocateParameters D3D11_AllocateParameters()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_AllocateParameters;
    }

    static PFN_D3D11_GetParameters D3D11_GetParameters()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_GetParameters;
    }

    static PFN_D3D11_DestroyParameters D3D11_DestroyParameters()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_DestroyParameters;
    }

    static PFN_D3D11_CreateFeature D3D11_CreateFeature()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_CreateFeature;
    }

    static PFN_D3D11_EvaluateFeature D3D11_EvaluateFeature()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_EvaluateFeature;
    }

    static PFN_D3D11_ReleaseFeature D3D11_ReleaseFeature()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_ReleaseFeature;
    }

    static PFN_D3D11_Shutdown D3D11_Shutdown()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_Shutdown;
    }

    static PFN_D3D11_Shutdown1 D3D11_Shutdown1()
    {
        if (!_dx11Inited)
            return nullptr;

        return _D3D11_Shutdown1;
    }

    // DirectX12
    static bool InitDx12(ID3D12Device* InDevice)
    {
        if (_dx12Inited)
            return true;

        InitNVNGX();

        if (_dll == nullptr)
            return false;

        NVSDK_NGX_FeatureCommonInfo fcInfo{};
        GetFeatureCommonInfo(&fcInfo);
        NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

        if (Config::Instance()->NVNGX_ProjectId != "" && _D3D12_Init_ProjectID != nullptr)
        {
            LOG_INFO("_D3D12_Init_ProjectID!");

            nvResult = _D3D12_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
                                             Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
        }
        else if (_D3D12_Init_Ext != nullptr)
        {
            LOG_INFO("_D3D12_Init_Ext!");
            nvResult = _D3D12_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
        }

        LOG_INFO("result: {0:X}", (UINT)nvResult);

        _dx12Inited = (nvResult == NVSDK_NGX_Result_Success);
        return _dx12Inited;
    }

    static void SetDx12Inited(bool value)
    {
        _dx12Inited = value;
    }

    static bool IsDx12Inited()
    {
        return _dx12Inited;
    }

    static PFN_D3D12_Init_ProjectID D3D12_Init_ProjectID()
    {
        return _D3D12_Init_ProjectID;
    }

    static PFN_D3D12_Init D3D12_Init()
    {
        return _D3D12_Init;
    }

    static PFN_D3D12_Init_Ext D3D12_Init_Ext()
    {
        return _D3D12_Init_Ext;
    }

    static PFN_D3D12_GetFeatureRequirements D3D12_GetFeatureRequirements()
    {
        return _D3D12_GetFeatureRequirements;
    }

    static PFN_D3D12_GetCapabilityParameters D3D12_GetCapabilityParameters()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_GetCapabilityParameters;
    }

    static PFN_D3D12_AllocateParameters D3D12_AllocateParameters()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_AllocateParameters;
    }

    static PFN_D3D12_GetParameters D3D12_GetParameters()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_GetParameters;
    }

    static PFN_D3D12_DestroyParameters D3D12_DestroyParameters()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_DestroyParameters;
    }

    static PFN_D3D12_CreateFeature D3D12_CreateFeature()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_CreateFeature;
    }

    static PFN_D3D12_EvaluateFeature D3D12_EvaluateFeature()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_EvaluateFeature;
    }

    static PFN_D3D12_ReleaseFeature D3D12_ReleaseFeature()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_ReleaseFeature;
    }

    static PFN_D3D12_Shutdown D3D12_Shutdown()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_Shutdown;
    }

    static PFN_D3D12_Shutdown1 D3D12_Shutdown1()
    {
        if (!_dx12Inited)
            return nullptr;

        return _D3D12_Shutdown1;
    }

    // Vulkan
    static bool InitVulkan(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA)
    {
        if (_vulkanInited)
            return true;

        InitNVNGX();

        if (_dll == nullptr)
            return false;

        NVSDK_NGX_FeatureCommonInfo fcInfo{};
        GetFeatureCommonInfo(&fcInfo);
        NVSDK_NGX_Result nvResult = NVSDK_NGX_Result_Fail;

        if (Config::Instance()->NVNGX_ProjectId != "" && _VULKAN_Init_ProjectID != nullptr)
        {
            LOG_DEBUG("_VULKAN_Init_ProjectID!");
            nvResult = _VULKAN_Init_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
                                              Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InInstance, InPD, InDevice, InGIPA, InGDPA, Config::Instance()->NVNGX_Version, &fcInfo);
        }
        else if (_VULKAN_Init_Ext != nullptr)
        {
            LOG_DEBUG("_VULKAN_Init_Ext!");
            nvResult = _VULKAN_Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InInstance, InPD, InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
        }

        LOG_DEBUG("result: {0:X}", (UINT)nvResult);

        _vulkanInited = (nvResult == NVSDK_NGX_Result_Success);

        return true;
    }

    static void SetVulkanInited(bool value)
    {
        _vulkanInited = value;
    }

    static bool IsVulkanInited()
    {
        return _vulkanInited;
    }

    static PFN_VULKAN_Init_ProjectID VULKAN_Init_ProjectID()
    {
        return _VULKAN_Init_ProjectID;
    }

    static PFN_VULKAN_Init_ProjectID_Ext VULKAN_Init_ProjectID_Ext()
    {
        return _VULKAN_Init_ProjectID_Ext;
    }

    static PFN_VULKAN_Init_Ext VULKAN_Init_Ext()
    {
        return _VULKAN_Init_Ext;
    }

    static PFN_VULKAN_Init_Ext2 VULKAN_Init_Ext2()
    {
        return _VULKAN_Init_Ext2;
    }

    static PFN_VULKAN_Init VULKAN_Init()
    {
        return _VULKAN_Init;
    }

    static PFN_VULKAN_GetFeatureDeviceExtensionRequirements VULKAN_GetFeatureDeviceExtensionRequirements()
    {
        return _VULKAN_GetFeatureDeviceExtensionRequirements;
    }

    static PFN_VULKAN_GetFeatureInstanceExtensionRequirements VULKAN_GetFeatureInstanceExtensionRequirements()
    {
        return _VULKAN_GetFeatureInstanceExtensionRequirements;
    }

    static PFN_VULKAN_GetFeatureRequirements VULKAN_GetFeatureRequirements()
    {
        return _VULKAN_GetFeatureRequirements;
    }

    static PFN_VULKAN_GetCapabilityParameters VULKAN_GetCapabilityParameters()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_GetCapabilityParameters;
    }

    static PFN_VULKAN_AllocateParameters VULKAN_AllocateParameters()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_AllocateParameters;
    }

    static PFN_VULKAN_GetParameters VULKAN_GetParameters()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_GetParameters;
    }

    static PFN_VULKAN_DestroyParameters VULKAN_DestroyParameters()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_DestroyParameters;
    }

    static PFN_VULKAN_CreateFeature VULKAN_CreateFeature()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_CreateFeature;
    }

    static PFN_VULKAN_CreateFeature1 VULKAN_CreateFeature1()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_CreateFeature1;
    }

    static PFN_VULKAN_EvaluateFeature VULKAN_EvaluateFeature()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_EvaluateFeature;
    }

    static PFN_VULKAN_ReleaseFeature VULKAN_ReleaseFeature()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_ReleaseFeature;
    }

    static PFN_VULKAN_Shutdown VULKAN_Shutdown()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_Shutdown;
    }

    static PFN_VULKAN_Shutdown1 VULKAN_Shutdown1()
    {
        if (!_vulkanInited)
            return nullptr;

        return _VULKAN_Shutdown1;
    }

    static PFN_UpdateFeature UpdateFeature()
    {
        if (_dll == nullptr)
            InitNVNGX();

        return _UpdateFeature;
    }


};