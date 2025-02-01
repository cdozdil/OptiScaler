#include "Hudfix_Dx12.h"

// This will be removed/changed
#include <hooks/HooksDx.h>

inline bool Hudfix_Dx12::CreateBufferResource(ID3D12Device* InDevice, ResourceInfo* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource)
{
    if (InDevice == nullptr || InSource == nullptr)
        return false;

    if (*OutResource != nullptr)
    {
        auto bufDesc = (*OutResource)->GetDesc();

        if (bufDesc.Width != (UINT64)(InSource->width) || bufDesc.Height != (UINT)(InSource->height) || bufDesc.Format != InSource->format)
        {
            (*OutResource)->Release();
            (*OutResource) = nullptr;
        }
        else
            return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = InSource->buffer->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        LOG_ERROR("GetHeapProperties result: {:X}", (UINT64)hr);
        return false;
    }

    D3D12_RESOURCE_DESC texDesc = InSource->buffer->GetDesc();
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(OutResource));

    if (hr != S_OK)
    {
        LOG_ERROR("CreateCommittedResource result: {:X}", (UINT64)hr);
        return false;
    }

    return true;
}

inline void Hudfix_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = InResource;
    barrier.Transition.StateBefore = InBeforeState;
    barrier.Transition.StateAfter = InAfterState;
    barrier.Transition.Subresource = 0;
    InCommandList->ResourceBarrier(1, &barrier);
}

inline int Hudfix_Dx12::GetIndex()
{
    return _upscaleCounter % 4;
}

inline void Hudfix_Dx12::UpscaleStart()
{
}

inline void Hudfix_Dx12::UpscaleEnd(float lastFrameTime)
{
    // Update counter after upscaling so _upscaleCounter == _presentCounter check at IsResourceCheckActive will work
    _upscaleCounter++;

    // Get new index and clear resources
    auto index = GetIndex();
    _capturedHudless[index] = nullptr;
    _captureCounter[index] = 0;

    _targetTime = Util::MillisecondsNow() + lastFrameTime - 1.0;
}

inline UINT64 Hudfix_Dx12::ActiveUpscaleFrame()
{
    return _upscaleCounter;
}

inline UINT64 Hudfix_Dx12::ActivePresentFrame()
{
    return _presentCounter;
}

inline bool Hudfix_Dx12::IsResourceCheckActive()
{
    if (_upscaleCounter == _presentCounter)
        return false;

    if (!Config::Instance()->FGEnabled.value_or_default() || !Config::Instance()->FGHUDFix.value_or_default())
        return false;

    if (FrameGen_Dx12::fgContext == nullptr || State::Instance().currentFeature == nullptr || !FrameGen_Dx12::fgIsActive || State::Instance().FGchanged)
        return false;

    if (FrameGen_Dx12::fgTarget > State::Instance().currentFeature->FrameCount())
        return false;

    return true;
}
