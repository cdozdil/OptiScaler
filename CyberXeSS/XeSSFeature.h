#include "xess_d3d12.h"
#include "xess_debug.h"

#include "IFeature.h"

inline static std::string ResultToString(xess_result_t result)
{
	switch (result)
	{
	case XESS_RESULT_WARNING_NONEXISTING_FOLDER: return "Warning Nonexistent Folder";
	case XESS_RESULT_WARNING_OLD_DRIVER: return "Warning Old Driver";
	case XESS_RESULT_SUCCESS: return "Success";
	case XESS_RESULT_ERROR_UNSUPPORTED_DEVICE: return "Unsupported Device";
	case XESS_RESULT_ERROR_UNSUPPORTED_DRIVER: return "Unsupported Driver";
	case XESS_RESULT_ERROR_UNINITIALIZED: return "Uninitialized";
	case XESS_RESULT_ERROR_INVALID_ARGUMENT: return "Invalid Argument";
	case XESS_RESULT_ERROR_DEVICE_OUT_OF_MEMORY: return "Device Out of Memory";
	case XESS_RESULT_ERROR_DEVICE: return "Device Error";
	case XESS_RESULT_ERROR_NOT_IMPLEMENTED: return "Not Implemented";
	case XESS_RESULT_ERROR_INVALID_CONTEXT: return "Invalid Context";
	case XESS_RESULT_ERROR_OPERATION_IN_PROGRESS: return "Operation in Progress";
	case XESS_RESULT_ERROR_UNSUPPORTED: return "Unsupported";
	case XESS_RESULT_ERROR_CANT_LOAD_LIBRARY: return "Cannot Load Library";
	case XESS_RESULT_ERROR_UNKNOWN:
	default: return "Unknown";
	}
}

inline void logCallback(const char* Message, xess_logging_level_t Level)
{
	spdlog::log((spdlog::level::level_enum)((int)Level + 1), "FeatureContext::LogCallback XeSS Runtime ({0})", Message);
}

class XeSSFeature : public IFeature
{
private:
	bool _isInited = false;

protected:
	xess_context_handle_t _xessContext;

	bool InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters)
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

		if (GetConfig()->OverrideQuality.has_value())
		{
			xessParams.qualitySetting = (xess_quality_settings_t)GetConfig()->OverrideQuality.value();
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

		if (GetConfig()->DepthInverted.value_or(DepthInverted))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
		}

		if (GetConfig()->AutoExposure.value_or(AutoExposure))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
		}
		else
		{
			xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
		}

		if (!GetConfig()->HDR.value_or(Hdr))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (!HDR) {0:b}", xessParams.initFlags);
		}
		else
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (HDR) {0:b}", xessParams.initFlags);

		if (GetConfig()->JitterCancellation.value_or(JitterMotion))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
		}

		if (GetConfig()->DisplayResolution.value_or(!LowRes))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (LowRes) {0:b}", xessParams.initFlags);
		}

		if (!GetConfig()->DisableReactiveMask.value_or(true))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
			spdlog::info("XeSSContext::InitXeSS xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
		}

		if (GetConfig()->BuildPipelines.value_or(true))
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

	void SetRenderResolution(const NVSDK_NGX_Parameter* InParameters, xess_d3d12_execute_params_t* params) const
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

	void SetInit(bool value) override;
	
public:

	XeSSFeature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters, Config* config) : IFeature(handleId, InParameters, config)
	{
	}
	
	bool IsInited() override;

	~XeSSFeature()
	{
		spdlog::debug("XeSSContext::Destroy!");

		if (!_xessContext)
			return;

		auto result = xessDestroyContext(_xessContext);

		if (result != XESS_RESULT_SUCCESS)
			spdlog::error("XeSSContext::Destroy xessDestroyContext error: {0}", ResultToString(result));
	}

};