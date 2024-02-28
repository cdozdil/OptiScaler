#include "IFeatureContextDx12.h"
#include "xess_d3d12.h"
#include "xess_debug.h"

static std::string ResultToString(xess_result_t result)
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

class XeSSFeatureDx12 : public IFeatureContext, public IFeatureContextDx12
{
private:
	xess_context_handle_t _xessContext;

public:
	using IFeatureContextDx12::IFeatureContextDx12;

	// Inherited via IFeatureContextDx12
	bool Init(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters) override
	{
		if (IFeatureContext::_isInited)
			return true;

		if (!device)
		{
			spdlog::error("IFeatureContextDx12::Init D3D12Device is null!");
			return false;
		}

		Device = device;

		// XeSS init starts
		xess_version_t ver;
		xess_result_t ret = xessGetVersion(&ver);

		if (ret == XESS_RESULT_SUCCESS)
			spdlog::info("IFeatureContextDx12::Init XeSS Version: {0}.{1}.{2}", ver.major, ver.minor, ver.patch);
		else
			spdlog::warn("IFeatureContextDx12::Init xessGetVersion error: {0}", ResultToString(ret));

		ret = xessD3D12CreateContext(device, &_xessContext);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("IFeatureContextDx12::Init xessD3D12CreateContext error: {0}", ResultToString(ret));
			return false;
		}

		ret = xessIsOptimalDriver(_xessContext);
		spdlog::debug("IFeatureContextDx12::Init xessIsOptimalDriver : {0}", ResultToString(ret));

		ret = xessSetLoggingCallback(_xessContext, XESS_LOGGING_LEVEL_DEBUG, logCallback);
		spdlog::debug("IFeatureContextDx12::Init xessSetLoggingCallback : {0}", ResultToString(ret));

#pragma region Create Parameters for XeSS

		xess_d3d12_init_params_t xessParams{};

		xessParams.outputResolution.x = IFeatureContext::DisplayWidth();
		xessParams.outputResolution.y = IFeatureContext::DisplayHeight();

		switch (IFeatureContext::PerfQualityValue())
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

		xessParams.initFlags = XESS_INIT_FLAG_NONE;

		int featureFlags;
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

		bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
		bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
		bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
		bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
		bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
		bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

		if (DepthInverted)
		{
			xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
		}

		if (AutoExposure)
		{
			xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
		}
		else
		{
			xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
		}

		if (Hdr)
		{
			xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (!HDR) {0:b}", xessParams.initFlags);
		}

		if (JitterMotion)
		{
			xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
		}

		if (!LowRes)
		{
			xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
			spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (LowRes) {0:b}", xessParams.initFlags);
		}

		//if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(true))
		//{
		//	xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		//	spdlog::info("IFeatureContextDx12::Init xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
		//}

#pragma endregion

#pragma region Build Pipelines

		ret = xessD3D12BuildPipelines(_xessContext, NULL, false, xessParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("IFeatureContextDx12::Init xessD3D12BuildPipelines error: {0}", ResultToString(ret));
			return false;
		}

#pragma endregion

		spdlog::debug("IFeatureContextDx12::Init xessD3D12Init!");

		ret = xessD3D12Init(_xessContext, &xessParams);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("IFeatureContextDx12::Init xessD3D12Init error: {0}", ResultToString(ret));
			return false;
		}

		IFeatureContext::_isInited = true;

		return true;
	}

	bool Evaluate(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* InParameters) override
	{
		return false;
	}

};