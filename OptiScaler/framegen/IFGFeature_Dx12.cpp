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
