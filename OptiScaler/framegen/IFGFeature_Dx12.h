#pragma once
#include <pch.h>

#include "IFGFeature.h"

#include <d3d12.h>

typedef struct SwapChainInfo
{
    IDXGISwapChain* swapChain = nullptr;
    DXGI_FORMAT swapChainFormat = DXGI_FORMAT_UNKNOWN;
    int swapChainBufferCount = 0;
    ID3D12CommandQueue* fgCommandQueue = nullptr;
    ID3D12CommandQueue* gameCommandQueue = nullptr;
};

class IFGFeature_Dx12 : public virtual IFGFeature
{
protected:
    ID3D12Resource* _paramVelocity[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Resource* _paramVelocityCopy[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Resource* _paramDepth[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Resource* _paramDepthCopy[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Resource* _paramHudless[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Resource* _paramHudlessCopy[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };

    ID3D12CommandQueue* _commandQueue = nullptr;
    ID3D12GraphicsCommandList* _commandList[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12CommandAllocator* _commandAllocators[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };

    ID3D12CommandQueue* _copyCommandQueue = nullptr;
    ID3D12GraphicsCommandList* _copyCommandList[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12CommandAllocator* _copyCommandAllocator[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    ID3D12Fence* _copyFence = nullptr;

    bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource);
    void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState);
    bool CopyResource(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* target, D3D12_RESOURCE_STATES sourceState);

public:

    void SetVelocity(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* velocity, D3D12_RESOURCE_STATES state);
    void SetDepth(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* depth, D3D12_RESOURCE_STATES state);
    void SetHudless(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* hudless, D3D12_RESOURCE_STATES state, bool makeCopy = false);

};