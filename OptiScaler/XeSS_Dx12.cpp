#include "Config.h"
#include "Util.h"

#include "resource.h"
#include "Util.h"
#include "xess_d3d12.h"
#include "xess_d3d12_debug.h"
#include "XeSS_Proxy.h"
#include "NVNGX_Parameter.h"
#include "imgui/imgui_overlay_dx.h"

typedef struct MotionScale
{
    float x;
    float y;
} motion_scale;

static UINT64 _handleCounter = 13370000;
static UINT64 _frameCounter = 0;
static xess_context_handle_t _currentContext = nullptr;
static std::map<xess_context_handle_t, xess_d3d12_init_params_t> _initParams;
static std::map<xess_context_handle_t, NVSDK_NGX_Parameter*> _nvParams;
static std::map<xess_context_handle_t, NVSDK_NGX_Handle*> _contexts;
static std::map<xess_context_handle_t, MotionScale> _motionScales;
static ID3D12Device* _d3d12Device = nullptr;

static bool CreateDLSSContext(xess_context_handle_t handle, ID3D12GraphicsCommandList* commandList, const xess_d3d12_execute_params_t* pExecParams)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(handle))
        return false;

    NVSDK_NGX_Handle* nvHandle = nullptr;
    auto params = _nvParams[handle];
    auto initParams = &_initParams[handle];

    UINT initFlags = 0;

    if ((initParams->initFlags & XESS_INIT_FLAG_LDR_INPUT_COLOR) == 0)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    if (initParams->initFlags & XESS_INIT_FLAG_INVERTED_DEPTH)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;

    if (initParams->initFlags & XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (initParams->initFlags & XESS_INIT_FLAG_JITTERED_MV)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;

    if ((initParams->initFlags & XESS_INIT_FLAG_HIGH_RES_MV) == 0)
        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;

    params->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, initFlags);

    params->Set(NVSDK_NGX_Parameter_Width, pExecParams->inputWidth);
    params->Set(NVSDK_NGX_Parameter_Height, pExecParams->inputHeight);
    params->Set(NVSDK_NGX_Parameter_OutWidth, initParams->outputResolution.x);
    params->Set(NVSDK_NGX_Parameter_OutHeight, initParams->outputResolution.y);

    switch (initParams->qualitySetting)
    {
        case XESS_QUALITY_SETTING_ULTRA_PERFORMANCE:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraPerformance);
            break;

        case XESS_QUALITY_SETTING_PERFORMANCE:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
            break;

        case XESS_QUALITY_SETTING_BALANCED:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_Balanced);
            break;

        case XESS_QUALITY_SETTING_QUALITY:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
            break;

        case XESS_QUALITY_SETTING_ULTRA_QUALITY:
        case XESS_QUALITY_SETTING_ULTRA_QUALITY_PLUS:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraQuality);
            break;

        case XESS_QUALITY_SETTING_AA:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_DLAA);
            break;

        default:
            params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_Balanced);
            break;
    }

    if (NVSDK_NGX_D3D12_CreateFeature(commandList, NVSDK_NGX_Feature_SuperSampling, params, &nvHandle) != NVSDK_NGX_Result_Success)
        return false;

    _contexts[handle] = nvHandle;

    return true;
}

static std::optional<float> GetQualityOverrideRatio(const xess_quality_settings_t input)
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
        case XESS_QUALITY_SETTING_ULTRA_PERFORMANCE:
            if (Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0);

            break;

        case XESS_QUALITY_SETTING_PERFORMANCE:
            if (Config::Instance()->QualityRatio_Performance.value_or(2.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Performance.value_or(2.0);

            break;

        case XESS_QUALITY_SETTING_BALANCED:
            if (Config::Instance()->QualityRatio_Balanced.value_or(1.7) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);

            break;

        case XESS_QUALITY_SETTING_QUALITY:
            if (Config::Instance()->QualityRatio_Quality.value_or(1.5) >= sliderLimit)
                output = Config::Instance()->QualityRatio_Quality.value_or(1.5);

            break;

        case XESS_QUALITY_SETTING_ULTRA_QUALITY:
        case XESS_QUALITY_SETTING_ULTRA_QUALITY_PLUS:
            if (Config::Instance()->QualityRatio_UltraQuality.value_or(1.3) >= sliderLimit)
                output = Config::Instance()->QualityRatio_UltraQuality.value_or(1.3);

            break;

        case XESS_QUALITY_SETTING_AA:
            if (Config::Instance()->QualityRatio_DLAA.value_or(1.0) >= sliderLimit)
                output = Config::Instance()->QualityRatio_DLAA.value_or(1.0);

            break;

        default:
            LOG_WARN("Unknown quality: {0}", (int)input);
            break;
    }

    return output;
}

