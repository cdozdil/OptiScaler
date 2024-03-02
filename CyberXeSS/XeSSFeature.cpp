#include "pch.h"
#include "XeSSFeature.h"
#include "Config.h"

inline void logCallback(const char* Message, xess_logging_level_t Level)
{
	spdlog::log((spdlog::level::level_enum)((int)Level + 1), "FeatureContext::LogCallback XeSS Runtime ({0})", Message);
}

bool XeSSFeature::IsInited() 
{ 
	return _isInited; 
}

void XeSSFeature::SetInit(bool value) 
{ 
	_isInited = value; 
}

bool XeSSFeature::InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters)
{
	if (IsInited())
		return true;

	if (device == nullptr)
	{
		spdlog::error("XeSSContext::InitXeSS D3D12Device is null!");
		return false;
	}

	xess_version_t ver;
	auto ret = xessGetVersion(&ver);

	if (ret == XESS_RESULT_SUCCESS)
		spdlog::info("XeSSContext::InitXeSS XeSS Version: {0}.{1}.{2}", ver.major, ver.minor, ver.patch);
	else
		spdlog::warn("XeSSContext::InitXeSS xessGetVersion error: {0}", ResultToString(ret));

	ret = xessD3D12CreateContext(device, &_xessContext);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSContext::InitXeSS xessD3D12CreateContext error: {0}", ResultToString(ret));
		return false;
	}

	ret = xessIsOptimalDriver(_xessContext);
	spdlog::debug("XeSSContext::InitXeSS xessIsOptimalDriver : {0}", ResultToString(ret));

	ret = xessSetLoggingCallback(_xessContext, XESS_LOGGING_LEVEL_DEBUG, logCallback);
	spdlog::debug("XeSSContext::InitXeSS xessSetLoggingCallback : {0}", ResultToString(ret));

	xess_d3d12_init_params_t xessParams{};

	xessParams.outputResolution.x = DisplayWidth();
	xessParams.outputResolution.y = DisplayHeight();

	if (Config::Instance()->OverrideQuality.has_value())
	{
		xessParams.qualitySetting = (xess_quality_settings_t)Config::Instance()->OverrideQuality.value();
	}
	else
	{
		switch (PerfQualityValue())
		{
		case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
			break;

		case NVSDK_NGX_PerfQuality_Value_MaxPerf:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
			break;

		case NVSDK_NGX_PerfQuality_Value_Balanced:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED;
			break;

		case NVSDK_NGX_PerfQuality_Value_MaxQuality:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_QUALITY;
			break;

		case NVSDK_NGX_PerfQuality_Value_UltraQuality:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;
			break;

		case NVSDK_NGX_PerfQuality_Value_DLAA:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;
			break;

		default:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing XESS_QUALITY_SETTING_BALANCED mode
			break;
		}
	}

	xessParams.initFlags = XESS_INIT_FLAG_NONE;

	int featureFlags;
	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

	bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
	bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
	bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
	bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
	bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
	bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

	if (Config::Instance()->DepthInverted.value_or(DepthInverted))
	{
		Config::Instance()->DepthInverted = true;
		xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = false;
		xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (!HDR) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (HDR) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (!LowResMV) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (LowResMV) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		Config::Instance()->DisableReactiveMask = false;
		xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->BuildPipelines.value_or(true))
	{
		spdlog::debug("XeSSContext::InitXeSS xessD3D12BuildPipelines!");

		ret = xessD3D12BuildPipelines(_xessContext, NULL, false, xessParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSContext::InitXeSS xessD3D12BuildPipelines error: {0}", ResultToString(ret));
			return false;
		}
	}

	spdlog::debug("XeSSContext::InitXeSS xessD3D12Init!");

	ret = xessD3D12Init(_xessContext, &xessParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSContext::InitXeSS xessD3D12Init error: {0}", ResultToString(ret));
		return false;
	}

	_isInited = true;

	return true;
}

void XeSSFeature::SetRenderResolution(const NVSDK_NGX_Parameter* InParameters, xess_d3d12_execute_params_t* params) const
{
	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, &params->inputWidth) != NVSDK_NGX_Result_Success ||
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, &params->inputHeight) != NVSDK_NGX_Result_Success)
	{
		unsigned int width, height, outWidth, outHeight;

		if (InParameters->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
			InParameters->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success)
		{
			if (InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
				InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success)
			{
				if (width < outWidth)
				{
					params->inputWidth = width;
					params->inputHeight = height;
					return;
				}

				params->inputWidth = outWidth;
				params->inputHeight = outHeight;
			}
			else
			{
				if (width < RenderWidth())
				{
					params->inputWidth = width;
					params->inputHeight = height;
					return;
				}

				params->inputWidth = RenderWidth();
				params->inputHeight = RenderHeight();
				return;
			}
		}

		params->inputWidth = RenderWidth();
		params->inputHeight = RenderHeight();
	}
}

XeSSFeature::~XeSSFeature()
{
	spdlog::debug("XeSSContext::Destroy!");

	if (!_xessContext)
		return;

	auto result = xessDestroyContext(_xessContext);

	if (result != XESS_RESULT_SUCCESS)
		spdlog::error("XeSSContext::Destroy xessDestroyContext error: {0}", ResultToString(result));
}