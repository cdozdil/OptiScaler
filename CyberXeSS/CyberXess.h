#pragma once
#define FFX_CAS

#include "pch.h"
#include "NGXParameter.h"
#include "xess_d3d12.h"
#include "xess_debug.h"
#include "xess_d3d12_debug.h"
#include "dxgi1_6.h"
#include "FidelityFX/host/ffx_cas.h"
#include <FidelityFX/host/backends/dx12/ffx_dx12.h>

#define ASSIGN_DESC(dest, src) dest->Width = src.Width; dest->Height = src.Height; dest->Format = src.Format; dest->BindFlags = src.BindFlags; 

using D3D11_TEXTURE2D_DESC_C = struct D3D11_TEXTURE2D_DESC_C
{
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
	UINT BindFlags;
	void* pointer = nullptr;
	HANDLE handle = NULL;
};

using D3D11_TEXTURE2D_RESOURCE_C = struct D3D11_TEXTURE2D_RESOURCE_C
{
	D3D11_TEXTURE2D_DESC_C Desc = {};
	HANDLE SharedHandle = NULL;
	ID3D11Texture2D* SharedTexture = nullptr;
};

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
	std::string s = Message;
	spdlog::log((spdlog::level::level_enum)((int)Level + 1), "FeatureContext::LogCallback XeSS Runtime ({0})", s);
}

class FeatureContext;

//Global Context
class CyberXessContext
{
	unsigned int handleCounter = 1000000;

	CyberXessContext();

	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter** ppAdapter, D3D_FEATURE_LEVEL featureLevel, bool requestHighPerformanceAdapter) const
	{
		*ppAdapter = nullptr;

		IDXGIAdapter1* adapter;

		IDXGIFactory6* factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter));
					++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.

				auto result = D3D12CreateDevice(adapter, featureLevel, _uuidof(ID3D12Device), nullptr);

				if (result == S_FALSE)
				{
					*ppAdapter = adapter;
					break;
				}
			}
		}
		else
		{
			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.

				auto result = D3D12CreateDevice(adapter, featureLevel, _uuidof(ID3D12Device), nullptr);

				if (result == S_FALSE)
				{
					*ppAdapter = adapter;
					break;
				}
			}
		}

	}

public:
	std::shared_ptr<Config> MyConfig;

	//const NvParameter* CreateFeatureParams;

	// D3D12 stuff
	ID3D12Device* Dx12Device = nullptr;

	// D3D11 stuff
	ID3D11Device5* Dx11Device = nullptr;
	ID3D11DeviceContext4* Dx11DeviceContext = nullptr;

	// D3D11with12 stuff
	ID3D12CommandQueue* Dx12CommandQueue = nullptr;
	ID3D12CommandAllocator* Dx12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* Dx12CommandList = nullptr;

	// Vulkan stuff
	VkDevice VulkanDevice = nullptr;
	VkInstance VulkanInstance = nullptr;
	VkPhysicalDevice VulkanPhysicalDevice = nullptr;

	//std::shared_ptr<NvParameter> NvParameterInstance = NvParameter::instance();

	ankerl::unordered_dense::map <unsigned int, std::unique_ptr<FeatureContext>> Contexts;
	FeatureContext* CreateContext();
	void DeleteContext(NVSDK_NGX_Handle* handle);

	static std::shared_ptr<CyberXessContext> instance()
	{
		static std::shared_ptr<CyberXessContext> INSTANCE{ std::make_shared<CyberXessContext>(CyberXessContext()) };
		return INSTANCE;
	}

	void Shutdown(bool fromDx11 = false, bool shutdownEvent = false) const
	{
		if (fromDx11 && shutdownEvent)
		{
			SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandList);
			SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandQueue);
			SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandAllocator);
			SAFE_RELEASE(CyberXessContext::instance()->Dx12Device);
		}
	}

	HRESULT CreateDx12Device(D3D_FEATURE_LEVEL featureLevel)
	{
		if (Dx12Device != nullptr)
			return S_OK;

		HRESULT result;

		IDXGIFactory4* factory;
		result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

		if (result != S_OK)
		{
			spdlog::error("CyberXessContext::CreateDx12Device Can't create factory: {0:x}", result);
			return result;
		}

		IDXGIAdapter* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter, featureLevel, true);

		if (hardwareAdapter == nullptr)
		{
			spdlog::error("CyberXessContext::CreateDx12Device Can't get hardwareAdapter!");
			return E_NOINTERFACE;
		}

		result = D3D12CreateDevice(hardwareAdapter, featureLevel, IID_PPV_ARGS(&Dx12Device));

		if (result != S_OK)
		{
			spdlog::error("CyberXessContext::CreateDx12Device Can't create device: {0:x}", result);
			return result;
		}

		SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandList);
		SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandQueue);
		SAFE_RELEASE(CyberXessContext::instance()->Dx12CommandAllocator);

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// CreateCommandQueue
		result = Dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));
		spdlog::debug("CyberXessContext::CreateDx12Device CreateCommandQueue result: {0:x}", result);

		if (result != S_OK || Dx12CommandQueue == nullptr)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		return S_OK;
	}
};