XESS_API xess_result_t xessD3D12CreateContext(ID3D12Device* pDevice, xess_context_handle_t* phContext)
{
    LOG_DEBUG("");

    if (pDevice == nullptr)
        return XESS_RESULT_ERROR_DEVICE;

    Util::DllPath();

    NVSDK_NGX_FeatureCommonInfo fcInfo{};
    wchar_t const** paths = new const wchar_t* [1];
    auto dllPath = Util::DllPath().remove_filename().wstring();
    paths[0] = dllPath.c_str();
    fcInfo.PathListInfo.Path = paths;
    fcInfo.PathListInfo.Length = 1;

    auto nvResult = NVSDK_NGX_D3D12_Init_with_ProjectID("OptiScaler", NVSDK_NGX_ENGINE_TYPE_CUSTOM, VER_PRODUCT_VERSION_STR, dllPath.c_str(),
                                                        pDevice, &fcInfo, Config::Instance()->NVNGX_Version);

    if (nvResult != NVSDK_NGX_Result_Success)
        return XESS_RESULT_ERROR_UNINITIALIZED;

    _d3d12Device = pDevice;
    *phContext = (xess_context_handle_t)++_handleCounter;

    NVSDK_NGX_Parameter* params = nullptr;

    if (NVSDK_NGX_D3D12_GetCapabilityParameters(&params) != NVSDK_NGX_Result_Success)
        return XESS_RESULT_ERROR_INVALID_ARGUMENT;

    _nvParams[*phContext] = params;
    _motionScales[*phContext] = { 1.0, 1.0 };

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12BuildPipelines(xess_context_handle_t hContext, ID3D12PipelineLibrary* pPipelineLibrary, bool blocking, uint32_t initFlags)
{
    LOG_WARN("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12Init(xess_context_handle_t hContext, const xess_d3d12_init_params_t* pInitParams)
{
    LOG_DEBUG("");

    xess_d3d12_init_params_t ip{};
    ip.bufferHeapOffset = pInitParams->bufferHeapOffset;
    ip.creationNodeMask = pInitParams->creationNodeMask;
    ip.initFlags = pInitParams->initFlags;
    ip.outputResolution = pInitParams->outputResolution;
    ip.pPipelineLibrary = pInitParams->pPipelineLibrary;
    ip.pTempBufferHeap = pInitParams->pTempBufferHeap;
    ip.pTempTextureHeap = pInitParams->pTempTextureHeap;
    ip.qualitySetting = pInitParams->qualitySetting;
    ip.textureHeapOffset = pInitParams->textureHeapOffset;
    ip.visibleNodeMask = pInitParams->visibleNodeMask;

    _initParams[hContext] = ip;

    if (!_contexts.contains(hContext))
        return XESS_RESULT_SUCCESS;

    NVSDK_NGX_D3D12_ReleaseFeature(_contexts[hContext]);
    _contexts.erase(hContext);

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12Execute(xess_context_handle_t hContext, ID3D12GraphicsCommandList* pCommandList, const xess_d3d12_execute_params_t* pExecParams)
{
    LOG_DEBUG("");

    if (pCommandList == nullptr)
        return XESS_RESULT_ERROR_INVALID_ARGUMENT;

    if (!_contexts.contains(hContext) && !CreateDLSSContext(hContext, pCommandList, pExecParams))
        return XESS_RESULT_ERROR_UNKNOWN;

    NVSDK_NGX_Parameter* params = _nvParams[hContext];
    NVSDK_NGX_Handle* handle = _contexts[hContext];
    xess_d3d12_init_params_t* initParams = &_initParams[hContext];

    if (_motionScales.contains(hContext))
    {
        auto scales = &_motionScales[hContext];

        if ((initParams->initFlags & XESS_INIT_FLAG_USE_NDC_VELOCITY))
        {
            if (initParams->initFlags & XESS_INIT_FLAG_HIGH_RES_MV)
            {
                params->Set(NVSDK_NGX_Parameter_MV_Scale_X, initParams->outputResolution.x * 0.5 * scales->x);
                params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, initParams->outputResolution.y * -0.5 * scales->y);
            }
            else
            {
                params->Set(NVSDK_NGX_Parameter_MV_Scale_X, pExecParams->inputWidth * 0.5 * scales->x);
                params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, pExecParams->inputHeight * -0.5 * scales->y);
            }
        }
        else
        {
            params->Set(NVSDK_NGX_Parameter_MV_Scale_X, scales->x);
            params->Set(NVSDK_NGX_Parameter_MV_Scale_Y, scales->y);
        }
    }

    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_X, pExecParams->jitterOffsetX);
    params->Set(NVSDK_NGX_Parameter_Jitter_Offset_Y, pExecParams->jitterOffsetY);
    params->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, pExecParams->exposureScale);
    params->Set(NVSDK_NGX_Parameter_Reset, pExecParams->resetHistory);
    params->Set(NVSDK_NGX_Parameter_Width, pExecParams->inputWidth);
    params->Set(NVSDK_NGX_Parameter_Height, pExecParams->inputHeight);
    params->Set(NVSDK_NGX_Parameter_Depth, pExecParams->pDepthTexture);
    params->Set(NVSDK_NGX_Parameter_ExposureTexture, pExecParams->pExposureScaleTexture);
    params->Set(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, pExecParams->pResponsivePixelMaskTexture);
    params->Set(NVSDK_NGX_Parameter_Color, pExecParams->pColorTexture);
    params->Set(NVSDK_NGX_Parameter_MotionVectors, pExecParams->pVelocityTexture);
    params->Set(NVSDK_NGX_Parameter_Output, pExecParams->pOutputTexture);

    Config::Instance()->setInputApiName = "XeSS";

    if (NVSDK_NGX_D3D12_EvaluateFeature(pCommandList, handle, params, nullptr) == NVSDK_NGX_Result_Success)
        return XESS_RESULT_SUCCESS;

    return XESS_RESULT_ERROR_UNKNOWN;
}

