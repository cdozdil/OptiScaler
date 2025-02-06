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

void FSRFG_Dx12::ConfigureFramePaceTuning()
{
    if (_swapChainContext == nullptr)
        return;

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
    cfgDesc.key = 2; // FfxSwapchainFramePacingTuning
    cfgDesc.ptr = &fpt;

    auto result = FfxApiProxy::D3D12_Configure()(&_swapChainContext, &cfgDesc.header);
    LOG_DEBUG("HybridSpin D3D12_Configure result: {}", FfxApiProxy::ReturnCodeToString(result));
}

void FSRFG_Dx12::UpscaleEnd(float lastFrameTime)
{

}

feature_version FSRFG_Dx12::Version()
{
    return feature_version();
}

const char* FSRFG_Dx12::Name()
{
    return "FSR-FG";
}

bool FSRFG_Dx12::Dispatch(UINT64 frameId, double frameTime)
{
    return false;
}

bool FSRFG_Dx12::DispatchHudless(UINT64 frameId, double frameTime, ID3D12Resource* output)
{
    LOG_DEBUG("(FG) running, frame: {0}", frameId);
    _frameCount = frameId;

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Waiting ffxMutex 1, current: {}", Mutex.getOwner());
        Mutex.lock(1);
        LOG_TRACE("Accuired ffxMutex: {}", Mutex.getOwner());
    }

    // Update frame generation config
    auto desc = output->GetDesc();

    ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};

    if (desc.Format == _swapChainDesc.BufferDesc.Format)
    {
        LOG_DEBUG("(FG) desc.Format == HooksDx::swapchainFormat, using for hudless!");
        m_FrameGenerationConfig.HUDLessColor = ffxApiGetResourceDX12(output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS, 0);
    }
    else
    {
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});
    }

    m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
    m_FrameGenerationConfig.frameGenerationEnabled = true;
    m_FrameGenerationConfig.flags = 0;

    if (Config::Instance()->FGDebugView.value_or_default())
        m_FrameGenerationConfig.flags |= FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW;

    m_FrameGenerationConfig.allowAsyncWorkloads = Config::Instance()->FGAsync.value_or_default();
    m_FrameGenerationConfig.generationRect.left = Config::Instance()->FGRectLeft.value_or(0);
    m_FrameGenerationConfig.generationRect.top = Config::Instance()->FGRectTop.value_or(0);

    // use swapchain buffer info 
    DXGI_SWAP_CHAIN_DESC scDesc{};
    if (State::Instance().currentSwapchain->GetDesc(&scDesc) == S_OK)
    {
        m_FrameGenerationConfig.generationRect.width = Config::Instance()->FGRectWidth.value_or(scDesc.BufferDesc.Width);
        m_FrameGenerationConfig.generationRect.height = Config::Instance()->FGRectHeight.value_or(scDesc.BufferDesc.Height);
    }
    else
    {
        m_FrameGenerationConfig.generationRect.width = Config::Instance()->FGRectWidth.value_or(deviceContext->DisplayWidth());
        m_FrameGenerationConfig.generationRect.height = Config::Instance()->FGRectHeight.value_or(deviceContext->DisplayHeight());
    }

    m_FrameGenerationConfig.frameGenerationCallbackUserContext = &_fgContext;
    m_FrameGenerationConfig.frameGenerationCallback = (FfxApiFrameGenerationDispatchFunc)HudlessCallback;

    m_FrameGenerationConfig.onlyPresentGenerated = State::Instance().FGonlyGenerated; // check here
    m_FrameGenerationConfig.frameID = frameId;
    m_FrameGenerationConfig.swapChain = _swapChain;

    //State::Instance().skipSpoofing = true;
    ffxReturnCode_t retCode = FfxApiProxy::D3D12_Configure()(&_fgContext, &m_FrameGenerationConfig.header);
    //State::Instance().skipSpoofing = false;

    if (retCode != FFX_API_RETURN_OK)
        LOG_ERROR("(FG) D3D12_Configure error: {}({})", retCode, FfxApiProxy::ReturnCodeToString(retCode));

    if (retCode == FFX_API_RETURN_OK)
    {
        ffxCreateBackendDX12Desc backendDesc{};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = D3D12Device;

        ffxDispatchDescFrameGenerationPrepare dfgPrepare{};
        dfgPrepare.header.type = FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATION_PREPARE;
        dfgPrepare.header.pNext = &backendDesc.header;

#ifdef USE_QUEUE_FOR_FG
        dfgPrepare.commandList = FrameGen_Dx12::fgCommandList[fIndex];
#else
        dfgPrepare.commandList = InCmdList;
#endif
        dfgPrepare.frameID = deviceContext->FrameCount();
        dfgPrepare.flags = m_FrameGenerationConfig.flags;
        dfgPrepare.renderSize = { deviceContext->RenderWidth(), deviceContext->RenderHeight() };

        InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &dfgPrepare.jitterOffset.x);
        InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &dfgPrepare.jitterOffset.y);

        if (Config::Instance()->FGMakeMVCopy.value_or_default())
        {
            dfgPrepare.motionVectors = ffxApiGetResourceDX12(FrameGen_Dx12::paramVelocityCopy[frameIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
        }
        else
        {
            ID3D12Resource* paramVelocity;
            if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
                InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

            ResourceBarrier(InCmdList, paramVelocity, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

            dfgPrepare.motionVectors = ffxApiGetResourceDX12(paramVelocity, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }

        if (Config::Instance()->FGMakeDepthCopy.value_or_default())
        {
            dfgPrepare.depth = ffxApiGetResourceDX12(FrameGen_Dx12::paramDepthCopy[frameIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
        }
        else
        {
            ID3D12Resource* paramDepth;
            if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
                InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

            ResourceBarrier(InCmdList, paramDepth, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

            dfgPrepare.depth = ffxApiGetResourceDX12(paramDepth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }

        float MVScaleX = 1.0f;
        float MVScaleY = 1.0f;

        if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
            InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
        {
            dfgPrepare.motionVectorScale.x = MVScaleX;
            dfgPrepare.motionVectorScale.y = MVScaleY;
        }
        else
        {
            LOG_WARN("(FG) Can't get motion vector scales!");

            dfgPrepare.motionVectorScale.x = MVScaleX;
            dfgPrepare.motionVectorScale.y = MVScaleY;
        }

        dfgPrepare.cameraFar = cameraFar;
        dfgPrepare.cameraNear = cameraNear;
        dfgPrepare.cameraFovAngleVertical = cameraVFov;
        dfgPrepare.frameTimeDelta = ftDelta;
        dfgPrepare.viewSpaceToMetersFactor = meterFactor;

        retCode = FfxApiProxy::D3D12_Dispatch()(&FrameGen_Dx12::fgContext, &dfgPrepare.header);

        if (retCode != FFX_API_RETURN_OK)
            LOG_ERROR("(FG) D3D12_Dispatch result: {}({})", retCode, FfxApiProxy::ReturnCodeToString(retCode));
        else
            LOG_DEBUG("(FG) Dispatch ok.");
    }

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.unlockThis(1);
    }
}

ffxReturnCode_t FSRFG_Dx12::HudlessCallback(ffxDispatchDescFrameGeneration* params, void* pUserCtx)
{
    auto fIndex = _frameCount;

    params->reset = (_reset != 0);

    // check for status
    if (!Config::Instance()->FGEnabled.value_or_default() ||
        _fgContext == nullptr || State::Instance().SCchanged
#ifdef USE_QUEUE_FOR_FG
        || FrameGen_Dx12::fgCommandList[fIndex] == nullptr || FrameGen_Dx12::fgCommandQueue == nullptr
#endif
        )
    {
        LOG_WARN("(FG) Cancel async dispatch fIndex: {}", fIndex);
        //HooksDx::fgSkipHudlessChecks = false;
        params->numGeneratedFrames = 0;
    }

    // If fg is active but upscaling paused
    if (State::Instance().currentFeature == nullptr || !_isActive ||
        State::Instance().FGchanged || State::Instance().currentFeature->FrameCount() == 0)
    {
        LOG_WARN("(FG) Callback without active FG! fIndex:{}", fIndex);

#ifdef USE_QUEUE_FOR_FG
        auto allocator = FrameGen_Dx12::fgCommandAllocators[fIndex];
        auto result = allocator->Reset();
        result = FrameGen_Dx12::fgCommandList[fIndex]->Reset(allocator, nullptr);
#endif

        params->numGeneratedFrames = 0;
        //return FFX_API_RETURN_OK;
    }


    auto dispatchResult = FfxApiProxy::D3D12_Dispatch()(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
    LOG_DEBUG("(FG) D3D12_Dispatch result: {}, fIndex: {}", (UINT)dispatchResult, fIndex);

#ifdef USE_QUEUE_FOR_FG
    if (dispatchResult == FFX_API_RETURN_OK)
    {
        ID3D12CommandList* cl[1] = { nullptr };
        auto result = FrameGen_Dx12::fgCommandList[fIndex]->Close();
        cl[0] = FrameGen_Dx12::fgCommandList[fIndex];
        FrameGen_Dx12::fgCommandQueue->ExecuteCommandLists(1, cl);

        if (result != S_OK)
        {
            LOG_ERROR("(FG) Close result: {}", (UINT)result);
        }
    }
#endif

    return dispatchResult;
};

void FSRFG_Dx12::StopAndDestroyContext(bool destroy, bool shutDown, bool useMutex)
{
    _frameCount = 0;

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default() && useMutex)
    {
        LOG_TRACE("Waiting ffxMutex 1, current: {}", Mutex.getOwner());
        Mutex.lock(1);
        LOG_TRACE("Accuired ffxMutex: {}", Mutex.getOwner());
    }

    if (!(shutDown || State::Instance().isShuttingDown) && _fgContext != nullptr)
    {
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = false;
        m_FrameGenerationConfig.swapChain = State::Instance().currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        ffxReturnCode_t result;
        result = FfxApiProxy::D3D12_Configure()(&_fgContext, &m_FrameGenerationConfig.header);

        _isActive = false;

        if (!(shutDown || State::Instance().isShuttingDown))
            LOG_INFO("D3D12_Configure result: {0:X}", result);
    }

    if (destroy && _fgContext != nullptr)
    {
        auto result = FfxApiProxy::D3D12_DestroyContext()(&_fgContext, nullptr);

        if (!(shutDown || State::Instance().isShuttingDown))
            LOG_INFO("D3D12_DestroyContext result: {0:X}", result);

        _fgContext = nullptr;
    }

    if ((shutDown || State::Instance().isShuttingDown) || destroy)
        ReleaseObjects();

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default() && useMutex)
    {
        LOG_TRACE("Releasing ffxMutex: {}", Mutex.getOwner());
        Mutex.unlockThis(1);
    }
}

void FSRFG_Dx12::CreateSwapchain(IDXGIFactory* factory, ID3D12CommandQueue* cmdQueue, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain)
{
    ffxCreateContextDescFrameGenerationSwapChainNewDX12 createSwapChainDesc{};
    createSwapChainDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_NEW_DX12;
    createSwapChainDesc.dxgiFactory = factory;
    createSwapChainDesc.gameQueue = cmdQueue;
    createSwapChainDesc.desc = desc;
    createSwapChainDesc.swapchain = (IDXGISwapChain4**)swapChain;

    auto result = FfxApiProxy::D3D12_CreateContext()(&_swapChainContext, &createSwapChainDesc.header, nullptr);

    if (result == FFX_API_RETURN_OK)
    {
        _gameCommandQueue = cmdQueue;
        _swapChainDesc = *desc;
        _swapChain = *swapChain;
    }
}

void FSRFG_Dx12::ReleaseSwapchain(IDXGISwapChain* swapChain)
{
    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Waiting ffxMutex 1, current: {}", Mutex.getOwner());
        Mutex.lock(1);
        LOG_TRACE("Accuired ffxMutex: {}", Mutex.getOwner());
    }

    MenuOverlayDx::CleanupRenderTarget(true, NULL);

    if (_fgContext != nullptr)
        StopAndDestroyContext(true, true, false);

    if (_swapChainContext != nullptr)
    {
        auto result = FfxApiProxy::D3D12_DestroyContext()(&_swapChainContext, nullptr);
        LOG_INFO("Destroy Ffx Swapchain Result: {}({})", result, FfxApiProxy::ReturnCodeToString(result));

        _swapChainContext = nullptr;
    }

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", Mutex.getOwner());
        Mutex.unlockThis(1);
    }
}

void FSRFG_Dx12::CreateContext(ID3D12Device* device, IFeature* upscalerContext)
{
    if (_fgContext != nullptr)
    {
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = true;
        m_FrameGenerationConfig.swapChain = State::Instance().currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        auto result = FfxApiProxy::D3D12_Configure()(&_fgContext, &m_FrameGenerationConfig.header);

        _isActive = (result == FFX_API_RETURN_OK);

        LOG_DEBUG("Reactivate");

        return;
    }

    ffxCreateBackendDX12Desc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;

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
        createFg.displaySize = { upscalerContext->DisplayWidth(), upscalerContext->DisplayHeight() };
        createFg.maxRenderSize = { upscalerContext->DisplayWidth(), upscalerContext->DisplayHeight() };
    }

    createFg.flags = 0;

    if (upscalerContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE;

    if (upscalerContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED;

    if (upscalerContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVJittered)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;

    if ((upscalerContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;

    if (Config::Instance()->FGAsync.value_or_default())
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT;


    createFg.backBufferFormat = ffxApiGetSurfaceFormatDX12(_swapChainDesc.BufferDesc.Format);
    createFg.header.pNext = &backendDesc.header;

    State::Instance().skipSpoofing = true;
    State::Instance().skipHeapCapture = true;
    ffxReturnCode_t retCode = FfxApiProxy::D3D12_CreateContext()(&_fgContext, &createFg.header, nullptr);
    State::Instance().skipHeapCapture = false;
    State::Instance().skipSpoofing = false;
    LOG_INFO("D3D12_CreateContext result: {0:X}", retCode);

    _isActive = (retCode == FFX_API_RETURN_OK);

    LOG_DEBUG("Create");
}


