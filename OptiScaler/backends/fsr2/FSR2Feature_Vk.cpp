#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Vk.h"

#include "nvsdk_ngx_vk.h"

bool FSR2FeatureVk::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    if (PhysicalDevice == nullptr)
    {
        LOG_ERROR("PhysicalDevice is null!");
        return false;
    }

    State::Instance().skipSpoofing = true;

    auto scratchBufferSize = ffxFsr2GetScratchMemorySizeVK(PhysicalDevice);
    void* scratchBuffer = calloc(scratchBufferSize, 1);

    auto errorCode = ffxFsr2GetInterfaceVK(&_contextDesc.callbacks, scratchBuffer, scratchBufferSize, PhysicalDevice, vkGetDeviceProcAddr);

    if (errorCode != FFX_OK)
    {
        LOG_ERROR("ffxGetInterfaceVK error: {0}", ResultToString(errorCode));
        free(scratchBuffer);
        return false;
    }

    _contextDesc.device = ffxGetDeviceVK(Device);
    
    if (Config::Instance()->ExtendedLimits.value_or(false))
    {
        _contextDesc.maxRenderSize.width = RenderWidth() < DisplayWidth() ? DisplayWidth() : RenderWidth();
        _contextDesc.maxRenderSize.height = RenderHeight() < DisplayHeight() ? DisplayHeight() : RenderHeight();
    }
    else
    {
        _contextDesc.maxRenderSize.width = DisplayWidth();
        _contextDesc.maxRenderSize.height = DisplayHeight();
    }

    _contextDesc.displaySize.width = DisplayWidth();
    _contextDesc.displaySize.height = DisplayHeight();
    _contextDesc.flags = 0;

    bool Hdr = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

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

#if _DEBUG
    _contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
    _contextDesc.fpMessage = FfxLogCallback;
#endif

    LOG_DEBUG("ffxFsr2ContextCreate!");

    auto ret = ffxFsr2ContextCreate(&_context, &_contextDesc);

    State::Instance().skipSpoofing = false;

    if (ret != FFX_OK)
    {
        LOG_ERROR("ffxFsr2ContextCreate error: {0}", ResultToString(ret));
        return false;
    }

    SetInit(true);

    return true;
}

bool FSR2FeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    Instance = InInstance;
    PhysicalDevice = InPD;
    Device = InDevice;
    GIPA = InGIPA;
    GDPA = InGDPA;

    return InitFSR2(InParameters);
}

bool FSR2FeatureVk::Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (!IsInited())
        return false;

    FfxFsr2DispatchDescription params{};

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    LOG_DEBUG("Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    params.commandList = ffxGetCommandListVK(InCmdBuffer);

    void* paramColor;
    InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor);

    if (paramColor)
    {
        LOG_DEBUG("Color exist..");

        params.color = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Image,
                                               ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.ImageView,
                                               ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Height,
                                               ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Color", FFX_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        LOG_ERROR("Color not exist!!");
        return false;
    }

    void* paramVelocity;
    InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity);

    if (paramVelocity)
    {
        LOG_DEBUG("MotionVectors exist..");

        params.motionVectors = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Image,
                                                       ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.ImageView,
                                                       ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Height,
                                                       ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_MotionVectors", FFX_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        LOG_ERROR("MotionVectors not exist!!");
        return false;
    }

    void* paramOutput;
    InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);

    if (paramOutput)
    {
        LOG_DEBUG("Output exist..");

        params.output = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Image,
                                                ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.ImageView,
                                                ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Height,
                                                ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    else
    {
        LOG_ERROR("Output not exist!!");
        return false;
    }

    void* paramDepth;
    InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth);

    if (paramDepth)
    {
        LOG_DEBUG("Depth exist..");

        params.depth = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Image,
                                               ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.ImageView,
                                               ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Height,
                                               ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Depth", FFX_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false))
            LOG_ERROR("Depth not exist!!");
        else
            LOG_INFO("Using high res motion vectors, depth is not needed!!");
    }

    void* paramExp = nullptr;
    if (!Config::Instance()->AutoExposure.value_or(false))
    {
        InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp);

        if (paramExp)
        {
            LOG_DEBUG("ExposureTexture exist..");

            params.exposure = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Image,
                                                      ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.ImageView,
                                                      ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Height,
                                                      ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Exposure", FFX_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            LOG_DEBUG("AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            Config::Instance()->AutoExposure = true;
            State::Instance().changeBackend = true;
            return true;
        }
    }
    else
        LOG_DEBUG("AutoExposure enabled!");

    void* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramReactiveMask);

    if (paramReactiveMask && Config::Instance()->FsrUseMaskForTransparency.value_or(true))
    {
        params.transparencyAndComposition = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Image,
                                                                    ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.ImageView,
                                                                    ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Height,
                                                                    ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Transparency", FFX_RESOURCE_STATE_COMPUTE_READ);
    }

    if (!Config::Instance()->DisableReactiveMask.value_or(paramReactiveMask == nullptr))
    {
        if (paramReactiveMask)
        {
            LOG_DEBUG("Bias mask exist..");

            if (Config::Instance()->DlssReactiveMaskBias.value_or_default() > 0.0f)
            {
                params.reactive = ffxGetTextureResourceVK(&_context, ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Image,
                                                          ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.ImageView,
                                                          ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Height,
                                                          ((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);
            }
        }
        else
        {
            LOG_DEBUG("Bias mask not exist and its enabled in config, it may cause problems!!");
            Config::Instance()->DisableReactiveMask = true;
            State::Instance().changeBackend = true;
            return true;
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

    if (Config::Instance()->OverrideSharpness.value_or(false))
    {
        params.enableSharpening = Config::Instance()->Sharpness.value_or_default() > 0.0f;
        params.sharpness = Config::Instance()->Sharpness.value_or_default();
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

    _frameCount++;

    return true;
}
