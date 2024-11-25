#pragma once

#include "../pch.h"
#include "../format_transfer/FT_Dx12.h"
#include "../backends/IFeature.h"

#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <shared_mutex>

// According to https://gpuopen.com/manuals/fidelityfx_sdk/fidelityfx_sdk-page_techniques_super-resolution-interpolation/#id11 
// Will use mutex to prevent race condutions
#define USE_MUTEX_FOR_FFX

#ifdef USE_MUTEX_FOR_FFX
#define USE_PRESENT_FOR_FT
#endif

// Enable D3D12 Debug Layers
//#define ENABLE_DEBUG_LAYER_DX12

// Enable D3D11 Debug Layers
//#define ENABLE_DEBUG_LAYER_DX11

#ifdef ENABLE_DEBUG_LAYER_DX12
// Enable GPUValidation
//#define ENABLE_GPU_VALIDATION

#include <d3d12sdklayers.h>
#endif

#include "../FfxApi_Proxy.h"
#include "../nvapi/fakenvapi.h"
#include "../nvapi/ReflexHooks.h"
#include <dx12/ffx_api_dx12.h>
#include <ffx_framegeneration.h>


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

    inline ID3D12CommandQueue* GameCommandQueue = nullptr;
    inline IDXGISwapChain* currentSwapchain = nullptr;

    inline int currentFrameIndex = 0;
    inline int previousFrameIndex = 0;

    void UnHookDx();
    void HookDx11();
    void HookDx12();
    void HookDxgi();
    DXGI_FORMAT CurrentSwapchainFormat();
}

namespace FrameGen_Dx12
{
    inline ffxContext fgSwapChainContext = nullptr;
    inline ffxContext fgContext = nullptr;
    inline float jitterX = 0.0;
    inline float jitterY = 0.0;
    inline float mvScaleX = 0.0;
    inline float mvScaleY = 0.0;
    inline const int FG_BUFFER_SIZE = 4;
    inline ID3D12Resource* paramVelocity[FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12Resource* paramDepth[FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline INT64 fgHUDlessCaptureCounter[FG_BUFFER_SIZE] = { 0,0,0,0 };
    inline bool upscaleRan = false;
    inline bool fgSkipHudlessChecks = false;
    inline double fgFrameTime = 0.0;
    inline ID3D12CommandQueue* fgFSRCommandQueue = nullptr;
    inline ID3D12CommandQueue* gameCommandQueue = nullptr;

    inline ID3D12CommandQueue* fgCommandQueue = nullptr;
    inline ID3D12GraphicsCommandList* fgCommandList[FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12CommandAllocator* fgCommandAllocators[FG_BUFFER_SIZE] = { };

    inline ID3D12CommandQueue* fgCopyCommandQueue = nullptr;
    inline ID3D12GraphicsCommandList* fgCopyCommandList = nullptr;
    inline ID3D12CommandAllocator* fgCopyCommandAllocator = nullptr;

    inline UINT64 fgTarget = 10;
    inline FT_Dx12* fgFormatTransfer = nullptr;
    inline bool fgIsActive = false;

#ifdef USE_MUTEX_FOR_FFX
    inline std::shared_mutex ffxMutex;
#endif

    UINT NewFrame();
    UINT GetFrame();
    void ReleaseFGSwapchain(HWND hWnd);
    void ReleaseFGObjects();
    void CreateFGObjects(ID3D12Device* InDevice);
    void CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext);
    void StopAndDestroyFGContext(bool destroy, bool shutDown, bool useMutex = true);
    void CheckUpscaledFrame(ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InUpscaled);
}
