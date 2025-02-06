#include "IFGFeature_Dx12.h"

bool IFGFeature_Dx12::CreateBufferResource(ID3D12Device* device, ID3D12Resource* source, D3D12_RESOURCE_STATES state, ID3D12Resource** target)
{
    if (device == nullptr || source == nullptr)
        return false;

    auto inDesc = source->GetDesc();

    if (*target != nullptr)
    {
        auto bufDesc = (*target)->GetDesc();

        if (bufDesc.Width != inDesc.Width || bufDesc.Height != inDesc.Height || bufDesc.Format != inDesc.Format)
        {
            (*target)->Release();
            (*target) = nullptr;
        }
        else
            return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = source->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        LOG_ERROR("GetHeapProperties result: {:X}", (UINT64)hr);
        return false;
    }

    hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &inDesc, state, nullptr, IID_PPV_ARGS(target));

    if (hr != S_OK)
    {
        LOG_ERROR("CreateCommittedResource result: {:X}", (UINT64)hr);
        return false;
    }

    return true;
}

void IFGFeature_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = beforeState;
    barrier.Transition.StateAfter = afterState;
    barrier.Transition.Subresource = 0;
    cmdList->ResourceBarrier(1, &barrier);
}

bool IFGFeature_Dx12::CopyResource(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource* target, D3D12_RESOURCE_STATES sourceState)
{
    auto result = true;

    ResourceBarrier(cmdList, source, sourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);

    if (CreateBufferResource(State::Instance().currentD3D12Device, source, D3D12_RESOURCE_STATE_COPY_DEST, &target))
        cmdList->CopyResource(target, source);
    else
        result = false;

    ResourceBarrier(cmdList, source, D3D12_RESOURCE_STATE_COPY_SOURCE, sourceState);
}

void IFGFeature_Dx12::SetVelocity(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* velocity, D3D12_RESOURCE_STATES state)
{
    if (cmdList == nullptr)
    {
        _paramVelocity[GetIndex()] = velocity;
        return;
    }

    auto index = GetIndex();
    if (Config::Instance()->FGMakeMVCopy.value_or_default() && CopyResource(cmdList, velocity, _paramVelocityCopy[index], state))
        _paramVelocity[index] = _paramVelocityCopy[index];
    else
        _paramVelocity[GetIndex()] = velocity;
}

void IFGFeature_Dx12::SetDepth(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* depth, D3D12_RESOURCE_STATES state)
{
    if (cmdList == nullptr)
    {
        _paramDepth[GetIndex()] = depth;
        return;
    }

    auto index = GetIndex();
    if (Config::Instance()->FGMakeDepthCopy.value_or_default() && CopyResource(cmdList, depth, _paramDepthCopy[index], state))
        _paramDepth[index] = _paramDepthCopy[index];
    else
        _paramDepth[GetIndex()] = depth;
}

void IFGFeature_Dx12::SetHudless(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* hudless, D3D12_RESOURCE_STATES state, bool makeCopy = false)
{
    if (cmdList == nullptr)
    {
        _paramHudless[GetIndex()] = hudless;
        return;
    }

    auto index = GetIndex();
    if (makeCopy && CopyResource(cmdList, hudless, _paramHudlessCopy[index], state))
        _paramHudless[index] = _paramHudlessCopy[index];
    else
        _paramHudless[GetIndex()] = hudless;
}

void IFGFeature_Dx12::CreateObjects(ID3D12Device* InDevice)
{
    if (_commandQueue != nullptr)
        return;

    do
    {
        HRESULT result;

        for (size_t i = 0; i < 4; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocators[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocators _commandAllocators[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            _commandAllocators[i]->SetName(L"_commandAllocator");


            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocators[i], NULL, IID_PPV_ARGS(&_commandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList _commandList[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            _commandList[i]->SetName(L"_commandList");

            result = _commandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("_commandList[{}]->Close: {:X}", i, (unsigned long)result);
                break;
            }
        }

        for (size_t i = 0; i < 4; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_copyCommandAllocator[i]));

            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _copyCommandAllocator[i], NULL, IID_PPV_ARGS(&_copyCommandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList _copyCommandList[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            _copyCommandList[i]->SetName(L"_copyCommandList");

            result = _copyCommandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("_copyCommandList[{}]->Close: {:X}", i, (unsigned long)result);
                break;
            }
        }

        // Create a command queue for frame generation
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        if (Config::Instance()->FGHighPriority.value_or_default())
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        else
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

        HRESULT hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue _commandQueue: {0:X}", (unsigned long)result);
            break;
        }
        _commandQueue->SetName(L"_commandQueue");

        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_copyCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue _copyCommandQueue: {0:X}", (unsigned long)result);
            break;
        }
        _copyCommandQueue->SetName(L"_copyCommandQueue");

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_copyFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence fgCopyFence: {0:X}", (unsigned long)result);
            break;
        }

    } while (false);
}

void IFGFeature_Dx12::ReleaseObjects()
{
    for (size_t i = 0; i < 4; i++)
    {
        if (_commandAllocators[i] != nullptr)
        {
            _commandAllocators[i]->Release();
            _commandAllocators[i] = nullptr;
        }

        if (_commandList[i] != nullptr)
        {
            _commandList[i]->Release();
            _commandList[i] = nullptr;
        }

        if (_copyCommandAllocator[i] != nullptr)
        {
            _copyCommandAllocator[i]->Release();
            _copyCommandAllocator[i] = nullptr;
        }

        if (_copyCommandList[i] != nullptr)
        {
            _copyCommandList[i]->Release();
            _copyCommandList[i] = nullptr;
        }
    }

    if (_commandQueue != nullptr)
    {
        _commandQueue->Release();
        _commandQueue = nullptr;
    }

    if (_copyFence != nullptr)
    {
        _copyFence->Release();
        _copyFence = nullptr;
    }

    if (_copyCommandQueue != nullptr)
    {
        _copyCommandQueue->Release();
        _copyCommandQueue = nullptr;
    }
}
