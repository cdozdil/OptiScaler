#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "DLSSFeature.h"

void DLSSFeature::ProcessEvaluateParams(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    float floatValue;

    // override sharpness
    if (Config::Instance()->OverrideSharpness.value_or_default() && !(State::Instance().api != Vulkan && Config::Instance()->RcasEnabled.value_or_default()))
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

    LOG_INFO("Render Size: {}x{}, Target Size: {}x{}, Display Size: {}x{}", RenderWidth(), RenderHeight(), TargetWidth(), TargetHeight(), DisplayWidth(), DisplayHeight());
}

void DLSSFeature::ProcessInitParams(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    unsigned int uintValue;

    // Create flags -----------------------------
    unsigned int featureFlags = 0;

    bool isHdr = false;
    bool mvLowRes = false;
    bool mvJittered = false;
    bool depthInverted = false;
    bool sharpening = false;
    bool autoExposure = false;

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &uintValue) == NVSDK_NGX_Result_Success)
    {
        LOG_INFO("featureFlags {0:X}", uintValue);

        isHdr = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_IsHDR);
        mvLowRes = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
        mvJittered = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_MVJittered);
        depthInverted = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted);
        sharpening = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening);
        autoExposure = (uintValue & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure);
    }

    if (Config::Instance()->DepthInverted.value_or(depthInverted))
    {
        Config::Instance()->DepthInverted = true;
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
        LOG_INFO("featureFlags (DepthInverted) {0:b}", featureFlags);
    }
    else
    {
        LOG_INFO("featureFlags (!DepthInverted) {0:b}", featureFlags);
    }

    if (Config::Instance()->AutoExposure.value_or(autoExposure))
    {
        Config::Instance()->AutoExposure = true;
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
        LOG_INFO("featureFlags (AutoExposure) {0:b}", featureFlags);
    }
    else
    {
        LOG_INFO("featureFlags (!AutoExposure) {0:b}", featureFlags);
    }

    if (Config::Instance()->HDR.value_or(isHdr))
    {
        Config::Instance()->HDR = true;
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
        LOG_INFO("featureFlags (HDR) {0:b}", featureFlags);
    }
    else
    {
        Config::Instance()->HDR = false;
        LOG_INFO("featureFlags (!HDR) {0:b}", featureFlags);
    }

    if (Config::Instance()->JitterCancellation.value_or(mvJittered))
    {
        Config::Instance()->JitterCancellation = true;
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
        LOG_INFO("featureFlags (JitterCancellation) {0:b}", featureFlags);
    }
    else
    {
        Config::Instance()->JitterCancellation = false;
        LOG_INFO("featureFlags (!JitterCancellation) {0:b}", featureFlags);
    }

    if (Config::Instance()->DisplayResolution.value_or(!mvLowRes))
    {
        LOG_INFO("featureFlags (!LowResMV) {0:b}", featureFlags);
    }
    else
    {
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
        LOG_INFO("featureFlags (LowResMV) {0:b}", featureFlags);
    }

    if (Config::Instance()->OverrideSharpness.value_or(sharpening) && !(State::Instance().api == DX12 && Config::Instance()->RcasEnabled.value_or_default()))
    {
        featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
        LOG_INFO("featureFlags (Sharpening) {0:b}", featureFlags);
    }
    else
    {
        LOG_INFO("featureFlags (!Sharpening) {0:b}", featureFlags);
    }

    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, featureFlags);

    // Resolution -----------------------------
    if (State::Instance().api != Vulkan && Config::Instance()->OutputScalingEnabled.value_or_default() && !Config::Instance()->DisplayResolution.value_or(false))
    {
        LOG_DEBUG("Output Scaling is active");

        float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or_default();

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

    if (Config::Instance()->ExtendedLimits.value_or_default() && RenderWidth() > DisplayWidth())
    {
        LOG_DEBUG("Extended limits is active and render size is bigger than display size");

        _targetWidth = RenderWidth();
        _targetHeight = RenderHeight();

        // enable output scaling to restore image
        if (!Config::Instance()->DisplayResolution.value_or(false))
        {
            Config::Instance()->OutputScalingMultiplier = 1.0f;
            Config::Instance()->OutputScalingEnabled = true;
        }
    }

    InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
    InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());
    InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
    InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());

    LOG_INFO("Render Size: {}x{}, Target Size: {}x{}, Display Size: {}x{}", RenderWidth(), RenderHeight(), TargetWidth(), TargetHeight(), DisplayWidth(), DisplayHeight());

    bool signedEnum = false;
    uint32_t RenderPresetDLAA = 0;
    uint32_t RenderPresetUltraQuality = 0;
    uint32_t RenderPresetQuality = 0;
    uint32_t RenderPresetBalanced = 0;
    uint32_t RenderPresetPerformance = 0;
    uint32_t RenderPresetUltraPerformance = 0;

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, &RenderPresetDLAA) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.DLAA", &RenderPresetDLAA);

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, &RenderPresetUltraQuality) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.UltraQuality", &RenderPresetUltraQuality);

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, &RenderPresetQuality) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Quality", &RenderPresetQuality);

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, &RenderPresetBalanced) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Balanced", &RenderPresetBalanced);

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, &RenderPresetPerformance) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.Performance", &RenderPresetPerformance);

    if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, &RenderPresetUltraPerformance) != NVSDK_NGX_Result_Success)
        InParameters->Get("RayReconstruction.Hint.Render.Preset.UltraPerformance", &RenderPresetUltraPerformance);


    if (RenderPresetDLAA >= 2147483648)
    {
        RenderPresetDLAA -= 2147483648;
        signedEnum = true;
    }

    if (RenderPresetUltraQuality >= 2147483648)
    {
        RenderPresetUltraQuality -= 2147483648;
        signedEnum = true;
    }

    if (RenderPresetQuality >= 2147483648)
    {
        RenderPresetQuality -= 2147483648;
        signedEnum = true;
    }

    if (RenderPresetBalanced >= 2147483648)
    {
        RenderPresetBalanced -= 2147483648;
        signedEnum = true;
    }

    if (RenderPresetPerformance >= 2147483648)
    {
        RenderPresetPerformance -= 2147483648;
        signedEnum = true;
    }

    if (RenderPresetUltraPerformance >= 2147483648)
    {
        RenderPresetUltraPerformance -= 2147483648;
        signedEnum = true;
    }

    if (Config::Instance()->RenderPresetOverride.value_or_default())
    {
        LOG_DEBUG("Preset override active, config overrides:");
        LOG_DEBUG("Preset_DLAA {}", Config::Instance()->RenderPresetDLAA.value_or(RenderPresetDLAA));
        LOG_DEBUG("Preset_UltraQuality {}", Config::Instance()->RenderPresetUltraQuality.value_or(RenderPresetUltraQuality));
        LOG_DEBUG("Preset_Quality {}", Config::Instance()->RenderPresetQuality.value_or(RenderPresetQuality));
        LOG_DEBUG("Preset_Balanced {}", Config::Instance()->RenderPresetBalanced.value_or(RenderPresetBalanced));
        LOG_DEBUG("Preset_Performance {}", Config::Instance()->RenderPresetPerformance.value_or(RenderPresetPerformance));
        LOG_DEBUG("Preset_UltraPerformance {}", Config::Instance()->RenderPresetUltraPerformance.value_or(RenderPresetUltraPerformance));

        RenderPresetDLAA = Config::Instance()->RenderPresetDLAA.value_or(RenderPresetDLAA);
        RenderPresetUltraQuality = Config::Instance()->RenderPresetUltraQuality.value_or(RenderPresetUltraQuality);
        RenderPresetQuality = Config::Instance()->RenderPresetQuality.value_or(RenderPresetQuality);
        RenderPresetBalanced = Config::Instance()->RenderPresetBalanced.value_or(RenderPresetBalanced);
        RenderPresetPerformance = Config::Instance()->RenderPresetPerformance.value_or(RenderPresetPerformance);
        RenderPresetUltraPerformance = Config::Instance()->RenderPresetUltraPerformance.value_or(RenderPresetUltraPerformance);
    }

    if (RenderPresetDLAA < 0 || RenderPresetDLAA > 7)
        RenderPresetDLAA = 0;

    if (RenderPresetUltraQuality < 0 || RenderPresetUltraQuality > 7)
        RenderPresetUltraQuality = 0;

    if (RenderPresetQuality < 0 || RenderPresetQuality > 7)
        RenderPresetQuality = 0;

    if (RenderPresetBalanced < 0 || RenderPresetBalanced > 7)
        RenderPresetBalanced = 0;

    if (RenderPresetPerformance < 0 || RenderPresetPerformance > 7)
        RenderPresetPerformance = 0;

    if (RenderPresetUltraPerformance < 0 || RenderPresetUltraPerformance > 7)
        RenderPresetUltraPerformance = 0;

    LOG_DEBUG("Signed Enum: {}", signedEnum);
    if (signedEnum)
    {
        RenderPresetDLAA += 2147483648;
        RenderPresetUltraQuality += 2147483648;
        RenderPresetQuality += 2147483648;
        RenderPresetBalanced += 2147483648;
        RenderPresetPerformance += 2147483648;
        RenderPresetUltraPerformance += 2147483648;
    }

    LOG_DEBUG("Final Presets:");
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, RenderPresetDLAA);
    LOG_DEBUG("Preset_DLAA {}", RenderPresetDLAA);
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, RenderPresetUltraQuality);
    LOG_DEBUG("Preset_UltraQuality {}", RenderPresetUltraQuality);
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, RenderPresetQuality);
    LOG_DEBUG("Preset_Quality {}", RenderPresetQuality);
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, RenderPresetBalanced);
    LOG_DEBUG("Preset_Balanced {}", RenderPresetBalanced);
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, RenderPresetPerformance);
    LOG_DEBUG("Preset_Performance {}", RenderPresetPerformance);
    InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, RenderPresetUltraPerformance);
    LOG_DEBUG("Preset_UltraPerformance {}", RenderPresetUltraPerformance);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.DLAA", RenderPresetDLAA);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraQuality", RenderPresetUltraQuality);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.Quality", RenderPresetQuality);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.Balanced", RenderPresetBalanced);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.Performance", RenderPresetPerformance);
    InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraPerformance", RenderPresetUltraPerformance);

    UINT perfQ = NVSDK_NGX_PerfQuality_Value_Balanced;
    if (InParameters->Get(NVSDK_NGX_Parameter_PerfQualityValue, &perfQ) == NVSDK_NGX_Result_Success &&
        perfQ == NVSDK_NGX_PerfQuality_Value_UltraQuality)
    {
        InParameters->Set(NVSDK_NGX_Parameter_PerfQualityValue, NVSDK_NGX_PerfQuality_Value_MaxQuality);
    }

    LOG_FUNC_RESULT(0);
}