class FeatureContext
{
	ID3D12Device* dx12device = nullptr;

	// xess
	xess_context_handle_t xessContext = nullptr;
	bool xessMaskEnabled = true;
	bool xessInit = false;

	// cas
	FfxCasContext casContext;
	bool casInit = false;
	bool casContextCreated = false;
	bool casUpscale = false;
	bool casActive = false;
	float casSharpness = 0.4f;
	ID3D12Resource* casBuffer = nullptr;
	FfxCasContextDescription casContextDesc = {};

	// dx11
	D3D11_TEXTURE2D_RESOURCE_C dx11Color = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Mv = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Depth = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Tm = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Exp = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Out = {};

#pragma region cas methods

	void CasInit()
	{
		if (casInit)
			return;

		spdlog::debug("FeatureContext::CasInit Start!");

		casActive = CyberXessContext::instance()->MyConfig->CasEnabled.value_or(true);
		casSharpness = CyberXessContext::instance()->MyConfig->CasSharpness.value_or(0.3);

		if (casSharpness > 1 || casSharpness < 0)
			casSharpness = 0.4f;

		casInit = true;
	}

	bool CreateCasContext()
	{
		if (!casInit)
			return false;

		if (!casActive)
			return true;

		spdlog::debug("FeatureContext::CreateCasContext Start!");

		DestroyCasContext();

		const size_t scratchBufferSize = ffxGetScratchMemorySizeDX12(FFX_CAS_CONTEXT_COUNT);
		void* casScratchBuffer = calloc(scratchBufferSize, 1);

		auto errorCode = ffxGetInterfaceDX12(&casContextDesc.backendInterface, dx12device, casScratchBuffer, scratchBufferSize, FFX_CAS_CONTEXT_COUNT);

		if (errorCode != FFX_OK)
		{
			spdlog::error("FeatureContext::CreateCasContext ffxGetInterfaceDX12 error: {0:x}", errorCode);
			free(casScratchBuffer);
			return false;
		}

		casUpscale = false;
		casContextDesc.flags |= FFX_CAS_SHARPEN_ONLY;

		/*
		* We will probably never use CAS upscale 
		if (RenderWidth == DisplayWidth && RenderHeight == DisplayHeight)
		{
			casUpscale = false;
			casContextDesc.flags |= FFX_CAS_SHARPEN_ONLY;
		}
		else
		{
			casUpscale = true;
			casContextDesc.flags &= ~FFX_CAS_SHARPEN_ONLY;
		}
		*/

		auto casCSC = CyberXessContext::instance()->MyConfig->ColorSpaceConversion.value_or(FFX_CAS_COLOR_SPACE_LINEAR);

		if (casCSC < 0 || casCSC > 4)
			casCSC = 0;

		casContextDesc.colorSpaceConversion = static_cast<FfxCasColorSpaceConversion>(casCSC);
		casContextDesc.maxRenderSize.width = RenderWidth;
		casContextDesc.maxRenderSize.height = RenderHeight;
		casContextDesc.displaySize.width = DisplayWidth;
		casContextDesc.displaySize.height = DisplayHeight;

		errorCode = ffxCasContextCreate(&casContext, &casContextDesc);



		if (errorCode != FFX_OK)
		{
			spdlog::error("FeatureContext::CreateCasContext ffxCasContextCreate error: {0:x}", errorCode);
			return false;
		}

		casContextCreated = true;
		return true;
	}

	void DestroyCasContext()
	{
		spdlog::debug("FeatureContext::DestroyCasContext Start!");

		if (!casActive || !casContextCreated)
			return;

		auto errorCode = ffxCasContextDestroy(&casContext);

		if (errorCode != FFX_OK)
			spdlog::error("FeatureContext::DestroyCasContext ffxCasContextDestroy error: {0:x}", errorCode);

		free(casContextDesc.backendInterface.scratchBuffer);

		casContextCreated = false;
	}

