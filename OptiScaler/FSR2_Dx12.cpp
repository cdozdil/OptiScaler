#include "FSR2_Dx12.h"

#include "Config.h"
#include "Util.h"

#include "resource.h"
#include "NVNGX_Parameter.h"

#include "detours/detours.h"
#include "fsr2_212/include/ffx_fsr2.h"
#include "fsr2_212/include/dx12/ffx_fsr2_dx12.h"

typedef struct FfxResource20 {
    void* resource;                               ///< pointer to the resource.
    Fsr212::FfxResourceDescription          description;
    Fsr212::FfxResourceStates               state;
    bool                            isDepth;
    uint64_t                        descriptorData;
} FfxResource;

typedef struct FfxFsr20DispatchDescription {

    Fsr212::FfxCommandList              commandList;                        ///< The <c><i>FfxCommandList</i></c> to record FSR2 rendering commands into.
    FfxResource20                 color;                              ///< A <c><i>FfxResource</i></c> containing the color buffer for the current frame (at render resolution).
    FfxResource20                 depth;                              ///< A <c><i>FfxResource</i></c> containing 32bit depth values for the current frame (at render resolution).
    FfxResource20                 motionVectors;                      ///< A <c><i>FfxResource</i></c> containing 2-dimensional motion vectors (at render resolution if <c><i>FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS</i></c> is not set).
    FfxResource20                 exposure;                           ///< A optional <c><i>FfxResource</i></c> containing a 1x1 exposure value.
    FfxResource20                 reactive;                           ///< A optional <c><i>FfxResource</i></c> containing alpha value of reactive objects in the scene.
    FfxResource20                 transparencyAndComposition;         ///< A optional <c><i>FfxResource</i></c> containing alpha value of special objects in the scene.
    FfxResource20                 output;                             ///< A <c><i>FfxResource</i></c> containing the output color buffer for the current frame (at presentation resolution).
    Fsr212::FfxFloatCoords2D            jitterOffset;                       ///< The subpixel jitter offset applied to the camera.
    Fsr212::FfxFloatCoords2D            motionVectorScale;                  ///< The scale factor to apply to motion vectors.
    Fsr212::FfxDimensions2D             renderSize;                         ///< The resolution that was used for rendering the input resources.
    bool                        enableSharpening;                   ///< Enable an additional sharpening pass.
    float                       sharpness;                          ///< The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness.
    float                       frameTimeDelta;                     ///< The time elapsed since the last frame (expressed in milliseconds).
    float                       preExposure;                        ///< The exposure value if not using <c><i>FFX_FSR2_ENABLE_AUTO_EXPOSURE</i></c>.
    bool                        reset;                              ///< A boolean value which when set to true, indicates the camera has moved discontinuously.
    float                       cameraNear;                         ///< The distance to the near plane of the camera.
    float                       cameraFar;                          ///< The distance to the far plane of the camera. This is used only used in case of non infinite depth.
    float                       cameraFovAngleVertical;             ///< The camera angle field of view in the vertical direction (expressed in radians).
} FfxFsr2DispatchDescription;

// FSR2
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2ContextCreate)(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2ContextDescription* contextDescription);
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2ContextDispatch)(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2DispatchDescription* dispatchDescription);
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr20ContextDispatch)(Fsr212::FfxFsr2Context* context, const FfxFsr20DispatchDescription* dispatchDescription);
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2ContextGenerateReactiveMask)(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2GenerateReactiveDescription* params);
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2ContextDestroy)(Fsr212::FfxFsr2Context* context);
typedef float(*PFN_ffxFsr2GetUpscaleRatioFromQualityMode)(Fsr212::FfxFsr2QualityMode qualityMode);
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2GetRenderResolutionFromQualityMode)(uint32_t* renderWidth, uint32_t* renderHeight, uint32_t displayWidth, uint32_t displayHeight, Fsr212::FfxFsr2QualityMode qualityMode);

