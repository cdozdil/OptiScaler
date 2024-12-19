#include "FSR3_Dx12.h"

#include "Config.h"
#include "Util.h"

#include "resource.h"
#include "NVNGX_Parameter.h"

#include "detours/detours.h"
#include "fsr3/include/ffx_fsr3upscaler.h"
#include "fsr3/include/dx12/ffx_dx12.h"

// FSR3
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerContextCreate)(Fsr3::FfxFsr3UpscalerContext* pContext, const Fsr3::FfxFsr3UpscalerContextDescription* pContextDescription);
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerGetSharedResourceDescriptions)(Fsr3::FfxFsr3UpscalerContext* context, Fsr3::FfxFsr3UpscalerSharedResourceDescriptions* SharedResources);
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerContextDispatch)(Fsr3::FfxFsr3UpscalerContext* pContext, const Fsr3::FfxFsr3UpscalerDispatchDescription* pDispatchDescription);
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerContextGenerateReactiveMask)(Fsr3::FfxFsr3UpscalerContext* pContext, const Fsr3::FfxFsr3UpscalerGenerateReactiveDescription* pParams);
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerContextDestroy)(Fsr3::FfxFsr3UpscalerContext* pContext);
typedef float(*PFN_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode)(Fsr3::FfxFsr3UpscalerQualityMode qualityMode);
typedef Fsr3::FfxErrorCode(*PFN_ffxFsr3UpscalerGetRenderResolutionFromQualityMode)(uint32_t* pRenderWidth, uint32_t* pRenderHeight, uint32_t displayWidth, uint32_t displayHeight, Fsr3::FfxFsr3UpscalerQualityMode qualityMode);
typedef bool(*PFN_ffxFsr3UpscalerResourceIsNull)(Fsr3::FfxResource resource);

// Dx12
typedef size_t(*PFN_ffxFSR3GetScratchMemorySizeDX12)(size_t maxContexts);
typedef Fsr3::FfxErrorCode(*PFN_ffxFSR3GetInterfaceDX12)(Fsr3::FfxInterface* backendInterface, Fsr3::FfxDevice device, void* scratchBuffer, size_t scratchBufferSize, uint32_t maxContexts);
typedef Fsr3::FfxResource(*PFN_ffxFSR3GetResourceDX12)(const ID3D12Resource* dx12Resource, Fsr3::FfxResourceDescription ffxResDescription, wchar_t* ffxResName, Fsr3::FfxResourceStates state);

static PFN_ffxFsr3UpscalerContextCreate o_ffxFsr3UpscalerContextCreate_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerContextDispatch o_ffxFsr3UpscalerContextDispatch_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerContextGenerateReactiveMask o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerContextDestroy o_ffxFsr3UpscalerContextDestroy_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerGetRenderResolutionFromQualityMode o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12 = nullptr;
static PFN_ffxFsr3UpscalerGetSharedResourceDescriptions o_ffxFsr3UpscalerGetSharedResourceDescriptions = nullptr;
static PFN_ffxFsr3UpscalerResourceIsNull o_ffxFsr3UpscalerResourceIsNull = nullptr;

static PFN_ffxFSR3GetScratchMemorySizeDX12 o_ffxFSR3GetScratchMemorySizeDX12 = nullptr;
static PFN_ffxFSR3GetInterfaceDX12 o_ffxFSR3GetInterfaceDX12 = nullptr;
static PFN_ffxFSR3GetResourceDX12 o_ffxGetFSR3ResourceDX12 = nullptr;

static std::map<Fsr3::FfxFsr3UpscalerContext*, Fsr3::FfxFsr3UpscalerContextDescription> _initParams;
static std::map<Fsr3::FfxFsr3UpscalerContext*, NVSDK_NGX_Parameter*> _nvParams;
static std::map<Fsr3::FfxFsr3UpscalerContext*, NVSDK_NGX_Handle*> _contexts;
static ID3D12Device* _d3d12Device = nullptr;
static bool _nvnxgInited = false;
static float qualityRatios[] = { 1.0, 1.5, 1.7, 2.0, 3.0 };