	bool CreateCasBufferResource(ID3D12Resource* source)
	{
		if (!casInit)
			return false;

		if (!casActive)
			return true;

		if (source == nullptr)
			return false;

		spdlog::debug("FeatureContext::CreateCasBufferResource Start!");

		D3D12_RESOURCE_DESC texDesc = source->GetDesc();

		D3D12_HEAP_PROPERTIES heapProperties;
		D3D12_HEAP_FLAGS heapFlags;
		HRESULT hr = source->GetHeapProperties(&heapProperties, &heapFlags);

		if (hr != S_OK)
		{
			spdlog::error("FeatureContext::CreateBufferResource GetHeapProperties result: {0:x}", hr);
			return false;
		}

		if (casBuffer != nullptr)
			casBuffer->Release();

		hr = dx12device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&casBuffer));

		if (hr != S_OK)
		{
			spdlog::error("FeatureContext::CreateBufferResource CreateCommittedResource result: {0:x}", hr);
			return false;
		}

		return true;
	}

	bool CasDispatch(ID3D12CommandList* commandList, const NVSDK_NGX_Parameter* initParams, ID3D12Resource* input, ID3D12Resource* output)
	{
		if (!casInit)
			return false;

		if (!casActive)
			return true;

		spdlog::debug("FeatureContext::CasDispatch Start!");

		FfxCasDispatchDescription dispatchParameters = {};
		dispatchParameters.commandList = ffxGetCommandListDX12(commandList);
		dispatchParameters.renderSize = { DisplayWidth, DisplayHeight };

		if (initParams->Get(NVSDK_NGX_Parameter_Sharpness, &casSharpness) != NVSDK_NGX_Result_Success ||
			CyberXessContext::instance()->MyConfig->CasOverrideSharpness.value_or(false))
		{
			casSharpness = CyberXessContext::instance()->MyConfig->CasSharpness.value_or(0.4);

			if (casSharpness > 1 || casSharpness < 0)
				casSharpness = 0.4f;
		}

		dispatchParameters.sharpness = casSharpness;

		dispatchParameters.color = ffxGetResourceDX12(input, GetFfxResourceDescriptionDX12(input), nullptr, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
		dispatchParameters.output = ffxGetResourceDX12(output, GetFfxResourceDescriptionDX12(output), nullptr, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);


		if (auto errorCode = ffxCasContextDispatch(&casContext, &dispatchParameters); errorCode != FFX_OK)
		{
			spdlog::error("FeatureContext::CasDispatch ffxCasContextDispatch error: {0:x}", errorCode);
			return false;
		}

		return true;
	}

#pragma endregion

	bool CopyTextureFrom11To12(ID3D11Resource* d3d11texture, ID3D11Texture2D** pSharedTexture, D3D11_TEXTURE2D_DESC_C* sharedDesc, bool copy = true)
	{
		ID3D11Texture2D* originalTexture = nullptr;
		D3D11_TEXTURE2D_DESC desc{};

		auto result = d3d11texture->QueryInterface(IID_PPV_ARGS(&originalTexture));

		if (result != S_OK)
			return false;

		originalTexture->GetDesc(&desc);

		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
		{
			if (desc.Width != sharedDesc->Width || desc.Height != sharedDesc->Height ||
				desc.Format != sharedDesc->Format || desc.BindFlags != sharedDesc->BindFlags ||
				(*pSharedTexture) == nullptr)
			{
				if ((*pSharedTexture) != nullptr)
					(*pSharedTexture)->Release();

				ASSIGN_DESC(sharedDesc, desc);
				sharedDesc->pointer = nullptr;
				sharedDesc->handle = NULL;

				desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
				result = CyberXessContext::instance()->Dx11Device->CreateTexture2D(&desc, nullptr, pSharedTexture);

				IDXGIResource1* resource;

				result = (*pSharedTexture)->QueryInterface(IID_PPV_ARGS(&resource));

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
					return false;
				}

				// Get shared handle
				result = resource->GetSharedHandle(&sharedDesc->handle);

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
					return false;
				}

				resource->Release();
				sharedDesc->pointer = (*pSharedTexture);
			}

			if (copy)
				CyberXessContext::instance()->Dx11DeviceContext->CopyResource(*pSharedTexture, d3d11texture);
		}
		else
		{
			if (sharedDesc->pointer != d3d11texture)
			{
				IDXGIResource1* resource;

				result = originalTexture->QueryInterface(IID_PPV_ARGS(&resource));

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
					return false;
				}

				// Get shared handle
				result = resource->GetSharedHandle(&sharedDesc->handle);

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
					return false;
				}

				resource->Release();
				sharedDesc->pointer = d3d11texture;
			}
		}

		originalTexture->Release();
		return true;
	}

	void ReleaseSharedResources()
	{
		spdlog::debug("FeatureContext::ReleaseSharedResources start!");

		SAFE_RELEASE(casBuffer);
		SAFE_RELEASE(dx11Color.SharedTexture);
		SAFE_RELEASE(dx11Mv.SharedTexture);
		SAFE_RELEASE(dx11Out.SharedTexture);
		SAFE_RELEASE(dx11Depth.SharedTexture);
		SAFE_RELEASE(dx11Tm.SharedTexture);
		SAFE_RELEASE(dx11Exp.SharedTexture);
	}

