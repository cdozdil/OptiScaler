#include "PAG_Dx12.h"
#include "../Config.h"

inline static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT InFormat)
{
    switch (InFormat)
    {
        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT;

        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_FLOAT;

        case DXGI_FORMAT_R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_FLOAT;

        default:
            break;
    }

    return InFormat;
}

inline static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT InFormat)
{
    switch (InFormat)
    {
        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT;

        case DXGI_FORMAT_R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_FLOAT;

        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_FLOAT;

        case DXGI_FORMAT_R8G8_TYPELESS:
            return DXGI_FORMAT_R8G8_UINT;

        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_UINT;


        default:
            break;
    }

    return InFormat;
}

inline static ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ID3D12DescriptorHeap* descriptorHeap;
    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
    return descriptorHeap;
}

inline static bool CreateReferencedResource(ID3D12Device* InDevice, ID3D12Resource* InResource, const wchar_t* InName, ID3D12Resource** OutResource, D3D12_RESOURCE_STATES InState,
                                            DXGI_FORMAT InFormat = DXGI_FORMAT_UNKNOWN, UINT64 InWidth = 0, UINT InHeight = 0,
                                            D3D12_RESOURCE_FLAGS InFlags = D3D12_RESOURCE_FLAG_NONE)
{
    if (InDevice == nullptr || InResource == nullptr)
        return false;

    D3D12_RESOURCE_DESC texDesc = InResource->GetDesc();

    if (InFormat != DXGI_FORMAT_UNKNOWN)
        texDesc.Format = InFormat;

    //texDesc.Format = GetTypelessFormat(texDesc.Format);

    if (InWidth != 0 && InHeight != 0)
    {
        texDesc.Width = InWidth;
        texDesc.Height = InHeight;
    }

    if (*OutResource != nullptr)
    {
        auto bufDesc = (*OutResource)->GetDesc();

        if (bufDesc.Width != (UINT64)(texDesc.Width) || bufDesc.Height != (UINT)(texDesc.Height) || bufDesc.Format != texDesc.Format)
        {
            (*OutResource)->Release();
            *OutResource = nullptr;
        }
        else
            return true;
    }

    spdlog::debug("PAG_Dx12::CreateReferencedResource Start!");

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = InResource->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        spdlog::error("PAG_Dx12::CreateReferencedResource GetHeapProperties result: {0:x}", hr);
        return false;
    }

    texDesc.Flags = InFlags;

    hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(OutResource));

    if (hr != S_OK)
    {
        spdlog::error("PAG_Dx12::CreateReferencedResource CreateCommittedResource result: {0:x}", hr);
        return false;
    }

    (*OutResource)->SetName(InName);

    return true;
}

inline static void SetResourceState(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InCurrentState, D3D12_RESOURCE_STATES InNewState)
{
    if (InCurrentState == InNewState)
        return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = InResource;
    barrier.Transition.StateBefore = InCurrentState;
    barrier.Transition.StateAfter = InNewState;
    barrier.Transition.Subresource = 0;
    InCommandList->ResourceBarrier(1, &barrier);
}