// Dx12
typedef size_t(*PFN_ffxFsr2GetScratchMemorySizeDX12)();
typedef Fsr212::FfxErrorCode(*PFN_ffxFsr2GetInterfaceDX12)(Fsr212::FfxFsr2Interface212* fsr2Interface, ID3D12Device* device, void* scratchBuffer, size_t scratchBufferSize);
typedef FfxResource20(*PFN_ffxGetResource20DX12)(Fsr212::FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name, Fsr212::FfxResourceStates state, UINT shaderComponentMapping);
typedef Fsr212::FfxResource(*PFN_ffxGetResourceDX12)(Fsr212::FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name, Fsr212::FfxResourceStates state, UINT shaderComponentMapping);
typedef ID3D12Resource* (*PFN_ffxGetDX12ResourcePtr)(Fsr212::FfxFsr2Context* context, uint32_t resId);

static PFN_ffxFsr2ContextCreate o_ffxFsr2ContextCreate_Dx12 = nullptr;
static PFN_ffxFsr2ContextDispatch o_ffxFsr2ContextDispatch_Dx12 = nullptr;
static PFN_ffxFsr2ContextGenerateReactiveMask o_ffxFsr2ContextGenerateReactiveMask_Dx12 = nullptr;
static PFN_ffxFsr2ContextDestroy o_ffxFsr2ContextDestroy_Dx12 = nullptr;
static PFN_ffxFsr2GetUpscaleRatioFromQualityMode o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 = nullptr;
static PFN_ffxFsr2GetRenderResolutionFromQualityMode o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 = nullptr;
static PFN_ffxFsr2GetScratchMemorySizeDX12 o_ffxFsr2GetScratchMemorySizeDX12 = nullptr;
static PFN_ffxFsr2GetInterfaceDX12 o_ffxFsr2GetInterfaceDX12 = nullptr;
static PFN_ffxGetResourceDX12 o_ffxGetResourceDX12 = nullptr;
static PFN_ffxGetDX12ResourcePtr o_ffxGetDX12ResourcePtr = nullptr;

static uint32_t _handleCounter = 0;

static std::map<uint32_t, Fsr212::FfxFsr2ContextDescription> _initParams;
static std::map<uint32_t, NVSDK_NGX_Parameter*> _nvParams;
static std::map<uint32_t, NVSDK_NGX_Handle*> _contexts;
static ID3D12Device* _d3d12Device = nullptr;
static bool _nvnxgInited = false;
static float qualityRatios[] = { 1.0, 1.5, 1.7, 2.0, 3.0 };

static bool CreateDLSSContext(Fsr212::FfxFsr2Context* handle, const Fsr212::FfxFsr2DispatchDescription* pExecParams)
{
    LOG_DEBUG("");

    auto handleId = handle->data[1];

    if (!_nvParams.contains(handleId))
        return false;

    NVSDK_NGX_Handle* nvHandle = nullptr;
    auto params = _nvParams[handleId];
    auto initParams = &_initParams[handleId];
    auto commandList = (ID3D12GraphicsCommandList*)pExecParams->commandList;

    UINT initFlags = 0;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_DEPTH_INVERTED)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;

    if ((initParams->flags & Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS) == 0)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;

    params->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, initFlags);

    params->Set(NVSDK_NGX_Parameter_Width, pExecParams->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_Height, pExecParams->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_OutWidth, initParams->displaySize.width);
    params->Set(NVSDK_NGX_Parameter_OutHeight, initParams->displaySize.height);

    auto ratio = (float)initParams->displaySize.width / (float)pExecParams->renderSize.width;

    if (ratio <= 3.0)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraPerformance);
    else if (ratio <= 2.0)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxPerf);
    else if (ratio <= 1.7)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_Balanced);
    else if (ratio <= 1.5)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
    else if (ratio <= 1.3)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraQuality);
    else
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_DLAA);

    if (NVSDK_NGX_D3D12_CreateFeature(commandList, NVSDK_NGX_Feature_SuperSampling, params, &nvHandle) != NVSDK_NGX_Result_Success)
        return false;

    _contexts[handleId] = nvHandle;

    return true;
}

