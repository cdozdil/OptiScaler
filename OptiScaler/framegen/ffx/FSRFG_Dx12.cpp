#include "FSRFG_Dx12.h"

#include <upscalers/IFeature.h>
#include <menu/menu_overlay_dx.h>

typedef struct FfxSwapchainFramePacingTuning
{
    float safetyMarginInMs; // in Millisecond. Default is 0.1ms
    float varianceFactor; // valid range [0.0,1.0]. Default is 0.1
    bool     allowHybridSpin; //Allows pacing spinlock to sleep. Default is false.
    uint32_t hybridSpinTime;  //How long to spin if allowHybridSpin is true. Measured in timer resolution units. Not recommended to go below 2. Will result in frequent overshoots. Default is 2.
    bool     allowWaitForSingleObjectOnFence; //Allows WaitForSingleObject instead of spinning for fence value. Default is false.
} FfxSwapchainFramePacingTuning;

void FSRFG_Dx12::ReleaseFGSwapchain(HWND hWnd)
{
    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Waiting ffxMutex 1, current: {}", ffxMutex.getOwner());
        ffxMutex.lock(1);
        LOG_TRACE("Accuired ffxMutex: {}", ffxMutex.getOwner());
    }

    MenuOverlayDx::CleanupRenderTarget(true, hWnd);

    if (fgContext != nullptr)
        StopAndDestroyFGContext(true, true, false);

    if (fgSwapChainContext != nullptr)
    {
        auto result = FfxApiProxy::D3D12_DestroyContext()(&fgSwapChainContext, nullptr);
        LOG_INFO("Destroy Ffx Swapchain Result: {}({})", result, FfxApiProxy::ReturnCodeToString(result));

        fgSwapChainContext = nullptr;
        fgSwapChains.erase(hWnd);
    }

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", FSRFG_Dx12::ffxMutex.getOwner());
        FSRFG_Dx12::ffxMutex.unlockThis(1);
    }
}

UINT FSRFG_Dx12::NewFrame()
{
    if (fgActiveFrameIndex == -1)
    {
        fgActiveFrameIndex = 0;
    }
    else
    {
        fgActiveFrameIndex = (fgActiveFrameIndex + 1) % HooksDx::FG_BUFFER_SIZE;

        //if (fgActiveFrameIndex == fgHudlessFrameIndex)
        //{
        //    fgActiveFrameIndex = (fgActiveFrameIndex + 1) % FG_BUFFER_SIZE;
        //    fgHudlessFrameIndex = fgActiveFrameIndex;
        //}
        //else
        //{
        //    fgActiveFrameIndex = (fgActiveFrameIndex + 1) % FG_BUFFER_SIZE;
        //}
    }

    LOG_DEBUG("fgActiveFrameIndex: {}", fgActiveFrameIndex);
    fgUpscaledFound = false;
    ClearNextFrame();

    return fgActiveFrameIndex;
}

UINT FSRFG_Dx12::GetFrame()
{
    if (fgActiveFrameIndex < 0)
        return 0;

    return fgActiveFrameIndex;
}

void FSRFG_Dx12::ResetIndexes()
{
    fgActiveFrameIndex = -1;

    for (size_t i = 0; i < HooksDx::FG_BUFFER_SIZE; i++)
    {
        HooksDx::fgHUDlessCaptureCounter[i] = 0;

        if (fgPossibleHudless[i].size() != 0)
            fgPossibleHudless[i].clear();
    }
}

