#include <pch.h>
#include <Config.h>
#include <Util.h>
#include <proxies/FfxApi_Proxy.h>
#include "FSR31Feature_Vk.h"

#include "nvsdk_ngx_vk.h"

static inline uint32_t ffxApiGetSurfaceFormatVKLocal(VkFormat fmt)
{
    switch (fmt)
    {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R32G32B32_FLOAT;
        case VK_FORMAT_R32G32B32A32_UINT:
            return FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT;
        case VK_FORMAT_R32G32_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R32G32_FLOAT;
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return FFX_API_SURFACE_FORMAT_R32_UINT;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SNORM:
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM;
        case VK_FORMAT_R16G16_UNORM:
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R16G16_SNORM:
            return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R16G16_FLOAT;
        case VK_FORMAT_R16G16_UINT:
            return FFX_API_SURFACE_FORMAT_R16G16_UINT;
        case VK_FORMAT_R16G16_SINT:
            return FFX_API_SURFACE_FORMAT_R16G16_SINT;
        case VK_FORMAT_R16_SFLOAT:
            return FFX_API_SURFACE_FORMAT_R16_FLOAT;
        case VK_FORMAT_R16_UINT:
            return FFX_API_SURFACE_FORMAT_R16_UINT;
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return FFX_API_SURFACE_FORMAT_R16_UNORM;
        case VK_FORMAT_R16_SNORM:
            return FFX_API_SURFACE_FORMAT_R16_SNORM;
        case VK_FORMAT_R8_UNORM:
            return FFX_API_SURFACE_FORMAT_R8_UNORM;
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_S8_UINT:
            return FFX_API_SURFACE_FORMAT_R8_UINT;
        case VK_FORMAT_R8G8_UNORM:
            return FFX_API_SURFACE_FORMAT_R8G8_UNORM;
        case VK_FORMAT_R8G8_UINT:
            return FFX_API_SURFACE_FORMAT_R8G8_UINT;
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return FFX_API_SURFACE_FORMAT_R32_FLOAT;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            return FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
        case VK_FORMAT_UNDEFINED:
            return FFX_API_SURFACE_FORMAT_UNKNOWN;

        default:
            // NOTE: we do not support typeless formats here
            //FFX_ASSERT_MESSAGE(false, "Format not yet supported");
            return FFX_API_SURFACE_FORMAT_UNKNOWN;
    }
}

static inline FfxApiResourceDescription ffxApiGetImageResourceDescriptionVKLocal(NVSDK_NGX_Resource_VK* vkResource)
{
    FfxApiResourceDescription resourceDescription = {};

    // This is valid
    if (vkResource->Resource.ImageViewInfo.Image == VK_NULL_HANDLE)
        return resourceDescription;

    // Set flags properly for resource registration
    resourceDescription.flags = FFX_API_RESOURCE_USAGE_READ_ONLY;

    // Unordered access use
    if (vkResource->ReadWrite)
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_UAV;

    // depth use
    if ((vkResource->Resource.ImageViewInfo.SubresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) > 0)
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_DEPTHTARGET;

    if ((vkResource->Resource.ImageViewInfo.SubresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) > 0)
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_STENCILTARGET;

    resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
    resourceDescription.width = vkResource->Resource.ImageViewInfo.Width;
    resourceDescription.height = vkResource->Resource.ImageViewInfo.Height;
    resourceDescription.mipCount = 1;
    resourceDescription.depth = 1;
    resourceDescription.flags = FFX_API_RESOURCE_FLAGS_NONE;
    resourceDescription.format = ffxApiGetSurfaceFormatVKLocal(vkResource->Resource.ImageViewInfo.Format);

    return resourceDescription;
}


FSR31FeatureVk::FSR31FeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR31Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
    _moduleLoaded = FfxApiProxy::InitFfxVk();

    if (_moduleLoaded)
        LOG_INFO("amd_fidelityfx_vk.dll methods loaded!");
    else
        LOG_ERROR("Can't load amd_fidelityfx_vk.dll methods!");
}

