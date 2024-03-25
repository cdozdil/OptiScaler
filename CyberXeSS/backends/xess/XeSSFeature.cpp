#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "../../imgui/d3dx12.h"

#include "XeSSFeature.h"

#include <d3dcompiler.h>

struct Constants
{
	float Multiplier;
};

// pixel.rgb = pow(abs(pixel.rgb) * 4.0, 2.4) * sign(pixel.rgb);
const std::string _recEncodeShaderCode = R"(
Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

// Companding shader
[numthreads(16,16,1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = InputTexture[DTid.xy];
	pixel.rgb *= 10.0;
    OutputTexture[DTid.xy] = pixel;
})";

//    
// pixel.rgb = pow(abs(pixel.rgb), 0.4166666666666667) * sign(pixel.rgb) * 0.25;
const std::string _recDecodeShaderCode = R"(
Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

// Inverse companding shader
[numthreads(16,16,1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = InputTexture[DTid.xy];
	pixel.rgb *= 0.1;
    OutputTexture[DTid.xy] = pixel;
})";

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

static ID3DBlob* CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, 0, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return nullptr;
	}

	if (errorBlob)
		errorBlob->Release();

	return shaderBlob;
}

static bool CreateComputeShader(ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3D12PipelineState** pipelineState, ID3DBlob* shaderBlob)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature;
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	//psoDesc.CS = { reinterpret_cast<UINT8*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize() };
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState));

	if (FAILED(hr))
	{
		// Handle error
		return false;
	}

	return true;
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

	_initFlags = featureFlags;

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
	CreateCasContext(device);

	RecInit(device);

	SetInit(true);

	return true;
}

XeSSFeature::~XeSSFeature()
{
	spdlog::debug("XeSSFeature::XeSSFeature");

	if (_xessContext)
	{
		auto result = xessDestroyContext(_xessContext);
		_xessContext = nullptr;

		if (result != XESS_RESULT_SUCCESS)
			spdlog::error("XeSSFeature::Destroy xessDestroyContext error: {0}", ResultToString(result));
	}

	DestroyCasContext();

	if (casBuffer != nullptr)
	{
		casBuffer->Release();
		casBuffer = nullptr;
	}
}

void XeSSFeature::CasInit()
{
	if (casInit)
		return;

	spdlog::debug("FeatureContext::CasInit Start!");

	casSharpness = Config::Instance()->Sharpness.value_or(0.3);

	if (casSharpness > 1 || casSharpness < 0)
		casSharpness = 0.3f;

	casInit = true;
}

