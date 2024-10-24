#pragma once

#include "../pch.h"
#include "../format_transfer/FT_Dx12.h"
#include "../backends/IFeature.h"

#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "../FfxApi_Proxy.h"
#include "../NVNGX_Proxy.h"
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
    inline DXGI_FORMAT swapchainFormat = DXGI_FORMAT_UNKNOWN;

    inline int currentFrameIndex = 0;
    inline int previousFrameIndex = 0;

    void UnHookDx();
    void HookDx();
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
    inline ID3D12Resource* paramDepth[FG_BUFFER_SIZE] = { nullptr, nullptr,nullptr, nullptr };
    inline UINT64 fgHUDlessCaptureCounter[FG_BUFFER_SIZE] = { 0,0,0,0 };
    inline bool upscaleRan = false;
    inline bool fgSkipHudlessChecks = false;
    inline double fgFrameTime = 0.0;
    inline ID3D12CommandQueue* fgCommandQueue = nullptr;
    inline ID3D12CommandQueue* gameCommandQueue = nullptr;
    inline ID3D12CommandQueue* fgCopyCommandQueue = nullptr;
    inline ID3D12GraphicsCommandList* fgCopyCommandList = nullptr;
    inline ID3D12CommandAllocator* fgCopyCommandAllocators[FG_BUFFER_SIZE] = { };
    inline UINT64 fgTarget = 10;
    inline ID3D12Resource* fgUpscaledImage[FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
    inline FT_Dx12* fgFormatTransfer = nullptr;
    inline bool fgIsActive = false;

    UINT ClearFrameResources();
    UINT GetFrame();
    void NewFrame();
    void ReleaseFGObjects();
    void CreateFGObjects(ID3D12Device* InDevice);
    void CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext);
    void StopAndDestroyFGContext(bool destroy, bool shutDown);
}
