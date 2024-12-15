//#include "FSR2_Dx12.h"
//
//#include "Config.h"
//#include "Util.h"
//
//#include "resource.h"
//#include "NVNGX_Parameter.h"
//
//#include "fsr2_212/include/ffx_fsr2.h"
//#include "fsr2_212/include/dx12/ffx_fsr2_dx12.h"
//
//static UINT64 _handleCounter = 0x73310000;
//
//static Fsr212::FfxFsr2Context* _currentContext = nullptr;
//static std::map<Fsr212::FfxFsr2Context*, Fsr212::FfxFsr2ContextDescription> _initParams;
//static std::map<Fsr212::FfxFsr2Context*, NVSDK_NGX_Parameter*> _nvParams;
//static std::map<Fsr212::FfxFsr2Context*, NVSDK_NGX_Handle*> _contexts;
//static ID3D12Device* _d3d12Device = nullptr;
//static bool _nvnxgInited = false;
//
//static bool CreateDLSSContext(Fsr212::FfxFsr2Context* handle, const Fsr212::FfxFsr2DispatchDescription* pExecParams)
//{
//    LOG_DEBUG("");
//
//    if (!_nvParams.contains(handle))
//        return false;
//
//    NVSDK_NGX_Handle* nvHandle = nullptr;
//    auto params = _nvParams[handle];
//    auto initParams = &_initParams[handle];
//    auto commandList = (ID3D12GraphicsCommandList*)pExecParams->commandList;
//
//    UINT initFlags = 0;
//
//    if (initParams->flags & FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE)
//        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
//
//    if (initParams->flags & FFX_UPSCALE_ENABLE_DEPTH_INVERTED)
//        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
//
//    if (initParams->flags & FFX_UPSCALE_ENABLE_AUTO_EXPOSURE)
//        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
//
//    if (initParams->flags & FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION)
//        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
//
//    if ((initParams->flags & FFX_UPSCALE_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS) == 0)
//        initFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
//
//    params->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, initFlags);
//
//    params->Set(NVSDK_NGX_Parameter_Width, pExecParams->renderSize.width);
//    params->Set(NVSDK_NGX_Parameter_Height, pExecParams->renderSize.height);
//    params->Set(NVSDK_NGX_Parameter_OutWidth, initParams->maxUpscaleSize.width);
//    params->Set(NVSDK_NGX_Parameter_OutHeight, initParams->maxUpscaleSize.height);
//
//    auto ratio = (float)initParams->maxUpscaleSize.width / (float)pExecParams->renderSize.width;
//
//    if (ratio <= 3.0)
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraPerformance);
//    else if (ratio <= 2.0)
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxPerf);
//    else if (ratio <= 1.7)
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_Balanced);
//    else if (ratio <= 1.5)
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
//    else if (ratio <= 1.3)
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_UltraQuality);
//    else
//        params->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_DLAA);
//
//    if (NVSDK_NGX_D3D12_CreateFeature(commandList, NVSDK_NGX_Feature_SuperSampling, params, &nvHandle) != NVSDK_NGX_Result_Success)
//        return false;
//
//    _contexts[handle] = nvHandle;
//
//    return true;
//}
//
//static std::optional<float> GetQualityOverrideRatioFfx(const Fsr212::FfxFsr2QualityMode input)
//{
//    std::optional<float> output;
//
//    auto sliderLimit = Config::Instance()->ExtendedLimits.value_or(false) ? 0.1f : 1.0f;
//
//    if (Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false) &&
//        Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f) >= sliderLimit)
//    {
//        output = Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f);
//
//        return  output;
//    }
//
//    if (!Config::Instance()->QualityRatioOverrideEnabled.value_or(false))
//        return output; // override not enabled
//
//    switch (input)
//    {
//        case FFX_UPSCALE_QUALITY_MODE_ULTRA_PERFORMANCE:
//            if (Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0) >= sliderLimit)
//                output = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0);
//
//            break;
//
//        case FFX_UPSCALE_QUALITY_MODE_PERFORMANCE:
//            if (Config::Instance()->QualityRatio_Performance.value_or(2.0) >= sliderLimit)
//                output = Config::Instance()->QualityRatio_Performance.value_or(2.0);
//
//            break;
//
//        case FFX_UPSCALE_QUALITY_MODE_BALANCED:
//            if (Config::Instance()->QualityRatio_Balanced.value_or(1.7) >= sliderLimit)
//                output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);
//
//            break;
//
//        case FFX_UPSCALE_QUALITY_MODE_QUALITY:
//            if (Config::Instance()->QualityRatio_Quality.value_or(1.5) >= sliderLimit)
//                output = Config::Instance()->QualityRatio_Quality.value_or(1.5);
//
//            break;
//
//        case FFX_UPSCALE_QUALITY_MODE_NATIVEAA:
//            if (Config::Instance()->QualityRatio_DLAA.value_or(1.0) >= sliderLimit)
//                output = Config::Instance()->QualityRatio_DLAA.value_or(1.0);
//
//            break;
//
//        default:
//            LOG_WARN("Unknown quality: {0}", (int)input);
//            break;
//    }
//
//    return output;
//}
//
//// FS2 Upscaler
//static Fsr212::FfxErrorCode ffxFsr2ContextCreate_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2ContextDescription* contextDescription)
//{
//
//}
//
//static Fsr212::FfxErrorCode ffxFsr2ContextDispatch_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2DispatchDescription* dispatchDescription)
//{
//
//}
//
//static Fsr212::FfxErrorCode ffxFsr2ContextGenerateReactiveMask_Dx12(Fsr212::FfxFsr2Context* context, const Fsr212::FfxFsr2GenerateReactiveDescription* params)
//{
//
//}
//
//static Fsr212::FfxErrorCode ffxFsr2ContextDestroy_Dx12(Fsr212::FfxFsr2Context* context)
//{
//
//}
//
//static float ffxFsr2GetUpscaleRatioFromQualityMode_Dx12(Fsr212::FfxFsr2QualityMode qualityMode)
//{
//
//}
//
//static Fsr212::FfxErrorCode ffxFsr2GetRenderResolutionFromQualityMode_Dx12(uint32_t* renderWidth, uint32_t* renderHeight,
//                                                                    uint32_t displayWidth, uint32_t displayHeight, Fsr212::FfxFsr2QualityMode qualityMode)
//{
//
//}
//
//// Dx12 Backend
//static size_t hk_ffxFsr2GetScratchMemorySizeDX12()
//{
//
//}
//
//static Fsr212::FfxErrorCode hk_ffxFsr2GetInterfaceDX12(Fsr212::FfxFsr2Interface212* fsr2Interface, ID3D12Device* device, 
//                                                       void* scratchBuffer, size_t scratchBufferSize)
//{
//
//}
//
//static Fsr212::FfxResource hk_ffxGetResourceDX12(Fsr212::FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name = nullptr, 
//                                                 Fsr212::FfxResourceStates state = Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ, 
//                                                 UINT shaderComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
//{
//
//}
//
//static ID3D12Resource* hk_ffxGetDX12ResourcePtr(Fsr212::FfxFsr2Context* context, uint32_t resId)
//{
//
//}
//
//void HookFSR2Inputs()
//{
//
//}