bool XeSSFeature::CreateCasContext(ID3D12Device* device)
{
	if (!casInit)
		return false;

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
	casContextDesc.maxRenderSize.width = DisplayWidth() + 10;
	casContextDesc.maxRenderSize.height = DisplayHeight() + 10;
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

	if (!casContextCreated)
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

	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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

	spdlog::debug("XeSSFeature::CasDispatch Start!");

	FfxCas::FfxCasDispatchDescription dispatchParameters = {};
	dispatchParameters.commandList = FfxCas::ffxGetCommandListDX12Cas(commandList);
	dispatchParameters.renderSize = { DisplayWidth(), DisplayHeight() };

	float dlssSharpness = 0.0f;
	initParams->Get(NVSDK_NGX_Parameter_Sharpness, &dlssSharpness);
	_sharpness = dlssSharpness;

	if (Config::Instance()->OverrideSharpness.value_or(false))
		casSharpness = Config::Instance()->Sharpness.value_or(0.3f);
	else
		casSharpness = dlssSharpness;

	if (casSharpness <= 0.0f)
		return true;
	else if (casSharpness > 1.0f)
		casSharpness = 1.0f;

	dispatchParameters.sharpness = casSharpness;

	dispatchParameters.color = FfxCas::ffxGetResourceDX12Cas(input, GetFfxResourceDescriptionDX12(input), nullptr, FfxCas::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatchParameters.output = FfxCas::ffxGetResourceDX12Cas(output, GetFfxResourceDescriptionDX12(output), nullptr, FfxCas::FFX_RESOURCE_STATE_UNORDERED_ACCESS);

	if (auto errorCode = FfxCas::ffxCasContextDispatch(&casContext, &dispatchParameters); errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("FeatureContext::CasDispatch ffxCasContextDispatch error: {0}", ResultToString(errorCode));
		return false;
	}

	return true;
}

bool XeSSFeature::RecInit(ID3D12Device* InDevice)
{
	// Describe and create the root signature
	// ---------------------------------------------------
	D3D12_DESCRIPTOR_RANGE descriptorRange[2];

	// SRV Range (Input Texture)
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = 0; // Assuming t0 register in HLSL for SRV
	descriptorRange[0].RegisterSpace = 0;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// UAV Range (Output Texture)
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].BaseShaderRegister = 0; // Assuming u0 register in HLSL for UAV
	descriptorRange[1].RegisterSpace = 0;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Define the root parameter (descriptor table)
	// ---------------------------------------------------
	D3D12_ROOT_PARAMETER rootParameters[2];

	// Root Parameter for SRV
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1; // One range (SRV)
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0]; // Point to the SRV range
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root Parameter for UAV
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1; // One range (UAV)
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1]; // Point to the UAV range
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// A root signature is an array of root parameters
	// ---------------------------------------------------
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.NumParameters = 2; // Two root parameters
	rootSigDesc.pParameters = rootParameters;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ID3DBlob* errorBlob;
	ID3DBlob* signatureBlob;

	do
	{
		auto hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

		if (FAILED(hr))
		{
			break;
		}

		hr = InDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&_recRootSignatureDecode));

		if (FAILED(hr))
		{
			break;
		}

	} while (false);

	if (errorBlob != nullptr)
	{
		errorBlob->Release();
		errorBlob = nullptr;
	}

	if (signatureBlob != nullptr)
	{
		signatureBlob->Release();
		signatureBlob = nullptr;
	}

	if (_recRootSignatureDecode == nullptr)
		return false;

	ID3DBlob* errorBlob2;
	ID3DBlob* signatureBlob2;

	do
	{
		auto hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob2, &errorBlob2);

		if (FAILED(hr))
		{
			break;
		}

		hr = InDevice->CreateRootSignature(0, signatureBlob2->GetBufferPointer(), signatureBlob2->GetBufferSize(), IID_PPV_ARGS(&_recRootSignatureEncode));

		if (FAILED(hr))
		{
			break;
		}

	} while (false);

	if (errorBlob2 != nullptr)
	{
		errorBlob2->Release();
		errorBlob2 = nullptr;
	}

	if (signatureBlob2 != nullptr)
	{
		signatureBlob2->Release();
		signatureBlob2 = nullptr;
	}

	if (_recRootSignatureEncode == nullptr)
		return false;

	// Compile shader blobs
	auto _recEncodeShader = CompileShader(_recEncodeShaderCode.c_str(), "main", "cs_5_0");

	if (_recEncodeShader == nullptr)
	{
		return false;
	}

	auto _recDecodeShader = CompileShader(_recDecodeShaderCode.c_str(), "main", "cs_5_0");

	if (_recDecodeShader == nullptr)
	{
		return false;
	}

	do
	{
		// create pso objects
		if (!CreateComputeShader(InDevice, _recRootSignatureEncode, &_recPSOEncode, _recEncodeShader))
		{
			break;
		}

		if (!CreateComputeShader(InDevice, _recRootSignatureDecode, &_recPSODecode, _recDecodeShader))
		{
			break;
		}
	} while (false);

	if (_recEncodeShader != nullptr)
	{
		_recEncodeShader->Release();
		_recEncodeShader = nullptr;
	}

	if (_recDecodeShader != nullptr)
	{
		_recDecodeShader->Release();
		_recDecodeShader = nullptr;
	}

	_recInit = _recPSOEncode != nullptr && _recPSODecode != nullptr;

	return _recInit;
}

