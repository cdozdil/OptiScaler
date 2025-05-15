#include "IFGFeature_Dx12.h"

#include <State.h>
#include <Config.h>

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

    LOG_DEBUG("Created new one: {}x{}", inDesc.Width, inDesc.Height);

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

bool IFGFeature_Dx12::CopyResource(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* source, ID3D12Resource** target, D3D12_RESOURCE_STATES sourceState)
{
    auto result = true;

    ResourceBarrier(cmdList, source, sourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);

    if (CreateBufferResource(State::Instance().currentD3D12Device, source, D3D12_RESOURCE_STATE_COPY_DEST, target))
        cmdList->CopyResource(*target, source);
    else
        result = false;

    ResourceBarrier(cmdList, source, D3D12_RESOURCE_STATE_COPY_SOURCE, sourceState);

    return result;
}

void IFGFeature_Dx12::WaitForFenceValue(ID3D12Fence* fence, UINT64 targetValue, HANDLE fenceEvent)
{
    // Check if the fence has already been reached
    if (fence->GetCompletedValue() < targetValue)
    {
        // Instruct the fence to signal the event when the target value is reached
        HRESULT hr = fence->SetEventOnCompletion(targetValue, fenceEvent);
        if (hr != S_OK)
        {
            LOG_ERROR("Failed to set event on fence completion.");
            return;
        }

        // Wait for the event
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void IFGFeature_Dx12::SetVelocity(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* velocity, D3D12_RESOURCE_STATES state)
{
    auto index = GetIndex();
    LOG_TRACE("Setting velocity, index: {}", index);

    if (cmdList == nullptr)
    {
        _paramVelocity[index] = velocity;
        return;
    }

    if (Config::Instance()->FGMakeMVCopy.value_or_default() && CopyResource(cmdList, velocity, &_paramVelocityCopy[index], state))
        _paramVelocity[index] = _paramVelocityCopy[index];
    else
        _paramVelocity[index] = velocity;
}

void IFGFeature_Dx12::SetDepth(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* depth, D3D12_RESOURCE_STATES state)
{
    auto index = GetIndex();
    LOG_TRACE("Setting depth, index: {}", index);

    if (cmdList == nullptr)
    {
        _paramDepth[index] = depth;
        return;
    }

    if (Config::Instance()->FGMakeDepthCopy.value_or_default() && CopyResource(cmdList, depth, &_paramDepthCopy[index], state))
        _paramDepth[index] = _paramDepthCopy[index];
    else
        _paramDepth[index] = depth;
}

void IFGFeature_Dx12::SetHudless(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* hudless, D3D12_RESOURCE_STATES state, bool makeCopy)
{
    auto index = GetIndex();
    LOG_TRACE("Setting hudless, index: {}, resource: {:X}", index, (size_t)hudless);

    if (cmdList == nullptr && !makeCopy)
    {
        _paramHudless[index] = hudless;
        return;
    }

    if (makeCopy)
    {
        if (CopyResource(_hudlessCommandList[index], hudless, &_paramHudlessCopy[index], state))
        {
            auto result = _hudlessCommandList[index]->Close();
            if (result == S_OK)
            {
                ID3D12CommandList* cl[] = { _hudlessCommandList[index] };
                _hudlessCommandQueue->ExecuteCommandLists(1, cl);
                _hudlessCommandQueue->Signal(_hudlessFence, _frameCount);
            }
            else
            {
                LOG_ERROR("_hudlessCommandList[]->Close error: {:}", index, (UINT)result);
            }

            _paramHudless[index] = _paramHudlessCopy[index];
        }
        else
        {
            _paramHudless[index] = hudless;
        }
    }
    else
    {
        _paramHudless[index] = hudless;
    }
}

void IFGFeature_Dx12::CreateObjects(ID3D12Device* InDevice)
{
    if (_commandQueue != nullptr)
        return;

    LOG_DEBUG("");

    do
    {
        HRESULT result;

        for (size_t i = 0; i < BUFFER_COUNT; i++)
        {
            ID3D12CommandAllocator* allocator = nullptr;
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocators _commandAllocators[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            allocator->SetName(L"_commandAllocator");
            if (!CheckForRealObject(__FUNCTION__, allocator, (IUnknown**)&_commandAllocators[i]))
                _commandAllocators[i] = allocator;

            ID3D12GraphicsCommandList* cmdList = nullptr;
            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocators[i], NULL, IID_PPV_ARGS(&cmdList));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList _commandList[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            cmdList->SetName(L"_commandList");
            if (!CheckForRealObject(__FUNCTION__, cmdList, (IUnknown**)&_commandList[i]))
                _commandList[i] = cmdList;

            result = _commandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("_commandList[{}]->Close: {:X}", i, (unsigned long)result);
                break;
            }
        }

        for (size_t i = 0; i < BUFFER_COUNT; i++)
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

        for (size_t i = 0; i < BUFFER_COUNT; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_hudlessCommandAllocator[i]));

            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _hudlessCommandAllocator[i], NULL, IID_PPV_ARGS(&_hudlessCommandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList _hudlessCommandList[{}]: {:X}", i, (unsigned long)result);
                break;
            }
            _hudlessCommandList[i]->SetName(L"_hudlessCommandList");

            result = _hudlessCommandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("_hudlessCommandList[{}]->Close: {:X}", i, (unsigned long)result);
                break;
            }
        }

        // Create a command queue for frame generation
        ID3D12CommandQueue* queue = nullptr;
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

        HRESULT hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue _commandQueue: {0:X}", (unsigned long)result);
            break;
        }
        queue->SetName(L"_commandQueue");
        if (!CheckForRealObject(__FUNCTION__, queue, (IUnknown**)&_commandQueue))
            _commandQueue = queue;

        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_copyCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue _copyCommandQueue: {0:X}", (unsigned long)result);
            break;
        }
        _copyCommandQueue->SetName(L"_copyCommandQueue");

        hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_hudlessCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue _hudlessCommandQueue: {0:X}", (unsigned long)result);
            break;
        }
        _hudlessCommandQueue->SetName(L"_hudlessCommandQueue");

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_copyFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence _copyFence: {0:X}", (unsigned long)result);
            break;
        }
        _copyFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fgFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence _fgFence: {0:X}", (unsigned long)result);
            break;
        }
        _fgFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_hudlessFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence _hudlessFence: {0:X}", (unsigned long)result);
            break;
        }
        _hudlessFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_hudlessCopyFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence _hudlessCopyFence: {0:X}", (unsigned long)result);
            break;
        }
        _hudlessCopyFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    } while (false);
}

void IFGFeature_Dx12::ReleaseObjects()
{
    LOG_DEBUG("");

    for (size_t i = 0; i < BUFFER_COUNT; i++)
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

ID3D12Fence* IFGFeature_Dx12::GetCopyFence()
{
    return _copyFence;
}

ID3D12Fence* IFGFeature_Dx12::GetHudlessFence()
{
    return _hudlessCopyFence;
}

void IFGFeature_Dx12::SetWaitOnGameQueue(UINT64 value)
{
    _gameCommandQueue->Wait(_copyFence, value);
}

bool IFGFeature_Dx12::IsFGCommandList(void* cmdList)
{
    auto found = false;

    for (size_t i = 0; i < BUFFER_COUNT; i++)
    {
        if (_commandList[i] == cmdList)
        {
            found = true;
            break;
        }
    }

    return found;
}