static bool CreateDLSSContext20(Fsr212::FfxFsr2Context* handle, const FfxFsr20DispatchDescription* pExecParams)
{
    LOG_DEBUG("");

    auto handleId = handle->data[1];

    if (!_nvParams.contains(handleId))
        return false;

    NVSDK_NGX_Handle* nvHandle = nullptr;
    auto params = _nvParams[handleId];
    auto initParams = &_initParams[handleId];
    auto commandList = (ID3D12GraphicsCommandList*)pExecParams->commandList;

    UINT initFlags = 0;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_DEPTH_INVERTED)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (initParams->flags & Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;

    if ((initParams->flags & Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS) == 0)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;

    params->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, initFlags);

    params->Set(NVSDK_NGX_Parameter_Width, pExecParams->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_Height, pExecParams->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_OutWidth, initParams->displaySize.width);
    params->Set(NVSDK_NGX_Parameter_OutHeight, initParams->displaySize.height);

    auto ratio = (float)initParams->displaySize.width / (float)pExecParams->renderSize.width;

    if (ratio <= 3.0)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraPerformance);
    else if (ratio <= 2.0)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxPerf);
    else if (ratio <= 1.7)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_Balanced);
    else if (ratio <= 1.5)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
    else if (ratio <= 1.3)
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraQuality);
    else
        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_DLAA);

    if (NVSDK_NGX_D3D12_CreateFeature(commandList, NVSDK_NGX_Feature_SuperSampling, params, &nvHandle) != NVSDK_NGX_Result_Success)
        return false;

    _contexts[handleId] = nvHandle;

    return true;
}