public:
	NVSDK_NGX_Handle Handle;

	uint32_t DisplayWidth{};
	uint32_t DisplayHeight{};
	uint32_t RenderWidth{};
	uint32_t RenderHeight{};

	NVSDK_NGX_PerfQuality_Value PerfQualityValue = NVSDK_NGX_PerfQuality_Value_Balanced;

#pragma region xess methods

	bool XeSSMaskEnabled()
	{
		return xessMaskEnabled;
	}

	bool XeSSInit(ID3D12Device* device, const NVSDK_NGX_Parameter* initParams)
	{
		if (device == nullptr)
		{
			spdlog::error("FeatureContext::XeSSInit D3D12Device is null!");
			return false;
		}

		if (xessInit)
			return true;

		dx12device = device;

		unsigned int width, outWidth, height, outHeight;
		initParams->Get(NVSDK_NGX_Parameter_Width, &width);
		initParams->Get(NVSDK_NGX_Parameter_Height, &height);
		initParams->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth);
		initParams->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight);

		DisplayWidth = width > outWidth ? width : outWidth;
		DisplayHeight = height > outHeight ? height : outHeight;
		RenderWidth = width < outWidth ? width : outWidth;
		RenderHeight = height < outHeight ? height : outHeight;

		spdlog::info("FeatureContext::XeSSInit Output Resolution: {0}x{1}", DisplayWidth, DisplayHeight);

		xess_version_t ver;
		xess_result_t ret = xessGetVersion(&ver);

		if (ret == XESS_RESULT_SUCCESS)
			spdlog::info("FeatureContext::XeSSInit XeSS Version: {0}.{1}.{2}", ver.major, ver.minor, ver.patch);
		else
			spdlog::warn("FeatureContext::XeSSInit xessGetVersion error: {0}", ResultToString(ret));

		ret = xessD3D12CreateContext(dx12device, &xessContext);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("FeatureContext::XeSSInit xessD3D12CreateContext error: {0}", ResultToString(ret));
			return false;
		}

		ret = xessIsOptimalDriver(xessContext);
		spdlog::debug("FeatureContext::XeSSInit xessIsOptimalDriver : {0}", ResultToString(ret));

		ret = xessSetLoggingCallback(xessContext, XESS_LOGGING_LEVEL_DEBUG, logCallback);
		spdlog::debug("FeatureContext::XeSSInit xessSetLoggingCallback : {0}", ResultToString(ret));

#pragma region Create Parameters for XeSS

		xess_d3d12_init_params_t xessParams{};

		xessParams.outputResolution.x = DisplayWidth;
		xessParams.outputResolution.y = DisplayHeight;

		int pqValue;
		initParams->Get(NVSDK_NGX_Parameter_PerfQualityValue, &pqValue);

		if (CyberXessContext::instance()->MyConfig->OverrideQuality.has_value())
		{
			xessParams.qualitySetting = (xess_quality_settings_t)CyberXessContext::instance()->MyConfig->OverrideQuality.value();
		}
		else
		{
			switch ((NVSDK_NGX_PerfQuality_Value)pqValue)
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
		initParams->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

		bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
		bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
		bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
		bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
		bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
		bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

		if (CyberXessContext::instance()->MyConfig->DepthInverted.value_or(DepthInverted))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
			CyberXessContext::instance()->MyConfig->DepthInverted = true;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
		}

		if (AutoExposure || CyberXessContext::instance()->MyConfig->AutoExposure.value_or(AutoExposure))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
			CyberXessContext::instance()->MyConfig->AutoExposure = true;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
		}
		else
		{
			xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
		}

		if (!CyberXessContext::instance()->MyConfig->HDR.value_or(!Hdr))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
			CyberXessContext::instance()->MyConfig->HDR = false;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (HDR) {0:b}", xessParams.initFlags);
		}

		if (CyberXessContext::instance()->MyConfig->JitterCancellation.value_or(JitterMotion))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
			CyberXessContext::instance()->MyConfig->JitterCancellation = true;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
		}

		if (CyberXessContext::instance()->MyConfig->DisplayResolution.value_or(!LowRes))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
			CyberXessContext::instance()->MyConfig->DisplayResolution = true;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (LowRes) {0:b}", xessParams.initFlags);
		}

		if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(false))
		{
			xessMaskEnabled = true;
			xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
			spdlog::info("FeatureContext::XeSSInit xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
		}
		else
			xessMaskEnabled = false;

#pragma endregion

#pragma region Build Pipelines

		if (CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true))
		{
			spdlog::debug("FeatureContext::XeSSInit xessD3D12BuildPipelines!");

			ret = xessD3D12BuildPipelines(xessContext, NULL, false, xessParams.initFlags);

			if (ret != XESS_RESULT_SUCCESS)
			{
				spdlog::error("FeatureContext::XeSSInit xessD3D12BuildPipelines error: {0}", ResultToString(ret));
				return false;
			}
		}

