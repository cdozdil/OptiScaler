#pragma once

#include <pch.h>
#include <upscalers/IFeature.h>
#include <shaders/format_transfer/FT_Dx12.h>

#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <shared_mutex>
#include <OwnedMutex.h>

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

#include <proxies/FfxApi_Proxy.h>
#include <nvapi/fakenvapi.h>

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

    // FG
    inline const int FG_BUFFER_SIZE = 4;

    inline INT64 fgHUDlessCaptureCounter[FG_BUFFER_SIZE] = { 0, 0, 0, 0 };
    inline UINT64 fgHudlessFrameIndexes[FG_BUFFER_SIZE] = { 9999999999999, 9999999999999, 9999999999999, 9999999999999 };
    inline bool upscaleRan = false;
    inline bool fgSkipHudlessChecks = false;
    inline double fgFrameTime = 0.0;
    inline std::deque<float> fgFrameTimes;
    inline ID3D12CommandQueue* fgFSRCommandQueue = nullptr;
    inline ID3D12CommandQueue* gameCommandQueue = nullptr;

    void UnHookDx();
    void HookDx11(HMODULE dx11Module);
    void HookDx12(HMODULE dx12Module);
    void HookDxgi(HMODULE dxgiModule);
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
    inline float cameraNear = 0.0;
    inline float cameraFar = 0.0;
    inline float cameraVFov = 0.0;
    inline float meterFactor = 0.0;
    inline float ftDelta = 0.0;
    inline UINT reset = 0;

    inline ID3D12Resource* paramVelocity[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12Resource* paramDepth[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12Resource* paramVelocityCopy[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12Resource* paramDepthCopy[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };

    inline ID3D12CommandQueue* fgCommandQueue = nullptr;
    inline ID3D12GraphicsCommandList* fgCommandList[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12CommandAllocator* fgCommandAllocators[HooksDx::FG_BUFFER_SIZE] = { };

    inline ID3D12CommandQueue* fgCopyCommandQueue = nullptr;
    inline ID3D12GraphicsCommandList* fgCopyCommandList[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12CommandAllocator* fgCopyCommandAllocator[HooksDx::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline ID3D12Fence* fgCopyFence = nullptr;

    inline UINT64 fgTarget = 10;
    inline FT_Dx12* fgFormatTransfer = nullptr;
    inline bool fgIsActive = false;

    // Owners
    // 1: Framegen
    // 2: Swapchain Present <---
    // 3: Wrapped swapchain
    inline OwnedMutex ffxMutex;

    UINT NewFrame();
    UINT GetFrame();
    void ResetIndexes();
    void ReleaseFGSwapchain(HWND hWnd);
    void ReleaseFGObjects();
    void CreateFGObjects(ID3D12Device* InDevice);
    void CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext);
    void StopAndDestroyFGContext(bool destroy, bool shutDown, bool useMutex = true);
    void CheckUpscaledFrame(ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InUpscaled);
    void AddFrameTime(float ft);
    float GetFrameTime();
    void ConfigureFramePaceTuning();
}