bool XeSSFeature::RecDecode(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* input, ID3D12Resource* output)
{
	if (!_recInit)
		return false;

	ID3D12DescriptorHeap* srvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2; // One for SRV and one for UAV
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap));

	if (FAILED(hr))
	{
		return false;
	}

	auto srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto uavHandle = srvHandle;
	uavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuUavHandle = gpuSrvHandle;
	gpuUavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create SRV for Input Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = input->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	InDevice->CreateShaderResourceView(input, &srvDesc, srvHandle);

	// Create UAV for Output Texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = output->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	InDevice->CreateUnorderedAccessView(output, nullptr, &uavDesc, uavHandle);

	ID3D12DescriptorHeap* heaps[] = { srvHeap };
	InCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	InCmdList->SetComputeRootSignature(_recRootSignatureDecode);
	InCmdList->SetPipelineState(_recPSODecode);

	InCmdList->SetComputeRootDescriptorTable(0, gpuSrvHandle);
	InCmdList->SetComputeRootDescriptorTable(1, gpuUavHandle);

	UINT dispatchWidth = (input->GetDesc().Width + 7) / 16;
	UINT dispatchHeight = (input->GetDesc().Height + 7) / 16;
	InCmdList->Dispatch(dispatchWidth, dispatchHeight, 1);

	//ID3D12Fence* d3d12Fence;
	//InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
	//d3d12Fence->Signal(999);

	//auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	//if (d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
	//{
	//	WaitForSingleObject(fenceEvent, INFINITE);
	//	CloseHandle(fenceEvent);
	//}
	//else
	//	spdlog::warn("IFeature_Dx11wDx12::~IFeature_Dx11wDx12 can't get fenceEvent handle");

	//d3d12Fence->Release();

	return true;
}

bool XeSSFeature::RecEncode(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* input, ID3D12Resource* output)
{
	if (!_recInit)
		return false;

	ID3D12DescriptorHeap* srvHeap;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2; // One for SRV and one for UAV
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap));

	if (FAILED(hr))
	{
		return false;
	}

	auto srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto uavHandle = srvHandle;
	uavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuUavHandle = gpuSrvHandle;
	gpuUavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create SRV for Input Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = input->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	InDevice->CreateShaderResourceView(input, &srvDesc, srvHandle);

	// Create UAV for Output Texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = output->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	InDevice->CreateUnorderedAccessView(output, nullptr, &uavDesc, uavHandle);

	ID3D12DescriptorHeap* heaps[] = { srvHeap };
	InCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	InCmdList->SetComputeRootSignature(_recRootSignatureEncode);
	InCmdList->SetPipelineState(_recPSOEncode);

	InCmdList->SetComputeRootDescriptorTable(0, gpuSrvHandle);
	InCmdList->SetComputeRootDescriptorTable(1, gpuUavHandle);

	UINT dispatchWidth = (input->GetDesc().Width + 7) / 16;
	UINT dispatchHeight = (input->GetDesc().Height + 7) / 16;
	InCmdList->Dispatch(dispatchWidth, dispatchHeight, 1);

	return true;
}

bool XeSSFeature::CreateRecBufferResource(ID3D12Resource* source, ID3D12Device* device, ID3D12Resource** output)
{
	if (!_recInit)
		return false;

	if (source == nullptr)
		return false;

	D3D12_RESOURCE_DESC texDesc = source->GetDesc();

	if (*output != nullptr)
	{
		D3D12_RESOURCE_DESC outDesc = (*output)->GetDesc();

		if (outDesc.Width != texDesc.Width || outDesc.Height != texDesc.Height || outDesc.Format != texDesc.Format)
		{
			(*output)->Release();
			(*output) = nullptr;
		}
		else
		{
			return true;
		}
	}
	spdlog::debug("XeSSFeature::CreateRecBufferResource Start!");

	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags;
	HRESULT hr = source->GetHeapProperties(&heapProperties, &heapFlags);

	if (hr != S_OK)
	{
		spdlog::error("XeSSFeature::CreateRecBufferResource GetHeapProperties result: {0:x}", hr);
		return false;
	}

	if (*output != nullptr)
		(*output)->Release();

	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

 	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(output));

	if (hr != S_OK)
	{
		spdlog::error("XeSSFeature::CreateRecBufferResource CreateCommittedResource result: {0:x}", hr);
		return false;
	}

	return true;
}