void FSRFG_Dx12::ReleaseFGObjects()
{
    for (size_t i = 0; i < 4; i++)
    {
        if (FSRFG_Dx12::fgCommandAllocators[i] != nullptr)
        {
            FSRFG_Dx12::fgCommandAllocators[i]->Release();
            FSRFG_Dx12::fgCommandAllocators[i] = nullptr;
        }

        if (fgCommandListSL[i] != nullptr)
        {
            fgCommandListSL[i]->Release();
            fgCommandListSL[i] = nullptr;
            FSRFG_Dx12::fgCommandList[i] = nullptr;
        }
        else if (FSRFG_Dx12::fgCommandList[i] != nullptr)
        {
            FSRFG_Dx12::fgCommandList[i]->Release();
            FSRFG_Dx12::fgCommandList[i] = nullptr;
        }

        if (FSRFG_Dx12::fgCopyCommandAllocator[i] != nullptr)
        {
            FSRFG_Dx12::fgCopyCommandAllocator[i]->Release();
            FSRFG_Dx12::fgCopyCommandAllocator[i] = nullptr;
        }

        if (fgCopyCommandListSL[i] != nullptr)
        {
            fgCopyCommandListSL[i]->Release();
            fgCopyCommandListSL[i] = nullptr;
            FSRFG_Dx12::fgCopyCommandList[i] = nullptr;
        }
        else if (FSRFG_Dx12::fgCopyCommandList[i] != nullptr)
        {
            FSRFG_Dx12::fgCopyCommandList[i]->Release();
            FSRFG_Dx12::fgCopyCommandList[i] = nullptr;
        }
    }

    if (fgCommandQueueSL != nullptr)
    {
        fgCommandQueueSL->Release();
        fgCommandQueueSL = nullptr;
        FSRFG_Dx12::fgCommandQueue = nullptr;
    }
    else if (FSRFG_Dx12::fgCommandQueue != nullptr)
    {
        FSRFG_Dx12::fgCommandQueue->Release();
        FSRFG_Dx12::fgCommandQueue = nullptr;
    }

    if (fgCopyFence != nullptr)
    {
        FSRFG_Dx12::fgCopyFence->Release();
        FSRFG_Dx12::fgCopyFence = nullptr;
    }

    if (fgCopyCommandQueueSL != nullptr)
    {
        fgCopyCommandQueueSL->Release();
        fgCopyCommandQueueSL = nullptr;
        FSRFG_Dx12::fgCopyCommandQueue = nullptr;
    }
    else if (FSRFG_Dx12::fgCopyCommandQueue != nullptr)
    {
        FSRFG_Dx12::fgCopyCommandQueue->Release();
        FSRFG_Dx12::fgCopyCommandQueue = nullptr;
    }

    if (FSRFG_Dx12::fgFormatTransfer != nullptr)
    {
        delete FSRFG_Dx12::fgFormatTransfer;
        FSRFG_Dx12::fgFormatTransfer = nullptr;
    }
}

void FSRFG_Dx12::CreateFGObjects(ID3D12Device* InDevice)
{
    if (FSRFG_Dx12::fgCommandQueue != nullptr)
        return;

    do
    {
        HRESULT result;
        IID riid;
        auto iidResult = IIDFromString(L"{ADEC44E2-61F0-45C3-AD9F-1B37379284FF}", &riid);

        ID3D12GraphicsCommandList* realCL = nullptr;

        for (size_t i = 0; i < 4; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FSRFG_Dx12::fgCommandAllocators[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocators[{0}]: {1:X}", i, (unsigned long)result);
                break;
            }
            FSRFG_Dx12::fgCommandAllocators[i]->SetName(L"fgCommandAllocator");


            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FSRFG_Dx12::fgCommandAllocators[i], NULL, IID_PPV_ARGS(&FSRFG_Dx12::fgCommandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList: {0:X}", (unsigned long)result);
                break;
            }

            // check for SL proxy
            if (CheckForRealObject(__FUNCTION__, FSRFG_Dx12::fgCommandList[i], (IUnknown**)&realCL))
            {
                fgCommandListSL[i] = FSRFG_Dx12::fgCommandList[i];
                FSRFG_Dx12::fgCommandList[i] = realCL;
            }

            FSRFG_Dx12::fgCommandList[i]->SetName(L"fgCommandList");

            result = FSRFG_Dx12::fgCommandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("HooksDx::fgCommandList->Close: {0:X}", (unsigned long)result);
                break;
            }
        }

        for (size_t i = 0; i < 4; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FSRFG_Dx12::fgCopyCommandAllocator[i]));

            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FSRFG_Dx12::fgCopyCommandAllocator[i], NULL, IID_PPV_ARGS(&FSRFG_Dx12::fgCopyCommandList[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandList: {0:X}", (unsigned long)result);
                break;
            }

            if (CheckForRealObject(__FUNCTION__, FSRFG_Dx12::fgCopyCommandList[i], (IUnknown**)&realCL))
            {
                fgCopyCommandListSL[i] = FSRFG_Dx12::fgCopyCommandList[i];
                FSRFG_Dx12::fgCopyCommandList[i] = realCL;
            }
            FSRFG_Dx12::fgCopyCommandList[i]->SetName(L"fgCopyCommandList");

            result = FSRFG_Dx12::fgCopyCommandList[i]->Close();
            if (result != S_OK)
            {
                LOG_ERROR("HooksDx::fgCopyCommandList->Close: {0:X}", (unsigned long)result);
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

        HRESULT hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&FSRFG_Dx12::fgCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue fgCommandQueue: {0:X}", (unsigned long)result);
            break;
        }

        // check for SL proxy
        ID3D12CommandQueue* realCQ = nullptr;
        if (CheckForRealObject(__FUNCTION__, FSRFG_Dx12::fgCommandQueue, (IUnknown**)&realCQ))
        {
            fgCommandQueueSL = FSRFG_Dx12::fgCommandQueue;
            FSRFG_Dx12::fgCommandQueue = realCQ;
        }

        FSRFG_Dx12::fgCommandQueue->SetName(L"fgCommandQueue");

        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&FSRFG_Dx12::fgCopyCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue fgCopyCommandQueue: {0:X}", (unsigned long)result);
            break;
        }

        if (CheckForRealObject(__FUNCTION__, FSRFG_Dx12::fgCopyCommandQueue, (IUnknown**)&realCQ))
        {
            fgCommandQueueSL = FSRFG_Dx12::fgCopyCommandQueue;
            FSRFG_Dx12::fgCopyCommandQueue = realCQ;
        }
        FSRFG_Dx12::fgCopyCommandQueue->SetName(L"fgCopyCommandQueue");

        hr = InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fgCopyFence));
        if (result != S_OK)
        {
            LOG_ERROR("CreateFence fgCopyFence: {0:X}", (unsigned long)result);
            break;
        }

        State::Instance().skipHeapCapture = true;
        FSRFG_Dx12::fgFormatTransfer = new FT_Dx12("FormatTransfer", InDevice, HooksDx::CurrentSwapchainFormat());
        State::Instance().skipHeapCapture = false;

    } while (false);
}

