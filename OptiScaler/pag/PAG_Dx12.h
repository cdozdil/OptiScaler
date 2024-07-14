#pragma once

#include "../pch.h"
#include "PAG_Common.h"

#include <d3d12.h>
#include "../d3dx/d3dx12.h"

class PAG_Dx12
{
private:
    struct alignas(256) InternalJitterBuff
    {
        DirectX::XMFLOAT4 Scale;        // 16 bytes
        DirectX::XMFLOAT4 MoreData;     // 16 bytes
        DirectX::XMFLOAT2 Jitter;       //  8 bytes
        float VelocityScaler;           //  4 bytes
        int JitterMotion;               //  4 bytes
    };

    std::string _name = "";
    bool _init = false;
    int _counter = 0;

    ID3D12RootSignature* _rootSignature = nullptr;
    ID3D12PipelineState* _pipelineState = nullptr;
    ID3D12DescriptorHeap* _srvHeap[2] = { nullptr, nullptr };

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuSrvHandle[6][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuUavHandle[3][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuCbvHandle[2] = { NULL, NULL };

    D3D12_GPU_DESCRIPTOR_HANDLE _gpuSrvHandle[6][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuUavHandle[3][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuCbvHandle[2] = { NULL, NULL };

    ID3D12Device* _device = nullptr;
    
    ID3D12Resource* _historyDepth = nullptr;
    ID3D12Resource* _reactiveMask = nullptr;
    ID3D12Resource* _motionVector = nullptr;
    ID3D12Resource* _debug = nullptr;

    ID3D12Resource* _constantBuffer = nullptr;

    UINT InNumThreadsX = 16;
    UINT InNumThreadsY = 16;

public:
    bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList,
                  ID3D12Resource* InColor, ID3D12Resource* InDepth, ID3D12Resource* InMotionVectors,
                  DirectX::XMFLOAT2 InJitter);

    ID3D12Resource* ReactiveMask() { return _reactiveMask; }
    ID3D12Resource* MotionVector() { return _motionVector; }
    bool IsInit() const { return _init; }
    bool CanRender() const { return _init; }

    PAG_Dx12(std::string InName, ID3D12Device* InDevice);

    ~PAG_Dx12();
};