bool PAG_Dx12::Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList,
                        ID3D12Resource* InColor, ID3D12Resource* InDepth, ID3D12Resource* InMotionVectors,
                        DirectX::XMFLOAT2 InJitter)
{
    if (!_init || InDevice == nullptr || InCmdList == nullptr || InColor == nullptr || InDepth == nullptr ||
        InMotionVectors == nullptr || InMotionVectors == nullptr)
        return false;

    spdlog::debug("PAG_Dx12::Dispatch [{0}] Start!", _name);

    _counter++;
    _counter = _counter % 2;

    auto inColorDesc = InColor->GetDesc();
    auto inDepthDesc = InDepth->GetDesc();
    auto inMvDesc = InMotionVectors->GetDesc();

    // Create Depth History
    if (_historyDepth == nullptr)
        CreateReferencedResource(InDevice, InDepth, L"PAG_HistoryDepth", &_historyDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    // Create Output Motion Vectors
    if (_motionVector == nullptr)
        CreateReferencedResource(InDevice, InDepth, L"PAG_MotionVector", &_motionVector, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                 DXGI_FORMAT_R16G16_FLOAT, inColorDesc.Width, inColorDesc.Height, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    else
        SetResourceState(InCmdList, _motionVector, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // Create Output Rective Mask
    if (_reactiveMask == nullptr)
        CreateReferencedResource(InDevice, InColor, L"PAG_ReactiveMask", &_reactiveMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                 DXGI_FORMAT_R8_UNORM, 0, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    else
        SetResourceState(InCmdList, _reactiveMask, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // Create Debug Output
    if (_debug == nullptr)
        CreateReferencedResource(InDevice, InColor, L"PAG_Debug", &_debug, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                 DXGI_FORMAT_UNKNOWN, 0, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto _histDepthDesc = _historyDepth->GetDesc();
    auto _rMaskhDesc = _reactiveMask->GetDesc();
    auto _mvhDesc = _motionVector->GetDesc();
    auto _debugDesc = _debug->GetDesc();

    // CBV
    if (_cpuCbvHandle[_counter].ptr == NULL)
    {
        // CPU
        _cpuCbvHandle[_counter] = _srvHeap[_counter]->GetCPUDescriptorHandleForHeapStart();

        // GPU
        _gpuCbvHandle[_counter] = _srvHeap[_counter]->GetGPUDescriptorHandleForHeapStart();
    }


    // SRV
    for (size_t i = 0; i < 6; i++)
    {
        // CPU
        if (_cpuSrvHandle[i][_counter].ptr == NULL)
        {
            if (i == 0)
                _cpuSrvHandle[i][_counter] = _cpuCbvHandle[_counter];
            else
                _cpuSrvHandle[i][_counter] = _cpuSrvHandle[i - 1][_counter];

            _cpuSrvHandle[i][_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        // GPU
        if (_gpuSrvHandle[i][_counter].ptr == NULL)
        {
            if (i == 0)
                _gpuSrvHandle[i][_counter] = _gpuCbvHandle[_counter];
            else
                _gpuSrvHandle[i][_counter] = _gpuSrvHandle[i - 1][_counter];

            _gpuSrvHandle[i][_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
    }

    // UAV
    for (size_t i = 0; i < 3; i++)
    {
        // CPU
        if (_cpuUavHandle[i][_counter].ptr == NULL)
        {
            if (i == 0)
                _cpuUavHandle[i][_counter] = _cpuSrvHandle[5][_counter];
            else
                _cpuUavHandle[i][_counter] = _cpuUavHandle[i - 1][_counter];

            _cpuUavHandle[i][_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        // GPU
        if (_gpuUavHandle[i][_counter].ptr == NULL)
        {
            if (i == 0)
                _gpuUavHandle[i][_counter] = _gpuSrvHandle[5][_counter];
            else
                _gpuUavHandle[i][_counter] = _gpuUavHandle[i - 1][_counter];

            _gpuUavHandle[i][_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
    }

    // RVs for SRVs
    ID3D12Resource* srvResources[6] = { InColor, InDepth, InDepth, InMotionVectors, nullptr, _historyDepth };
    DXGI_FORMAT srvResourceFmts[6] = { inColorDesc.Format, GetDepthFormat(inDepthDesc.Format), GetStencilFormat(inDepthDesc.Format),
        inMvDesc.Format, DXGI_FORMAT_R16_FLOAT, GetStencilFormat(_histDepthDesc.Format) };

    for (size_t i = 0; i < 6; i++)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = srvResourceFmts[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        // Stencil ones
        if ((i == 2 || i == 5) && (srvResourceFmts[i] == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT || srvResourceFmts[i] == DXGI_FORMAT_D24_UNORM_S8_UINT))
            srvDesc.Texture2D.PlaneSlice = 1;

        InDevice->CreateShaderResourceView(srvResources[i], &srvDesc, _cpuSrvHandle[i][_counter]);
    }

    // RVs for UAVs
    ID3D12Resource* uavResources[3] = { _reactiveMask, _motionVector, _debug };
    D3D12_RESOURCE_DESC* uavResourceDescs[3] = { &_rMaskhDesc, &_mvhDesc, &_debugDesc };

    for (size_t i = 0; i < 3; i++)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = uavResourceDescs[i]->Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;

        InDevice->CreateUnorderedAccessView(uavResources[i], nullptr, &uavDesc, _cpuUavHandle[i][_counter]);
    }

    // CBV's for CB's
    InternalJitterBuff constants{};
    constants.Scale = { 1, 1, 0, INFINITY };
    constants.MoreData = { 0, 0, 0, 0 };
    constants.Jitter = { InJitter.x, InJitter.y };
    constants.VelocityScaler = 0;
    constants.JitterMotion = 0;

    // Copy the updated constant buffer data to the constant buffer resource
    BYTE* pCBDataBegin;
    CD3DX12_RANGE readRange(0, 0);  // We do not intend to read from this resource on the CPU
    auto result = _constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCBDataBegin));

    if (result != S_OK)
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] _constantBuffer->Map error {1:x}", _name, (unsigned int)result);
        return false;
    }

    if (pCBDataBegin == nullptr)
    {
        _constantBuffer->Unmap(0, nullptr);
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] pCBDataBegin is null!", _name);
        return false;
    }

    memcpy(pCBDataBegin, &constants, sizeof(constants));
    _constantBuffer->Unmap(0, nullptr);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = _constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = sizeof(constants);
    InDevice->CreateConstantBufferView(&cbvDesc, _cpuCbvHandle[_counter]);

    ID3D12DescriptorHeap* heaps[] = { _srvHeap[_counter] };
    InCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // Signature & PSO
    InCmdList->SetComputeRootSignature(_rootSignature);
    InCmdList->SetPipelineState(_pipelineState);

    //// GPU CBV Handles
    InCmdList->SetComputeRootDescriptorTable(0, _gpuCbvHandle[_counter]);

    // GPU SRV Handles
    InCmdList->SetComputeRootDescriptorTable(1, _gpuSrvHandle[0][_counter]);

    // GPU UAV Handles
    InCmdList->SetComputeRootDescriptorTable(2, _gpuUavHandle[0][_counter]);


    UINT dispatchWidth = 0;
    UINT dispatchHeight = 0;

    dispatchWidth = (inColorDesc.Width + InNumThreadsX - 1) / InNumThreadsX;
    dispatchHeight = (inColorDesc.Height + InNumThreadsY - 1) / InNumThreadsY;

    InCmdList->Dispatch(dispatchWidth, dispatchHeight, 1);

    InDepth->SetName(L"DLSS_DepthInput");
    SetResourceState(InCmdList, InDepth, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
    SetResourceState(InCmdList, _historyDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    SetResourceState(InCmdList, _debug, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    InCmdList->CopyResource(_historyDepth, InDepth);

    SetResourceState(InCmdList, _historyDepth, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    SetResourceState(InCmdList, InDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    SetResourceState(InCmdList, _motionVector, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    SetResourceState(InCmdList, _reactiveMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    return true;
}

PAG_Dx12::PAG_Dx12(std::string InName, ID3D12Device* InDevice) : _name(InName), _device(InDevice)
{
    if (InDevice == nullptr)
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 InDevice is nullptr!");
        return;
    }

    spdlog::debug("PAG_Dx12::PAG_Dx12 {0} start!", _name);

    // Prepare CBV Resource
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(InternalJitterBuff));
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    auto result = InDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_constantBuffer));

    if (result != S_OK)
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] CreateCommittedResource error {1:x}", _name, (unsigned int)result);
        return;
    }

    // Create the root signature
// Create the root signature
    CD3DX12_ROOT_PARAMETER rootParameters[3];

    // CBV
    rootParameters[0].InitAsConstantBufferView(0);

    // SRV descriptor table
    CD3DX12_DESCRIPTOR_RANGE srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0);
    rootParameters[1].InitAsDescriptorTable(1, &srvRange);

    // UAV descriptor table
    CD3DX12_DESCRIPTOR_RANGE uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);
    rootParameters[2].InitAsDescriptorTable(1, &uavRange);

    // Samplers
    CD3DX12_STATIC_SAMPLER_DESC samplers[2];
    samplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderRegister = 0;
    samplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[1].ShaderRegister = 1;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, _countof(samplers), samplers, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* signatureBlob = nullptr;

    do
    {
        auto hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr))
        {
            spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] D3D12SerializeRootSignature error {1:x}: {2}", _name, (unsigned int)hr, static_cast<const char*>(errorBlob->GetBufferPointer()));
            break;
        }

        hr = InDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));

        if (FAILED(hr))
        {
            spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] CreateRootSignature error {1:x}", _name, (unsigned int)hr);
            break;
        }

    } while (false);

    if (errorBlob != nullptr)
    {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    if (signatureBlob != nullptr)
    {
        signatureBlob->Release();
        signatureBlob = nullptr;
    }

    if (_rootSignature == nullptr)
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] _rootSignature is null!", _name);
        return;
    }

    // Create the compute pipeline state object (PSO)
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = _rootSignature;
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(reinterpret_cast<const void*>(pag_shader), pag_shader_size);
    auto hr = InDevice->CreateComputePipelineState(&computePsoDesc, __uuidof(ID3D12PipelineState*), (void**)&_pipelineState);

    if (FAILED(hr))
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] CreateComputePipelineState error: {1:X}", _name, hr);
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 10;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[0]));

    if (FAILED(hr))
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] CreateDescriptorHeap[0] error {1:x}", _name, (unsigned int)hr);
        return;
    }

    hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[1]));

    if (FAILED(hr))
    {
        spdlog::error("PAG_Dx12::PAG_Dx12 [{0}] CreateDescriptorHeap[1] error {1:x}", _name, (unsigned int)hr);
        return;
    }

    _init = _srvHeap[1] != nullptr;
}

