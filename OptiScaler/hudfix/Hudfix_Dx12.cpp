#include "Hudfix_Dx12.h"

#include <Util.h>
#include <State.h>
#include <Config.h>

#include <framegen/IFGFeature_Dx12.h>

#include <future>

bool Hudfix_Dx12::CreateObjects()
{
    if (_commandQueue != nullptr)
        return false;

    do
    {
        HRESULT result;

        for (size_t i = 0; i < BUFFER_COUNT; i++)
        {
            result = State::Instance().currentD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocator: {:X}", (unsigned long)result);
                break;
            }
            _commandAllocator[i]->SetName(L"Hudfix CommandAllocator");


            result = State::Instance().currentD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator[i], NULL, IID_PPV_ARGS(&_commandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList: {:X}", (unsigned long)result);
                break;
            }

            _commandList[i]->SetName(L"Hudfix CommandList");

            result = _commandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("_commandList->Close: {:X}", (unsigned long)result);
                break;
            }
        }

        // Create a command queue for frame generation
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

        HRESULT hr = State::Instance().currentD3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue: {:X}", (unsigned long)result);
            break;
        }

        _commandQueue->SetName(L"Hudfix CommandQueue");

        return true;

    } while (false);

    return false;
}

bool Hudfix_Dx12::CreateBufferResource(ID3D12Device* InDevice, ResourceInfo* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource)
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
            LOG_WARN("Release {}x{}, new one: {}x{}", bufDesc.Width, bufDesc.Height, InSource->width, InSource->height);
        }
        else
        {
            return true;
        }
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

    LOG_DEBUG("Created new one: {}x{}", texDesc.Width, texDesc.Height);
    return true;
}

void Hudfix_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = InResource;
    barrier.Transition.StateBefore = InBeforeState;
    barrier.Transition.StateAfter = InAfterState;
    barrier.Transition.Subresource = 0;
    InCommandList->ResourceBarrier(1, &barrier);
}

bool Hudfix_Dx12::CheckCapture()
{
    auto fIndex = GetIndex();

    // early exit
    if (_captureCounter[fIndex] > 999)
        return false;

    {
        std::unique_lock<std::shared_mutex> lock(_counterMutex);
        _captureCounter[fIndex]++;

        LOG_TRACE("frameCounter: {}, _captureCounter: {}, Limit: {}", State::Instance().currentFeature->FrameCount(), _captureCounter[fIndex], Config::Instance()->FGHUDLimit.value_or_default());

        if (_captureCounter[fIndex] > 999 || _captureCounter[fIndex] != Config::Instance()->FGHUDLimit.value_or_default())
            return false;
    }

    return true;

}