static std::optional<float> GetQualityOverrideRatioFfx(const Fsr212::FfxFsr2QualityMode input)
{
    std::optional<float> output;

    auto sliderLimit = Config::Instance()->ExtendedLimits.value_or(false) ? 0.1f : 1.0f;

    if (Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false) &&
        Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f) >= sliderLimit)
    {
        output = Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f);

        return  output;
    }

    if (!Config::Instance()->QualityRatioOverrideEnabled.value_or(false))
        return output; // override not enabled

    switch (input)
    {
        case Fsr212::FFX_FSR2_QUALITY_MODE_ULTRA_PERFORMANCE:
            if (Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0);

            break;

        case Fsr212::FFX_FSR2_QUALITY_MODE_PERFORMANCE:
            if (Config::Instance()->QualityRatio_Performance.value_or(2.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Performance.value_or(2.0);

            break;

        case Fsr212::FFX_FSR2_QUALITY_MODE_BALANCED:
            if (Config::Instance()->QualityRatio_Balanced.value_or(1.7) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);

            break;

        case Fsr212::FFX_FSR2_QUALITY_MODE_QUALITY:
            if (Config::Instance()->QualityRatio_Quality.value_or(1.5) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Quality.value_or(1.5);

            break;

        default:
            LOG_WARN("Unknown quality: {0}", (int)input);
            break;
    }

    return output;
}

// FS2 Upscaler
static Fsr212::FfxErrorCode ffxFsr2ContextCreate_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2ContextDescription* contextDescription)
{
    if (contextDescription == nullptr || contextDescription->device == nullptr)
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    _d3d12Device = (ID3D12Device*)contextDescription->device;

    NVSDK_NGX_FeatureCommonInfo fcInfo{};
    wchar_t const** paths = new const wchar_t* [1];
    auto dllPath = Util::DllPath().remove_filename().wstring();
    paths[0] = dllPath.c_str();
    fcInfo.PathListInfo.Path = paths;
    fcInfo.PathListInfo.Length = 1;

    if (!_nvnxgInited)
    {

        auto nvResult = NVSDK_NGX_D3D12_Init_with_ProjectID("OptiScaler", NVSDK_NGX_ENGINE_TYPE_CUSTOM, VER_PRODUCT_VERSION_STR, dllPath.c_str(),
                                                            _d3d12Device, &fcInfo, Config::Instance()->NVNGX_Version);

        if (nvResult != NVSDK_NGX_Result_Success)
            return Fsr212::FFX_ERROR_BACKEND_API_ERROR;

        _nvnxgInited = true;
    }

    *context = {};
    (*context).data[0] = 0x1337;
    (*context).data[1] = ++_handleCounter;

    NVSDK_NGX_Parameter* params = nullptr;

    if (NVSDK_NGX_D3D12_GetCapabilityParameters(&params) != NVSDK_NGX_Result_Success)
        return Fsr212::FFX_ERROR_BACKEND_API_ERROR;

    _nvParams[_handleCounter] = params;

    Fsr212::FfxFsr2ContextDescription ccd{};
    ccd.flags = contextDescription->flags;
    ccd.maxRenderSize = contextDescription->maxRenderSize;
    ccd.displaySize = contextDescription->displaySize;
    _initParams[_handleCounter] = ccd;

    LOG_INFO("context created: {:X}", (size_t)_handleCounter);

    return Fsr212::FFX_OK;
}

static Fsr212::FfxErrorCode ffxFsr2ContextDispatch_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2DispatchDescription* dispatchDescription)
{
    if (dispatchDescription == nullptr || context == nullptr || dispatchDescription->commandList == nullptr)
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    auto contextId = (*context).data[1];

    LOG_DEBUG("context: {:X}", contextId);

    // If not in contexts list create and add context
    if (!_contexts.contains(contextId) && _initParams.contains(contextId) && !CreateDLSSContext(context, dispatchDescription))
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    NVSDK_NGX_Parameter* params = _nvParams[contextId];
    NVSDK_NGX_Handle* handle = _contexts[contextId];

    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, dispatchDescription->jitterOffset.x);
    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, dispatchDescription->jitterOffset.y);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_X, dispatchDescription->motionVectorScale.x);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, dispatchDescription->motionVectorScale.y);
    params->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0);
    params->Set(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, dispatchDescription->preExposure);
    params->Set(NVSDK_NGX_Parameter_Reset, dispatchDescription->reset ? 1 : 0);
    params->Set(NVSDK_NGX_Parameter_Width, dispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_Height, dispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, dispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, dispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_Depth, dispatchDescription->depth.resource);
    params->Set(NVSDK_NGX_Parameter_ExposureTexture, dispatchDescription->exposure.resource);
    params->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, dispatchDescription->reactive.resource);
    params->Set(NVSDK_NGX_Parameter_Color, dispatchDescription->color.resource);
    params->Set(NVSDK_NGX_Parameter_MotionVectors, dispatchDescription->motionVectors.resource);
    params->Set(NVSDK_NGX_Parameter_Output, dispatchDescription->output.resource);
    params->Set("FSR.cameraNear", dispatchDescription->cameraNear);
    params->Set("FSR.cameraFar", dispatchDescription->cameraFar);
    params->Set("FSR.cameraFovAngleVertical", dispatchDescription->cameraFovAngleVertical);
    params->Set("FSR.frameTimeDelta", dispatchDescription->frameTimeDelta);
    params->Set("FSR.transparencyAndComposition", dispatchDescription->transparencyAndComposition.resource);
    params->Set("FSR.reactive", dispatchDescription->reactive.resource);
    params->Set(NVSDK_NGX_Parameter_Sharpness, dispatchDescription->sharpness);

    LOG_DEBUG("handle: {:X}, internalResolution: {}x{}", handle->Id, dispatchDescription->renderSize.width, dispatchDescription->renderSize.height);

    Config::Instance()->setInputApiName = "FSR2-DX12";

    auto evalResult = NVSDK_NGX_D3D12_EvaluateFeature((ID3D12GraphicsCommandList*)dispatchDescription->commandList, handle, params, nullptr);

    if (evalResult == NVSDK_NGX_Result_Success)
        return Fsr212::FFX_OK;

    LOG_ERROR("evalResult: {:X}", (UINT)evalResult);
    return Fsr212::FFX_ERROR_BACKEND_API_ERROR;
}