static bool CreateDLSSContext(Fsr3::FfxFsr3UpscalerContext* handle, const Fsr3::FfxFsr3UpscalerDispatchDescription* pExecParams)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(handle))
        return false;

    NVSDK_NGX_Handle* nvHandle = nullptr;
    auto params = _nvParams[handle];
    auto initParams = &_initParams[handle];
    auto commandList = (ID3D12GraphicsCommandList*)pExecParams->commandList;

    UINT initFlags = 0;

    if (initParams->flags & Fsr3::FFX_FSR3UPSCALER_ENABLE_HIGH_DYNAMIC_RANGE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    if (initParams->flags & Fsr3::FFX_FSR3UPSCALER_ENABLE_DEPTH_INVERTED)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;

    if (initParams->flags & Fsr3::FFX_FSR3UPSCALER_ENABLE_AUTO_EXPOSURE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (initParams->flags & Fsr3::FFX_FSR3UPSCALER_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;

    if ((initParams->flags & Fsr3::FFX_FSR3UPSCALER_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS) == 0)
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

    _contexts[handle] = nvHandle;

    return true;
}

static std::optional<float> GetQualityOverrideRatioFfx(const Fsr3::FfxFsr3UpscalerQualityMode input)
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
        case Fsr3::FFX_FSR3UPSCALER_QUALITY_MODE_ULTRA_PERFORMANCE:
            if (Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0);

            break;

        case Fsr3::FFX_FSR3UPSCALER_QUALITY_MODE_PERFORMANCE:
            if (Config::Instance()->QualityRatio_Performance.value_or(2.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Performance.value_or(2.0);

            break;

        case Fsr3::FFX_FSR3UPSCALER_QUALITY_MODE_BALANCED:
            if (Config::Instance()->QualityRatio_Balanced.value_or(1.7) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);

            break;

        case Fsr3::FFX_FSR3UPSCALER_QUALITY_MODE_QUALITY:
            if (Config::Instance()->QualityRatio_Quality.value_or(1.5) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Quality.value_or(1.5);

            break;

        case Fsr3::FFX_FSR3UPSCALER_QUALITY_MODE_NATIVEAA:
            if (Config::Instance()->QualityRatio_Quality.value_or(1.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Quality.value_or(1.0);

            break;

        default:
            LOG_WARN("Unknown quality: {0}", (int)input);
            break;
    }

    return output;
}

// FS2 Upscaler
static Fsr3::FfxErrorCode ffxFsr3ContextCreate_Dx12(Fsr3::FfxFsr3UpscalerContext* pContext, const Fsr3::FfxFsr3UpscalerContextDescription* pContextDescription)
{
    if (pContext == nullptr || pContextDescription->backendInterface.device == nullptr)
        return Fsr3::FFX_ERROR_INVALID_ARGUMENT;

    if (_d3d12Device == nullptr)
        _d3d12Device = (ID3D12Device*)pContextDescription->backendInterface.device;

    auto ccResult = o_ffxFsr3UpscalerContextCreate_Dx12(pContext, pContextDescription);

    if (ccResult != Fsr3::FFX_OK)
    {
        LOG_ERROR("create error: {}", (UINT)ccResult);
        return ccResult;
    }

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
            return Fsr3::FFX_ERROR_BACKEND_API_ERROR;

        _nvnxgInited = true;
    }

    NVSDK_NGX_Parameter* params = nullptr;

    if (NVSDK_NGX_D3D12_GetCapabilityParameters(&params) != NVSDK_NGX_Result_Success)
        return Fsr3::FFX_ERROR_BACKEND_API_ERROR;

    _nvParams[pContext] = params;

    Fsr3::FfxFsr3UpscalerContextDescription ccd{};
    ccd.flags = pContextDescription->flags;
    ccd.maxRenderSize = pContextDescription->maxRenderSize;
    ccd.displaySize = pContextDescription->displaySize;
    ccd.backendInterface.device = pContextDescription->backendInterface.device;
    _initParams[pContext] = ccd;

    LOG_INFO("context created: {:X}", (size_t)pContext);

    return Fsr3::FFX_OK;
}

static Fsr3::FfxErrorCode ffxFsr3ContextDispatch_Dx12(Fsr3::FfxFsr3UpscalerContext* pContext, Fsr3::FfxFsr3UpscalerDispatchDescription* pDispatchDescription)
{
    if (pDispatchDescription == nullptr || pContext == nullptr || pDispatchDescription->commandList == nullptr)
        return Fsr3::FFX_ERROR_INVALID_ARGUMENT;

    // If not in contexts list create and add context
    if (!_contexts.contains(pContext) && _initParams.contains(pContext) && !CreateDLSSContext(pContext, pDispatchDescription))
        return Fsr3::FFX_ERROR_INVALID_ARGUMENT;

    NVSDK_NGX_Parameter* params = _nvParams[pContext];
    NVSDK_NGX_Handle* handle = _contexts[pContext];

    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, pDispatchDescription->jitterOffset.x);
    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, pDispatchDescription->jitterOffset.y);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_X, pDispatchDescription->motionVectorScale.x);
    params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, pDispatchDescription->motionVectorScale.y);
    params->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0);
    params->Set(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, pDispatchDescription->preExposure);
    params->Set(NVSDK_NGX_Parameter_Reset, pDispatchDescription->reset ? 1 : 0);
    params->Set(NVSDK_NGX_Parameter_Width, pDispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_Height, pDispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, pDispatchDescription->renderSize.width);
    params->Set(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, pDispatchDescription->renderSize.height);
    params->Set(NVSDK_NGX_Parameter_Depth, pDispatchDescription->depth.resource);
    params->Set(NVSDK_NGX_Parameter_ExposureTexture, pDispatchDescription->exposure.resource);
    params->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, pDispatchDescription->reactive.resource);
    params->Set(NVSDK_NGX_Parameter_Color, pDispatchDescription->color.resource);
    params->Set(NVSDK_NGX_Parameter_MotionVectors, pDispatchDescription->motionVectors.resource);
    params->Set(NVSDK_NGX_Parameter_Output, pDispatchDescription->output.resource);
    params->Set("FSR.cameraNear", pDispatchDescription->cameraNear);
    params->Set("FSR.cameraFar", pDispatchDescription->cameraFar);
    params->Set("FSR.cameraFovAngleVertical", pDispatchDescription->cameraFovAngleVertical);
    params->Set("FSR.frameTimeDelta", pDispatchDescription->frameTimeDelta);
    params->Set("FSR.transparencyAndComposition", pDispatchDescription->transparencyAndComposition.resource);
    params->Set("FSR.reactive", pDispatchDescription->reactive.resource);
    params->Set("FSR.viewSpaceToMetersFactor", pDispatchDescription->viewSpaceToMetersFactor);
    params->Set(NVSDK_NGX_Parameter_Sharpness, pDispatchDescription->sharpness);

    LOG_DEBUG("handle: {:X}, internalResolution: {}x{}", handle->Id, pDispatchDescription->renderSize.width, pDispatchDescription->renderSize.height);

    Config::Instance()->setInputApiName = "FSR3-DX12";

    auto evalResult = NVSDK_NGX_D3D12_EvaluateFeature((ID3D12GraphicsCommandList*)pDispatchDescription->commandList, handle, params, nullptr);

    if (evalResult == NVSDK_NGX_Result_Success)
        return Fsr3::FFX_OK;

    LOG_ERROR("evalResult: {:X}", (UINT)evalResult);
    return Fsr3::FFX_ERROR_BACKEND_API_ERROR;
}