bool Hudfix_Dx12::CheckResource(ResourceInfo* resource)
{
    if (State::Instance().FGonlyUseCapturedResources)
    {
        auto result = _captureList.find(resource->buffer) != _captureList.end();
        return CheckCapture();
    }

    auto currentMs = Util::MillisecondsNow();

    if (!Config::Instance()->FGAlwaysTrackHeaps.value_or_default() &&
        resource->lastUsedFrame != 0 && (currentMs - resource->lastUsedFrame) > 400)
    {
        LOG_DEBUG("Resource {:X}, last used frame ({}) is too small ({}) from current one ({}) skipping resource!",
                  (size_t)resource->buffer, currentMs - resource->lastUsedFrame, resource->lastUsedFrame, currentMs);

        resource->lastUsedFrame = currentMs; // use it next time if timing is ok
        return false;
    }

    DXGI_SWAP_CHAIN_DESC scDesc{};
    if (State::Instance().currentSwapchain->GetDesc(&scDesc) != S_OK)
    {
        LOG_WARN("Can't get swapchain desc!");
        return false;
    }

    // dimensions not match
    if (resource->height != scDesc.BufferDesc.Height || resource->width != scDesc.BufferDesc.Width)
    {
        //LOG_DEBUG_ONLY("Width: {}/{}, Height: {}/{}, Format: {}/{}, Resource: {:X}, convertFormat: {} -> FALSE",
        //          resource->width, scDesc.BufferDesc.Width, resource->height, scDesc.BufferDesc.Height, (UINT)resource->format, (UINT)scDesc.BufferDesc.Format, (size_t)resource->buffer, Config::Instance()->FGHUDFixExtended.value_or_default());

        return false;
    }

    // check for resource flags
    if ((resource->flags & (D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY |
        D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE | D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY)) > 0)
    {
        LOG_TRACE("Skip by flag: {:X}", (UINT)resource->flags);
        return false;
    }

    // format match
    if (resource->format == scDesc.BufferDesc.Format)
    {
        LOG_DEBUG("Width: {}/{}, Height: {}/{}, Format: {}/{}, Resource: {:X}, convertFormat: {} -> TRUE",
                  resource->width, scDesc.BufferDesc.Width, resource->height, scDesc.BufferDesc.Height, (UINT)resource->format, (UINT)scDesc.BufferDesc.Format, (size_t)resource->buffer, Config::Instance()->FGHUDFixExtended.value_or_default());

        resource->lastUsedFrame = currentMs;

        return CheckCapture();
    }

    // extended not active 
    if (!Config::Instance()->FGHUDFixExtended.value_or_default())
    {
        LOG_TRACE("Width: {}/{}, Height: {}/{}, Format: {}/{}, Resource: {:X}, convertFormat: {} -> FALSE",
                  resource->width, scDesc.BufferDesc.Width, resource->height, scDesc.BufferDesc.Height, (UINT)resource->format, (UINT)scDesc.BufferDesc.Format, (size_t)resource->buffer, Config::Instance()->FGHUDFixExtended.value_or_default());

        return false;
    }

    // resource and target formats are supported by converter
    if ((resource->format == DXGI_FORMAT_R10G10B10A2_UNORM || resource->format == DXGI_FORMAT_R10G10B10A2_TYPELESS ||
        resource->format == DXGI_FORMAT_R16G16B16A16_FLOAT || resource->format == DXGI_FORMAT_R16G16B16A16_TYPELESS ||
        resource->format == DXGI_FORMAT_R11G11B10_FLOAT ||
        resource->format == DXGI_FORMAT_R32G32B32A32_FLOAT || resource->format == DXGI_FORMAT_R32G32B32A32_TYPELESS ||
        resource->format == DXGI_FORMAT_R32G32B32_FLOAT || resource->format == DXGI_FORMAT_R32G32B32_TYPELESS ||
        resource->format == DXGI_FORMAT_R8G8B8A8_TYPELESS || resource->format == DXGI_FORMAT_R8G8B8A8_UNORM || resource->format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
        resource->format == DXGI_FORMAT_B8G8R8A8_TYPELESS || resource->format == DXGI_FORMAT_B8G8R8A8_UNORM || resource->format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) &&
        (scDesc.BufferDesc.Format == DXGI_FORMAT_R10G10B10A2_UNORM || scDesc.BufferDesc.Format == DXGI_FORMAT_R10G10B10A2_TYPELESS ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT || scDesc.BufferDesc.Format == DXGI_FORMAT_R16G16B16A16_TYPELESS ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_R11G11B10_FLOAT ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_R32G32B32A32_FLOAT || scDesc.BufferDesc.Format == DXGI_FORMAT_R32G32B32A32_TYPELESS ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_R32G32B32_FLOAT || scDesc.BufferDesc.Format == DXGI_FORMAT_R32G32B32_TYPELESS ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_TYPELESS || scDesc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || scDesc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
        scDesc.BufferDesc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS || scDesc.BufferDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || scDesc.BufferDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB))
    {
        LOG_DEBUG("Width: {}/{}, Height: {}/{}, Format: {}/{}, Resource: {:X}, convertFormat: {} -> TRUE",
                  resource->width, scDesc.BufferDesc.Width, resource->height, scDesc.BufferDesc.Height, (UINT)resource->format, (UINT)scDesc.BufferDesc.Format, (size_t)resource->buffer, Config::Instance()->FGHUDFixExtended.value_or_default());

        if (_formatTransfer == nullptr || !_formatTransfer->IsFormatCompatible(scDesc.BufferDesc.Format))
        {
            LOG_DEBUG("Format change, recreate the FormatTransfer");

            if (_formatTransfer != nullptr)
                delete _formatTransfer;

            _formatTransfer = nullptr;
            State::Instance().skipHeapCapture = true;
            _formatTransfer = new FT_Dx12("FormatTransfer", State::Instance().currentD3D12Device, scDesc.BufferDesc.Format);
            State::Instance().skipHeapCapture = false;
        }

        resource->lastUsedFrame = currentMs;

        return CheckCapture();
    }

    //LOG_DEBUG_ONLY("Width: {}/{}, Height: {}/{}, Format: {}/{}, Resource: {:X}, convertFormat: {} -> FALSE",
    //               resource->width, scDesc.BufferDesc.Width, resource->height, scDesc.BufferDesc.Height, (UINT)resource->format, (UINT)scDesc.BufferDesc.Format, (size_t)resource->buffer, Config::Instance()->FGHUDFixExtended.value_or_default());

    return false;
}

int Hudfix_Dx12::GetIndex()
{
    return _upscaleCounter % 4;
}