void FSRFG_Dx12::CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext)
{
    if (FSRFG_Dx12::fgContext != nullptr)
    {
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = true;
        m_FrameGenerationConfig.swapChain = State::Instance().currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        auto result = FfxApiProxy::D3D12_Configure()(&FSRFG_Dx12::fgContext, &m_FrameGenerationConfig.header);

        FSRFG_Dx12::fgIsActive = (result == FFX_API_RETURN_OK);

        LOG_DEBUG("Reactivate");

        return;
    }

    ffxCreateBackendDX12Desc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;

    ID3D12Device* device;
    if (!CheckForRealObject(__FUNCTION__, InDevice, (IUnknown**)&device))
        device = InDevice;

    backendDesc.device = device;

    ffxCreateContextDescFrameGeneration createFg{};
    createFg.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION;

    // use swapchain buffer info 
    DXGI_SWAP_CHAIN_DESC desc{};
    if (State::Instance().currentSwapchain->GetDesc(&desc) == S_OK)
    {
        createFg.displaySize = { desc.BufferDesc.Width, desc.BufferDesc.Height };
        createFg.maxRenderSize = { desc.BufferDesc.Width, desc.BufferDesc.Height };
    }
    else
    {
        // this might cause issues
        createFg.displaySize = { deviceContext->DisplayWidth(), deviceContext->DisplayHeight() };
        createFg.maxRenderSize = { deviceContext->DisplayWidth(), deviceContext->DisplayHeight() };
    }

    createFg.flags = 0;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVJittered)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;

    if ((deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;

    if (Config::Instance()->FGAsync.value_or_default())
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT;


    createFg.backBufferFormat = ffxApiGetSurfaceFormatDX12(HooksDx::CurrentSwapchainFormat());
    createFg.header.pNext = &backendDesc.header;

    State::Instance().skipSpoofing = true;
    State::Instance().skipHeapCapture = true;
    ffxReturnCode_t retCode = FfxApiProxy::D3D12_CreateContext()(&FSRFG_Dx12::fgContext, &createFg.header, nullptr);
    State::Instance().skipHeapCapture = false;
    State::Instance().skipSpoofing = false;
    LOG_INFO("D3D12_CreateContext result: {0:X}", retCode);

    FSRFG_Dx12::fgIsActive = (retCode == FFX_API_RETURN_OK);

    LOG_DEBUG("Create");
}

void FSRFG_Dx12::StopAndDestroyFGContext(bool destroy, bool shutDown, bool useMutex)
{
    HooksDx::fgSkipHudlessChecks = false;
    ResetIndexes();

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default() && useMutex)
    {
        LOG_TRACE("Waiting ffxMutex 1, current: {}", FSRFG_Dx12::ffxMutex.getOwner());
        FSRFG_Dx12::ffxMutex.lock(1);
        LOG_TRACE("Accuired ffxMutex: {}", FSRFG_Dx12::ffxMutex.getOwner());
    }

    if (!(shutDown || State::Instance().isShuttingDown) && FSRFG_Dx12::fgContext != nullptr)
    {
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = false;
        m_FrameGenerationConfig.swapChain = State::Instance().currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        ffxReturnCode_t result;
        result = FfxApiProxy::D3D12_Configure()(&FSRFG_Dx12::fgContext, &m_FrameGenerationConfig.header);

        FSRFG_Dx12::fgIsActive = false;

        if (!(shutDown || State::Instance().isShuttingDown))
            LOG_INFO("D3D12_Configure result: {0:X}", result);
    }

    if (destroy && FSRFG_Dx12::fgContext != nullptr)
    {
        auto result = FfxApiProxy::D3D12_DestroyContext()(&FSRFG_Dx12::fgContext, nullptr);

        if (!(shutDown || State::Instance().isShuttingDown))
            LOG_INFO("D3D12_DestroyContext result: {0:X}", result);

        FSRFG_Dx12::fgContext = nullptr;
    }

    if ((shutDown || State::Instance().isShuttingDown) || destroy)
        ReleaseFGObjects();

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default() && useMutex)
    {
        LOG_TRACE("Releasing ffxMutex: {}", FSRFG_Dx12::ffxMutex.getOwner());
        FSRFG_Dx12::ffxMutex.unlockThis(1);
    }
}

