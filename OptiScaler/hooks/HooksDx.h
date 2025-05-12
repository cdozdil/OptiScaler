#pragma once

#include <pch.h>

#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_6.h>

// Use a dedicated Queue + CommandList for copying Depth + Velocity
// Looks like it is causing issues so disabled 
//#define USE_COPY_QUEUE_FOR_FG

// Enable D3D12 Debug Layers
//#define ENABLE_DEBUG_LAYER_DX12

// Enable D3D11 Debug Layers
//#define ENABLE_DEBUG_LAYER_DX11

#ifdef ENABLE_DEBUG_LAYER_DX12
// Enable GPUValidation
//#define ENABLE_GPU_VALIDATION

#include <d3d12sdklayers.h>
#endif

namespace HooksDx
{
    inline ID3D12QueryHeap* queryHeap = nullptr;
    inline ID3D12Resource* readbackBuffer = nullptr;
    inline bool dx12UpscaleTrig = false;

    inline const int QUERY_BUFFER_COUNT = 3;
    inline ID3D11Query* disjointQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
    inline ID3D11Query* startQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
    inline ID3D11Query* endQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
    inline bool dx11UpscaleTrig[QUERY_BUFFER_COUNT] = { false, false, false };

    HANDLE Dx12FenceEvent = nullptr;

    inline int currentFrameIndex = 0;
    inline int previousFrameIndex = 0;

    void UnHookDx();
    void HookDx11(HMODULE dx11Module);
    void HookDx12();
    void HookDxgi();
    void ReleaseDx12SwapChain(HWND hwnd);

    DXGI_FORMAT CurrentSwapchainFormat();
}
