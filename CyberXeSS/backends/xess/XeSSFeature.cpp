#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature.h"

inline static std::string ResultToString(FfxCas::FfxErrorCode result)
{
	switch (result)
	{
	case FfxCas::FFX_OK: return "The operation completed successfully.";
	case FfxCas::FFX_ERROR_INVALID_POINTER: return "The operation failed due to an invalid pointer.";
	case FfxCas::FFX_ERROR_INVALID_ALIGNMENT: return "The operation failed due to an invalid alignment.";
	case FfxCas::FFX_ERROR_INVALID_SIZE: return "The operation failed due to an invalid size.";
	case FfxCas::FFX_EOF: return "The end of the file was encountered.";
	case FfxCas::FFX_ERROR_INVALID_PATH: return "The operation failed because the specified path was invalid.";
	case FfxCas::FFX_ERROR_EOF: return "The operation failed because end of file was reached.";
	case FfxCas::FFX_ERROR_MALFORMED_DATA: return "The operation failed because of some malformed data.";
	case FfxCas::FFX_ERROR_OUT_OF_MEMORY: return "The operation failed because it ran out memory.";
	case FfxCas::FFX_ERROR_INCOMPLETE_INTERFACE: return "The operation failed because the interface was not fully configured.";
	case FfxCas::FFX_ERROR_INVALID_ENUM: return "The operation failed because of an invalid enumeration value.";
	case FfxCas::FFX_ERROR_INVALID_ARGUMENT: return "The operation failed because an argument was invalid.";
	case FfxCas::FFX_ERROR_OUT_OF_RANGE: return "The operation failed because a value was out of range.";
	case FfxCas::FFX_ERROR_NULL_DEVICE: return "The operation failed because a device was null.";
	case FfxCas::FFX_ERROR_BACKEND_API_ERROR: return "The operation failed because the backend API returned an error code.";
	case FfxCas::FFX_ERROR_INSUFFICIENT_MEMORY: return "The operation failed because there was not enough memory.";
	default: return "Unknown";
	}
}

inline void FfxCasLogCallback(const char* message)
{
	spdlog::debug("XeSSFeature::LogCallback FSR Runtime: {0}", message);
}