#pragma endregion

		spdlog::debug("FeatureContext::XeSSInit xessD3D12Init!");

		ret = xessD3D12Init(xessContext, &xessParams);

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("FeatureContext::XeSSInit xessD3D12Init error: {0}", ResultToString(ret));
			return false;
		}



		CasInit();

		if (casActive)
			CreateCasContext();

		xessInit = true;

		return true;
	}

	void XeSSDestroy()
	{
		if (!xessInit || xessContext == nullptr)
			return;

		spdlog::debug("FeatureContext::XeSSDestroy!");

		auto result = xessDestroyContext(xessContext);

		if (result != XESS_RESULT_SUCCESS)
			spdlog::error("FeatureContext::XeSSDestroy xessDestroyContext error: {0}", ResultToString(result));

		xessContext = nullptr;
		xessInit = false;

		DestroyCasContext();
		ReleaseSharedResources();
	}

	bool XeSSIsInited() const
	{
		return xessInit;
	}

	bool XeSSExecuteDx12(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* initParams, const FeatureContext* context)
	{
		if (!xessInit)
			return false;

		const auto instance = CyberXessContext::instance();

		xess_result_t xessResult;
		xess_d3d12_execute_params_t params{};

		initParams->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
		initParams->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

		if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
			params.exposureScale = 1.0f;

		initParams->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

		if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, &params.inputWidth) != NVSDK_NGX_Result_Success ||
			initParams->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, &params.inputHeight) != NVSDK_NGX_Result_Success)
		{
			unsigned int width, height, outWidth, outHeight;

			if (initParams->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
				initParams->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success)
			{
				if (initParams->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
					initParams->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success)
				{
					if (width < outWidth)
					{
						params.inputWidth = width;
						params.inputHeight = height;
					}
					else
					{
						params.inputWidth = outWidth;
						params.inputHeight = outHeight;
					}
				}
				else
				{
					if (width < context->RenderWidth)
					{
						params.inputWidth = width;
						params.inputHeight = height;
					}
					else
					{
						params.inputWidth = context->RenderWidth;
						params.inputHeight = context->RenderHeight;
					}
				}
			}
			else
			{
				params.inputWidth = context->RenderWidth;
				params.inputHeight = context->RenderHeight;
			}
		}

		spdlog::debug("FeatureContext::XeSSInit Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

		if (initParams->Get(NVSDK_NGX_Parameter_Color, &params.pColorTexture) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Color, (void**)&params.pColorTexture);

		if (params.pColorTexture)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx12 Color exist..");

			if (instance->MyConfig->ColorResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = params.pColorTexture;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->ColorResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx12 Color not exist!!");
			return false;
		}

		if (initParams->Get(NVSDK_NGX_Parameter_MotionVectors, &params.pVelocityTexture) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&params.pVelocityTexture);

		if (params.pVelocityTexture)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx12 MotionVectors exist..");

			if (instance->MyConfig->MVResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = params.pVelocityTexture;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->MVResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx12 MotionVectors not exist!!");
			return false;
		}

		ID3D12Resource* paramOutput;
		if (initParams->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

		if (paramOutput)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx12 Output exist..");

			if (instance->MyConfig->OutputResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = paramOutput;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->OutputResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}

			if (casActive)
			{
				if (casBuffer == nullptr && !CreateCasBufferResource(paramOutput))
				{
					spdlog::error("FeatureContext::XeSSExecuteDx12 Can't create cas buffer!");
					return false;
				}

				params.pOutputTexture = casBuffer;
			}
			else
				params.pOutputTexture = paramOutput;
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx12 Output not exist!!");
			return false;
		}

		if (initParams->Get(NVSDK_NGX_Parameter_Depth, &params.pDepthTexture) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Depth, (void**)&params.pDepthTexture);

		if (params.pDepthTexture)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx12 Depth exist..");

			if (instance->MyConfig->DepthResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = params.pDepthTexture;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->DepthResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}
		}
		else
		{
			if (!instance->MyConfig->DisplayResolution.value_or(false))
				spdlog::error("FeatureContext::XeSSExecuteDx12 Depth not exist!!");
			else
				spdlog::info("FeatureContext::XeSSExecuteDx12 Using high res motion vectors, depth is not needed!!");

			params.pDepthTexture = nullptr;
		}

		if (!instance->MyConfig->AutoExposure.value_or(false))
		{
			if (initParams->Get(NVSDK_NGX_Parameter_ExposureTexture, &params.pExposureScaleTexture) != NVSDK_NGX_Result_Success)
				initParams->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&params.pExposureScaleTexture);

			if (instance->MyConfig->ExposureResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = params.pExposureScaleTexture;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->ExposureResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}

			if (params.pExposureScaleTexture)
				spdlog::debug("FeatureContext::XeSSExecuteDx12 ExposureTexture exist..");
			else
				spdlog::debug("FeatureContext::XeSSExecuteDx12 AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
		}
		else
			spdlog::debug("FeatureContext::XeSSExecuteDx12 AutoExposure enabled!");

		if (!instance->MyConfig->DisableReactiveMask.value_or(false))
		{
			if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &params.pResponsivePixelMaskTexture) != NVSDK_NGX_Result_Success)
				initParams->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&params.pResponsivePixelMaskTexture);

			if (instance->MyConfig->MaskResourceBarrier.has_value())
			{
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Transition.pResource = params.pResponsivePixelMaskTexture;
				barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)instance->MyConfig->MaskResourceBarrier.value();
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				barrier.Transition.Subresource = 0;
				commandList->ResourceBarrier(1, &barrier);
			}

			if (params.pResponsivePixelMaskTexture)
				spdlog::debug("FeatureContext::XeSSExecuteDx12 Bias mask exist..");
			else
				spdlog::debug("FeatureContext::XeSSExecuteDx12 Bias mask not exist and its enabled in config, it may cause problems!!");
		}

		float MVScaleX;
		float MVScaleY;

		if (initParams->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
			initParams->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
		{
			xessResult = xessSetVelocityScale(xessContext, MVScaleX, MVScaleY);

			if (xessResult != XESS_RESULT_SUCCESS)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx12 xessSetVelocityScale: {0}", ResultToString(xessResult));
				return false;
			}
		}
		else
			spdlog::warn("FeatureContext::XeSSExecuteDx12 Can't get motion vector scales!");

		spdlog::debug("FeatureContext::XeSSExecuteDx12 Executing!!");
		xessResult = xessD3D12Execute(xessContext, commandList, &params);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("FeatureContext::XeSSExecuteDx12 xessD3D12Execute error: {0}", ResultToString(xessResult));
			return false;
		}

		//apply cas
		if (casActive && !CasDispatch(commandList, initParams, casBuffer, paramOutput))
			return false;

		// restore resource states
		if (params.pColorTexture && instance->MyConfig->ColorResourceBarrier.value_or(false))
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = params.pColorTexture;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->ColorResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		if (instance->MyConfig->MVResourceBarrier.has_value())
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = params.pVelocityTexture;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->MVResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		if (instance->MyConfig->OutputResourceBarrier.has_value())
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = paramOutput;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->OutputResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		if (instance->MyConfig->DepthResourceBarrier.has_value())
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = params.pDepthTexture;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->DepthResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		if (instance->MyConfig->ExposureResourceBarrier.has_value())
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = params.pExposureScaleTexture;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->ExposureResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		if (instance->MyConfig->MaskResourceBarrier.has_value())
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = params.pResponsivePixelMaskTexture;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)instance->MyConfig->MaskResourceBarrier.value();
			barrier.Transition.Subresource = 0;
			commandList->ResourceBarrier(1, &barrier);
		}

		return true;
	}

	bool XeSSExecuteDx11(ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue,
		ID3D11Device* dx11device, ID3D11DeviceContext* deviceContext,
		const NVSDK_NGX_Parameter* initParams, const FeatureContext* context)
	{
		if (!xessInit)
			return false;

		const auto instance = CyberXessContext::instance();

		// Fence for syncing
		ID3D12Fence* d3d12Fence;
		auto result = dx12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));

		if (result != S_OK)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx11 CreateFence d3d12fence error: {0:x}", result);
			return false;
		}

		// creatimg params for XeSS
		xess_result_t xessResult;
		xess_d3d12_execute_params_t params{};

		initParams->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
		initParams->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

		if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
			params.exposureScale = 1.0f;

		initParams->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

		if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, &params.inputWidth) != NVSDK_NGX_Result_Success ||
			initParams->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, &params.inputHeight) != NVSDK_NGX_Result_Success)
		{
			unsigned int width, height, outWidth, outHeight;

			if (initParams->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
				initParams->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success)
			{
				if (initParams->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
					initParams->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success)
				{
					if (width < outWidth)
					{
						params.inputWidth = width;
						params.inputHeight = height;
					}
					else
					{
						params.inputWidth = outWidth;
						params.inputHeight = outHeight;
					}
				}
				else
				{
					if (width < context->RenderWidth)
					{
						params.inputWidth = width;
						params.inputHeight = height;
					}
					else
					{
						params.inputWidth = context->RenderWidth;
						params.inputHeight = context->RenderHeight;
					}
				}
			}
			else
			{
				params.inputWidth = context->RenderWidth;
				params.inputHeight = context->RenderHeight;
			}
		}

		spdlog::debug("FeatureContext::XeSSInit Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

#pragma region Texture copies

		D3D11_QUERY_DESC pQueryDesc;
		pQueryDesc.Query = D3D11_QUERY_EVENT;
		pQueryDesc.MiscFlags = 0;
		ID3D11Query* query1 = nullptr;
		result = dx11device->CreateQuery(&pQueryDesc, &query1);

		if (result != S_OK || query1 == nullptr || query1 == NULL)
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 can't create query1!");
			return false;
		}

		// Associate the query with the copy operation
		deviceContext->Begin(query1);

		ID3D11Resource* paramColor;
		if (initParams->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

		if (paramColor)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx11 Color exist..");
			if (CopyTextureFrom11To12(paramColor, &dx11Color.SharedTexture, &dx11Color.Desc) == NULL)
				return false;
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 Color not exist!!");
			return false;
		}

		ID3D11Resource* paramMv;
		if (initParams->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMv) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramMv);

		if (paramMv)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx11 MotionVectors exist..");
			if (CopyTextureFrom11To12(paramMv, &dx11Mv.SharedTexture, &dx11Mv.Desc) == false)
				return false;
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 MotionVectors not exist!!");
			return false;
		}

		ID3D11Resource* paramOutput;
		if (initParams->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

		if (paramOutput)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx11 Output exist..");
			if (CopyTextureFrom11To12(paramOutput, &dx11Out.SharedTexture, &dx11Out.Desc, false) == false)
				return false;
		}
		else
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 Output not exist!!");
			return false;
		}

		ID3D11Resource* paramDepth;
		if (initParams->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
			initParams->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

		if (paramDepth)
		{
			spdlog::debug("FeatureContext::XeSSExecuteDx11 Depth exist..");
			if (CopyTextureFrom11To12(paramDepth, &dx11Depth.SharedTexture, &dx11Depth.Desc) == false)
				return false;
		}
		else
			spdlog::error("FeatureContext::XeSSExecuteDx11 Depth not exist!!");

		ID3D11Resource* paramExposure = nullptr;
		if (!instance->MyConfig->AutoExposure.value_or(false))
		{
			if (initParams->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExposure) != NVSDK_NGX_Result_Success)
				initParams->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExposure);

			if (paramExposure)
			{
				spdlog::debug("FeatureContext::XeSSExecuteDx11 ExposureTexture exist..");

				if (CopyTextureFrom11To12(paramExposure, &dx11Exp.SharedTexture, &dx11Exp.Desc) == false)
					return false;
			}
			else
				spdlog::warn("FeatureContext::XeSSExecuteDx11 AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
		}
		else
			spdlog::debug("FeatureContext::XeSSExecuteDx11 AutoExposure enabled!");

		ID3D11Resource* paramMask = nullptr;
		if (!instance->MyConfig->DisableReactiveMask.value_or(false))
		{
			if (initParams->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
				initParams->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

			if (paramMask)
			{
				spdlog::debug("FeatureContext::XeSSExecuteDx11 Bias mask exist..");
				if (CopyTextureFrom11To12(paramMask, &dx11Tm.SharedTexture, &dx11Tm.Desc) == false)
					return false;
			}
			else
				spdlog::warn("FeatureContext::XeSSExecuteDx11 bias mask not exist and its enabled in config, it may cause problems!!");
		}

		// Execute dx11 commands 
		deviceContext->End(query1);
		deviceContext->Flush();

		// Wait for the query to be ready
		while (instance->Dx11DeviceContext->GetData(query1, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
			//
		}

		// Release the query
		query1->Release();
		ID3D12Resource* dx11OutputBuffer = nullptr;

		if (paramColor)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Color.Desc.handle, IID_PPV_ARGS(&params.pColorTexture));

			if (result != S_OK)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 Color OpenSharedHandle error: {0:x}", result);
				return false;
			}
		}

		if (paramMv)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Mv.Desc.handle, IID_PPV_ARGS(&params.pVelocityTexture));

			if (result != S_OK)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 MotionVectors OpenSharedHandle error: {0:x}", result);
				return false;
			}
		}

		if (paramOutput)
		{
			if (casActive)
			{
				result = instance->Dx12Device->OpenSharedHandle(dx11Out.Desc.handle, IID_PPV_ARGS(&dx11OutputBuffer));
				dx11Out.SharedHandle = dx11Out.Desc.handle;

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::XeSSExecuteDx11 Output OpenSharedHandle error: {0:x}", result);
					return false;
				}

				if (casBuffer == nullptr && !CreateCasBufferResource(dx11OutputBuffer))
				{
					spdlog::error("FeatureContext::XeSSExecuteDx11 Can't create cas buffer!");
					return false;
				}

				params.pOutputTexture = casBuffer;
			}
			else
			{
				result = instance->Dx12Device->OpenSharedHandle(dx11Out.Desc.handle, IID_PPV_ARGS(&params.pOutputTexture));
				dx11Out.SharedHandle = dx11Out.Desc.handle;

				if (result != S_OK)
				{
					spdlog::error("FeatureContext::XeSSExecuteDx11 Output OpenSharedHandle error: {0:x}", result);
					return false;
				}
			}
		}

		if (paramDepth)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Depth.Desc.handle, IID_PPV_ARGS(&params.pDepthTexture));

			if (result != S_OK)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 Depth OpenSharedHandle error: {0:x}", result);
				return false;
			}
		}

		if (!instance->MyConfig->AutoExposure.value_or(false) && paramExposure)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Exp.Desc.handle, IID_PPV_ARGS(&params.pExposureScaleTexture));

			if (result != S_OK)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 ExposureTexture OpenSharedHandle error: {0:x}", result);
				return false;
			}
		}

		if (!instance->MyConfig->DisableReactiveMask.value_or(false) && paramMask)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Tm.Desc.handle, IID_PPV_ARGS(&params.pResponsivePixelMaskTexture));

			if (result != S_OK)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 TransparencyMask OpenSharedHandle error: {0:x}", result);
				return false;
			}
		}