static Fsr212::FfxErrorCode ffxFsr20ContextDispatch_Dx12(Fsr212::FfxFsr2Context* context, const FfxFsr20DispatchDescription* dispatchDescription)
{
    if (dispatchDescription == nullptr || context == nullptr || dispatchDescription->commandList == nullptr)
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    auto contextId = (*context).data[1];

    LOG_DEBUG("context: {:X}", contextId);

    // If not in contexts list create and add context
    if (!_contexts.contains(contextId) && _initParams.contains(contextId) && !CreateDLSSContext20(context, dispatchDescription))
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    NVSDK_NGX_Parameter* params = _nvParams[contextId];
    NVSDK_NGX_Handle* handle = _contexts[contextId];

    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, dispatchDescription->jitterOffset.x);
    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, dispatchDescription->jitterOffset.y);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_X, dispatchDescription->motionVectorScale.x);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, dispatchDescription->motionVectorScale.y);
    params->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0);
    params->Set(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, dispatchDescription->preExposure);
    params->Set(NVSDK_NGX_Parameter_Reset, dispatchDescription->reset ? 1 : 0);
    params->Set(NVSDK_NGX_Parameter_Width, dispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_Height, dispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, dispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, dispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_Depth, dispatchDescription->depth.resource);
    params->Set(NVSDK_NGX_Parameter_ExposureTexture, dispatchDescription->exposure.resource);
    params->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, dispatchDescription->reactive.resource);
    params->Set(NVSDK_NGX_Parameter_Color, dispatchDescription->color.resource);
    params->Set(NVSDK_NGX_Parameter_MotionVectors, dispatchDescription->motionVectors.resource);
    params->Set(NVSDK_NGX_Parameter_Output, dispatchDescription->output.resource);
    params->Set("FSR.cameraNear", dispatchDescription->cameraNear);
    params->Set("FSR.cameraFar", dispatchDescription->cameraFar);
    params->Set("FSR.cameraFovAngleVertical", dispatchDescription->cameraFovAngleVertical);
    params->Set("FSR.frameTimeDelta", dispatchDescription->frameTimeDelta);
    params->Set("FSR.transparencyAndComposition", dispatchDescription->transparencyAndComposition.resource);
    params->Set("FSR.reactive", dispatchDescription->reactive.resource);
    params->Set(NVSDK_NGX_Parameter_Sharpness, dispatchDescription->sharpness);

    LOG_DEBUG("handle: {:X}, internalResolution: {}x{}", handle->Id, dispatchDescription->renderSize.width, dispatchDescription->renderSize.height);

    Config::Instance()->setInputApiName = "FSR2-DX12";

    auto evalResult = NVSDK_NGX_D3D12_EvaluateFeature((ID3D12GraphicsCommandList*)dispatchDescription->commandList, handle, params, nullptr);

    if (evalResult == NVSDK_NGX_Result_Success)
        return Fsr212::FFX_OK;

    LOG_ERROR("evalResult: {:X}", (UINT)evalResult);
    return Fsr212::FFX_ERROR_BACKEND_API_ERROR;
}

static Fsr212::FfxErrorCode ffxFsr2ContextGenerateReactiveMask_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2GenerateReactiveDescription* params)
{
    LOG_WARN("");
    return Fsr212::FFX_OK;
}

static Fsr212::FfxErrorCode ffxFsr2ContextDestroy_Dx12(Fsr212::FfxFsr2Context* context)
{
    if (context == nullptr)
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    auto contextId = (*context).data[1];
    LOG_DEBUG("context: {:X}", contextId);

    if (!_initParams.contains(contextId))
        return Fsr212::FFX_ERROR_INVALID_ARGUMENT;

    if (_contexts.contains(contextId))
        NVSDK_NGX_D3D12_ReleaseFeature(_contexts[contextId]);

    _contexts.erase(contextId);
    _nvParams.erase(contextId);
    _initParams.erase(contextId);

    return Fsr212::FFX_OK;
}

static float ffxFsr2GetUpscaleRatioFromQualityMode_Dx12(Fsr212::FfxFsr2QualityMode qualityMode)
{
    auto ratio = GetQualityOverrideRatioFfx(qualityMode).value_or(qualityRatios[(UINT)qualityMode]);
    LOG_DEBUG("Quality mode: {}, Upscale ratio: {}", (UINT)qualityMode, ratio);
    return ratio;
}

static Fsr212::FfxErrorCode ffxFsr2GetRenderResolutionFromQualityMode_Dx12(uint32_t* renderWidth, uint32_t* renderHeight,
                                                                           uint32_t displayWidth, uint32_t displayHeight, Fsr212::FfxFsr2QualityMode qualityMode)
{
    auto ratio = GetQualityOverrideRatioFfx(qualityMode).value_or(qualityRatios[(UINT)qualityMode]);

    if (renderHeight != nullptr)
        *renderHeight = (uint32_t)((float)displayHeight / ratio);

    if (renderWidth != nullptr)
        *renderWidth = (uint32_t)((float)displayWidth / ratio);

    if (renderWidth != nullptr && renderHeight != nullptr)
    {
        LOG_DEBUG("Quality mode: {}, Render resolution: {}x{}", (UINT)qualityMode, *renderWidth, *renderHeight);
        return Fsr212::FFX_OK;
    }

    LOG_WARN("Quality mode: {}, pOutRenderWidth or pOutRenderHeight is null!", (UINT)qualityMode);
    return Fsr212::FFX_ERROR_INVALID_ARGUMENT;
}