void Hudfix_Dx12::DispatchFG(bool useHudless)
{
    LOG_DEBUG("useHudless: {}, _upscaleCounter: {}, _fgCounter: {}", useHudless, _upscaleCounter, _fgCounter);

    // Set it above 1000 to prvent capture
    _captureCounter[GetIndex()] = 9999;

    // Increase counter
    _fgCounter++;

    std::async(std::launch::async, [=]()
               {
                   _skipHudlessChecks = true;
                   reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG)->DispatchHudless(useHudless, _frameTime);
                   _skipHudlessChecks = false;
               });
}

void Hudfix_Dx12::UpscaleStart()
{
    LOG_DEBUG("");

    if (_upscaleCounter > _fgCounter && IsResourceCheckActive() && CheckCapture())
    {
        LOG_WARN("FG not run yet! _upscaleCounter: {}, _fgCounter: {}", _upscaleCounter, _fgCounter);
        DispatchFG(false);
    }
}

void Hudfix_Dx12::UpscaleEnd(UINT64 frameId, float lastFrameTime)
{

    // Update counter after upscaling so _upscaleCounter == _fgCounter check at IsResourceCheckActive will work
    _upscaleCounter++; // = frameId;
    _frameTime = lastFrameTime;

    // Get new index and clear resources
    auto index = GetIndex();
    _captureCounter[index] = 0;

    // Calculate target time for capturing hudless
    auto now = Util::MillisecondsNow();
    auto diff = 0.0f;

    //if (_upscaleEndTime <= 0.1f || _lastDiffTime <= 0.1f)
    //    diff = 4.0f;
    //else
    //    diff = _lastDiffTime - 0.5f;

    //if (diff < 0.0f || diff > 4.0f)
    //    diff = 4.0f;

    diff = 40.0f;

    _targetTime = now + diff;

    LOG_DEBUG("_upscaleCounter: {}, _fgCounter: {}, _lastDiffTime: {}, _frameTime: {}, currentTime: {}, limitTime: {}",
              _upscaleCounter, _fgCounter, _lastDiffTime, _frameTime, now, _targetTime);

    _upscaleEndTime = now;
    _lastDiffTime = 0.0f;
}

void Hudfix_Dx12::PresentStart()
{
    // Calculate last upscale to present time
    if (_upscaleEndTime > 0.1f)
        _lastDiffTime = Util::MillisecondsNow() - _upscaleEndTime;
    else
        _lastDiffTime = 0.0f;

    // FG not run yet!
    if (_upscaleCounter > _fgCounter && IsResourceCheckActive() && CheckCapture())
    {
        LOG_WARN("FG not run yet! _upscaleCounter: {}, _fgCounter: {}", _upscaleCounter, _fgCounter);
        DispatchFG(false);
    }
}

void Hudfix_Dx12::PresentEnd()
{
    LOG_DEBUG("");
}

UINT64 Hudfix_Dx12::ActiveUpscaleFrame()
{
    return _upscaleCounter;
}

UINT64 Hudfix_Dx12::ActivePresentFrame()
{
    return _fgCounter;
}

bool Hudfix_Dx12::IsResourceCheckActive()
{
    if (_upscaleCounter <= _fgCounter)
    {
        //LOG_TRACE("_upscaleCounter: {} <= _fgCounter: {}", _upscaleCounter, _fgCounter);
        return false;
    }

    if (!Config::Instance()->FGEnabled.value_or_default() || !Config::Instance()->FGHUDFix.value_or_default())
    {
        //LOG_TRACE("FGEnabled: {} <= FGHUDFix: {}", Config::Instance()->FGEnabled.value_or_default(), Config::Instance()->FGHUDFix.value_or_default());
        return false;
    }

    if (State::Instance().currentFeature == nullptr || State::Instance().currentFG == nullptr)
    {
        //LOG_TRACE("currentFeature: {:X} <= currentFG: {:X}", (size_t)State::Instance().currentFeature, (size_t)State::Instance().currentFG);
        return false;
    }

    if (!State::Instance().currentFG->IsActive() || State::Instance().FGchanged)
    {
        //LOG_TRACE("currentFG->IsActive(): {} <= State::Instance().FGchanged: {}", State::Instance().currentFG->IsActive(), State::Instance().FGchanged);
        return false;
    }

    auto fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);
    if (fg == nullptr)
    {
        //LOG_TRACE("currentFG is not Dx12!");
        return false;
    }

    return true;
}

bool Hudfix_Dx12::SkipHudlessChecks()
{
    return _skipHudlessChecks;
}