void FSRFG_Dx12::CheckUpscaledFrame(ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InUpscaled)
{
    ResourceInfo upscaledInfo{};
    FillResourceInfo(InUpscaled, &upscaledInfo);
    upscaledInfo.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    if (CheckForHudless(__FUNCTION__, &upscaledInfo) && CheckCapture(__FUNCTION__))
        CaptureHudless(InCmdList, &upscaledInfo, upscaledInfo.state);
}

void FSRFG_Dx12::AddFrameTime(float ft)
{
    if (ft < 0.2f || ft > 100.0f)
        return;

    if (HooksDx::fgFrameTimes.size() == HooksDx::FG_BUFFER_SIZE)
        HooksDx::fgFrameTimes.pop_front();

    HooksDx::fgFrameTimes.push_back(ft);

    if (HooksDx::fgFrameTimes.size() == HooksDx::FG_BUFFER_SIZE)
        LOG_TRACE("{}, {}, {}, {}", HooksDx::fgFrameTimes[0], HooksDx::fgFrameTimes[1], HooksDx::fgFrameTimes[2], HooksDx::fgFrameTimes[3]);
}

float FSRFG_Dx12::GetFrameTime()
{
    if (HooksDx::fgFrameTimes.size() > 0)
        return HooksDx::fgFrameTimes.back();

    return 0.0f;

    //float result = 0.0f;
    //float size = (float)fgFrameTimes.size();

    //for (size_t i = 0; i < fgFrameTimes.size(); i++)
    //{
    //    result += (fgFrameTimes[i] / size);
    //}

    //return result;
}

void FSRFG_Dx12::ConfigureFramePaceTuning()
{
    FfxSwapchainFramePacingTuning fpt{};
    if (Config::Instance()->FGFramePacingTuning.value_or_default())
    {
        fpt.allowHybridSpin = Config::Instance()->FGFPTAllowHybridSpin.value_or_default();
        fpt.allowWaitForSingleObjectOnFence = Config::Instance()->FGFPTAllowWaitForSingleObjectOnFence.value_or_default();
        fpt.hybridSpinTime = Config::Instance()->FGFPTHybridSpinTime.value_or_default();
        fpt.safetyMarginInMs = Config::Instance()->FGFPTSafetyMarginInMs.value_or_default();
        fpt.varianceFactor = Config::Instance()->FGFPTVarianceFactor.value_or_default();
    }

    ffxConfigureDescFrameGenerationSwapChainKeyValueDX12 cfgDesc{};
    cfgDesc.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_KEYVALUE_DX12;
    cfgDesc.key = 2;
    cfgDesc.ptr = &fpt;

    auto result = FfxApiProxy::D3D12_Configure()(&FSRFG_Dx12::fgSwapChainContext, &cfgDesc.header);
    LOG_DEBUG("HybridSpin D3D12_Configure result: {}", FfxApiProxy::ReturnCodeToString(result));
}