XESS_API xess_result_t xessGetVersion(xess_version_t* pVersion)
{
    LOG_DEBUG("");

    pVersion->major = XeSSProxy::Version().major;
    pVersion->minor = XeSSProxy::Version().minor;
    pVersion->patch = XeSSProxy::Version().patch;
    pVersion->reserved = XeSSProxy::Version().reserved;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessIsOptimalDriver(xess_context_handle_t hContext)
{
    LOG_DEBUG("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessSetLoggingCallback(xess_context_handle_t hContext, xess_logging_level_t loggingLevel, xess_app_log_callback_t loggingCallback)
{
    LOG_DEBUG("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetProperties(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_properties_t* pBindingProperties)
{
    *pBindingProperties = {};

    LOG_WARN("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessDestroyContext(xess_context_handle_t hContext)
{
    LOG_DEBUG("");

    if (!_contexts.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    NVSDK_NGX_D3D12_ReleaseFeature(_contexts[hContext]);

    _contexts.erase(hContext);
    _nvParams.erase(hContext);
    _initParams.erase(hContext);

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessSetVelocityScale(xess_context_handle_t hContext, float x, float y)
{
    LOG_DEBUG("");

    _motionScales[hContext] = { x, y };

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12GetInitParams(xess_context_handle_t hContext, xess_d3d12_init_params_t* pInitParams)
{
    LOG_DEBUG("");

    if (!_initParams.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    auto ip = &_initParams[hContext];

    pInitParams->bufferHeapOffset = ip->bufferHeapOffset;
    pInitParams->creationNodeMask = ip->creationNodeMask;
    pInitParams->initFlags = ip->initFlags;
    pInitParams->outputResolution = ip->outputResolution;
    pInitParams->pPipelineLibrary = ip->pPipelineLibrary;
    pInitParams->pTempBufferHeap = ip->pTempBufferHeap;
    pInitParams->pTempTextureHeap = ip->pTempTextureHeap;
    pInitParams->qualitySetting = ip->qualitySetting;
    pInitParams->textureHeapOffset = ip->textureHeapOffset;
    pInitParams->visibleNodeMask = ip->visibleNodeMask;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessForceLegacyScaleFactors(xess_context_handle_t hContext, bool force)
{
    LOG_DEBUG("");

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetExposureMultiplier(xess_context_handle_t hContext, float* pScale)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    if (_nvParams[hContext]->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, pScale) == NVSDK_NGX_Result_Success)
        return XESS_RESULT_SUCCESS;

    return XESS_RESULT_ERROR_UNKNOWN;
}

XESS_API xess_result_t xessGetInputResolution(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_quality_settings_t qualitySettings, xess_2d_t* pInputResolution)
{
    LOG_DEBUG("");

    unsigned int OutWidth;
    unsigned int OutHeight;
    float scalingRatio = 0.0f;

    const std::optional<float> QualityRatio = GetQualityOverrideRatio(qualitySettings);

    if (QualityRatio.has_value())
    {
        OutHeight = (unsigned int)((float)pOutputResolution->x / QualityRatio.value());
        OutWidth = (unsigned int)((float)pOutputResolution->y / QualityRatio.value());
        scalingRatio = 1.0f / QualityRatio.value();
    }
    else
    {
        switch (qualitySettings)
        {
            case XESS_QUALITY_SETTING_ULTRA_PERFORMANCE:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 3.0);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 3.0);
                scalingRatio = 0.33333333f;
                break;

            case XESS_QUALITY_SETTING_PERFORMANCE:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 2.0);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 2.0);
                scalingRatio = 0.5f;
                break;

            case XESS_QUALITY_SETTING_BALANCED:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.7);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.7);
                scalingRatio = 1.0f / 1.7f;
                break;

            case XESS_QUALITY_SETTING_QUALITY:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.5);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.5);
                scalingRatio = 1.0f / 1.5f;
                break;

            case XESS_QUALITY_SETTING_ULTRA_QUALITY:
            case XESS_QUALITY_SETTING_ULTRA_QUALITY_PLUS:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.3);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.3);
                scalingRatio = 1.0f / 1.3f;
                break;

            case XESS_QUALITY_SETTING_AA:
                OutHeight = pOutputResolution->y;
                OutWidth = pOutputResolution->x;
                scalingRatio = 1.0f;
                break;

            default:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.7);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.7);
                scalingRatio = 1.0f / 1.7f;
                break;

        }
    }

    if (Config::Instance()->RoundInternalResolution.has_value())
    {
        OutHeight -= OutHeight % Config::Instance()->RoundInternalResolution.value();
        OutWidth -= OutWidth % Config::Instance()->RoundInternalResolution.value();
        scalingRatio = (float)OutWidth / (float)pOutputResolution->x;
    }

    pInputResolution->x = OutWidth;
    pInputResolution->y = OutHeight;

    LOG_DEBUG("Display Resolution: {0}x{1} Render Resolution: {2}x{3}, Quality: {4}", pOutputResolution->x, pOutputResolution->y, pInputResolution->x, pInputResolution->y, (UINT)qualitySettings);

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetIntelXeFXVersion(xess_context_handle_t hContext, xess_version_t* pVersion)
{
    LOG_DEBUG("");

    pVersion->major = XeSSProxy::Version().major;
    pVersion->minor = XeSSProxy::Version().minor;
    pVersion->patch = XeSSProxy::Version().patch;
    pVersion->reserved = XeSSProxy::Version().reserved;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetJitterScale(xess_context_handle_t hContext, float* pX, float* pY)
{
    LOG_DEBUG("");

    *pX = 1.0f;
    *pY = 1.0f;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetOptimalInputResolution(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_quality_settings_t qualitySettings,
                                                     xess_2d_t* pInputResolutionOptimal, xess_2d_t* pInputResolutionMin, xess_2d_t* pInputResolutionMax)
{
    LOG_DEBUG("");

    unsigned int OutWidth;
    unsigned int OutHeight;
    float scalingRatio = 0.0f;

    const std::optional<float> QualityRatio = GetQualityOverrideRatio(qualitySettings);

    if (QualityRatio.has_value())
    {
        OutHeight = (unsigned int)((float)pOutputResolution->x / QualityRatio.value());
        OutWidth = (unsigned int)((float)pOutputResolution->y / QualityRatio.value());
        scalingRatio = 1.0f / QualityRatio.value();
    }
    else
    {
        switch (qualitySettings)
        {
            case XESS_QUALITY_SETTING_ULTRA_PERFORMANCE:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 3.0);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 3.0);
                scalingRatio = 0.33333333f;
                break;

            case XESS_QUALITY_SETTING_PERFORMANCE:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 2.0);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 2.0);
                scalingRatio = 0.5f;
                break;

            case XESS_QUALITY_SETTING_BALANCED:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.7);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.7);
                scalingRatio = 1.0f / 1.7f;
                break;

            case XESS_QUALITY_SETTING_QUALITY:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.5);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.5);
                scalingRatio = 1.0f / 1.5f;
                break;

            case XESS_QUALITY_SETTING_ULTRA_QUALITY:
            case XESS_QUALITY_SETTING_ULTRA_QUALITY_PLUS:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.3);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.3);
                scalingRatio = 1.0f / 1.3f;
                break;

            case XESS_QUALITY_SETTING_AA:
                OutHeight = pOutputResolution->y;
                OutWidth = pOutputResolution->x;
                scalingRatio = 1.0f;
                break;

            default:
                OutHeight = (unsigned int)((float)pOutputResolution->y / 1.7);
                OutWidth = (unsigned int)((float)pOutputResolution->x / 1.7);
                scalingRatio = 1.0f / 1.7f;
                break;

        }
    }

    if (Config::Instance()->RoundInternalResolution.has_value())
    {
        OutHeight -= OutHeight % Config::Instance()->RoundInternalResolution.value();
        OutWidth -= OutWidth % Config::Instance()->RoundInternalResolution.value();
        scalingRatio = (float)OutWidth / (float)pOutputResolution->x;
    }

    pInputResolutionOptimal->x = OutWidth;
    pInputResolutionOptimal->y = OutHeight;
    pInputResolutionMin->x = (unsigned int)((float)pOutputResolution->x / 3.0);
    pInputResolutionMin->y = (unsigned int)((float)pOutputResolution->y / 3.0);
    pInputResolutionMax->x = pOutputResolution->x;
    pInputResolutionMax->y = pOutputResolution->y;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessGetVelocityScale(xess_context_handle_t hContext, float* pX, float* pY)
{
    LOG_DEBUG("");

    if (!_motionScales.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    auto scales = &_motionScales[hContext];

    *pX = scales->x;
    *pY = scales->y;

    return XESS_RESULT_ERROR_UNKNOWN;
}

XESS_API xess_result_t xessSetJitterScale(xess_context_handle_t hContext, float x, float y)
{
    LOG_DEBUG("x: {}, y: {}", x, y);
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessSetExposureMultiplier(xess_context_handle_t hContext, float scale)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    _nvParams[hContext]->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, scale);

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12GetResourcesToDump(xess_context_handle_t hContext, xess_resources_to_dump_t** pResourcesToDump)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessD3D12GetProfilingData(xess_context_handle_t hContext, xess_profiling_data_t** pProfilingData)
{
    LOG_DEBUG("");

    if (!_nvParams.contains(hContext))
        return XESS_RESULT_ERROR_INVALID_CONTEXT;

    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessSetContextParameterF()
{
    LOG_DEBUG("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessAILGetDecision()
{
    LOG_DEBUG("");
    return XESS_RESULT_SUCCESS;
}

XESS_API xess_result_t xessAILGetVersion()
{
    LOG_DEBUG("");
    return XESS_RESULT_SUCCESS;
}