static Fsr3::FfxErrorCode ffxFsr3ContextGenerateReactiveMask_Dx12(Fsr3::FfxFsr3UpscalerContext* pContext, const Fsr3::FfxFsr3UpscalerGenerateReactiveDescription* pParams)
{
    LOG_DEBUG("");
    return o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12(pContext, pParams);
    //return Fsr3::FFX_OK;
}

static Fsr3::FfxErrorCode ffxFsr3ContextDestroy_Dx12(Fsr3::FfxFsr3UpscalerContext* pContext)
{
    if (pContext == nullptr)
        return Fsr3::FFX_ERROR_INVALID_ARGUMENT;

    LOG_DEBUG("context: {:X}", (size_t)pContext);

    if (!_initParams.contains(pContext))
        return Fsr3::FFX_ERROR_INVALID_ARGUMENT;

    auto cdResult = o_ffxFsr3UpscalerContextDestroy_Dx12(pContext);
    LOG_INFO("result: {:X}", (UINT)cdResult);

    if (_contexts.contains(pContext))
        NVSDK_NGX_D3D12_ReleaseFeature(_contexts[pContext]);

    _contexts.erase(pContext);
    _nvParams.erase(pContext);
    _initParams.erase(pContext);

    return Fsr3::FFX_OK;
}