// Dx12 Backend
static size_t hk_ffxFsr2GetScratchMemorySizeDX12()
{
    LOG_WARN("");
    return 1920 * 1080 * 31;
}

static Fsr212::FfxErrorCode hk_ffxFsr2GetInterfaceDX12(Fsr212::FfxFsr2Interface212* fsr2Interface, ID3D12Device* device,
                                                       void* scratchBuffer, size_t scratchBufferSize)
{
    LOG_DEBUG("");
    return Fsr212::FFX_OK;
}

static FfxResource20 hk_ffxGetResource20DX12(Fsr212::FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name = nullptr,
                                                 Fsr212::FfxResourceStates state = Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ,
                                                 UINT shaderComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
{
    FfxResource20 result{};
    result.resource = resDx12;
    result.state = state;

    return result;
}

static Fsr212::FfxResource hk_ffxGetResourceDX12(Fsr212::FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name = nullptr,
                                                 Fsr212::FfxResourceStates state = Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ,
                                                 UINT shaderComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
{
    std::wstring bufferName(name);

    LOG_DEBUG("name: {}", wstring_to_string(bufferName));

    Fsr212::FfxResource result{};
    std::wcscpy(result.name, bufferName.c_str());
    result.resource = resDx12;
    result.state = state;

    return result;
}

static ID3D12Resource* hk_ffxGetDX12ResourcePtr(Fsr212::FfxFsr2Context* context, uint32_t resId)
{
    LOG_ERROR("resId: {}", resId);
    return nullptr;
}

void HookFSR2ExeInputs()
{
    LOG_INFO("Trying to hook FSR2 methods");

    auto exeName = wstring_to_string(Util::ExePath().filename());

    o_ffxFsr2GetScratchMemorySizeDX12 = (PFN_ffxFsr2GetScratchMemorySizeDX12)DetourFindFunction(exeName.c_str(), "ffxFsr2GetScratchMemorySizeDX12");
    if (o_ffxFsr2GetScratchMemorySizeDX12 == nullptr)
        o_ffxFsr2GetScratchMemorySizeDX12 = (PFN_ffxFsr2GetScratchMemorySizeDX12)DetourFindFunction(exeName.c_str(), "?ffxFsr2GetScratchMemorySizeDX12@@YA_KXZ");

    o_ffxFsr2GetInterfaceDX12 = (PFN_ffxFsr2GetInterfaceDX12)DetourFindFunction(exeName.c_str(), "ffxFsr2GetInterfaceDX12");
    if (o_ffxFsr2GetInterfaceDX12 == nullptr)
        o_ffxFsr2GetInterfaceDX12 = (PFN_ffxFsr2GetInterfaceDX12)DetourFindFunction(exeName.c_str(), "?ffxFsr2GetInterfaceDX12@@YAHPEAUFfxFsr2Interface@@PEAUID3D12Device@@PEAX_K@Z");

    o_ffxGetResourceDX12 = (PFN_ffxGetResourceDX12)DetourFindFunction(exeName.c_str(), "ffxGetResourceDX12");
    if (o_ffxGetResourceDX12 == nullptr)
        o_ffxGetResourceDX12 = (PFN_ffxGetResourceDX12)DetourFindFunction(exeName.c_str(), "?ffxGetResourceDX12@@YA?AUFfxResource@@PEAUFfxFsr2Context@@PEAUID3D12Resource@@PEA_WW4FfxResourceStates@@I@Z");

    o_ffxGetDX12ResourcePtr = (PFN_ffxGetDX12ResourcePtr)DetourFindFunction(exeName.c_str(), "ffxGetDX12ResourcePtr");

    o_ffxFsr2ContextCreate_Dx12 = (PFN_ffxFsr2ContextCreate)DetourFindFunction(exeName.c_str(), "ffxFsr2ContextCreate");
    if (o_ffxFsr2ContextCreate_Dx12 == nullptr)
        o_ffxFsr2ContextCreate_Dx12 = (PFN_ffxFsr2ContextCreate)DetourFindFunction(exeName.c_str(), "?ffxFsr2ContextCreate@@YAHPEAUFfxFsr2Context@@PEBUFfxFsr2ContextDescription@@@Z");

    o_ffxFsr2ContextDispatch_Dx12 = (PFN_ffxFsr2ContextDispatch)DetourFindFunction(exeName.c_str(), "ffxFsr2ContextDispatch");
    if (o_ffxFsr2ContextDispatch_Dx12 == nullptr)
        o_ffxFsr2ContextDispatch_Dx12 = (PFN_ffxFsr2ContextDispatch)DetourFindFunction(exeName.c_str(), "?ffxFsr2ContextDispatch@@YAHPEAUFfxFsr2Context@@PEBUFfxFsr2DispatchDescription@@@Z");

    o_ffxFsr2ContextGenerateReactiveMask_Dx12 = (PFN_ffxFsr2ContextGenerateReactiveMask)DetourFindFunction(exeName.c_str(), "ffxFsr2ContextGenerateReactiveMask");
    if (o_ffxFsr2ContextGenerateReactiveMask_Dx12 == nullptr)
        o_ffxFsr2ContextGenerateReactiveMask_Dx12 = (PFN_ffxFsr2ContextGenerateReactiveMask)DetourFindFunction(exeName.c_str(), "?ffxFsr2GenerateReactiveMask@@YAHPEAUFfxFsr2Context@@PEBUFfxFsr2GenerateReactiveDescription@@@Z");

    o_ffxFsr2ContextDestroy_Dx12 = (PFN_ffxFsr2ContextDestroy)DetourFindFunction(exeName.c_str(), "ffxFsr2ContextDestroy");
    if (o_ffxFsr2ContextDestroy_Dx12 == nullptr)
        o_ffxFsr2ContextDestroy_Dx12 = (PFN_ffxFsr2ContextDestroy)DetourFindFunction(exeName.c_str(), "?ffxFsr2ContextDestroy@@YAHPEAUFfxFsr2Context@@@Z");

    o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 = (PFN_ffxFsr2GetUpscaleRatioFromQualityMode)DetourFindFunction(exeName.c_str(), "ffxFsr2GetUpscaleRatioFromQualityMode");
    if (o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 == nullptr)
        o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 = (PFN_ffxFsr2GetUpscaleRatioFromQualityMode)DetourFindFunction(exeName.c_str(), "?ffxFsr2GetUpscaleRatioFromQualityMode@@YAMW4FfxFsr2QualityMode@@@Z");

    o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 = (PFN_ffxFsr2GetRenderResolutionFromQualityMode)DetourFindFunction(exeName.c_str(), "ffxFsr2GetRenderResolutionFromQualityMode");
    if (o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 == nullptr)
        o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 = (PFN_ffxFsr2GetRenderResolutionFromQualityMode)DetourFindFunction(exeName.c_str(), "?ffxFsr2GetRenderResolutionFromQualityMode@@YAHPEAI0IIW4FfxFsr2QualityMode@@@Z");

    if (o_ffxFsr2GetInterfaceDX12 != nullptr || o_ffxFsr2ContextCreate_Dx12 != nullptr)
    {
        LOG_INFO("FSR2 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFsr2GetScratchMemorySizeDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetScratchMemorySizeDX12, hk_ffxFsr2GetScratchMemorySizeDX12);

        if (o_ffxFsr2GetInterfaceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetInterfaceDX12, hk_ffxFsr2GetInterfaceDX12);

        if (o_ffxGetResourceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetResourceDX12, hk_ffxGetResourceDX12);

        if (o_ffxGetDX12ResourcePtr != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetDX12ResourcePtr, hk_ffxGetDX12ResourcePtr);

        if (o_ffxFsr2ContextCreate_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextCreate_Dx12, ffxFsr2ContextCreate_Dx12);

        if (o_ffxFsr2ContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextDispatch_Dx12, ffxFsr2ContextDispatch_Dx12);

        if (o_ffxFsr2ContextGenerateReactiveMask_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextGenerateReactiveMask_Dx12, ffxFsr2ContextGenerateReactiveMask_Dx12);

        if (o_ffxFsr2ContextDestroy_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextDestroy_Dx12, ffxFsr2ContextDestroy_Dx12);

        if (o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12, ffxFsr2GetUpscaleRatioFromQualityMode_Dx12);

        if (o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12, ffxFsr2GetRenderResolutionFromQualityMode_Dx12);

        DetourTransactionCommit();
    }
}

void HookFSR2Dx12Inputs(HMODULE module)
{
    LOG_INFO("Trying to hook FSR2 methods");

    if (module != nullptr)
    {
        o_ffxFsr2GetScratchMemorySizeDX12 = (PFN_ffxFsr2GetScratchMemorySizeDX12)GetProcAddress(module, "ffxFsr2GetScratchMemorySizeDX12");
        o_ffxFsr2GetInterfaceDX12 = (PFN_ffxFsr2GetInterfaceDX12)GetProcAddress(module, "ffxFsr2GetInterfaceDX12");
        o_ffxGetResourceDX12 = (PFN_ffxGetResourceDX12)GetProcAddress(module, "ffxGetResourceDX12");
        o_ffxGetDX12ResourcePtr = (PFN_ffxGetDX12ResourcePtr)GetProcAddress(module, "ffxGetDX12ResourcePtr");
    }

    if (o_ffxFsr2GetScratchMemorySizeDX12 != nullptr)
    {
        LOG_INFO("FSR2 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFsr2GetScratchMemorySizeDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetScratchMemorySizeDX12, hk_ffxFsr2GetScratchMemorySizeDX12);

        if (o_ffxFsr2GetInterfaceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetInterfaceDX12, hk_ffxFsr2GetInterfaceDX12);

        if (o_ffxGetResourceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetResourceDX12, hk_ffxGetResourceDX12);

        if (o_ffxGetDX12ResourcePtr != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetDX12ResourcePtr, hk_ffxGetDX12ResourcePtr);

        DetourTransactionCommit();
    }
}

void HookFSR2Inputs(HMODULE module)
{
    LOG_INFO("Trying to hook FSR2 methods");

    if (module != nullptr)
    {
        o_ffxFsr2ContextCreate_Dx12 = (PFN_ffxFsr2ContextCreate)GetProcAddress(module, "ffxFsr2ContextCreate");
        o_ffxFsr2ContextDispatch_Dx12 = (PFN_ffxFsr2ContextDispatch)GetProcAddress(module, "ffxFsr2ContextDispatch");
        o_ffxFsr2ContextGenerateReactiveMask_Dx12 = (PFN_ffxFsr2ContextGenerateReactiveMask)GetProcAddress(module, "ffxFsr2ContextGenerateReactiveMask");
        o_ffxFsr2ContextDestroy_Dx12 = (PFN_ffxFsr2ContextDestroy)GetProcAddress(module, "ffxFsr2ContextDestroy");
        o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 = (PFN_ffxFsr2GetUpscaleRatioFromQualityMode)GetProcAddress(module, "ffxFsr2GetUpscaleRatioFromQualityMode");
        o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 = (PFN_ffxFsr2GetRenderResolutionFromQualityMode)GetProcAddress(module, "ffxFsr2GetRenderResolutionFromQualityMode");
    }

    if (o_ffxFsr2ContextCreate_Dx12 != nullptr)
    {
        LOG_INFO("FSR2 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFsr2ContextCreate_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextCreate_Dx12, ffxFsr2ContextCreate_Dx12);

        if (o_ffxFsr2ContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextDispatch_Dx12, ffxFsr2ContextDispatch_Dx12);

        if (o_ffxFsr2ContextGenerateReactiveMask_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextGenerateReactiveMask_Dx12, ffxFsr2ContextGenerateReactiveMask_Dx12);

        if (o_ffxFsr2ContextDestroy_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2ContextDestroy_Dx12, ffxFsr2ContextDestroy_Dx12);

        if (o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetUpscaleRatioFromQualityMode_Dx12, ffxFsr2GetUpscaleRatioFromQualityMode_Dx12);

        if (o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr2GetRenderResolutionFromQualityMode_Dx12, ffxFsr2GetRenderResolutionFromQualityMode_Dx12);

        DetourTransactionCommit();
    }
}
