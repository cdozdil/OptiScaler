#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "FSR2Feature_Dx11.h"

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; dest.Height = src.Height; dest.Format = src.Format; dest.BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);	

bool FSR2FeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    Device = InDevice;
    DeviceContext = InContext;

    if (InitFSR2(InParameters))
    {
        if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
            Imgui = std::make_unique<Imgui_Dx11>(Util::GetProcessWindow(), Device);

        OutputScaler = std::make_unique<OS_Dx11>("Output Scaling", InDevice, (TargetWidth() < DisplayWidth()));
        RCAS = std::make_unique<RCAS_Dx11>("RCAS", InDevice);
        Bias = std::make_unique<Bias_Dx11>("Bias", InDevice);

        return true;
    }

    return false;
}

bool FSR2FeatureDx11::CopyTexture(ID3D11Resource* InResource, D3D11_TEXTURE2D_RESOURCE_C* OutTextureRes, UINT bindFlags, bool InCopy)
{
    ID3D11Texture2D* originalTexture = nullptr;
    D3D11_TEXTURE2D_DESC desc{};

    auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

    if (result != S_OK)
        return false;

    originalTexture->GetDesc(&desc);

    if (desc.BindFlags == bindFlags)
    {
        ASSIGN_DESC(OutTextureRes->Desc, desc);
        OutTextureRes->Texture = originalTexture;
        OutTextureRes->usingOriginal = true;
        return true;
    }

    if (OutTextureRes->usingOriginal || OutTextureRes->Texture == nullptr ||
        desc.Width != OutTextureRes->Desc.Width || desc.Height != OutTextureRes->Desc.Height ||
        desc.Format != OutTextureRes->Desc.Format || desc.BindFlags != OutTextureRes->Desc.BindFlags)
    {
        if (OutTextureRes->Texture != nullptr)
        {
            if (!OutTextureRes->usingOriginal)
                OutTextureRes->Texture->Release();
            else
                OutTextureRes->Texture = nullptr;
        }

        OutTextureRes->usingOriginal = false;
        ASSIGN_DESC(OutTextureRes->Desc, desc);

        desc.BindFlags = bindFlags;

        result = Device->CreateTexture2D(&desc, nullptr, &OutTextureRes->Texture);

        if (result != S_OK)
        {
            LOG_ERROR("CreateTexture2D error: {0:x}", result);
            return false;
        }
    }

    if (InCopy)
        DeviceContext->CopyResource(OutTextureRes->Texture, InResource);

    return true;
}

void FSR2FeatureDx11::ReleaseResources()
{
    LOG_FUNC();

    if (!bufferColor.usingOriginal)
    {
        SAFE_RELEASE(bufferColor.Texture);
    }
}

