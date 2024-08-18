#include "Nvngx_Hooks.h"

#include "../pch.h"
#include "../detours/detours.h"

#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>
#include <vulkan/vulkan.hpp>


typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D1X_GetFeatureRequirements)(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements)(const VkInstance Instance, const VkPhysicalDevice PhysicalDevice, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
                                                                       NVSDK_NGX_FeatureRequirement* OutSupported);

static PFN_NVSDK_NGX_D3D1X_GetFeatureRequirements Original_D3D11_GetFeatureRequirements = nullptr;
static PFN_NVSDK_NGX_D3D1X_GetFeatureRequirements Original_D3D12_GetFeatureRequirements = nullptr;
static PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements Original_Vulkan_GetFeatureRequirements = nullptr;

inline static NVSDK_NGX_Result __stdcall Hooked_Dx12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_FUNC();

    auto result = Original_D3D12_GetFeatureRequirements(Adapter, FeatureDiscoveryInfo, OutSupported);

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

    auto result = Original_D3D11_GetFeatureRequirements(Adapter, FeatureDiscoveryInfo, OutSupported);

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


void Hooks::AttachNVNGXSpoofingHooks()
{
    if (Original_D3D11_GetFeatureRequirements != nullptr || Original_D3D12_GetFeatureRequirements != nullptr || Original_Vulkan_GetFeatureRequirements != nullptr)
        return;

    LOG_DEBUG("Trying to hook NgxApi");

    Original_D3D11_GetFeatureRequirements = reinterpret_cast<PFN_NVSDK_NGX_D3D1X_GetFeatureRequirements>(GetProcAddress(nvngxModule, "NVSDK_NGX_D3D11_GetFeatureRequirements"));
    Original_D3D12_GetFeatureRequirements = reinterpret_cast<PFN_NVSDK_NGX_D3D1X_GetFeatureRequirements>(GetProcAddress(nvngxModule, "NVSDK_NGX_D3D12_GetFeatureRequirements"));
    Original_Vulkan_GetFeatureRequirements = reinterpret_cast<PFN_NVSDK_NGX_VULKAN_GetFeatureRequirements>(GetProcAddress(nvngxModule, "NVSDK_NGX_VULKAN_GetFeatureRequirements"));

    LOG_INFO("NVSDK_NGX_XXXXXX_GetFeatureRequirements found, hooking!");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (Original_D3D11_GetFeatureRequirements != nullptr)
        DetourAttach(&(PVOID&)Original_D3D11_GetFeatureRequirements, Hooked_Dx11_GetFeatureRequirements);

    if (Original_D3D12_GetFeatureRequirements != nullptr)
        DetourAttach(&(PVOID&)Original_D3D12_GetFeatureRequirements, Hooked_Dx12_GetFeatureRequirements);

    if (Original_Vulkan_GetFeatureRequirements != nullptr)
        DetourAttach(&(PVOID&)Original_Vulkan_GetFeatureRequirements, Hooked_Vulkan_GetFeatureRequirements);

    DetourTransactionCommit();
}

void Hooks::DetachNVNGXSpoofingHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (Original_D3D11_GetFeatureRequirements != nullptr)
    {
        DetourDetach(&(PVOID&)Original_D3D11_GetFeatureRequirements, Hooked_Dx11_GetFeatureRequirements);
        Original_D3D11_GetFeatureRequirements = nullptr;
    }

    if (Original_D3D12_GetFeatureRequirements != nullptr)
    {
        DetourDetach(&(PVOID&)Original_D3D12_GetFeatureRequirements, Hooked_Dx12_GetFeatureRequirements);
        Original_D3D12_GetFeatureRequirements = nullptr;
    }

    if (Original_Vulkan_GetFeatureRequirements != nullptr)
    {
        DetourDetach(&(PVOID&)Original_Vulkan_GetFeatureRequirements, Hooked_Vulkan_GetFeatureRequirements);
        Original_Vulkan_GetFeatureRequirements = nullptr;
    }

    DetourTransactionCommit();
}
