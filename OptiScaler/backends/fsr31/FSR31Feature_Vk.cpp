#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "FSR31Feature_Vk.h"

#include "nvsdk_ngx_vk.h"

static inline FfxApiResourceDescription ffxApiGetImageResourceDescriptionVKLocal(NVSDK_NGX_Resource_VK* vkResource, bool uav = false)
{
    FfxApiResourceDescription resourceDescription = {};

    // This is valid
    if (vkResource->Resource.ImageViewInfo.Image == VK_NULL_HANDLE)
        return resourceDescription;

    // Set flags properly for resource registration
    resourceDescription.flags = FFX_API_RESOURCE_USAGE_READ_ONLY;

    // Unordered access use
    if (uav)
        resourceDescription.usage = FFX_API_RESOURCE_USAGE_UAV;

    resourceDescription.width = vkResource->Resource.ImageViewInfo.Width;
    resourceDescription.height = vkResource->Resource.ImageViewInfo.Height;
    resourceDescription.mipCount = vkResource->Resource.ImageViewInfo.SubresourceRange.levelCount;
    resourceDescription.format = ffxApiGetSurfaceFormatVK(vkResource->Resource.ImageViewInfo.Format);
    resourceDescription.depth = vkResource->Resource.ImageViewInfo.SubresourceRange.layerCount;
    resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;

    return resourceDescription;
}

FSR31FeatureVk::FSR31FeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR31Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
    spdlog::debug("FSR31FeatureVk::FSR31FeatureVk Loading amd_fidelityfx_vk.dll methods");

    _configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxConfigure");
    _createContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxCreateContext");
    _destroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxDestroyContext");
    _dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxDispatch");
    _query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxQuery");

    _moduleLoaded = _configure != nullptr;

    if (!_moduleLoaded)
    {
        auto file = Util::DllPath().parent_path() / "amd_fidelityfx_vk.dll";
        auto _dll = LoadLibrary(file.wstring().c_str());

        if (_dll != nullptr)
        {
            _configure = (PfnFfxConfigure)GetProcAddress(_dll, "ffxConfigure");
            _createContext = (PfnFfxCreateContext)GetProcAddress(_dll, "ffxCreateContext");
            _destroyContext = (PfnFfxDestroyContext)GetProcAddress(_dll, "ffxDestroyContext");
            _dispatch = (PfnFfxDispatch)GetProcAddress(_dll, "ffxDispatch");
            _query = (PfnFfxQuery)GetProcAddress(_dll, "ffxQuery");

            _moduleLoaded = _configure != nullptr;
        }
    }

    if (_moduleLoaded)
        spdlog::info("FSR31FeatureVk::FSR31FeatureVk amd_fidelityfx_vk.dll methods loaded!");
    else
        spdlog::error("FSR31FeatureVk::FSR31FeatureVk can't load amd_fidelityfx_vk.dll methods!");
}