#pragma endregion

		float MVScaleX;
		float MVScaleY;

		if (initParams->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
			initParams->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
		{
			xessResult = xessSetVelocityScale(xessContext, MVScaleX, MVScaleY);

			if (xessResult != XESS_RESULT_SUCCESS)
			{
				spdlog::error("FeatureContext::XeSSExecuteDx11 xessSetVelocityScale error: {0}", ResultToString(xessResult));
				return false;
			}
		}
		else
			spdlog::warn("FeatureContext::XeSSExecuteDx12 Can't get motion vector scales!");

		// Execute xess
		spdlog::debug("FeatureContext::XeSSExecuteDx11 Executing!!");
		xessResult = xessD3D12Execute(xessContext, commandList, &params);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 xessD3D12Execute error: {0}", ResultToString(xessResult));
			return false;
		}

		//apply cas
		if (casActive && !CasDispatch(commandList, initParams, casBuffer, dx11OutputBuffer))
			return false;

		// Execute dx12 commands to process xess
		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, ppCommandLists);

		// xess done
		commandQueue->Signal(d3d12Fence, 20);

		// wait for end of copy
		if (d3d12Fence->GetCompletedValue() < 20)
		{
			auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent12)
			{
				d3d12Fence->SetEventOnCompletion(20, fenceEvent12);
				WaitForSingleObject(fenceEvent12, INFINITE);
				CloseHandle(fenceEvent12);
			}
		}

		d3d12Fence->Release();

		//copy output back

		ID3D11Query* query2 = nullptr;
		result = dx11device->CreateQuery(&pQueryDesc, &query2);

		if (result != S_OK || query2 == nullptr || query2 == NULL)
		{
			spdlog::error("FeatureContext::XeSSExecuteDx11 can't create query2!");
			return false;
		}

		// Associate the query with the copy operation
		deviceContext->Begin(query2);
		deviceContext->CopyResource(paramOutput, dx11Out.SharedTexture);
		// Execute dx11 commands 
		deviceContext->End(query2);
		instance->Dx11DeviceContext->Flush();

		// Wait for the query to be ready
		while (instance->Dx11DeviceContext->GetData(query2, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
			//
		}

		// Release the query
		query2->Release();

		if (paramColor)
			params.pColorTexture->Release();

		if (paramMv)
			params.pVelocityTexture->Release();

		if (paramDepth)
			params.pDepthTexture->Release();

		if (!instance->MyConfig->AutoExposure.value_or(false) && paramExposure)
			params.pExposureScaleTexture->Release();

		if (!instance->MyConfig->DisableReactiveMask.value_or(false) && paramMask)
			params.pResponsivePixelMaskTexture->Release();

		if (paramOutput)
		{
			if (casActive)
				dx11OutputBuffer->Release();
			else
				params.pOutputTexture->Release();
		}

		return true;
	}
#pragma endregion 



};
