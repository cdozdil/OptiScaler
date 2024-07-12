#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR31Feature_Dx11On12.h"

FSR31FeatureDx11on12::FSR31FeatureDx11on12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR31Feature(InHandleId, InParameters), IFeature_Dx11wDx12(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
    spdlog::debug("FSR31FeatureDx11on12::FSR31FeatureDx11on12 Loading amd_fidelityfx_dx12.dll methods");

    _configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxConfigure");
    _createContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxCreateContext");
    _destroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDestroyContext");
    _dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDispatch");
    _query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxQuery");

    _moduleLoaded = _configure != nullptr;

    if (_moduleLoaded)
        spdlog::info("FSR31FeatureDx11on12::FSR31FeatureDx11on12 amd_fidelityfx_dx12.dll methods loaded!");
    else
        spdlog::error("FSR31FeatureDx11on12::FSR31FeatureDx11on12 can't load amd_fidelityfx_dx12.dll methods!");
}

bool FSR31FeatureDx11on12::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx11on12::Init!");

    if (IsInited())
        return true;

    Device = InDevice;
    DeviceContext = InContext;

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &_initFlags) == NVSDK_NGX_Result_Success)
        _initFlagsReady = true;

    _baseInit = false;

    return true;
}

bool FSR31FeatureDx11on12::Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx11on12::Evaluate");

    if (!_baseInit)
    {
        // to prevent creation dx12 device if we are going to recreate feature
        if (!Config::Instance()->DisplayResolution.has_value())
        {
            ID3D11Resource* paramVelocity = nullptr;
            if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
                InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

            if (paramVelocity != nullptr)
            {
                D3D11_TEXTURE2D_DESC desc;
                ID3D11Texture2D* pvTexture;
                paramVelocity->QueryInterface(IID_PPV_ARGS(&pvTexture));
                pvTexture->GetDesc(&desc);
                bool lowResMV = desc.Width < TargetWidth();
                bool displaySizeEnabled = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);

                if (displaySizeEnabled && lowResMV)
                {
                    spdlog::warn("FSR31FeatureDx11on12::Evaluate MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, TargetWidth(), displaySizeEnabled);
                    Config::Instance()->DisplayResolution = false;
                }

                pvTexture->Release();
                pvTexture = nullptr;

                Config::Instance()->DisplayResolution = displaySizeEnabled;
            }

            paramVelocity->Release();
            paramVelocity = nullptr;
        }

        if (!Config::Instance()->AutoExposure.has_value())
        {
            ID3D11Resource* paramExpo = nullptr;
            if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExpo) != NVSDK_NGX_Result_Success)
                InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExpo);

            if (paramExpo == nullptr)
            {
                spdlog::warn("FSR31FeatureDx11on12::Evaluate ExposureTexture is not exist, enabling AutoExposure!!");
                Config::Instance()->AutoExposure = true;
            }
        }

        if (!Config::Instance()->DisableReactiveMask.has_value())
        {
            ID3D11Resource* paramRM = nullptr;
            if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramRM) != NVSDK_NGX_Result_Success)
                InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramRM);

            if (paramRM == nullptr)
            {
                spdlog::warn("FSR31FeatureDx11on12::Evaluate Bias mask not exist, enabling DisableReactiveMask!!");
                Config::Instance()->DisableReactiveMask = true;
            }
        }

        if (!BaseInit(Device, InDeviceContext, InParameters))
        {
            spdlog::debug("FSR31FeatureDx11on12::Init BaseInit failed!");
            return false;
        }

        _baseInit = true;

        spdlog::debug("FSR31FeatureDx11on12::Init calling InitFSR2");

        if (Dx12Device == nullptr)
        {
            spdlog::error("FSR31FeatureDx11on12::Init Dx12on11Device is null!");
            return false;
        }

        if (!InitFSR3(InParameters))
        {
            spdlog::error("FSR31FeatureDx11on12::Init InitFSR2 fail!");
            return false;
        }

        if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
            Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);

        if (Config::Instance()->Dx11DelayedInit.value_or(false))
        {
            spdlog::trace("FSR31FeatureDx11on12::Evaluate sleeping after FSRContext creation for 1500ms");
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }

        OutputScaler = std::make_unique<BS_Dx12>("Output Downsample", Dx12Device, (TargetWidth() < DisplayWidth()));
        RCAS = std::make_unique<RCAS_Dx12>("RCAS", Dx12Device);
    }

    if (!IsInited())
        return false;

    if (!RCAS->IsInit())
        Config::Instance()->RcasEnabled = false;

    if (!OutputScaler->IsInit())
        Config::Instance()->OutputScalingEnabled = false;

    ID3D11DeviceContext4* dc;
    auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

    if (result != S_OK)
    {
        spdlog::error("FSR31FeatureDx11on12::Evaluate QueryInterface error: {0:x}", result);
        return false;
    }

    if (dc != Dx11DeviceContext)
    {
        spdlog::warn("FSR31FeatureDx11on12::Evaluate Dx11DeviceContext changed!");
        ReleaseSharedResources();
        Dx11DeviceContext = dc;
    }

    struct ffxDispatchDescUpscale params = { 0 };
    params.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;

    if (Config::Instance()->FsrDebugView.value_or(false))
        params.flags = FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    float sharpness = 0.0f;

    if (Config::Instance()->OverrideSharpness.value_or(false))
    {
        sharpness = Config::Instance()->Sharpness.value_or(0.3);
    }
    else if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &sharpness) == NVSDK_NGX_Result_Success)
    {
        if (sharpness > 1.0f)
            sharpness = 1.0f;

        _sharpness = sharpness;
    }

    if (Config::Instance()->RcasEnabled.value_or(false))
    {
        params.enableSharpening = false;
        params.sharpness = 0.0f;
    }
    else
    {
        params.enableSharpening = sharpness > 0.0f;
        params.sharpness = sharpness;
    }

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

    spdlog::debug("FSR31FeatureDx11on12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    params.commandList = Dx12CommandList;

    if (!ProcessDx11Textures(InParameters))
    {
        spdlog::error("FSR31FeatureDx11on12::Evaluate Can't process Dx11 textures!");

        Dx12CommandList->Close();
        ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
        Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

        Dx12CommandAllocator->Reset();
        Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

        return false;
    }

    // AutoExposure or ReactiveMask is nullptr
    if (Config::Instance()->changeBackend)
    {
        Dx12CommandList->Close();
        ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
        Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

        Dx12CommandAllocator->Reset();
        Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

        return true;
    }

    params.color = ffxApiGetResourceDX12(dx11Color.Dx12Resource, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    params.motionVectors = ffxApiGetResourceDX12(dx11Mv.Dx12Resource, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    params.depth = ffxApiGetResourceDX12(dx11Depth.Dx12Resource, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    params.exposure = ffxApiGetResourceDX12(dx11Exp.Dx12Resource, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    params.reactive = ffxApiGetResourceDX12(dx11Tm.Dx12Resource, FFX_API_RESOURCE_STATE_COMPUTE_READ);

    if (useSS)
    {
        if (OutputScaler->CreateBufferResource(Dx12Device, dx11Out.Dx12Resource, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
        {
            OutputScaler->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            params.output = ffxApiGetResourceDX12(OutputScaler->Buffer(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        }
        else
            params.output = ffxApiGetResourceDX12(dx11Out.Dx12Resource, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    else
        params.output = ffxApiGetResourceDX12(dx11Out.Dx12Resource, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);

    // RCAS
    if (Config::Instance()->RcasEnabled.value_or(false) &&
        (sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
        RCAS->IsInit() && RCAS->CreateBufferResource(Dx12Device, (ID3D12Resource*)params.output.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
    {
        RCAS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        params.output = ffxApiGetResourceDX12(RCAS->Buffer(), FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    _hasColor = params.color.resource != nullptr;
    _hasDepth = params.depth.resource != nullptr;
    _hasMV = params.motionVectors.resource != nullptr;
    _hasExposure = params.exposure.resource != nullptr;
    _hasTM = params.reactive.resource != nullptr;
    _hasOutput = params.output.resource != nullptr;

#pragma endregion

    float MVScaleX = 1.0f;
    float MVScaleY = 1.0f;

    if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
    {
        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }
    else
    {
        spdlog::warn("FSR31FeatureDx11on12::Evaluate Can't get motion vector scales!");

        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }

    if (Config::Instance()->OverrideSharpness.value_or(false))
    {
        params.enableSharpening = Config::Instance()->Sharpness.value_or(0.3) > 0.0f;
        params.sharpness = Config::Instance()->Sharpness.value_or(0.3);
    }
    else
    {
        float shapness = 0.0f;
        if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &shapness) == NVSDK_NGX_Result_Success)
        {
            _sharpness = shapness;

            params.enableSharpening = shapness > 0.0f;

            if (params.enableSharpening)
            {
                if (shapness > 1.0f)
                    params.sharpness = 1.0f;
                else
                    params.sharpness = shapness;
            }
        }
    }

    if (IsDepthInverted())
    {
        params.cameraFar = Config::Instance()->FsrCameraNear.value_or(0.0001f);
        params.cameraNear = Config::Instance()->FsrCameraFar.value_or(0.9999f);
    }
    else
    {
        params.cameraFar = Config::Instance()->FsrCameraFar.value_or(0.9999f);
        params.cameraNear = Config::Instance()->FsrCameraNear.value_or(0.0001f);
    }

    if (Config::Instance()->FsrVerticalFov.has_value())
        params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
    else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
        params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)TargetHeight() * (float)TargetWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float)GetDeltaTime();

    params.preExposure = 1.0f;

    spdlog::debug("FSR31FeatureDx11on12::Evaluate Dispatch!!");
    auto ffxresult = _dispatch(&_context, &params.header);

    if (ffxresult != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureDx11on12::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(ffxresult));

        Dx12CommandList->Close();
        ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
        Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

        Dx12CommandAllocator->Reset();
        Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

        return false;
    }

    // apply rcas
    if (Config::Instance()->RcasEnabled.value_or(false) &&
        (sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
        RCAS->CanRender())
    {
        spdlog::debug("XeSSFeatureDx11::Evaluate Apply CAS");
        if (params.output.resource != RCAS->Buffer())
            ResourceBarrier(Dx12CommandList, (ID3D12Resource*)params.output.resource,
                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        RCAS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        RcasConstants rcasConstants{};

        rcasConstants.Sharpness = sharpness;
        rcasConstants.DisplayWidth = TargetWidth();
        rcasConstants.DisplayHeight = TargetHeight();
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
        rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
        rcasConstants.RenderHeight = RenderHeight();
        rcasConstants.RenderWidth = RenderWidth();

        if (useSS)
        {
            if (!RCAS->Dispatch(Dx12Device, Dx12CommandList, (ID3D12Resource*)params.output.resource,
                (ID3D12Resource*)params.motionVectors.resource, rcasConstants, OutputScaler->Buffer()))
            {
                Config::Instance()->RcasEnabled = false;

                Dx12CommandList->Close();
                ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
                Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

                Dx12CommandAllocator->Reset();
                Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

                return true;
            }
        }
        else
        {
            if (!RCAS->Dispatch(Dx12Device, Dx12CommandList, (ID3D12Resource*)params.output.resource, (
                ID3D12Resource*)params.motionVectors.resource, rcasConstants, dx11Out.Dx12Resource))
            {
                Config::Instance()->RcasEnabled = false;

                Dx12CommandList->Close();
                ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
                Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

                Dx12CommandAllocator->Reset();
                Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

                return true;
            }
        }
    }

    if (useSS)
    {
        spdlog::debug("FSR31FeatureDx11on12::Evaluate downscaling output...");
        OutputScaler->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        if (!OutputScaler->Dispatch(Dx12Device, Dx12CommandList, OutputScaler->Buffer(), dx11Out.Dx12Resource))
        {
            Config::Instance()->OutputScalingEnabled = false;
            Config::Instance()->changeBackend = true;

            Dx12CommandList->Close();
            ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
            Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

            Dx12CommandAllocator->Reset();
            Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

            return true;
        }
    }

    if (!Config::Instance()->SyncAfterDx12.value_or(true))
    {
        if (!CopyBackOutput())
        {
            spdlog::error("FSR31FeatureDx11on12::Evaluate Can't copy output texture back!");

            Dx12CommandList->Close();
            ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
            Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

            Dx12CommandAllocator->Reset();
            Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

            return false;
        }

        // imgui
        if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
        {
            if (Imgui != nullptr && Imgui.get() != nullptr)
            {
                spdlog::debug("FSR31FeatureDx11on12::Evaluate Apply Imgui");

                if (Imgui->IsHandleDifferent())
                {
                    spdlog::debug("FSR31FeatureDx11on12::Evaluate Imgui handle different, reset()!");
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    Imgui.reset();
                }
                else
                    Imgui->Render(InDeviceContext, paramOutput[_frameCount % 2]);
            }
            else
            {
                if (Imgui == nullptr || Imgui.get() == nullptr)
                    Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
            }
        }
    }

    // Execute dx12 commands to process fsr
    Dx12CommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
    Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

    if (Config::Instance()->SyncAfterDx12.value_or(true))
    {
        if (!CopyBackOutput())
        {
            spdlog::error("FSR31FeatureDx11on12::Evaluate Can't copy output texture back!");

            Dx12CommandList->Close();
            ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
            Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

            Dx12CommandAllocator->Reset();
            Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

            return false;
        }

        // imgui
        if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
        {
            if (Imgui != nullptr && Imgui.get() != nullptr)
            {
                spdlog::debug("FSR31FeatureDx11on12::Evaluate Apply Imgui");

                if (Imgui->IsHandleDifferent())
                {
                    spdlog::debug("FSR31FeatureDx11on12::Evaluate Imgui handle different, reset()!");
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    Imgui.reset();
                }
                else
                    Imgui->Render(InDeviceContext, paramOutput[_frameCount % 2]);
            }
            else
            {
                if (Imgui == nullptr || Imgui.get() == nullptr)
                    Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
            }
        }
    }

    _frameCount++;

    Dx12CommandAllocator->Reset();
    Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

    return true;
}

FSR31FeatureDx11on12::~FSR31FeatureDx11on12()
{
}

bool FSR31FeatureDx11on12::InitFSR3(const NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureDx11on12::InitFSR2");

    if (!ModuleLoaded())
        return false;

    if (IsInited())
        return true;

    if (Device == nullptr)
    {
        spdlog::error("FSR31FeatureDx11on12::InitFSR2 D3D12Device is null!");
        return false;
    }

    Config::Instance()->dxgiSkipSpoofing = true;

    ffxQueryDescGetVersions versionQuery{};
    versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    //versionQuery.device = Dx12Device; // only for DirectX 12 applications
    uint64_t versionCount = 0;
    versionQuery.outputCount = &versionCount;
    // get number of versions for allocation
    _query(nullptr, &versionQuery.header);

    Config::Instance()->fsr3xVersionIds.resize(versionCount);
    Config::Instance()->fsr3xVersionNames.resize(versionCount);
    versionQuery.versionIds = Config::Instance()->fsr3xVersionIds.data();
    versionQuery.versionNames = Config::Instance()->fsr3xVersionNames.data();
    // fill version ids and names arrays.
    _query(nullptr, &versionQuery.header);


    _contextDesc.flags = 0;
    _contextDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;

    _contextDesc.fpMessage = FfxLogCallback;

    unsigned int featureFlags = 0;
    if (!_initFlagsReady)
    {
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);
        _initFlags = featureFlags;
        _initFlagsReady = true;
    }
    else
        featureFlags = _initFlags;

    bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted = false;

    if (Config::Instance()->AutoExposure.value_or(AutoExposure))
    {
        Config::Instance()->AutoExposure = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->AutoExposure = false;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR = false;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation = false;

    if (Config::Instance()->DisplayResolution.value_or(!LowRes))
    {
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
    }
    else
    {
        spdlog::info("FSR31FeatureDx11on12::InitFSR2 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
    {
        float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or(1.5f);

        if (ssMulti < 0.5f)
        {
            ssMulti = 0.5f;
            Config::Instance()->OutputScalingMultiplier = ssMulti;
        }
        else if (ssMulti > 3.0f)
        {
            ssMulti = 3.0f;
            Config::Instance()->OutputScalingMultiplier = ssMulti;
        }

        _targetWidth = DisplayWidth() * ssMulti;
        _targetHeight = DisplayHeight() * ssMulti;
    }
    else
    {
        _targetWidth = DisplayWidth();
        _targetHeight = DisplayHeight();
    }

    _contextDesc.maxRenderSize.width = TargetWidth();
    _contextDesc.maxRenderSize.height = TargetHeight();
    _contextDesc.maxUpscaleSize.width = TargetWidth();
    _contextDesc.maxUpscaleSize.height = TargetHeight();

    ffxCreateBackendDX12Desc backendDesc = { 0 };
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
    backendDesc.device = Dx12Device;
    _contextDesc.header.pNext = &backendDesc.header;

    if (Config::Instance()->Fsr3xIndex.value_or(0) < 0 || Config::Instance()->Fsr3xIndex.value_or(0) >= Config::Instance()->fsr3xVersionIds.size())
        Config::Instance()->Fsr3xIndex = 0;

    ffxOverrideVersion ov = { 0 };
    ov.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
    ov.versionId = Config::Instance()->fsr3xVersionIds[Config::Instance()->Fsr3xIndex.value_or(0)];
    backendDesc.header.pNext = &ov.header;

    spdlog::debug("FSR31FeatureDx11on12::InitFSR2 _createContext!");
    auto ret = _createContext(&_context, &_contextDesc.header, NULL);

    if (ret != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureDx11on12::InitFSR2 _createContext error: {0}", ResultToString(ret));
        return false;
    }

    auto version = Config::Instance()->fsr3xVersionNames[Config::Instance()->Fsr3xIndex.value_or(0)];
    _name = std::format("FSR {}", version);
    parse_version(version);

    spdlog::trace("FSR31FeatureDx11on12::InitFSR2 sleeping after _createContext creation for 500ms");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Config::Instance()->dxgiSkipSpoofing = false;

    SetInit(true);

    return true;
}