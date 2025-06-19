#include <pch.h>
#include <Config.h>
#include <Util.h>

#include "FSR2Feature_Dx11.h"

#include <magic_enum.hpp>

#define ASSIGN_DESC(dest, src)                                                                                         \
    dest.Width = src.Width;                                                                                            \
    dest.Height = src.Height;                                                                                          \
    dest.Format = src.Format;                                                                                          \
    dest.BindFlags = src.BindFlags;

#define SAFE_RELEASE(p)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (p && p != nullptr)                                                                                         \
        {                                                                                                              \
            (p)->Release();                                                                                            \
            (p) = nullptr;                                                                                             \
        }                                                                                                              \
    } while ((void) 0, 0);

static inline DXGI_FORMAT resolveTypelessFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case DXGI_FORMAT_R16G16_TYPELESS:
        return DXGI_FORMAT_R16G16_FLOAT;

    case DXGI_FORMAT_R32G32_TYPELESS:
        return DXGI_FORMAT_R32G32_FLOAT;

    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return DXGI_FORMAT_R8G8B8A8_UNORM;

    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return DXGI_FORMAT_R32G32_FLOAT;

    case DXGI_FORMAT_R32_TYPELESS:
        return DXGI_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R8G8_TYPELESS:
        return DXGI_FORMAT_R8G8_UNORM;

    case DXGI_FORMAT_R16_TYPELESS:
        return DXGI_FORMAT_R16_FLOAT;

    case DXGI_FORMAT_R8_TYPELESS:
        return DXGI_FORMAT_R8_UNORM;

    case DXGI_FORMAT_R24G8_TYPELESS:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    default:
        return format; // Already typed or unknown
    }
}

bool FSR2FeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    Device = InDevice;
    DeviceContext = InContext;

    if (InitFSR2(InParameters))
    {
        if (!Config::Instance()->OverlayMenu.value_or_default() && (Imgui == nullptr || Imgui.get() == nullptr))
            Imgui = std::make_unique<Menu_Dx11>(Util::GetProcessWindow(), Device);

        OutputScaler = std::make_unique<OS_Dx11>("Output Scaling", InDevice, (TargetWidth() < DisplayWidth()));
        RCAS = std::make_unique<RCAS_Dx11>("RCAS", InDevice);
        Bias = std::make_unique<Bias_Dx11>("Bias", InDevice);

        return true;
    }

    return false;
}

