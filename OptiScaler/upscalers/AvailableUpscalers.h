#pragma once

#include "proxies/XeSS_Proxy.h"
#include "proxies/NVNGX_Proxy.h"
#include "proxies/FfxApi_Proxy.h"

inline AvailableUpscalers getAvailableUpscalers() {
    LOG_FUNC();

    AvailableUpscalers upscalers{};

    // statically linked upscalers are always available
    upscalers.Set(Upscaler::FSR_21);
    upscalers.Set(Upscaler::FSR_22);
    upscalers.Set(Upscaler::FFX_SDK_DX11);

    if (NVNGXProxy::NVNGXModule() != nullptr)
        upscalers.Set(Upscaler::DLSS);

    XeSSProxy::InitXeSS();

    if (GetModuleHandle(L"libxess.dll") != nullptr)
        upscalers.Set(Upscaler::XESS);

    FfxApiProxy::InitFfxDx12();
    FfxApiProxy::InitFfxVk();

    if (GetModuleHandle(L"amd_fidelityfx_dx12.dll") != nullptr)
        upscalers.Set(Upscaler::FFX_SDK_DX12);

    if (GetModuleHandle(L"amd_fidelityfx_vk.dll") != nullptr)
        upscalers.Set(Upscaler::FFX_SDK_VK);

    return upscalers;
}