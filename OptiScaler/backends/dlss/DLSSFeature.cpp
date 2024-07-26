#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "DLSSFeature.h"

void DLSSFeature::ProcessEvaluateParams(NVSDK_NGX_Parameter* InParameters)
{
	LOG_FUNC();

	float floatValue;

	// override sharpness
	if (Config::Instance()->OverrideSharpness.value_or(false) && !(Config::Instance()->Api == NVNGX_DX12 && Config::Instance()->RcasEnabled.value_or(false)))
	{
		auto sharpness = Config::Instance()->Sharpness.value_or(0.3f);
		InParameters->Set(NVSDK_NGX_Parameter_Sharpness, sharpness);
	}
	// rcas enabled
	else if (Config::Instance()->Api == NVNGX_DX12 && Config::Instance()->RcasEnabled.value_or(false))
	{
		InParameters->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);
	}
	// dlss value
	else if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &floatValue) != NVSDK_NGX_Result_Success)
	{
		InParameters->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);
	}

	// Read render resolution
	unsigned int width;
	unsigned int height;
	GetRenderResolution(InParameters, &width, &height);

	LOG_FUNC_RESULT(0);
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

	if (Config::Instance()->OverrideSharpness.value_or(sharpening) && !(Config::Instance()->Api == NVNGX_DX12 && Config::Instance()->RcasEnabled.value_or(false)))
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
	if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
	{
		float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or(1.5f);

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

	InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
	InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());
	InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
	InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());

	unsigned int RenderPresetDLAA = 0;
	unsigned int RenderPresetUltraQuality = 0;
	unsigned int RenderPresetQuality = 0;
	unsigned int RenderPresetBalanced = 0;
	unsigned int RenderPresetPerformance = 0;
	unsigned int RenderPresetUltraPerformance = 0;

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

	if (Config::Instance()->RenderPresetOverride.value_or(false))
	{
		RenderPresetDLAA = Config::Instance()->RenderPresetDLAA.value_or(RenderPresetDLAA);
		RenderPresetUltraQuality = Config::Instance()->RenderPresetUltraQuality.value_or(RenderPresetUltraQuality);
		RenderPresetQuality = Config::Instance()->RenderPresetQuality.value_or(RenderPresetQuality);
		RenderPresetBalanced = Config::Instance()->RenderPresetBalanced.value_or(RenderPresetBalanced);
		RenderPresetPerformance = Config::Instance()->RenderPresetPerformance.value_or(RenderPresetPerformance);
		RenderPresetUltraPerformance = Config::Instance()->RenderPresetUltraPerformance.value_or(RenderPresetUltraPerformance);
	}

	if (RenderPresetDLAA < 0 || RenderPresetDLAA > 5)
		RenderPresetDLAA = 0;

	if (RenderPresetUltraQuality < 0 || RenderPresetUltraQuality > 5)
		RenderPresetUltraQuality = 0;

	if (RenderPresetQuality < 0 || RenderPresetQuality > 5)
		RenderPresetQuality = 0;

	if (RenderPresetBalanced < 0 || RenderPresetBalanced > 5)
		RenderPresetBalanced = 0;

	if (RenderPresetPerformance < 0 || RenderPresetPerformance > 5)
		RenderPresetPerformance = 0;

	if (RenderPresetUltraPerformance < 0 || RenderPresetUltraPerformance > 5)
		RenderPresetUltraPerformance = 0;

	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, RenderPresetDLAA);
	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, RenderPresetUltraQuality);
	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, RenderPresetQuality);
	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, RenderPresetBalanced);
	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, RenderPresetPerformance);
	InParameters->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, RenderPresetUltraPerformance);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.DLAA", RenderPresetDLAA);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraQuality", RenderPresetUltraQuality);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.Quality", RenderPresetQuality);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.Balanced", RenderPresetBalanced);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.Performance", RenderPresetPerformance);
	InParameters->Set("RayReconstruction.Hint.Render.Preset.UltraPerformance", RenderPresetUltraPerformance);

	LOG_FUNC_RESULT(0);
}

void DLSSFeature::ReadVersion()
{
	LOG_FUNC();

	PFN_NVSDK_NGX_GetSnippetVersion _GetSnippetVersion = nullptr;

	_GetSnippetVersion = (PFN_NVSDK_NGX_GetSnippetVersion)DetourFindFunction("nvngx_dlss.dll", "NVSDK_NGX_GetSnippetVersion");

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

	if (NVNGXProxy::NVNGXModule() != nullptr && !Config::Instance()->DE_Available)
	{
		HookNvApi();
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