bool FSR2FeatureDx11::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    if (Device == nullptr)
    {
        LOG_ERROR("D3D12Device is null!");
        return false;
    }

    Config::Instance()->dxgiSkipSpoofing = true;

    const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeDX11();
    void* scratchBuffer = calloc(scratchBufferSize, 1);

    auto errorCode = ffxFsr2GetInterfaceDX11(&_contextDesc.callbacks, Device, scratchBuffer, scratchBufferSize);

    if (errorCode != FFX_OK)
    {
        LOG_ERROR("ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
        free(scratchBuffer);
        return false;
    }

    _contextDesc.device = ffxGetDeviceDX11(Device);

    _contextDesc.flags = 0;

    unsigned int featureFlags = 0;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

    _initFlags = featureFlags;

    bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted = true;
        _contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        LOG_INFO("contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted = false;

    if (Config::Instance()->AutoExposure.value_or(AutoExposure))
    {
        Config::Instance()->AutoExposure = true;
        _contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;
        LOG_INFO("contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->AutoExposure = false;
        LOG_INFO("contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR = true;
        _contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
        LOG_INFO("contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR = false;
        LOG_INFO("contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation = true;
        _contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        LOG_INFO("contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation = false;

    if (Config::Instance()->DisplayResolution.value_or(!LowRes))
    {
        _contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        LOG_INFO("contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
    }
    else
    {
        LOG_INFO("contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
    }

    // output scaling
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

    // extended limits changes how resolution 
    if (Config::Instance()->ExtendedLimits.value_or(false) && RenderWidth() > DisplayWidth())
    {
        _contextDesc.maxRenderSize.width = RenderWidth();
        _contextDesc.maxRenderSize.height = RenderHeight();

        Config::Instance()->OutputScalingMultiplier = 1.0f;

        // if output scaling active let it to handle downsampling
        if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
        {

            _contextDesc.displaySize.width = _contextDesc.maxRenderSize.width;
            _contextDesc.displaySize.height = _contextDesc.maxRenderSize.height;

            // update target res
            _targetWidth = _contextDesc.maxRenderSize.width;
            _targetHeight = _contextDesc.maxRenderSize.height;
        }
        else
        {
            _contextDesc.displaySize.width = DisplayWidth();
            _contextDesc.displaySize.height = DisplayHeight();
        }
    }
    else
    {
        _contextDesc.maxRenderSize.width = TargetWidth() > DisplayWidth() ? TargetWidth() : DisplayWidth();
        _contextDesc.maxRenderSize.height = TargetHeight() > DisplayHeight() ? TargetHeight() : DisplayHeight();
        _contextDesc.displaySize.width = TargetWidth();
        _contextDesc.displaySize.height = TargetHeight();
    }

#if _DEBUG
    _contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
    _contextDesc.fpMessage = FfxLogCallback;
#endif

    LOG_DEBUG("ffxFsr2ContextCreate!");

    errorCode = ffxFsr2ContextCreate(&_context, &_contextDesc);

    if (errorCode != FFX_OK)
    {
        LOG_ERROR("ffxFsr2ContextCreate error: {0}", ResultToString(errorCode));
        return false;
    }

    Config::Instance()->dxgiSkipSpoofing = false;

    SetInit(true);

    return true;
}

FSR2FeatureDx11::FSR2FeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), FSR2Feature(InHandleId, InParameters)
{
}

bool FSR2FeatureDx11::Evaluate(ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (!IsInited())
        return false;

    if (!RCAS->IsInit())
        Config::Instance()->RcasEnabled = false;

    FfxFsr2DispatchDescription params{};

    params.commandList = InContext;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    LOG_DEBUG("Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

    if (Config::Instance()->OverrideSharpness.value_or(false))
        _sharpness = Config::Instance()->Sharpness.value_or(0.3);
    else 
        _sharpness = GetSharpness(InParameters);

    if (Config::Instance()->RcasEnabled.value_or(false))
    {
        params.enableSharpening = false;
        params.sharpness = 0.0f;
    }
    else
    {
        params.enableSharpening = _sharpness > 0.0f;
        params.sharpness = _sharpness;
    }

    ID3D11Resource* paramColor;
    if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

    if (paramColor)
    {
        LOG_DEBUG("Color exist..");

        if (!CopyTexture(paramColor, &bufferColor, 40, true))
        {
            LOG_DEBUG("Can't copy Color!");
            return false;
        }

        if (bufferColor.Texture != nullptr)
            params.color = ffxGetResourceDX11(&_context, bufferColor.Texture, (wchar_t*)L"FSR2_Color");
        else
            params.color = ffxGetResourceDX11(&_context, paramColor, (wchar_t*)L"FSR2_Color");
    }
    else
    {
        LOG_ERROR("Color not exist!!");
        return false;
    }

    ID3D11Resource* paramVelocity;
    if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

    if (paramVelocity)
    {
        LOG_DEBUG("MotionVectors exist..");
        params.motionVectors = ffxGetResourceDX11(&_context, paramVelocity, (wchar_t*)L"FSR2_MotionVectors");

        if (!Config::Instance()->DisplayResolution.has_value())
        {
            D3D11_TEXTURE2D_DESC desc;
            ((ID3D11Texture2D*)paramVelocity)->GetDesc(&desc);
            bool lowResMV = desc.Width < TargetWidth();
            bool displaySizeEnabled = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);

            if (displaySizeEnabled && lowResMV)
            {
                LOG_WARN("MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, DisplayWidth(), displaySizeEnabled);
                Config::Instance()->DisplayResolution = false;
                Config::Instance()->changeBackend = true;
                return true;
            }

            Config::Instance()->DisplayResolution = displaySizeEnabled;
        }
    }
    else
    {
        LOG_ERROR("MotionVectors not exist!!");
        return false;
    }

    ID3D11Resource* paramOutput;
    if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

    if (paramOutput)
    {
        LOG_DEBUG("Output exist..");

        if (useSS)
        {
            if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight()))
            {
                params.output = ffxGetResourceDX11(&_context, OutputScaler->Buffer(), (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
            }
            else
                params.output = ffxGetResourceDX11(&_context, paramOutput, (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
        else
            params.output = ffxGetResourceDX11(&_context, paramOutput, (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

        if (Config::Instance()->RcasEnabled.value_or(false) &&
            (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
            RCAS != nullptr && RCAS.get() != nullptr && RCAS->IsInit() && RCAS->CreateBufferResource(Device, (ID3D11Texture2D*)params.output.resource))
        {
            params.output = ffxGetResourceDX11(&_context, RCAS->Buffer(), (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
    }
    else
    {
        LOG_ERROR("Output not exist!!");
        return false;
    }

    ID3D11Resource* paramDepth;
    if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

    if (paramDepth)
    {
        LOG_DEBUG("Depth exist..");
        params.depth = ffxGetResourceDX11(&_context, paramDepth, (wchar_t*)L"FSR2_Depth");
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false))
            LOG_ERROR("Depth not exist!!");
        else
            LOG_INFO("Using high res motion vectors, depth is not needed!!");
    }

    ID3D11Resource* paramExp = nullptr;
    if (!Config::Instance()->AutoExposure.value_or(false))
    {
        if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

        if (paramExp)
        {
            params.exposure = ffxGetResourceDX11(&_context, paramExp, (wchar_t*)L"FSR2_Exposure");
            LOG_DEBUG("ExposureTexture exist..");
        }
        else
        {
            LOG_DEBUG("AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            Config::Instance()->AutoExposure = true;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }
    else
        LOG_DEBUG("AutoExposure enabled!");

    ID3D11Resource* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramReactiveMask);

    if (!Config::Instance()->DisableReactiveMask.value_or(paramReactiveMask == nullptr))
    {
        if (paramReactiveMask)
        {
            LOG_DEBUG("Input Bias mask exist..");
            Config::Instance()->DisableReactiveMask = false;

            if (Config::Instance()->FsrUseMaskForTransparency.value_or(true))
                params.transparencyAndComposition = ffxGetResourceDX11(&_context, paramReactiveMask, (wchar_t*)L"FSR2_Transparency", FFX_RESOURCE_STATE_COMPUTE_READ);

            if (Config::Instance()->DlssReactiveMaskBias.value_or(0.45f) > 0.0f &&
                Bias->IsInit() && Bias->CreateBufferResource(Device, paramReactiveMask) && Bias->CanRender())
            {
                if (Bias->Dispatch(Device, InContext, (ID3D11Texture2D*)paramReactiveMask, Config::Instance()->DlssReactiveMaskBias.value_or(0.45f), Bias->Buffer()))
                {
                    params.reactive = ffxGetResourceDX11(&_context, Bias->Buffer(), (wchar_t*)L"FSR2_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);
                }
            }
            else
            {
                LOG_DEBUG("Skipping reactive mask, Bias: {0}, Bias Init: {1}, Bias CanRender: {2}",
                          Config::Instance()->DlssReactiveMaskBias.value_or(0.45f), Bias->IsInit(), Bias->CanRender());
            }
        }
    }

    _hasColor = params.color.resource != nullptr;
    _hasDepth = params.depth.resource != nullptr;
    _hasMV = params.motionVectors.resource != nullptr;
    _hasExposure = params.exposure.resource != nullptr;
    _hasTM = params.transparencyAndComposition.resource != nullptr;
    _accessToReactiveMask = paramReactiveMask != nullptr;
    _hasOutput = params.output.resource != nullptr;

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
        LOG_WARN("Can't get motion vector scales!");

        params.motionVectorScale.x = MVScaleX;
        params.motionVectorScale.y = MVScaleY;
    }

    if (IsDepthInverted())
    {
        params.cameraFar = Config::Instance()->FsrCameraNear.value_or(0.01f);
        params.cameraNear = Config::Instance()->FsrCameraFar.value_or(0.99f);
    }
    else
    {
        params.cameraFar = Config::Instance()->FsrCameraFar.value_or(0.99f);
        params.cameraNear = Config::Instance()->FsrCameraNear.value_or(0.01f);
    }

    if (Config::Instance()->FsrVerticalFov.has_value())
        params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
    else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
        params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float)GetDeltaTime();

    params.preExposure = 1.0f;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure);

    LOG_DEBUG("Dispatch!!");
    auto result = ffxFsr2ContextDispatch(&_context, &params);

    if (result != FFX_OK)
    {
        LOG_ERROR("ffxFsr2ContextDispatch error: {0}", ResultToString(result));
        return false;
    }

    // apply rcas
    if (Config::Instance()->RcasEnabled.value_or(false) &&
        (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
        RCAS != nullptr && RCAS.get() != nullptr && RCAS->CanRender())
    {
        RcasConstants rcasConstants{};

        rcasConstants.Sharpness = _sharpness;
        rcasConstants.DisplayWidth = TargetWidth();
        rcasConstants.DisplayHeight = TargetHeight();
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
        rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
        rcasConstants.RenderHeight = RenderHeight();
        rcasConstants.RenderWidth = RenderWidth();


        if (useSS)
        {
            if (!RCAS->Dispatch(Device, InContext, (ID3D11Texture2D*)params.output.resource, (ID3D11Texture2D*)params.motionVectors.resource,
                rcasConstants, OutputScaler->Buffer()))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
        else
        {
            if (!RCAS->Dispatch(Device, InContext, (ID3D11Texture2D*)params.output.resource, (ID3D11Texture2D*)params.motionVectors.resource,
                rcasConstants, (ID3D11Texture2D*)paramOutput))
            {
                Config::Instance()->RcasEnabled = false;
                return true;
            }
        }
    }

    if (useSS)
    {
        LOG_DEBUG("scaling output...");
        if (!OutputScaler->Dispatch(Device, InContext, OutputScaler->Buffer(), (ID3D11Texture2D*)paramOutput))
        {
            Config::Instance()->OutputScalingEnabled = false;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }

    // imgui
    if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
    {
        if (Imgui != nullptr && Imgui.get() != nullptr)
        {
            if (Imgui->IsHandleDifferent())
            {
                Imgui.reset();
            }
            else
                Imgui->Render(InContext, paramOutput);
        }
        else
        {
            if (Imgui == nullptr || Imgui.get() == nullptr)
                Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
        }
    }

    _frameCount++;

    return true;
}

FSR2FeatureDx11::~FSR2FeatureDx11()
{
}