void DLSSFeature::ReadVersion()
{
    LOG_FUNC();

    PFN_NVSDK_NGX_GetSnippetVersion _GetSnippetVersion = nullptr;

    if (!Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        _GetSnippetVersion = (PFN_NVSDK_NGX_GetSnippetVersion)DetourFindFunction("nvngx_dlss.dll", "NVSDK_NGX_GetSnippetVersion");
    }
    else
    {
        std::filesystem::path dlssPath(Config::Instance()->NVNGX_DLSS_Library.value());
        
        if (dlssPath.has_filename())
            _GetSnippetVersion = (PFN_NVSDK_NGX_GetSnippetVersion)DetourFindFunction(dlssPath.filename().string().c_str(), "NVSDK_NGX_GetSnippetVersion");
    }

    if (_GetSnippetVersion != nullptr)
    {
        LOG_TRACE("_GetSnippetVersion ptr: {0:X}", (ULONG64)_GetSnippetVersion);

        auto result = _GetSnippetVersion();

        _version.major = (result & 0x00FF0000) / 0x00010000;
        _version.minor = (result & 0x0000FF00) / 0x00000100;
        _version.patch = result & 0x000000FF / 0x00000001;

        LOG_INFO("DLSS v{0}.{1}.{2} loaded.", _version.major, _version.minor, _version.patch);
        return;
    }

    LOG_WARN("GetProcAddress for NVSDK_NGX_GetSnippetVersion failed!");
}

DLSSFeature::DLSSFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
{
    LOG_FUNC();

    if (NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (NVNGXProxy::NVNGXModule() != nullptr && !State::Instance().enablerAvailable)
    {
        HookNgxApi(NVNGXProxy::NVNGXModule());
    }

    _moduleLoaded = NVNGXProxy::NVNGXModule() != nullptr;

    LOG_FUNC_RESULT(_moduleLoaded);
}

DLSSFeature::~DLSSFeature()
{
}

void DLSSFeature::Shutdown()
{
    LOG_FUNC();

    if (NVNGXProxy::NVNGXModule() != nullptr)
        UnhookApis();

    LOG_FUNC_RESULT(0);
}

float DLSSFeature::GetSharpness(const NVSDK_NGX_Parameter* InParameters)
{
    if (Config::Instance()->OverrideSharpness.value_or_default())
        return Config::Instance()->Sharpness.value_or_default();

    float sharpness = 0.0f;
    InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &sharpness);

    return sharpness;
}