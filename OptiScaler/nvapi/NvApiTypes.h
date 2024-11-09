#pragma once

#include <pch.h>

#include <dxgi.h>
#include <d3d12.h>
#include <include/nvapi/nvapi.h>

// Separate to break up a circular dependency 
class NvApiTypes {
public:
    enum class NV_INTERFACE : uint32_t
    {
        GPU_GetArchInfo = 0xD8265D24,
        D3D12_SetRawScgPriority = 0x5DB3048A,
        D3D_SetSleepMode = 0xac1ca9e0,
        D3D_Sleep = 0x852cd1d2,
        D3D_GetLatency = 0x1a587f9c,
        D3D_SetLatencyMarker = 0xd9984c05,
        D3D12_SetAsyncFrameMarker = 0x13c98f73,
    };

    typedef void* (__stdcall* PFN_NvApi_QueryInterface)(NV_INTERFACE InterfaceId);

    using PfnNvAPI_GPU_GetArchInfo = uint32_t(__stdcall*)(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo);
};