static float ffxFsr3GetUpscaleRatioFromQualityMode_Dx12(Fsr3::FfxFsr3UpscalerQualityMode qualityMode)
{
    auto ratio = GetQualityOverrideRatioFfx(qualityMode).value_or(qualityRatios[(UINT)qualityMode]);
    LOG_DEBUG("Quality mode: {}, Upscale ratio: {}", (UINT)qualityMode, ratio);
    return ratio;
}

static Fsr3::FfxErrorCode ffxFsr3GetRenderResolutionFromQualityMode_Dx12(uint32_t* pRenderWidth, uint32_t* pRenderHeight, uint32_t displayWidth, uint32_t displayHeight, Fsr3::FfxFsr3UpscalerQualityMode qualityMode)
{
    auto ratio = GetQualityOverrideRatioFfx(qualityMode).value_or(qualityRatios[(UINT)qualityMode]);

    if (pRenderWidth != nullptr)
        *pRenderWidth = (uint32_t)((float)displayWidth / ratio);

    if (pRenderHeight != nullptr)
        *pRenderHeight = (uint32_t)((float)displayHeight / ratio);

    if (pRenderWidth != nullptr && pRenderHeight != nullptr)
    {
        LOG_DEBUG("Quality mode: {}, Render resolution: {}x{}", (UINT)qualityMode, *pRenderWidth, *pRenderHeight);
        return Fsr3::FFX_OK;
    }

    LOG_WARN("Quality mode: {}, pOutRenderWidth or pOutRenderHeight is null!", (UINT)qualityMode);
    return Fsr3::FFX_ERROR_INVALID_ARGUMENT;
}

static Fsr3::FfxErrorCode ffxFsr3UpscalerGetSharedResourceDescriptions(Fsr3::FfxFsr3UpscalerContext* pContext, Fsr3::FfxFsr3UpscalerSharedResourceDescriptions* SharedResources)
{
    LOG_DEBUG("");

    if (pContext == nullptr || SharedResources == nullptr)
        return Fsr3::FFX_ERROR_INVALID_POINTER;

    auto initParams = _initParams[pContext];

    SharedResources->dilatedDepth = { Fsr3::FFX_HEAP_TYPE_DEFAULT, { Fsr3::FFX_RESOURCE_TYPE_TEXTURE2D, Fsr3::FFX_SURFACE_FORMAT_R32_FLOAT, initParams.maxRenderSize.width, initParams.maxRenderSize.height, 1, 1, Fsr3::FFX_RESOURCE_FLAGS_ALIASABLE, (Fsr3::FfxResourceUsage)(Fsr3::FFX_RESOURCE_USAGE_RENDERTARGET | Fsr3::FFX_RESOURCE_USAGE_UAV) },
            Fsr3::FFX_RESOURCE_STATE_UNORDERED_ACCESS, 0, nullptr, L"FSR3UPSCALER_DilatedDepth", FFX_FSR3UPSCALER_RESOURCE_IDENTIFIER_DILATED_DEPTH };

    SharedResources->dilatedMotionVectors = { Fsr3::FFX_HEAP_TYPE_DEFAULT, { Fsr3::FFX_RESOURCE_TYPE_TEXTURE2D, Fsr3::FFX_SURFACE_FORMAT_R16G16_FLOAT, initParams.maxRenderSize.width, initParams.maxRenderSize.height, 1, 1, Fsr3::FFX_RESOURCE_FLAGS_ALIASABLE, (Fsr3::FfxResourceUsage)(Fsr3::FFX_RESOURCE_USAGE_RENDERTARGET | Fsr3::FFX_RESOURCE_USAGE_UAV) },
            Fsr3::FFX_RESOURCE_STATE_UNORDERED_ACCESS, 0, nullptr, L"FSR3UPSCALER_DilatedVelocity", FFX_FSR3UPSCALER_RESOURCE_IDENTIFIER_DILATED_MOTION_VECTORS };

    SharedResources->reconstructedPrevNearestDepth = { Fsr3::FFX_HEAP_TYPE_DEFAULT, { Fsr3::FFX_RESOURCE_TYPE_TEXTURE2D, Fsr3::FFX_SURFACE_FORMAT_R32_UINT, initParams.maxRenderSize.width, initParams.maxRenderSize.height, 1, 1, Fsr3::FFX_RESOURCE_FLAGS_ALIASABLE, (Fsr3::FfxResourceUsage)(Fsr3::FFX_RESOURCE_USAGE_UAV) },
            Fsr3::FFX_RESOURCE_STATE_UNORDERED_ACCESS, 0, nullptr, L"FSR3UPSCALER_ReconstructedPrevNearestDepth", FFX_FSR3UPSCALER_RESOURCE_IDENTIFIER_RECONSTRUCTED_PREVIOUS_NEAREST_DEPTH };

    return Fsr3::FFX_OK;
}