bool FSR31FeatureVk::InitFSR3(const NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureVk::InitFSR3");

    if (!ModuleLoaded())
        return false;

    if (IsInited())
        return true;

    if (PhysicalDevice == nullptr)
    {
        spdlog::error("FSR31FeatureVk::InitFSR3 PhysicalDevice is null!");
        return false;
    }

    ffxQueryDescGetVersions versionQuery{};
    versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    //versionQuery.device = Device; // only for DirectX 12 applications
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

    _contextDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;

    _contextDesc.fpMessage = FfxLogCallback;

    unsigned int featureFlags;
    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

    bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
    bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
    bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
    bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
    bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
    bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    _initFlags = featureFlags;

    _contextDesc.flags = 0;
    _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;

    if (Config::Instance()->DepthInverted.value_or(DepthInverted))
    {
        Config::Instance()->DepthInverted = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DEPTH_INVERTED;
        SetDepthInverted(true);
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->DepthInverted = false;

    if (Config::Instance()->AutoExposure.value_or(AutoExposure))
    {
        Config::Instance()->AutoExposure = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->AutoExposure = false;
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->HDR.value_or(Hdr))
    {
        Config::Instance()->HDR = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE;
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
    }
    else
    {
        Config::Instance()->HDR = false;
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
    }

    if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
    {
        Config::Instance()->JitterCancellation = true;
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
        spdlog::info("FSR31FeatureVk::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
    }
    else
        Config::Instance()->JitterCancellation = false;

    if (Config::Instance()->DisplayResolution.value_or(!LowRes))
    {
        _contextDesc.flags |= FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
    }
    else
    {
        spdlog::info("FSR31FeatureVk::InitFSR3 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
    }

    _contextDesc.maxRenderSize.width = TargetWidth();
    _contextDesc.maxRenderSize.height = TargetHeight();
    _contextDesc.maxUpscaleSize.width = TargetWidth();
    _contextDesc.maxUpscaleSize.height = TargetHeight();

    ffxCreateBackendVKDesc backendDesc = { 0 };
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK;
    backendDesc.vkDevice = Device;
    backendDesc.vkPhysicalDevice = PhysicalDevice;

    if (GDPA == nullptr)
        backendDesc.vkDeviceProcAddr = vkGetDeviceProcAddr;
    else
        backendDesc.vkDeviceProcAddr = GDPA;

    _contextDesc.header.pNext = &backendDesc.header;

    if (Config::Instance()->Fsr3xIndex.value_or(0) < 0 || Config::Instance()->Fsr3xIndex.value_or(0) >= Config::Instance()->fsr3xVersionIds.size())
        Config::Instance()->Fsr3xIndex = 0;

    ffxOverrideVersion ov = { 0 };
    ov.header.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION;
    ov.versionId = Config::Instance()->fsr3xVersionIds[Config::Instance()->Fsr3xIndex.value_or(0)];
    backendDesc.header.pNext = &ov.header;

    spdlog::debug("FSR31FeatureVk::InitFSR3 _createContext!");
    auto ret = _createContext(&_context, &_contextDesc.header, NULL);

    if (ret != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureVk::InitFSR3 _createContext error: {0}", ResultToString(ret));
        return false;
    }

    auto version = Config::Instance()->fsr3xVersionNames[Config::Instance()->Fsr3xIndex.value_or(0)];
    _name = std::format("FSR {}", version);
    parse_version(version);


    SetInit(true);

    return true;
}

bool FSR31FeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("FSR31FeatureVk::Init");

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
    spdlog::debug("FSR31FeatureVk::Evaluate");

    if (!IsInited())
        return false;

    struct ffxDispatchDescUpscale params = { 0 };
    params.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;

    if (Config::Instance()->FsrDebugView.value_or(false))
        params.flags = FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW;

    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

    unsigned int reset;
    InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
    params.reset = (reset == 1);

    GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

    spdlog::debug("FSR31FeatureVk::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

    params.commandList = InCmdBuffer;

    void* paramColor;
    InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor);

    if (paramColor)
    {
        spdlog::debug("FSR31FeatureVk::Evaluate Color exist..");

        params.color = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Image,
                                           ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramColor),
                                           FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        spdlog::error("FSR31FeatureVk::Evaluate Color not exist!!");
        return false;
    }

    void* paramVelocity;
    InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity);

    if (paramVelocity)
    {
        spdlog::debug("FSR31FeatureVk::Evaluate MotionVectors exist..");

        params.motionVectors = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Image,
                                                   ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramVelocity),
                                                   FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        spdlog::error("FSR31FeatureVk::Evaluate MotionVectors not exist!!");
        return false;
    }

    void* paramOutput;
    InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);

    if (paramOutput)
    {
        spdlog::debug("FSR31FeatureVk::Evaluate Output exist..");

        params.output = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Image,
                                            ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramOutput, true),
                                            FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    else
    {
        spdlog::error("FSR31FeatureVk::Evaluate Output not exist!!");
        return false;
    }

    void* paramDepth;
    InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth);

    if (paramDepth)
    {
        spdlog::debug("FSR31FeatureVk::Evaluate Depth exist..");

        params.depth = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Image,
                                           ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramDepth),
                                           FFX_API_RESOURCE_STATE_COMPUTE_READ);
    }
    else
    {
        if (!Config::Instance()->DisplayResolution.value_or(false))
            spdlog::error("FSR31FeatureVk::Evaluate Depth not exist!!");
        else
            spdlog::info("FSR31FeatureVk::Evaluate Using high res motion vectors, depth is not needed!!");
    }

    void* paramExp = nullptr;
    if (!Config::Instance()->AutoExposure.value_or(false))
    {
        InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp);

        if (paramExp)
        {
            spdlog::debug("FSR31FeatureVk::Evaluate ExposureTexture exist..");

            params.exposure = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Image,
                                                  ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramExp),
                                                  FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            spdlog::debug("FSR31FeatureVk::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
            Config::Instance()->AutoExposure = true;
            Config::Instance()->changeBackend = true;
            return true;
        }
    }
    else
        spdlog::debug("FSR31FeatureVk::Evaluate AutoExposure enabled!");

    void* paramReactiveMask = nullptr;
    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask) != NVSDK_NGX_Result_Success)
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramReactiveMask);

    if (!Config::Instance()->DisableReactiveMask.value_or(true))
    {
        InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramReactiveMask);

        if (paramReactiveMask)
        {
            spdlog::debug("FSR31FeatureVk::Evaluate Bias mask exist..");

            params.reactive = ffxApiGetResourceVK(((NVSDK_NGX_Resource_VK*)paramReactiveMask)->Resource.ImageViewInfo.Image,
                                                  ffxApiGetImageResourceDescriptionVKLocal((NVSDK_NGX_Resource_VK*)paramReactiveMask),
                                                  FFX_API_RESOURCE_STATE_COMPUTE_READ);
        }
        else
        {
            spdlog::debug("FSR31FeatureVk::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");
            Config::Instance()->DisableReactiveMask = true;
            Config::Instance()->changeBackend = true;
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
        spdlog::warn("FSR31FeatureVk::Evaluate Can't get motion vector scales!");

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

    params.preExposure = 1.0f;    InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure);


    spdlog::debug("FSR31FeatureVk::Evaluate Dispatch!!");
    auto result = _dispatch(&_context, &params.header);

    if (result != FFX_API_RETURN_OK)
    {
        spdlog::error("FSR31FeatureVk::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(result));
        return false;
    }

    _frameCount++;

    // restore resource states
    //if (paramColor && Config::Instance()->ColorResourceBarrier.value_or(false))
    //	ResourceBarrier(InCommandList, paramColor,
    //		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

    //if (paramVelocity && Config::Instance()->MVResourceBarrier.has_value())
    //	ResourceBarrier(InCommandList, paramVelocity,
    //		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

    //if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
    //	ResourceBarrier(InCommandList, paramOutput,
    //		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

    //if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
    //	ResourceBarrier(InCommandList, paramDepth,
    //		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

    //if (paramExp && Config::Instance()->ExposureResourceBarrier.has_value())
    //	ResourceBarrier(InCommandList, paramExp,
    //		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

    //if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
    //	ResourceBarrier(InCommandList, paramMask,
    //		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
    //		(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

    return true;
}
