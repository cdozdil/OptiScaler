#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "DLSSDFeature.h"

#include "../../detours/detours.h"

void DLSSDFeature::ProcessEvaluateParams(NVSDK_NGX_Parameter* InParameters)
{
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
}

void DLSSDFeature::ProcessInitParams(NVSDK_NGX_Parameter* InParameters)
{
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
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags {0:X}", uintValue);

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
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (DepthInverted) {0:b}", featureFlags);
	}
	else
	{
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!DepthInverted) {0:b}", featureFlags);
	}

	if (Config::Instance()->AutoExposure.value_or(autoExposure))
	{
		Config::Instance()->AutoExposure = true;
		featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (AutoExposure) {0:b}", featureFlags);
	}
	else
	{
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!AutoExposure) {0:b}", featureFlags);
	}

	if (Config::Instance()->HDR.value_or(isHdr))
	{
		Config::Instance()->HDR = true;
		featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (HDR) {0:b}", featureFlags);
	}
	else
	{
		Config::Instance()->HDR = false;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!HDR) {0:b}", featureFlags);
	}

	if (Config::Instance()->JitterCancellation.value_or(mvJittered))
	{
		Config::Instance()->JitterCancellation = true;
		featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (JitterCancellation) {0:b}", featureFlags);
	}
	else
	{
		Config::Instance()->JitterCancellation = false;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!JitterCancellation) {0:b}", featureFlags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!mvLowRes))
	{
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!LowResMV) {0:b}", featureFlags);
	}
	else
	{
		featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (LowResMV) {0:b}", featureFlags);
	}

	if (Config::Instance()->OverrideSharpness.value_or(sharpening) && !(Config::Instance()->Api == NVNGX_DX12 && Config::Instance()->RcasEnabled.value_or(false)))
	{
		featureFlags |= NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (Sharpening) {0:b}", featureFlags);
	}
	else
	{
		LOG_INFO("DLSSDFeature::ProcessInitParams featureFlags (!Sharpening) {0:b}", featureFlags);
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

	// extended limits changes how resolution 
	if (Config::Instance()->ExtendedLimits.value_or(false))
	{
		InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
		InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());

		_targetWidth = RenderWidth() < TargetWidth() ? TargetWidth() : RenderWidth();
		_targetHeight = RenderHeight() < TargetHeight() ? TargetHeight() : RenderHeight();
		InParameters->Set(NVSDK_NGX_Parameter_OutWidth, _targetWidth);
		InParameters->Set(NVSDK_NGX_Parameter_OutHeight, _targetHeight);

		// enable output scaling to restore image
		if (!Config::Instance()->DisplayResolution.value_or(false) && _targetWidth == RenderWidth())
		{
			Config::Instance()->OutputScalingMultiplier = 1.0f;
			Config::Instance()->OutputScalingEnabled = true;
		}
	}
	else
	{
		InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
		InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());
		InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
		InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());
	}

	//if (Config::Instance()->ExtendedLimits.value_or(false))
	//{
	//	InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth() < TargetWidth() ? TargetWidth() : RenderWidth());
	//	InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight() < TargetHeight() ? TargetHeight() : RenderHeight());

	//	// if output scaling active let it to handle downsampling
	//	if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
	//	{
	//		// update target res
	//		_targetWidth = RenderWidth() < TargetWidth() ? TargetWidth() : RenderWidth();
	//		_targetHeight = RenderHeight() < TargetHeight() ? TargetHeight() : RenderHeight();
	//		InParameters->Set(NVSDK_NGX_Parameter_OutWidth, _targetWidth);
	//		InParameters->Set(NVSDK_NGX_Parameter_OutHeight, _targetHeight);
	//	}
	//	else
	//	{
	//		InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
	//		InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());
	//	}
	//}
	//else
	//{
	//	InParameters->Set(NVSDK_NGX_Parameter_Width, RenderWidth());
	//	InParameters->Set(NVSDK_NGX_Parameter_Height, RenderHeight());
	//	InParameters->Set(NVSDK_NGX_Parameter_OutWidth, TargetWidth());
	//	InParameters->Set(NVSDK_NGX_Parameter_OutHeight, TargetHeight());
	//}

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
}

void DLSSDFeature::ReadVersion()
{
	PFN_NVSDK_NGX_GetSnippetVersion _GetSnippetVersion = nullptr;

	_GetSnippetVersion = (PFN_NVSDK_NGX_GetSnippetVersion)DetourFindFunction("nvngx_dlssd.dll", "NVSDK_NGX_GetSnippetVersion");

	if (_GetSnippetVersion != nullptr)
	{
		LOG_TRACE("_GetSnippetVersion ptr: {0:X}", (ULONG64)_GetSnippetVersion);

		auto result = _GetSnippetVersion();

		_version.major = (result & 0x00FF0000) / 0x00010000;
		_version.minor = (result & 0x0000FF00) / 0x00000100;
		_version.patch = result & 0x000000FF / 0x00000001;

		LOG_INFO("v{0}.{1}.{2} loaded.", _version.major, _version.minor, _version.patch);
		return;
	}

	LOG_INFO("GetProcAddress for NVSDK_NGX_GetSnippetVersion failed!");
}

DLSSDFeature::DLSSDFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
		NVNGXProxy::InitNVNGX();

	_moduleLoaded = NVNGXProxy::NVNGXModule() != nullptr;
}

DLSSDFeature::~DLSSDFeature()
{
}

void DLSSDFeature::Shutdown()
{
}