bool Hudfix_Dx12::CheckForHudless(std::string callerName, ID3D12GraphicsCommandList* cmdList, ResourceInfo* resource, D3D12_RESOURCE_STATES state)
{
    if (State::Instance().currentFG == nullptr)
        return false;

    if (!IsResourceCheckActive())
        return false;

    do
    {
        // Prevent double capture
        //LOG_DEBUG("Waiting mutex");

        if (!CheckResource(resource))
            break;

        auto fIndex = GetIndex();

        DXGI_SWAP_CHAIN_DESC scDesc{};
        if (State::Instance().currentSwapchain->GetDesc(&scDesc) != S_OK)
        {
            LOG_WARN("Can't get swapchain desc!");
            _captureCounter[fIndex]--;
            break;
        }

        LOG_TRACE("Capture resource: {:X}, index: {}", (size_t)resource->buffer, fIndex);

        if (_commandQueue == nullptr && !CreateObjects())
        {
            _captureCounter[fIndex]--;
            return false;
        }

        // Make a copy of resource to capture current state
        if (CreateBufferResource(State::Instance().currentD3D12Device, resource, D3D12_RESOURCE_STATE_COPY_DEST, &_captureBuffer[fIndex]))
        {
            ResourceBarrier(cmdList, resource->buffer, state, D3D12_RESOURCE_STATE_COPY_SOURCE);
            cmdList->CopyResource(_captureBuffer[fIndex], resource->buffer);
            ResourceBarrier(cmdList, resource->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, state);

            // Reset command list
            //_commandAllocator[fIndex]->Reset();
            //_commandList[fIndex]->Reset(_commandAllocator[fIndex], nullptr);

            //ResourceBarrier(_commandList[fIndex], resource->buffer, state, D3D12_RESOURCE_STATE_COPY_SOURCE);
            //_commandList[fIndex]->CopyResource(_captureBuffer[fIndex], resource->buffer);
            //ResourceBarrier(_commandList[fIndex], resource->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, state);

            //_commandList[fIndex]->Close();
            //ID3D12CommandList* cmdLists[1] = { _commandList[fIndex] };
            //_commandQueue->ExecuteCommandLists(1, cmdLists);
        }
        else
        {
            LOG_WARN("Can't create _captureBuffer!");
            _captureCounter[fIndex]--;
            break;
        }

        // needs conversion?
        if (resource->format != scDesc.BufferDesc.Format)
        {
            if (_formatTransfer != nullptr && _formatTransfer->CreateBufferResource(State::Instance().currentD3D12Device, resource->buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
            {
                // Reset command list
                _commandAllocator[fIndex]->Reset();
                _commandList[fIndex]->Reset(_commandAllocator[fIndex], nullptr);

                ResourceBarrier(_commandList[fIndex], _captureBuffer[fIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                _formatTransfer->Dispatch(State::Instance().currentD3D12Device, _commandList[fIndex], _captureBuffer[fIndex], _formatTransfer->Buffer());
                ResourceBarrier(_commandList[fIndex], _captureBuffer[fIndex], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

                _commandList[fIndex]->Close();
                ID3D12CommandList* cmdLists[1] = { _commandList[fIndex] };
                _commandQueue->ExecuteCommandLists(1, cmdLists);

                LOG_TRACE("Using _formatTransfer->Buffer()");

                auto fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);
                if (fg != nullptr)
                    fg->SetHudless(nullptr, _formatTransfer->Buffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, false);
            }
            else
            {
                LOG_WARN("_formatTransfer is null or can't create _formatTransfer buffer!");
                _captureCounter[fIndex]--;
                break;
            }
        }
        else
        {
            auto fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);
            if (fg != nullptr)
                fg->SetHudless(nullptr, _captureBuffer[fIndex], D3D12_RESOURCE_STATE_COPY_DEST, false);
        }

        {
            if (State::Instance().FGcaptureResources)
            {
                std::unique_lock<std::shared_mutex> lock(_captureMutex);
                _captureList.insert(resource->buffer);
                State::Instance().FGcapturedResourceCount = _captureList.size();
            }
        }

        LOG_DEBUG("Calling FG with hudless");
        DispatchFG(true);

        return true;

    } while (false);

    // Check for limit time
    auto now = Util::MillisecondsNow();
    if (now > _targetTime && IsResourceCheckActive() && CheckCapture())
    {
        LOG_WARN("Reached limit time: {} > {}", now, _targetTime);
        DispatchFG(false);
    }

    return false;
}

void Hudfix_Dx12::ResetCounters()
{
    _fgCounter = 0;
    _upscaleCounter = 0;

    _lastDiffTime = 0.0;
    _upscaleEndTime = 0.0;
    _targetTime = 0.0;
    _frameTime = 0.0;
}