static bool ffxFsr3UpscalerResourceIsNull(Fsr3::FfxResource resource)
{
    return resource.resource == nullptr;
}

static size_t hk_ffxFsr3GetScratchMemorySizeDX12(size_t maxContexts)
{
    auto result = o_ffxFSR3GetScratchMemorySizeDX12(maxContexts);
    LOG_WARN("result: {} ", result);
    return result;
}

// Dx12 Backend
static Fsr3::FfxErrorCode hk_ffxFsr3GetInterfaceDX12(Fsr3::FfxInterface* backendInterface, Fsr3::FfxDevice device, void* scratchBuffer, size_t scratchBufferSize, uint32_t maxContexts)
{
    LOG_DEBUG("");
    _d3d12Device = (ID3D12Device*)device;
    auto result = o_ffxFSR3GetInterfaceDX12(backendInterface, device, scratchBuffer, scratchBufferSize, maxContexts);
    return result;
}

static Fsr3::FfxResource hk_ffxFsr3GetResourceDX12(const ID3D12Resource* dx12Resource, Fsr3::FfxResourceDescription ffxResDescription,
                                                   wchar_t* ffxResName, Fsr3::FfxResourceStates state = Fsr3::FFX_RESOURCE_STATE_COMPUTE_READ)
{
    std::wstring bufferName(ffxResName);

    LOG_DEBUG("name: {}", wstring_to_string(bufferName));

    Fsr3::FfxResource result{};
    std::wcscpy(result.name, bufferName.c_str());
    result.resource = (void*)dx12Resource;
    result.state = state;

    return result;
}

