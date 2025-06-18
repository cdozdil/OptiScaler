#include "DLSSDFeature.h"

#include <pch.h>
#include <Config.h>
#include <Util.h>

#include <detours/detours.h>

void DLSSDFeature::ProcessEvaluateParams(NVSDK_NGX_Parameter* InParameters)
{
    float floatValue;

    // override sharpness
    if (Config::Instance()->OverrideSharpness.value_or_default() &&
        !(State::Instance().api == DX12 && Config::Instance()->RcasEnabled.value_or_default()))
    {
        auto sharpness = Config::Instance()->Sharpness.value_or_default();

        if (sharpness > 1.0f)
            sharpness = 1.0f;

        InParameters->Set(NVSDK_NGX_Parameter_Sharpness, sharpness);
    }
    // rcas enabled
    else
    {
        InParameters->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);
    }

    // Read render resolution
    unsigned int width;
    unsigned int height;
    GetRenderResolution(InParameters, &width, &height);
}

void DLSSDFeature::ProcessInitParams(NVSDK_NGX_Parameter* InParameters)
{
    unsigned int uintValue;

    // Create flags -----------------------------
    unsigned int featureFlags = 0;

    if (DepthInverted())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;

    if (AutoExposure())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

    if (IsHdr())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;

    if (JitteredMV())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;

    if (LowResMV())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;

    if (SharpenEnabled())
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;

    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, featureFlags);

    // Resolution -----------------------------
    if (State::Instance().api != Vulkan && Config::Instance()->OutputScalingEnabled.value_or_default() && LowResMV())
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
        _targetWidth = RenderWidth();
        _targetHeight = RenderHeight();

        // enable output scaling to restore image
        if (LowResMV())
        {
            Config::Instance()->OutputScalingMultiplier.set_volatile_value(1.0f);
            Config::Instance()->OutputScalingEnabled.set_volatile_value(true);
        }
    }

    InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
    InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());
    InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
    InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());

    if (Config::Instance()->RenderPresetOverride.value_or_default())
    {
        uint32_t RenderPresetDLAA = 0;
        uint32_t RenderPresetUltraQuality = 0;
        uint32_t RenderPresetQuality = 0;
        uint32_t RenderPresetBalanced = 0;
        uint32_t RenderPresetPerformance = 0;
        uint32_t RenderPresetUltraPerformance = 0;

        InParameters->Get("RayReconstruction.Hint.Render.Preset.DLAA", &RenderPresetDLAA);
        InParameters->Get("RayReconstruction.Hint.Render.Preset.UltraQuality", &RenderPresetUltraQuality);
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Quality", &RenderPresetQuality);
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Balanced", &RenderPresetBalanced);
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Performance", &RenderPresetPerformance);
        InParameters->Get("RayReconstruction.Hint.Render.Preset.UltraPerformance", &RenderPresetUltraPerformance);

        if (Config::Instance()->RenderPresetOverride.value_or_default())
        {
            RenderPresetDLAA = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetDLAA.value_or(RenderPresetDLAA));
            RenderPresetUltraQuality = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetUltraQuality.value_or(RenderPresetUltraQuality));
            RenderPresetQuality = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetQuality.value_or(RenderPresetQuality));
            RenderPresetBalanced = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetBalanced.value_or(RenderPresetBalanced));
            RenderPresetPerformance = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetPerformance.value_or(RenderPresetPerformance));
            RenderPresetUltraPerformance = Config::Instance()->RenderPresetForAll.value_or(
                Config::Instance()->RenderPresetUltraPerformance.value_or(RenderPresetUltraPerformance));

            LOG_DEBUG("Preset override active, config overrides:");
            LOG_DEBUG("Preset_DLAA {}", RenderPresetDLAA);
            LOG_DEBUG("Preset_UltraQuality {}", RenderPresetUltraQuality);
            LOG_DEBUG("Preset_Quality {}", RenderPresetQuality);
            LOG_DEBUG("Preset_Balanced {}", RenderPresetBalanced);
            LOG_DEBUG("Preset_Performance {}", RenderPresetPerformance);
            LOG_DEBUG("Preset_UltraPerformance {}", RenderPresetUltraPerformance);
        }

        InParameters->Set("RayReconstruction.Hint.Render.Preset.DLAA", RenderPresetDLAA);
        InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraQuality", RenderPresetUltraQuality);
        InParameters->Set("RayReconstruction.Hint.Render.Preset.Quality", RenderPresetQuality);
        InParameters->Set("RayReconstruction.Hint.Render.Preset.Balanced", RenderPresetBalanced);
        InParameters->Set("RayReconstruction.Hint.Render.Preset.Performance", RenderPresetPerformance);
        InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraPerformance", RenderPresetUltraPerformance);
    }

    UINT perfQ = NVSDK_NGX_PerfQuality_Value_Balanced;
    if (InParameters->Get(NVSDK_NGX_Parameter_PerfQualityValue, &perfQ) == NVSDK_NGX_Result_Success &&
        perfQ == NVSDK_NGX_PerfQuality_Value_UltraQuality)
    {
        InParameters->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
    }

    int paramValue = -1;

    if (InParameters->Get("DLSS.Use.HW.Depth", &paramValue) == NVSDK_NGX_Result_Success)
        LOG_DEBUG("DLSS.Use.HW.Depth: {}", paramValue);

    if (InParameters->Get("DLSS.Denoise.Mode", &paramValue) == NVSDK_NGX_Result_Success)
        LOG_DEBUG("DLSS.Denoise.Mode: {}", paramValue);

    if (InParameters->Get("DLSS.Roughness.Mode", &paramValue) == NVSDK_NGX_Result_Success)
        LOG_DEBUG("DLSS.Roughness.Mode: {}", paramValue);

    LOG_FUNC_RESULT(0);
}

void DLSSDFeature::ReadVersion()
{
    LOG_FUNC();

    std::vector<std::string> possibleDlls;

    if (!State::Instance().NGX_OTA_Dlssd.empty())
        possibleDlls.push_back(State::Instance().NGX_OTA_Dlssd);

    possibleDlls.push_back("nvngx_dlssd.dll");

    _version = GetVersionUsingNGXSnippet(possibleDlls);

    if (_version > feature_version { 0, 0, 0 })
        LOG_INFO("DLSSD v{0}.{1}.{2} loaded.", _version.major, _version.minor, _version.patch);
    else
        LOG_WARN("Failed to get version using NVSDK_NGX_GetSnippetVersion!");
}

DLSSDFeature::DLSSDFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
{
    _initParameters = SetInitParameters(InParameters);

    if (NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (NVNGXProxy::NVNGXModule() != nullptr && !State::Instance().enablerAvailable)
    {
        HookNgxApi(NVNGXProxy::NVNGXModule());
    }

    _moduleLoaded = NVNGXProxy::NVNGXModule() != nullptr;
}

DLSSDFeature::~DLSSDFeature() {}

void DLSSDFeature::Shutdown() {}

float DLSSDFeature::GetSharpness(const NVSDK_NGX_Parameter* InParameters)
{
    if (Config::Instance()->OverrideSharpness.value_or_default())
        return Config::Instance()->Sharpness.value_or_default();

    float sharpness = 0.0f;
    InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &sharpness);

    return sharpness;
}