bool FSR31FeatureVk::InitFSR3(const NVSDK_NGX_Parameter* InParameters)
{
    LOG_DEBUG("FSR31FeatureVk::InitFSR3");

    if (!ModuleLoaded())
        return false;

    if (IsInited())
        return true;

    if (PhysicalDevice == nullptr)
    {
        LOG_ERROR("PhysicalDevice is null!");
        return false;
    }

    State::Instance().skipSpoofing = true;

    ffxQueryDescGetVersions versionQuery{};
    versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    //versionQuery.device = Device; // only for DirectX 12 applications
    uint64_t versionCount = 0;
    versionQuery.outputCount = &versionCount;
    // get number of versions for allocation
    FfxApiProxy::VULKAN_Query()(nullptr, &versionQuery.header);

    State::Instance().fsr3xVersionIds.resize(versionCount);
    State::Instance().fsr3xVersionNames.resize(versionCount);
    versionQuery.versionIds = State::Instance().fsr3xVersionIds.data();
    versionQuery.versionNames = State::Instance().fsr3xVersionNames.data();
    // fill version ids and names arrays.
    FfxApiProxy::VULKAN_Query()(nullptr, &versionQuery.header);

    _contextDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    _contextDesc.fpMessage = FfxLogCallback;

    bool Hdr = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    _contextDesc.flags = 0;
    _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted.set_volatile_value(true);
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        LOG_INFO("contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted.set_volatile_value(false);

    if (Config::Instance()->AutoExposure.value_or(AutoExposure) || State::Instance().AutoExposure.value_or(false))
    {
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
        LOG_INFO("contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
    }
    else
    {
        LOG_INFO("contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR.set_volatile_value(true);
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
        LOG_INFO("contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR.set_volatile_value(false);
        LOG_INFO("contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation.set_volatile_value(true);
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        LOG_INFO("FSR31FeatureVk::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation.set_volatile_value(false);

    if (Config::Instance()->DisplayResolution.value_or(!LowRes || State::Instance().DisplaySizeMV.value_or(false)))
    {
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        LOG_INFO("contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
    }
    else
    {
        LOG_INFO("contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->ExtendedLimits.value_or_default())
    {
        _contextDesc.maxRenderSize.width = RenderWidth() < DisplayWidth() ? DisplayWidth() : RenderWidth();
        _contextDesc.maxRenderSize.height = RenderHeight() < DisplayHeight() ? DisplayHeight() : RenderHeight();
    }
    else
    {
        _contextDesc.maxRenderSize.width = DisplayWidth();
        _contextDesc.maxRenderSize.height = DisplayHeight();
    }

    _contextDesc.maxUpscaleSize.width = DisplayWidth();
    _contextDesc.maxUpscaleSize.height = DisplayHeight();

    ffxCreateBackendVKDesc backendDesc = { 0 };
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.vkDevice = Device;
    backendDesc.vkPhysicalDevice = PhysicalDevice;

    if (GDPA == nullptr)
        backendDesc.vkDeviceProcAddr = vkGetDeviceProcAddr;
    else
        backendDesc.vkDeviceProcAddr = GDPA;

    _contextDesc.header.pNext = &backendDesc.header;

    if (Config::Instance()->Fsr3xIndex.value_or_default() < 0 || Config::Instance()->Fsr3xIndex.value_or_default() >= State::Instance().fsr3xVersionIds.size())
        Config::Instance()->Fsr3xIndex.set_volatile_value(0);

    ffxOverrideVersion ov = { 0 };
    ov.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
    ov.versionId = State::Instance().fsr3xVersionIds[Config::Instance()->Fsr3xIndex.value_or_default()];
    backendDesc.header.pNext = &ov.header;

    LOG_DEBUG("_createContext!");
    auto ret = FfxApiProxy::VULKAN_CreateContext()(&_context, &_contextDesc.header, NULL);

    State::Instance().skipSpoofing = false;

    if (ret != FFX_API_RETURN_OK)
    {
        LOG_ERROR("_createContext error: {0}", FfxApiProxy::ReturnCodeToString(ret));
        return false;
    }

    auto version = State::Instance().fsr3xVersionNames[Config::Instance()->Fsr3xIndex.value_or_default()];
    _name = std::format("FSR {}", version);
    parse_version(version);


    SetInit(true);

    return true;
}

bool FSR31FeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (IsInited())
        return true;

    Instance = InInstance;
    PhysicalDevice = InPD;
    Device = InDevice;
    GIPA = InGIPA;
    GDPA = InGDPA;

    return InitFSR3(InParameters);
}

bool FSR31FeatureVk::Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (!IsInited())
        return false;

    struct ffxDispatchDescUpscale params = { 0 };
    params.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;

    if (Config::Instance()->FsrDebugView.value_or_default())
        params.flags = FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    LOG_DEBUG("Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    params.commandList = InCmdBuffer;

    void* paramColor;
    InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor);

    if (paramColor)
    {
        LOG_DEBUG("Color exist..");

        params.color = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Image,
                                           ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramColor),
                                           FFX_API_RESOURCE_STATE_COMPUTE_READ);
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

        params.motionVectors = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Image,
                                                   ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramVelocity),
                                                   FFX_API_RESOURCE_STATE_COMPUTE_READ);
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

        params.output = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Image,
                                            ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramOutput),
                                            FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
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

        params.depth = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Image,
                                           ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramDepth),
                                           FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false) && !State::Instance().DisplaySizeMV.value_or(false))
        {
            LOG_ERROR("Depth not exist!!");
            return false;
        }
    }

    void* paramExp = nullptr;
    if (Config::Instance()->AutoExposure.value_or(false) || State::Instance().AutoExposure.value_or(false))
    {
        LOG_DEBUG("AutoExposure enabled!");
    }
    else
    {
        InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp);

        if (paramExp)
        {
            LOG_DEBUG("ExposureTexture exist..");

            params.exposure = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Image,
                                                  ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramExp),
                                                  FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            LOG_DEBUG("AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            State::Instance().AutoExposure = true;
            State::Instance().changeBackend[Handle()->Id] = true;
            return true;
        }
    }

    void* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramReactiveMask);

    if (paramReactiveMask && Config::Instance()->FsrUseMaskForTransparency.value_or_default())
    {
        params.transparencyAndComposition = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Image,
                                                                ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramReactiveMask),
                                                                FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }

    if (!Config::Instance()->DisableReactiveMask.value_or(paramReactiveMask == nullptr))
    {
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask);

        if (paramReactiveMask)
        {
            LOG_DEBUG("Bias mask exist..");

            if (Config::Instance()->DlssReactiveMaskBias.value_or_default() > 0.0f)
            {
                params.reactive = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Image,
                                                      ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramReactiveMask),
                                                      FFX_API_RESOURCE_STATE_COMPUTE_READ);
            }
        }
        else
        {
            LOG_DEBUG("Bias mask not exist and its enabled in config, it may cause problems!!");
            Config::Instance()->DisableReactiveMask.set_volatile_value(true);
            State::Instance().changeBackend[Handle()->Id] = true;
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

    if (Config::Instance()->OverrideSharpness.value_or_default())
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
    else if (Config::Instance()->FsrHorizontalFov.value_or_default() > 0.0f)
        params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
    else
        params.cameraFovAngleVertical = 1.0471975511966f;

    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
        params.frameTimeDelta = (float)GetDeltaTime();

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure) != NVSDK_NGX_Result_Success)
        params.preExposure = 1.0f;

    if (isVersionOrBetter(Version(), { 1, 1, 1 }) && _velocity != Config::Instance()->FsrVelocity.value_or_default())
    {
        _velocity = Config::Instance()->FsrVelocity.value_or_default();
        ffxConfigureDescUpscaleKeyValue m_upscalerKeyValueConfig{};
        m_upscalerKeyValueConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_UPSCALE_KEYVALUE;
        m_upscalerKeyValueConfig.key = FFX_API_CONFIGURE_UPSCALE_KEY_FVELOCITYFACTOR;
        m_upscalerKeyValueConfig.ptr = &_velocity;
        auto result = FfxApiProxy::VULKAN_Configure()(&_context, &m_upscalerKeyValueConfig.header);

        if (result != FFX_API_RETURN_OK)
            LOG_WARN("Velocity configure result: {}", (UINT)result);
    }

    if (InParameters->Get("FSR.upscaleSize.width", &params.upscaleSize.width) == NVSDK_NGX_Result_Success && Config::Instance()->OutputScalingEnabled.value_or_default())
        params.upscaleSize.width *= Config::Instance()->OutputScalingMultiplier.value_or_default();

    if (InParameters->Get("FSR.upscaleSize.height", &params.upscaleSize.height) == NVSDK_NGX_Result_Success && Config::Instance()->OutputScalingEnabled.value_or_default())
        params.upscaleSize.height *= Config::Instance()->OutputScalingMultiplier.value_or_default();

    LOG_DEBUG("Dispatch!!");
    auto result = FfxApiProxy::VULKAN_Dispatch()(&_context, &params.header);

    if (result != FFX_API_RETURN_OK)
    {
        LOG_ERROR("ffxFsr2ContextDispatch error: {0}", FfxApiProxy::ReturnCodeToString(result));
        return false;
    }

    _frameCount++;

    return true;
}