void HookFSR3ExeInputs()
{
    LOG_INFO("Trying to hook FSR3 methods");

    auto exeName = wstring_to_string(Util::ExePath().filename());

    o_ffxFSR3GetScratchMemorySizeDX12 = (PFN_ffxFSR3GetScratchMemorySizeDX12)DetourFindFunction(exeName.c_str(), "ffxGetScratchMemorySizeDX12");
    o_ffxFSR3GetInterfaceDX12 = (PFN_ffxFSR3GetInterfaceDX12)DetourFindFunction(exeName.c_str(), "ffxGetInterfaceDX12");
    o_ffxGetFSR3ResourceDX12 = (PFN_ffxFSR3GetResourceDX12)DetourFindFunction(exeName.c_str(), "ffxGetResourceDX12");

    o_ffxFsr3UpscalerContextCreate_Dx12 = (PFN_ffxFsr3UpscalerContextCreate)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerContextCreate");
    o_ffxFsr3UpscalerContextDispatch_Dx12 = (PFN_ffxFsr3UpscalerContextDispatch)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerContextDispatch");
    o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12 = (PFN_ffxFsr3UpscalerContextGenerateReactiveMask)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerContextGenerateReactiveMask");
    o_ffxFsr3UpscalerContextDestroy_Dx12 = (PFN_ffxFsr3UpscalerContextDestroy)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerContextDestroy");
    o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12 = (PFN_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerGetUpscaleRatioFromQualityMode");
    o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12 = (PFN_ffxFsr3UpscalerGetRenderResolutionFromQualityMode)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerGetRenderResolutionFromQualityMode");
    o_ffxFsr3UpscalerGetSharedResourceDescriptions = (PFN_ffxFsr3UpscalerGetSharedResourceDescriptions)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerGetSharedResourceDescriptions");
    o_ffxFsr3UpscalerResourceIsNull = (PFN_ffxFsr3UpscalerResourceIsNull)DetourFindFunction(exeName.c_str(), "ffxFsr3UpscalerResourceIsNull");

    if (o_ffxFSR3GetInterfaceDX12 != nullptr || o_ffxFsr3UpscalerContextCreate_Dx12 != nullptr)
    {
        LOG_INFO("FSR3 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFSR3GetScratchMemorySizeDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFSR3GetScratchMemorySizeDX12, hk_ffxFsr3GetScratchMemorySizeDX12);

        if (o_ffxFSR3GetInterfaceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFSR3GetInterfaceDX12, hk_ffxFsr3GetInterfaceDX12);

        if (o_ffxGetFSR3ResourceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetFSR3ResourceDX12, hk_ffxFsr3GetResourceDX12);

        if (o_ffxFsr3UpscalerContextCreate_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextCreate_Dx12, ffxFsr3ContextCreate_Dx12);

        if (o_ffxFsr3UpscalerContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDispatch_Dx12, ffxFsr3ContextDispatch_Dx12);

        if (o_ffxFsr3UpscalerContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDispatch_Dx12, ffxFsr3ContextDispatch_Dx12);

        if (o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12, ffxFsr3ContextGenerateReactiveMask_Dx12);

        if (o_ffxFsr3UpscalerContextDestroy_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDestroy_Dx12, ffxFsr3ContextDestroy_Dx12);

        if (o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12, ffxFsr3GetUpscaleRatioFromQualityMode_Dx12);

        if (o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12, ffxFsr3GetRenderResolutionFromQualityMode_Dx12);

        if (o_ffxFsr3UpscalerGetSharedResourceDescriptions != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetSharedResourceDescriptions, ffxFsr3UpscalerGetSharedResourceDescriptions);

        if (o_ffxFsr3UpscalerResourceIsNull != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerResourceIsNull, ffxFsr3UpscalerResourceIsNull);

        DetourTransactionCommit();
    }

    LOG_DEBUG("ffxFSR3GetScratchMemorySizeDX12: {:X}", (size_t)o_ffxFSR3GetScratchMemorySizeDX12);
    LOG_DEBUG("ffxFSR3GetInterfaceDX12: {:X}", (size_t)o_ffxFSR3GetInterfaceDX12);
    LOG_DEBUG("ffxGetFSR3ResourceDX12: {:X}", (size_t)o_ffxGetFSR3ResourceDX12);
    LOG_DEBUG("ffxFsr3UpscalerContextCreate_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextCreate_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDispatch_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDispatch_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDispatch_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDispatch_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextGenerateReactiveMask_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDestroy_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDestroy_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetSharedResourceDescriptions: {:X}", (size_t)o_ffxFsr3UpscalerGetSharedResourceDescriptions);
    LOG_DEBUG("ffxFsr3UpscalerResourceIsNull: {:X}", (size_t)o_ffxFsr3UpscalerResourceIsNull);
}

void HookFSR3Dx12Inputs(HMODULE module)
{
    LOG_INFO("Trying to hook FSR3 methods");

    if (o_ffxFSR3GetInterfaceDX12 != nullptr)
        return;

    if (module != nullptr)
    {
        o_ffxFSR3GetScratchMemorySizeDX12 = (PFN_ffxFSR3GetScratchMemorySizeDX12)GetProcAddress(module, "ffxGetScratchMemorySizeDX12");
        o_ffxFSR3GetInterfaceDX12 = (PFN_ffxFSR3GetInterfaceDX12)GetProcAddress(module, "ffxGetInterfaceDX12");
        o_ffxGetFSR3ResourceDX12 = (PFN_ffxFSR3GetResourceDX12)GetProcAddress(module, "ffxGetResourceDX12");
    }

    if (o_ffxFSR3GetInterfaceDX12 != nullptr)
    {
        LOG_INFO("FSR3 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFSR3GetScratchMemorySizeDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFSR3GetScratchMemorySizeDX12, hk_ffxFsr3GetScratchMemorySizeDX12);

        if (o_ffxFSR3GetInterfaceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFSR3GetInterfaceDX12, hk_ffxFsr3GetInterfaceDX12);

        if (o_ffxGetFSR3ResourceDX12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxGetFSR3ResourceDX12, hk_ffxFsr3GetResourceDX12);

        DetourTransactionCommit();
    }

    LOG_DEBUG("ffxFSR3GetScratchMemorySizeDX12: {:X}", (size_t)o_ffxFSR3GetScratchMemorySizeDX12);
    LOG_DEBUG("ffxFSR3GetInterfaceDX12: {:X}", (size_t)o_ffxFSR3GetInterfaceDX12);
    LOG_DEBUG("ffxGetFSR3ResourceDX12: {:X}", (size_t)o_ffxGetFSR3ResourceDX12);
}

void HookFSR3Inputs(HMODULE module)
{
    LOG_INFO("Trying to hook FSR3 methods");

    if (o_ffxFsr3UpscalerContextCreate_Dx12 != nullptr)
        return;

    if (module != nullptr)
    {
        o_ffxFsr3UpscalerContextCreate_Dx12 = (PFN_ffxFsr3UpscalerContextCreate)GetProcAddress(module, "ffxFsr3UpscalerContextCreate");
        o_ffxFsr3UpscalerContextDispatch_Dx12 = (PFN_ffxFsr3UpscalerContextDispatch)GetProcAddress(module, "ffxFsr3UpscalerContextDispatch");
        o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12 = (PFN_ffxFsr3UpscalerContextGenerateReactiveMask)GetProcAddress(module, "ffxFsr3UpscalerContextGenerateReactiveMask");
        o_ffxFsr3UpscalerContextDestroy_Dx12 = (PFN_ffxFsr3UpscalerContextDestroy)GetProcAddress(module, "ffxFsr3UpscalerContextDestroy");
        o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12 = (PFN_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode)GetProcAddress(module, "ffxFsr3UpscalerGetUpscaleRatioFromQualityMode");
        o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12 = (PFN_ffxFsr3UpscalerGetRenderResolutionFromQualityMode)GetProcAddress(module, "ffxFsr3UpscalerGetRenderResolutionFromQualityMode");
        o_ffxFsr3UpscalerGetSharedResourceDescriptions = (PFN_ffxFsr3UpscalerGetSharedResourceDescriptions)GetProcAddress(module, "ffxFsr3UpscalerGetSharedResourceDescriptions");
        o_ffxFsr3UpscalerResourceIsNull = (PFN_ffxFsr3UpscalerResourceIsNull)GetProcAddress(module, "ffxFsr3UpscalerResourceIsNull");
    }

    if (o_ffxFsr3UpscalerContextCreate_Dx12 != nullptr)
    {
        LOG_INFO("FSR3 methods found, now hooking");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ffxFsr3UpscalerContextCreate_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextCreate_Dx12, ffxFsr3ContextCreate_Dx12);

        if (o_ffxFsr3UpscalerContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDispatch_Dx12, ffxFsr3ContextDispatch_Dx12);

        if (o_ffxFsr3UpscalerContextDispatch_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDispatch_Dx12, ffxFsr3ContextDispatch_Dx12);

        if (o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12, ffxFsr3ContextGenerateReactiveMask_Dx12);

        if (o_ffxFsr3UpscalerContextDestroy_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerContextDestroy_Dx12, ffxFsr3ContextDestroy_Dx12);

        if (o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12, ffxFsr3GetUpscaleRatioFromQualityMode_Dx12);

        if (o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12 != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12, ffxFsr3GetRenderResolutionFromQualityMode_Dx12);

        if (o_ffxFsr3UpscalerGetSharedResourceDescriptions != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerGetSharedResourceDescriptions, ffxFsr3UpscalerGetSharedResourceDescriptions);

        if (o_ffxFsr3UpscalerResourceIsNull != nullptr)
            DetourAttach(&(PVOID&)o_ffxFsr3UpscalerResourceIsNull, ffxFsr3UpscalerResourceIsNull);

        DetourTransactionCommit();
    }

    LOG_DEBUG("ffxFsr3UpscalerContextCreate_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextCreate_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDispatch_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDispatch_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDispatch_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDispatch_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextGenerateReactiveMask_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextGenerateReactiveMask_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerContextDestroy_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerContextDestroy_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerGetUpscaleRatioFromQualityMode_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12: {:X}", (size_t)o_ffxFsr3UpscalerGetRenderResolutionFromQualityMode_Dx12);
    LOG_DEBUG("ffxFsr3UpscalerGetSharedResourceDescriptions: {:X}", (size_t)o_ffxFsr3UpscalerGetSharedResourceDescriptions);
    LOG_DEBUG("ffxFsr3UpscalerResourceIsNull: {:X}", (size_t)o_ffxFsr3UpscalerResourceIsNull);
}