bool FSR2FeatureDx11::CopyTexture(ID3D11Resource* InResource, D3D11_TEXTURE2D_RESOURCE_C* OutTextureRes, UINT bindFlags,
                                  bool InCopy)
{
    ID3D11Texture2D* originalTexture = nullptr;
    D3D11_TEXTURE2D_DESC desc {};

    auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

    if (result != S_OK)
        return false;

    originalTexture->GetDesc(&desc);
    auto format = resolveTypelessFormat(desc.Format);

    if ((bindFlags == 9999 || desc.BindFlags == bindFlags) && desc.Format == format)
    {
        ASSIGN_DESC(OutTextureRes->Desc, desc);
        OutTextureRes->Texture = originalTexture;
        OutTextureRes->usingOriginal = true;
        return true;
    }

    if (OutTextureRes->usingOriginal || OutTextureRes->Texture == nullptr || desc.Width != OutTextureRes->Desc.Width ||
        desc.Height != OutTextureRes->Desc.Height || format != OutTextureRes->Desc.Format ||
        desc.BindFlags != OutTextureRes->Desc.BindFlags)
    {
        if (OutTextureRes->Texture != nullptr)
        {
            if (!OutTextureRes->usingOriginal)
                OutTextureRes->Texture->Release();
            else
                OutTextureRes->Texture = nullptr;
        }

        desc.Format = format;
        OutTextureRes->usingOriginal = false;
        ASSIGN_DESC(OutTextureRes->Desc, desc);

        if (bindFlags != 9999)
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
    if (!bufferColor.usingOriginal)
    {
        SAFE_RELEASE(bufferColor.Texture);
    }

    if (!bufferDepth.usingOriginal)
    {
        SAFE_RELEASE(bufferDepth.Texture);
    }

    if (!bufferExposure.usingOriginal)
    {
        SAFE_RELEASE(bufferExposure.Texture);
    }

    if (!bufferReactive.usingOriginal)
    {
        SAFE_RELEASE(bufferReactive.Texture);
    }

    if (!bufferVelocity.usingOriginal)
    {
        SAFE_RELEASE(bufferVelocity.Texture);
    }

    if (!bufferTransparency.usingOriginal)
    {
        SAFE_RELEASE(bufferTransparency.Texture);
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

    State::Instance().skipSpoofing = true;

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

    if (DepthInverted())
        _contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INVERTED;

    if (AutoExposure())
        _contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;

    if (IsHdr())
        _contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;

    if (JitteredMV())
        _contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;

    if (!LowResMV())
        _contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;

    // output scaling
    if (Config::Instance()->OutputScalingEnabled.value_or_default() && LowResMV())
    {
        float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or_default();

        if (ssMulti < 0.5f)
        {
            ssMulti = 0.5f;
            Config::Instance()->OutputScalingMultiplier.set_volatile_value(ssMulti);
        }
        else if (ssMulti > 3.0f)
        {
            ssMulti = 3.0f;
            Config::Instance()->OutputScalingMultiplier.set_volatile_value(ssMulti);
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
    if (Config::Instance()->ExtendedLimits.value_or_default() && RenderWidth() > DisplayWidth())
    {
        _contextDesc.maxRenderSize.width = RenderWidth();
        _contextDesc.maxRenderSize.height = RenderHeight();

        Config::Instance()->OutputScalingMultiplier.set_volatile_value(1.0f);

        // if output scaling active let it to handle downsampling
        if (Config::Instance()->OutputScalingEnabled.value_or_default() && LowResMV())
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

    State::Instance().skipSpoofing = false;

    SetInit(true);

    return true;
}

FSR2FeatureDx11::FSR2FeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters)
    : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), FSR2Feature(InHandleId, InParameters)
{
}

void FSR2FeatureDx11::LogResource(std::string name, ID3D11Texture2D* resource)
{
    if (_frameCount > 1)
        return;

    D3D11_TEXTURE2D_DESC desc {};
    resource->GetDesc(&desc);

    LOG_DEBUG("{}: {}x{}, Format: {}, Usage: {}, Bind: {:X}, Misc: {:X}, CPU: {:X}, ArraySize: {}, MipLevels: {}, "
              "SD.Count: {}, SD.Quality: {}",
              name, desc.Width, desc.Height, magic_enum::enum_name(desc.Format), magic_enum::enum_name(desc.Usage),
              desc.BindFlags, desc.MiscFlags, desc.CPUAccessFlags, desc.ArraySize, desc.MipLevels,
              desc.SampleDesc.Count, desc.SampleDesc.Quality);
}

void FSR2FeatureDx11::LogParams(FfxFsr2DispatchDescription* params)
{
    if (_frameCount > 1)
        return;

    LOG_DEBUG("Color: {}x{}, Format: {}", params->color.description.width, params->color.description.height,
              magic_enum::enum_name(params->color.description.format));
    LOG_DEBUG("Depth: {}x{}, Format: {}", params->depth.description.width, params->depth.description.height,
              magic_enum::enum_name(params->depth.description.format));
    LOG_DEBUG("Velocity: {}x{}, Format: {}", params->motionVectors.description.width,
              params->motionVectors.description.height,
              magic_enum::enum_name(params->motionVectors.description.format));
    LOG_DEBUG("Output: {}x{}, Format: {}", params->output.description.width, params->output.description.height,
              magic_enum::enum_name(params->output.description.format));
}

bool FSR2FeatureDx11::Evaluate(ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (!IsInited())
        return false;

    if (!RCAS->IsInit())
        Config::Instance()->RcasEnabled.set_volatile_value(false);

    ID3D11ShaderResourceView* restoreSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    ID3D11SamplerState* restoreSamplerStates[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
    ID3D11Buffer* restoreCBVs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
    ID3D11UnorderedAccessView* restoreUAVs[D3D11_1_UAV_SLOT_COUNT] = {};

    // backup compute shader resources
    for (UINT i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
    {
        restoreSRVs[i] = nullptr;
        InContext->CSGetShaderResources(i, 1, &restoreSRVs[i]);
    }

    for (UINT i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++)
    {
        restoreSamplerStates[i] = nullptr;
        InContext->CSGetSamplers(i, 1, &restoreSamplerStates[i]);
    }

    for (UINT i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; i++)
    {
        restoreCBVs[i] = nullptr;
        InContext->CSGetConstantBuffers(i, 1, &restoreCBVs[i]);
    }

    for (UINT i = 0; i < D3D11_1_UAV_SLOT_COUNT; i++)
    {
        restoreUAVs[i] = nullptr;
        InContext->CSGetUnorderedAccessViews(i, 1, &restoreUAVs[i]);
    }

    FfxFsr2DispatchDescription params {};
    params.commandList = InContext;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    LOG_DEBUG("Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    bool useSS = Config::Instance()->OutputScalingEnabled.value_or_default() && LowResMV();

    if (Config::Instance()->OverrideSharpness.value_or_default())
        _sharpness = Config::Instance()->Sharpness.value_or_default();
    else
        _sharpness = GetSharpness(InParameters);

    if (Config::Instance()->RcasEnabled.value_or_default())
    {
        params.enableSharpening = false;
        params.sharpness = 0.0f;
    }
    else
    {
        if (_sharpness > 1.0f)
            _sharpness = 1.0f;

        params.enableSharpening = _sharpness > 0.0f;
        params.sharpness = _sharpness;
    }

    ID3D11Resource* paramColor;
    if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Color, (void**) &paramColor);

    if (paramColor)
    {
        LOG_DEBUG("Color exist..");

        LogResource("Color", (ID3D11Texture2D*) paramColor);

        if (!CopyTexture(paramColor, &bufferColor, 40, true))
        {
            LOG_DEBUG("Can't copy Color!");
            return false;
        }

        if (bufferColor.Texture != nullptr)
            params.color = ffxGetResourceDX11(&_context, bufferColor.Texture, (wchar_t*) L"FSR2_Color");
        else
            params.color = ffxGetResourceDX11(&_context, paramColor, (wchar_t*) L"FSR2_Color");
    }
    else
    {
        LOG_ERROR("Color not exist!!");
        return false;
    }

    ID3D11Resource* paramVelocity;
    if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**) &paramVelocity);

    if (paramVelocity)
    {
        LogResource("Velocity", (ID3D11Texture2D*) paramVelocity);

        if (!CopyTexture(paramVelocity, &bufferVelocity, 9999, true))
        {
            LOG_DEBUG("Can't copy Velocity!");
            return false;
        }

        LOG_DEBUG("MotionVectors exist..");
        if (bufferVelocity.Texture != nullptr)
            params.motionVectors =
                ffxGetResourceDX11(&_context, bufferVelocity.Texture, (wchar_t*) L"FSR2_MotionVectors");
        else
            params.motionVectors = ffxGetResourceDX11(&_context, paramVelocity, (wchar_t*) L"FSR2_MotionVectors");
    }
    else
    {
        LOG_ERROR("MotionVectors not exist!!");
        return false;
    }

    auto outIndex = _frameCount % 2;

    ID3D11Resource* paramOutput;
    if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Output, (void**) &paramOutput);

    if (paramOutput)
    {
        LogResource("Output", (ID3D11Texture2D*) paramOutput);

        LOG_DEBUG("Output exist..");

        if (useSS)
        {
            if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight()))
            {
                params.output = ffxGetResourceDX11(&_context, OutputScaler->Buffer(), (wchar_t*) L"FSR2_Output",
                                                   FFX_RESOURCE_STATE_UNORDERED_ACCESS);
            }
            else
                params.output = ffxGetResourceDX11(&_context, paramOutput, (wchar_t*) L"FSR2_Output",
                                                   FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
        else
            params.output = ffxGetResourceDX11(&_context, paramOutput, (wchar_t*) L"FSR2_Output",
                                               FFX_RESOURCE_STATE_UNORDERED_ACCESS);

        if (Config::Instance()->RcasEnabled.value_or_default() &&
            (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or_default() &&
                                   Config::Instance()->MotionSharpness.value_or_default() > 0.0f)) &&
            RCAS != nullptr && RCAS.get() != nullptr && RCAS->IsInit() &&
            RCAS->CreateBufferResource(Device, (ID3D11Texture2D*) params.output.resource))
        {
            params.output = ffxGetResourceDX11(&_context, RCAS->Buffer(), (wchar_t*) L"FSR2_Output",
                                               FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        }
    }
    else
    {
        LOG_ERROR("Output not exist!!");
        return false;
    }

    ID3D11Resource* paramDepth;
    if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**) &paramDepth);

    if (paramDepth)
    {
        LOG_DEBUG("Depth exist..");
        auto depthTexture = (ID3D11Texture2D*) paramDepth;
        LogResource("Depth", depthTexture);

        D3D11_TEXTURE2D_DESC desc {};
        depthTexture->GetDesc(&desc);

        if (desc.Format == DXGI_FORMAT_R24G8_TYPELESS || desc.Format == DXGI_FORMAT_R32G8X24_TYPELESS)
        {
            if (DT == nullptr || DT.get() == nullptr)
                DT = std::make_unique<DepthTransfer_Dx11>("DT", Device);

            if (DT->Buffer() == nullptr)
                DT->CreateBufferResource(Device, paramDepth);

            if (DT->CanRender() && DT->Dispatch(Device, DeviceContext, depthTexture, DT->Buffer()))
                params.depth = ffxGetResourceDX11(&_context, DT->Buffer(), (wchar_t*) L"FSR2_Depth");
            else
                params.depth = ffxGetResourceDX11(&_context, paramDepth, (wchar_t*) L"FSR2_Depth");
        }
        else
        {
            if (!CopyTexture(paramDepth, &bufferDepth, 9999, true))
            {
                LOG_DEBUG("Can't copy Depth!");
                return false;
            }

            if (bufferDepth.Texture != nullptr)
                params.depth = ffxGetResourceDX11(&_context, bufferDepth.Texture, (wchar_t*) L"FSR2_Depth");
            else
                params.depth = ffxGetResourceDX11(&_context, paramDepth, (wchar_t*) L"FSR2_Depth");
        }
    }
    else
    {
        LOG_ERROR("Depth not exist!!");
        return false;
    }

    ID3D11Resource* paramExp = nullptr;
    if (AutoExposure())
    {
        LOG_DEBUG("AutoExposure enabled!");
    }
    else
    {
        if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**) &paramExp);

        if (paramExp)
        {
            LogResource("Exposure", (ID3D11Texture2D*) paramExp);

            if (!CopyTexture(paramExp, &bufferExposure, 9999, true))
            {
                LOG_DEBUG("Can't copy Exposure!");
                return false;
            }

            LOG_DEBUG("ExposureTexture exist..");
            if (bufferExposure.Texture != nullptr)
                params.exposure = ffxGetResourceDX11(&_context, bufferExposure.Texture, (wchar_t*) L"FSR2_Exposure");
            else
                params.exposure = ffxGetResourceDX11(&_context, paramExp, (wchar_t*) L"FSR2_Exposure");
        }
        else
        {
            LOG_DEBUG("AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            State::Instance().AutoExposure = true;
            State::Instance().changeBackend[Handle()->Id] = true;
            return true;
        }
    }

    ID3D11Resource* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) !=
        NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**) &paramReactiveMask);

    if (!Config::Instance()->DisableReactiveMask.value_or(paramReactiveMask == nullptr))
    {
        if (paramReactiveMask)
        {
            LogResource("Input Bias", (ID3D11Texture2D*) paramReactiveMask);

            LOG_DEBUG("Input Bias mask exist..");
            Config::Instance()->DisableReactiveMask.set_volatile_value(false);

            if (Config::Instance()->FsrUseMaskForTransparency.value_or_default())
            {
                if (!CopyTexture(paramReactiveMask, &bufferTransparency, 9999, true))
                {
                    LOG_DEBUG("Can't copy Exposure!");
                    return false;
                }

                if (bufferTransparency.Texture != nullptr)
                    params.transparencyAndComposition =
                        ffxGetResourceDX11(&_context, bufferTransparency.Texture, (wchar_t*) L"FSR2_Transparency",
                                           FFX_RESOURCE_STATE_COMPUTE_READ);
                else
                    params.transparencyAndComposition = ffxGetResourceDX11(
                        &_context, paramReactiveMask, (wchar_t*) L"FSR2_Transparency", FFX_RESOURCE_STATE_COMPUTE_READ);
            }

            if (Config::Instance()->DlssReactiveMaskBias.value_or_default() > 0.0f && Bias->IsInit() &&
                Bias->CreateBufferResource(Device, paramReactiveMask) && Bias->CanRender())
            {
                if (Bias->Dispatch(Device, InContext, (ID3D11Texture2D*) paramReactiveMask,
                                   Config::Instance()->DlssReactiveMaskBias.value_or_default(), Bias->Buffer()))
                {
                    params.reactive = ffxGetResourceDX11(&_context, Bias->Buffer(), (wchar_t*) L"FSR2_Reactive",
                                                         FFX_RESOURCE_STATE_COMPUTE_READ);
                }
            }
            else
            {
                LOG_DEBUG("Skipping reactive mask, Bias: {0}, Bias Init: {1}, Bias CanRender: {2}",
                          Config::Instance()->DlssReactiveMaskBias.value_or_default(), Bias->IsInit(),
                          Bias->CanRender());
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

    if (DepthInverted())
    {
        params.cameraFar = Config::Instance()->FsrCameraNear.value_or_default();
        params.cameraNear = Config::Instance()->FsrCameraFar.value_or_default();
    }
    else
    {
        params.cameraFar = Config::Instance()->FsrCameraFar.value_or_default();
        params.cameraNear = Config::Instance()->FsrCameraNear.value_or_default();
    }

    if (Config::Instance()->FsrVerticalFov.has_value())
        params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
    else if (Config::Instance()->FsrHorizontalFov.value_or_default() > 0.0f)
        params.cameraFovAngleVertical =
            2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) /
                        (float) DisplayHeight() * (float) DisplayWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) !=
            NVSDK_NGX_Result_Success ||
        params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float) GetDeltaTime();

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure) != NVSDK_NGX_Result_Success)
        params.preExposure = 1.0f;

    LogParams(&params);

    LOG_DEBUG("Dispatch!!");
    auto result = ffxFsr2ContextDispatch(&_context, &params);

    if (result != FFX_OK)
    {
        LOG_ERROR("ffxFsr2ContextDispatch error: {0}", ResultToString(result));
        return false;
    }

    // apply rcas
    if (Config::Instance()->RcasEnabled.value_or_default() &&
        (_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or_default() &&
                               Config::Instance()->MotionSharpness.value_or_default() > 0.0f)) &&
        RCAS != nullptr && RCAS.get() != nullptr && RCAS->CanRender())
    {
        RcasConstants rcasConstants {};

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
            if (!RCAS->Dispatch(Device, InContext, (ID3D11Texture2D*) params.output.resource,
                                (ID3D11Texture2D*) params.motionVectors.resource, rcasConstants,
                                OutputScaler->Buffer()))
            {
                Config::Instance()->RcasEnabled.set_volatile_value(false);
                return true;
            }
        }
        else
        {
            if (!RCAS->Dispatch(Device, InContext, (ID3D11Texture2D*) params.output.resource,
                                (ID3D11Texture2D*) params.motionVectors.resource, rcasConstants,
                                (ID3D11Texture2D*) paramOutput))
            {
                Config::Instance()->RcasEnabled.set_volatile_value(false);
                return true;
            }
        }
    }

    if (useSS)
    {
        LOG_DEBUG("scaling output...");
        if (!OutputScaler->Dispatch(Device, InContext, OutputScaler->Buffer(), (ID3D11Texture2D*) paramOutput))
        {
            Config::Instance()->OutputScalingEnabled.set_volatile_value(false);
            State::Instance().changeBackend[Handle()->Id] = true;
            return true;
        }
    }

    // imgui
    if (!Config::Instance()->OverlayMenu.value_or_default() && _frameCount > 30)
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
                Imgui = std::make_unique<Menu_Dx11>(GetForegroundWindow(), Device);
        }
    }

    // restore compute shader resources
    for (UINT i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
    {
        if (restoreSRVs[i] != nullptr)
            InContext->CSSetShaderResources(i, 1, &restoreSRVs[i]);
    }

    for (UINT i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++)
    {
        if (restoreSamplerStates[i] != nullptr)
            InContext->CSSetSamplers(i, 1, &restoreSamplerStates[i]);
    }

    for (UINT i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; i++)
    {
        if (restoreCBVs[i] != nullptr)
            InContext->CSSetConstantBuffers(i, 1, &restoreCBVs[i]);
    }

    for (UINT i = 0; i < D3D11_1_UAV_SLOT_COUNT; i++)
    {
        if (restoreUAVs[i] != nullptr)
            InContext->CSSetUnorderedAccessViews(i, 1, &restoreUAVs[i], 0);
    }

    _frameCount++;

    return true;
}

FSR2FeatureDx11::~FSR2FeatureDx11() { ReleaseResources(); }