PAG_Dx12::~PAG_Dx12()
{
    if (!_init)
        return;

    ID3D12Fence* d3d12Fence = nullptr;

    do
    {
        if (_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
            break;

        d3d12Fence->Signal(999);

        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
        {
            WaitForSingleObject(fenceEvent, INFINITE);
            CloseHandle(fenceEvent);
        }

    } while (false);

    if (d3d12Fence != nullptr)
    {
        d3d12Fence->Release();
        d3d12Fence = nullptr;
    }

    if (_rootSignature != nullptr)
    {
        _rootSignature->Release();
        _rootSignature = nullptr;
    }

    if (_pipelineState != nullptr)
    {
        _pipelineState->Release();
        _pipelineState = nullptr;
    }

    if (_srvHeap[0] != nullptr)
    {
        _srvHeap[0]->Release();
        _srvHeap[0] = nullptr;
    }

    if (_srvHeap[1] != nullptr)
    {
        _srvHeap[1]->Release();
        _srvHeap[1] = nullptr;
    }

    if (_debug != nullptr)
    {
        _debug->Release();
        _debug = nullptr;
    }

    if (_reactiveMask != nullptr)
    {
        _reactiveMask->Release();
        _reactiveMask = nullptr;
    }

    if (_historyDepth != nullptr)
    {
        _historyDepth->Release();
        _historyDepth = nullptr;
    }

    if (_motionVector != nullptr)
    {
        _motionVector->Release();
        _motionVector = nullptr;
    }

    if (_constantBuffer != nullptr)
    {
        _constantBuffer->Release();
        _constantBuffer = nullptr;
    }
}