FfxCas::FfxSurfaceFormat ffxGetSurfaceFormatDX12(DXGI_FORMAT format)
{
	switch (format) {

	case(DXGI_FORMAT_R32G32B32A32_TYPELESS):
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
	case(DXGI_FORMAT_R32G32B32A32_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_UINT;
		//case DXGI_FORMAT_R32G32B32A32_SINT:
		//case DXGI_FORMAT_R32G32B32_TYPELESS:
		//case DXGI_FORMAT_R32G32B32_FLOAT:
		//case DXGI_FORMAT_R32G32B32_UINT:
		//case DXGI_FORMAT_R32G32B32_SINT:

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case(DXGI_FORMAT_R16G16B16A16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		//case DXGI_FORMAT_R16G16B16A16_UNORM:
		//case DXGI_FORMAT_R16G16B16A16_UINT:
		//case DXGI_FORMAT_R16G16B16A16_SNORM:
		//case DXGI_FORMAT_R16G16B16A16_SINT:

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
		return FfxCas::FFX_SURFACE_FORMAT_R32G32_FLOAT;
		//case DXGI_FORMAT_R32G32_FLOAT:
		//case DXGI_FORMAT_R32G32_UINT:
		//case DXGI_FORMAT_R32G32_SINT:

	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return FfxCas::FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return FfxCas::FFX_SURFACE_FORMAT_R32_UINT;

	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return FfxCas::FFX_SURFACE_FORMAT_R8_UINT;

		//case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		//case DXGI_FORMAT_R10G10B10A2_UNORM:
		//	return FfxCas::FFX_SURFACE_FORMAT_R10G10B10A2_UNORM;
			//case DXGI_FORMAT_R10G10B10A2_UINT:

	case (DXGI_FORMAT_R11G11B10_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R11G11B10_FLOAT;

	case (DXGI_FORMAT_R8G8B8A8_TYPELESS):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
	case (DXGI_FORMAT_R8G8B8A8_UNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
	case (DXGI_FORMAT_R8G8B8A8_UNORM_SRGB):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;
		//case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_SNORM;

	case DXGI_FORMAT_R16G16_TYPELESS:
	case (DXGI_FORMAT_R16G16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16_FLOAT;
		//case DXGI_FORMAT_R16G16_UNORM:
	case (DXGI_FORMAT_R16G16_UINT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16_UINT;
		//case DXGI_FORMAT_R16G16_SNORM
		//case DXGI_FORMAT_R16G16_SINT 

		//case DXGI_FORMAT_R32_SINT:
	//case DXGI_FORMAT_R32_UINT:
	//	return FFX_SURFACE_FORMAT_R32_UINT;
	case DXGI_FORMAT_R32_TYPELESS:
	case(DXGI_FORMAT_D32_FLOAT):
	case(DXGI_FORMAT_R32_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R8G8_TYPELESS:
		//case (DXGI_FORMAT_R8G8_UINT):
		//	return FfxCas::FFX_SURFACE_FORMAT_R8G8_UINT;
			//case DXGI_FORMAT_R8G8_UNORM:
			//case DXGI_FORMAT_R8G8_SNORM:
			//case DXGI_FORMAT_R8G8_SINT:

	case DXGI_FORMAT_R16_TYPELESS:
	case (DXGI_FORMAT_R16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16_FLOAT;
	case (DXGI_FORMAT_R16_UINT):
		return FfxCas::FFX_SURFACE_FORMAT_R16_UINT;
	case DXGI_FORMAT_D16_UNORM:
	case (DXGI_FORMAT_R16_UNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R16_UNORM;
	case (DXGI_FORMAT_R16_SNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R16_SNORM;
		//case DXGI_FORMAT_R16_SINT:

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_A8_UNORM:
		return FfxCas::FFX_SURFACE_FORMAT_R8_UNORM;
		//case DXGI_FORMAT_R8_UINT:
		//	return FFX_SURFACE_FORMAT_R8_UINT;
			//case DXGI_FORMAT_R8_SNORM:
			//case DXGI_FORMAT_R8_SINT:
			//case DXGI_FORMAT_R1_UNORM:

	case(DXGI_FORMAT_UNKNOWN):
		return FfxCas::FFX_SURFACE_FORMAT_UNKNOWN;
	default:
		//FFX_ASSERT_MESSAGE(false, "Format not yet supported");
		return FfxCas::FFX_SURFACE_FORMAT_UNKNOWN;
	}
}

bool IsDepthDX12(DXGI_FORMAT format)
{
	return (format == DXGI_FORMAT_D16_UNORM) ||
		(format == DXGI_FORMAT_D32_FLOAT) ||
		(format == DXGI_FORMAT_D24_UNORM_S8_UINT) ||
		(format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
}

FfxCas::FfxResourceDescription GetFfxResourceDescriptionDX12(ID3D12Resource* pResource)
{
	FfxCas::FfxResourceDescription resourceDescription = {};

	// This is valid
	if (!pResource)
		return resourceDescription;

	if (pResource)
	{
		D3D12_RESOURCE_DESC desc = pResource->GetDesc();

		// Set flags properly for resource registration
		resourceDescription.flags = FfxCas::FFX_RESOURCE_FLAGS_NONE;
		resourceDescription.usage = IsDepthDX12(desc.Format) ? FfxCas::FFX_RESOURCE_USAGE_DEPTHTARGET : FfxCas::FFX_RESOURCE_USAGE_READ_ONLY;
		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			resourceDescription.usage = (FfxCas::FfxResourceUsage)(resourceDescription.usage | FfxCas::FFX_RESOURCE_USAGE_UAV);

		resourceDescription.width = (uint32_t)desc.Width;
		resourceDescription.height = (uint32_t)desc.Height;
		resourceDescription.depth = desc.DepthOrArraySize;
		resourceDescription.mipCount = desc.MipLevels;
		resourceDescription.format = ffxGetSurfaceFormatDX12(desc.Format);

		resourceDescription.type = FfxCas::FFX_RESOURCE_TYPE_TEXTURE2D;
	}

	return resourceDescription;
}

inline void XeSSLogCallback(const char* Message, xess_logging_level_t Level)
{
	spdlog::log((spdlog::level::level_enum)((int)Level + 1), "FeatureContext::LogCallback XeSS Runtime ({0})", Message);
}

bool XeSSFeature::InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeature::InitXeSS!");

	if (IsInited())
		return true;

	if (device == nullptr)
	{
		spdlog::error("XeSSFeature::InitXeSS D3D12Device is null!");
		return false;
	}

	xess_version_t ver;
	auto ret = xessGetVersion(&ver);

	if (ret == XESS_RESULT_SUCCESS)
		spdlog::info("XeSSFeature::InitXeSS XeSS Version: {0}.{1}.{2}", ver.major, ver.minor, ver.patch);
	else
		spdlog::warn("XeSSFeature::InitXeSS xessGetVersion error: {0}", ResultToString(ret));

	ret = xessD3D12CreateContext(device, &_xessContext);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeature::InitXeSS xessD3D12CreateContext error: {0}", ResultToString(ret));
		return false;
	}

	ret = xessIsOptimalDriver(_xessContext);
	spdlog::debug("XeSSFeature::InitXeSS xessIsOptimalDriver : {0}", ResultToString(ret));

	ret = xessSetLoggingCallback(_xessContext, XESS_LOGGING_LEVEL_DEBUG, XeSSLogCallback);
	spdlog::debug("XeSSFeature::InitXeSS xessSetLoggingCallback : {0}", ResultToString(ret));

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
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = false;
		xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!HDR) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (HDR) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!LowResMV) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (LowResMV) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		Config::Instance()->DisableReactiveMask = false;
		xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->BuildPipelines.value_or(true))
	{
		spdlog::debug("XeSSFeature::InitXeSS xessD3D12BuildPipelines!");

		ret = xessD3D12BuildPipelines(_xessContext, NULL, false, xessParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeature::InitXeSS xessD3D12BuildPipelines error: {0}", ResultToString(ret));
			return false;
		}
	}

	spdlog::debug("XeSSFeature::InitXeSS xessD3D12Init!");

	ret = xessD3D12Init(_xessContext, &xessParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeature::InitXeSS xessD3D12Init error: {0}", ResultToString(ret));
		return false;
	}

	CasInit();

	if (casActive)
		CreateCasContext(device);

	SetInit(true);

	return true;
}

XeSSFeature::~XeSSFeature()
{
	spdlog::debug("XeSSFeature::XeSSFeature");

	if (!_xessContext)
		return;

	auto result = xessDestroyContext(_xessContext);

	if (result != XESS_RESULT_SUCCESS)
		spdlog::error("XeSSFeature::Destroy xessDestroyContext error: {0}", ResultToString(result));

	DestroyCasContext();

	if (casBuffer != nullptr)
		casBuffer->Release();
}

void XeSSFeature::CasInit()
{
	if (casInit)
		return;

	spdlog::debug("FeatureContext::CasInit Start!");

	casActive = Config::Instance()->CasEnabled.value_or(false);
	casSharpness = Config::Instance()->Sharpness.value_or(0.3);

	if (casSharpness > 1 || casSharpness < 0)
		casSharpness = 0.4f;

	casInit = true;
}

bool XeSSFeature::CreateCasContext(ID3D12Device* device)
{
	if (!casInit)
		return false;

	if (!casActive)
		return true;

	spdlog::debug("XeSSFeature::CreateCasContext Start!");

	DestroyCasContext();

	const size_t scratchBufferSize = FfxCas::ffxGetScratchMemorySizeDX12Cas(FFX_CAS_CONTEXT_COUNT);
	void* casScratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = FfxCas::ffxGetInterfaceDX12Cas(&casContextDesc.backendInterface, device, casScratchBuffer, scratchBufferSize, FFX_CAS_CONTEXT_COUNT);

	if (errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("XeSSFeature::CreateCasContext ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
		free(casScratchBuffer);
		return false;
	}

	casUpscale = false;
	casContextDesc.flags |= FfxCas::FFX_CAS_SHARPEN_ONLY;

	auto casCSC = Config::Instance()->CasColorSpaceConversion.value_or(FfxCas::FFX_CAS_COLOR_SPACE_LINEAR);

	if (casCSC < 0 || casCSC > 4)
		casCSC = 0;

	casContextDesc.colorSpaceConversion = static_cast<FfxCas::FfxCasColorSpaceConversion>(casCSC);
	casContextDesc.maxRenderSize.width = DisplayWidth();
	casContextDesc.maxRenderSize.height = DisplayHeight();
	casContextDesc.displaySize.width = DisplayWidth();
	casContextDesc.displaySize.height = DisplayHeight();

	errorCode = FfxCas::ffxCasContextCreate(&casContext, &casContextDesc);

	if (errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("XeSSFeature::CreateCasContext ffxCasContextCreate error: {0:x}", errorCode);
		return false;
	}

	FfxCas::ffxAssertSetPrintingCallback(FfxCasLogCallback);

	casContextCreated = true;
	return true;
}

void XeSSFeature::DestroyCasContext()
{
	spdlog::debug("XeSSFeature::DestroyCasContext Start!");

	if (!casActive || !casContextCreated)
		return;

	auto errorCode = FfxCas::ffxCasContextDestroy(&casContext);

	if (errorCode != FfxCas::FFX_OK)
		spdlog::error("FeatureContext::DestroyCasContext ffxCasContextDestroy error: {0:x}", errorCode);

	free(casContextDesc.backendInterface.scratchBuffer);

	casContextCreated = false;
}

bool XeSSFeature::CreateCasBufferResource(ID3D12Resource* source, ID3D12Device* device)
{
	if (!casInit)
		return false;

	if (!casActive)
		return true;

	if (source == nullptr)
		return false;

	spdlog::debug("XeSSFeature::CreateCasBufferResource Start!");

	D3D12_RESOURCE_DESC texDesc = source->GetDesc();

	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags;
	HRESULT hr = source->GetHeapProperties(&heapProperties, &heapFlags);

	if (hr != S_OK)
	{
		spdlog::error("XeSSFeature::CreateBufferResource GetHeapProperties result: {0:x}", hr);
		return false;
	}

	if (casBuffer != nullptr)
		casBuffer->Release();

	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&casBuffer));

	if (hr != S_OK)
	{
		spdlog::error("XeSSFeature::CreateBufferResource CreateCommittedResource result: {0:x}", hr);
		return false;
	}

	return true;
}

bool XeSSFeature::CasDispatch(ID3D12CommandList* commandList, const NVSDK_NGX_Parameter* initParams, ID3D12Resource* input, ID3D12Resource* output)
{
	if (!casInit)
		return false;

	if (!casActive)
		return true;

	spdlog::debug("XeSSFeature::CasDispatch Start!");

	FfxCas::FfxCasDispatchDescription dispatchParameters = {};
	dispatchParameters.commandList = FfxCas::ffxGetCommandListDX12Cas(commandList);
	dispatchParameters.renderSize = { DisplayWidth(), DisplayHeight() };

	if (initParams->Get(NVSDK_NGX_Parameter_Sharpness, &casSharpness) != NVSDK_NGX_Result_Success ||
		Config::Instance()->OverrideSharpness.value_or(false))
	{
		casSharpness = Config::Instance()->Sharpness.value_or(0.4);

		if (casSharpness > 1 || casSharpness < 0)
			casSharpness = 0.4f;
	}

	dispatchParameters.sharpness = casSharpness;

	dispatchParameters.color = FfxCas::ffxGetResourceDX12Cas(input, GetFfxResourceDescriptionDX12(input), nullptr, FfxCas::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatchParameters.output = FfxCas::ffxGetResourceDX12Cas(output, GetFfxResourceDescriptionDX12(output), nullptr, FfxCas::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);

	if (auto errorCode = FfxCas::ffxCasContextDispatch(&casContext, &dispatchParameters); errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("FeatureContext::CasDispatch ffxCasContextDispatch error: {0}", ResultToString(errorCode));
		return false;
	}

	return true;
}