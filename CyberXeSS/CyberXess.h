#pragma once
#define FFX_CAS

#include "pch.h"
#include "NvParameter.h"
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
	LOG("FeatureContext::LogCallback XeSS Runtime (" + std::to_string(Level) + ") : " + s, spdlog::level::info);
}

class FeatureContext;

//Global Context
class CyberXessContext
{
	unsigned int handleCounter = 1000;

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

	const NvParameter* CreateFeatureParams;

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

	std::shared_ptr<NvParameter> NvParameterInstance = NvParameter::instance();

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
			LOG("CyberXessContext::CreateDx12Device Can't create factory: " + int_to_hex(result), spdlog::level::err);
			return result;
		}

		IDXGIAdapter* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter, featureLevel, true);

		if (hardwareAdapter == nullptr)
		{
			LOG("CyberXessContext::CreateDx12Device Can't get hardwareAdapter!", spdlog::level::err);
			return E_NOINTERFACE;
		}

		result = D3D12CreateDevice(hardwareAdapter, featureLevel, IID_PPV_ARGS(&Dx12Device));

		if (result != S_OK)
		{
			LOG("CyberXessContext::CreateDx12Device Can't create device: " + int_to_hex(result), spdlog::level::err);
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
		LOG("CyberXessContext::CreateDx12Device CreateCommandQueue result: " + int_to_hex(result), spdlog::level::debug);

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
	bool xessInit = false;

	// cas
	FfxCasContext casContext;
	bool casInit = false;
	bool casContextCreated = false;
	bool casUpscale = false;
	bool casActive = false;
	float casSharpness = 0.4f;
	ID3D12Resource* casBuffer = nullptr;

	// dx11
	D3D11_TEXTURE2D_RESOURCE_C dx11Color = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Mv = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Depth = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Tm = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Exp = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Out = {};
	ID3D12Resource* dx11OutputBuffer;


#pragma region cas methods

	void CasInit()
	{
		LOG("FeatureContext::CasInit Start!", spdlog::level::debug);

		if (casInit)
			return;

		casActive = CyberXessContext::instance()->MyConfig->CasEnabled.value_or(false);
		casSharpness = CyberXessContext::instance()->MyConfig->CasSharpness.value_or(0.4);

		if (casSharpness > 1 || casSharpness < 0)
			casSharpness = 0.4f;

		casInit = true;
	}

	bool CreateCasContext()
	{
		LOG("FeatureContext::CreateCasContext Start!", spdlog::level::debug);

		if (!casInit)
			return false;

		if (!casActive)
			return true;

		DestroyCasContext();

		const size_t scratchBufferSize = ffxGetScratchMemorySizeDX12(FFX_CAS_CONTEXT_COUNT);
		void* scratchBuffer = calloc(scratchBufferSize, 1);

		FfxCasContextDescription casContextDesc = {};
		auto errorCode = ffxGetInterfaceDX12(&casContextDesc.backendInterface, dx12device, scratchBuffer, scratchBufferSize, FFX_CAS_CONTEXT_COUNT);

		if (errorCode != FFX_OK)
		{
			LOG("FeatureContext::CreateCasContext ffxGetInterfaceDX12 error: " + int_to_hex(errorCode), spdlog::level::err);
			return false;
		}

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
			LOG("FeatureContext::CreateCasContext ffxCasContextCreate error: " + int_to_hex(errorCode), spdlog::level::err);
			return false;
		}

		casContextCreated = true;
		return true;
	}

	void DestroyCasContext()
	{
		LOG("FeatureContext::DestroyCasContext Start!", spdlog::level::debug);

		if (!casActive || !casContextCreated)
			return;

		auto errorCode = ffxCasContextDestroy(&casContext);

		if (errorCode != FFX_OK)
			LOG("FeatureContext::DestroyCasContext ffxCasContextDestroy error: " + int_to_hex(errorCode), spdlog::level::err);

		casContextCreated = false;
	}

	bool CreateCasBufferResource(ID3D12Resource* source)
	{
		LOG("FeatureContext::CreateCasBufferResource Start!", spdlog::level::debug);

		if (!casInit)
			return false;

		if (!casActive)
			return true;

		if (source == nullptr)
			return false;

		D3D12_RESOURCE_DESC texDesc = source->GetDesc();

		D3D12_HEAP_PROPERTIES heapProperties;
		D3D12_HEAP_FLAGS heapFlags;
		HRESULT hr = source->GetHeapProperties(&heapProperties, &heapFlags);

		if (hr != S_OK)
		{
			LOG("FeatureContext::CreateBufferResource GetHeapProperties result: " + int_to_hex(hr), spdlog::level::err);
			return false;
		}

		if (casBuffer != nullptr)
			casBuffer->Release();

		hr = dx12device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&casBuffer));

		if (hr != S_OK)
		{
			LOG("FeatureContext::CreateBufferResource CreateCommittedResource result: " + int_to_hex(hr), spdlog::level::err);
			return false;
		}

		return true;
	}

	bool CasDispatch(ID3D12CommandList* commandList, ID3D12Resource* input, ID3D12Resource* output)
	{
		LOG("FeatureContext::CasDispatch Start!", spdlog::level::debug);

		if (!casInit)
			return false;

		if (!casActive)
			return true;

		FfxCasDispatchDescription dispatchParameters = {};
		dispatchParameters.commandList = ffxGetCommandListDX12(commandList);
		dispatchParameters.renderSize = { DisplayWidth, DisplayHeight };
		dispatchParameters.sharpness = casSharpness;

		dispatchParameters.color = ffxGetResourceDX12(input, GetFfxResourceDescriptionDX12(input), nullptr, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
		dispatchParameters.output = ffxGetResourceDX12(output, GetFfxResourceDescriptionDX12(output), nullptr, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);


		if (auto errorCode = ffxCasContextDispatch(&casContext, &dispatchParameters); errorCode != FFX_OK)
		{
			LOG("FeatureContext::CasDispatch ffxCasContextDispatch error: " + int_to_hex(errorCode), spdlog::level::err);
			return false;
		}

		return true;
	}

#pragma endregion

	bool CopyTextureFrom11To12(ID3D11Resource* d3d11texture, ID3D11Texture2D** pSharedTexture, D3D11_TEXTURE2D_DESC_C* sharedDesc, bool copy = false)
	{
		ID3D11Texture2D* originalTexture = nullptr;
		D3D11_TEXTURE2D_DESC desc{};

		auto result = d3d11texture->QueryInterface(IID_PPV_ARGS(&originalTexture));

		if (result != S_OK)
			return false;

		originalTexture->GetDesc(&desc);

		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 || copy)
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
					LOG("FeatureContext::CopyTextureFrom11To12 QueryInterface(resource) error: " + int_to_hex(result), spdlog::level::err);
					return false;
				}

				// Get shared handle
				result = resource->GetSharedHandle(&sharedDesc->handle);

				if (result != S_OK)
				{
					LOG("FeatureContext::CopyTextureFrom11To12 GetSharedHandle error: " + int_to_hex(result), spdlog::level::err);
					return false;
				}

				resource->Release();
				sharedDesc->pointer = (*pSharedTexture);
			}

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
					LOG("FeatureContext::CopyTextureFrom11To12 QueryInterface(resource) error: " + int_to_hex(result), spdlog::level::err);
					return false;
				}

				// Get shared handle
				result = resource->GetSharedHandle(&sharedDesc->handle);

				if (result != S_OK)
				{
					LOG("FeatureContext::CopyTextureFrom11To12 GetSharedHandle error: " + int_to_hex(result), spdlog::level::err);
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
		LOG("FeatureContext::ReleaseSharedResources start!", spdlog::level::debug);

		SAFE_RELEASE(casBuffer);
		SAFE_RELEASE(dx11OutputBuffer);
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

	bool XeSSInit(ID3D12Device* device, const NvParameter* initParams)
	{
		if (device == nullptr)
		{
			LOG("FeatureContext::XeSSInit D3D12Device is null!", spdlog::level::err);
			return false;
		}

		if (xessInit)
			return true;

		dx12device = device;

		RenderWidth = initParams->OutWidth;
		RenderHeight = initParams->OutHeight;
		DisplayWidth = initParams->OutWidth;
		DisplayHeight = initParams->OutHeight;

		xess_version_t ver;
		xess_result_t ret = xessGetVersion(&ver);

		if (ret == XESS_RESULT_SUCCESS)
			LOG("FeatureContext::XeSSInit XeSS Version: " + std::to_string(ver.major) + "." + std::to_string(ver.minor) + std::to_string(ver.patch), spdlog::level::info);
		else
			LOG("FeatureContext::XeSSInit xessGetVersion error: " + ResultToString(ret), spdlog::level::warn);

		ret = xessD3D12CreateContext(dx12device, &xessContext);

		if (ret != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSInit xessD3D12CreateContext error: " + ResultToString(ret), spdlog::level::err);
			return false;
		}

		ret = xessIsOptimalDriver(xessContext);
		LOG("FeatureContext::XeSSInit xessIsOptimalDriver : " + ResultToString(ret), spdlog::level::debug);

		ret = xessSetLoggingCallback(xessContext, XESS_LOGGING_LEVEL_DEBUG, logCallback);
		LOG("FeatureContext::XeSSInit xessSetLoggingCallback : " + ResultToString(ret), spdlog::level::debug);

		ret = xessSetVelocityScale(xessContext, initParams->MVScaleX, initParams->MVScaleY);
		LOG("FeatureContext::XeSSInit xessSetVelocityScale : " + ResultToString(ret), spdlog::level::debug);

#pragma region Create Parameters for XeSS

		xess_d3d12_init_params_t xessParams{};

		xessParams.outputResolution.x = initParams->OutWidth;
		xessParams.outputResolution.y = initParams->OutHeight;

		switch (initParams->PerfQualityValue)
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
		default:
			xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing XESS_QUALITY_SETTING_BALANCED mode
			break;
		}

		xessParams.initFlags = XESS_INIT_FLAG_NONE;

		if (CyberXessContext::instance()->MyConfig->DepthInverted.value_or(initParams->DepthInverted))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
			CyberXessContext::instance()->MyConfig->DepthInverted = true;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (DepthInverted) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

		if (CyberXessContext::instance()->MyConfig->AutoExposure.value_or(initParams->AutoExposure))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
			CyberXessContext::instance()->MyConfig->AutoExposure = true;
			LOG("FeatureContext::XeSSInit initParams.initFlags (AutoExposure) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}
		else
		{
			xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (!AutoExposure) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

		if (!CyberXessContext::instance()->MyConfig->HDR.value_or(!initParams->Hdr))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
			CyberXessContext::instance()->MyConfig->HDR = false;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (HDR) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

		if (CyberXessContext::instance()->MyConfig->JitterCancellation.value_or(initParams->JitterMotion))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
			CyberXessContext::instance()->MyConfig->JitterCancellation = true;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (JitterCancellation) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

		if (CyberXessContext::instance()->MyConfig->DisplayResolution.value_or(!initParams->LowRes))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
			CyberXessContext::instance()->MyConfig->DisplayResolution = true;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (LowRes) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

		if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(true))
		{
			xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
			LOG("FeatureContext::XeSSInit xessParams.initFlags (DisableReactiveMask) " + std::to_string(xessParams.initFlags), spdlog::level::info);
		}

#pragma endregion

#pragma region Build Pipelines

		if (CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true))
		{
			LOG("FeatureContext::XeSSInit xessD3D12BuildPipelines!", spdlog::level::debug);

			ret = xessD3D12BuildPipelines(xessContext, NULL, false, xessParams.initFlags);

			if (ret != XESS_RESULT_SUCCESS)
			{
				LOG("FeatureContext::XeSSInit xessD3D12BuildPipelines error: " + ResultToString(ret), spdlog::level::err);
				return false;
			}
		}

#pragma endregion

		LOG("FeatureContext::XeSSInit xessD3D12Init!", spdlog::level::debug);

		ret = xessD3D12Init(xessContext, &xessParams);

		if (ret != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSInit xessD3D12Init error: " + ResultToString(ret), spdlog::level::err);
			return false;
		}

		CasInit();
		CreateCasContext();

		xessInit = true;

		return true;
	}

	void XeSSDestroy()
	{
		if (!xessInit || xessContext == nullptr)
			return;

		auto result = xessDestroyContext(xessContext);

		if (result != XESS_RESULT_SUCCESS)
			LOG("FeatureContext::XeSSDestroy xessDestroyContext error: " + ResultToString(result), spdlog::level::err);

		xessContext = nullptr;

		DestroyCasContext();
		ReleaseSharedResources();
	}

	bool XeSSIsInited() const
	{
		return xessInit;
	}

	bool XeSSExecuteDx12(ID3D12GraphicsCommandList* commandList, const NvParameter* initParams)
	{
		if (!xessInit)
			return false;

		const auto instance = CyberXessContext::instance();

		xess_result_t xessResult;
		xess_d3d12_execute_params_t params{};

		params.jitterOffsetX = initParams->JitterOffsetX;
		params.jitterOffsetY = initParams->JitterOffsetY;

		params.exposureScale = initParams->ExposureScale;
		params.resetHistory = initParams->ResetRender;

		params.inputWidth = initParams->Width;
		params.inputHeight = initParams->Height;

		if (initParams->Color != nullptr)
		{
			LOG("FeatureContext::XeSSExecuteDx12 Color exist..", spdlog::level::debug);
			params.pColorTexture = (ID3D12Resource*)initParams->Color;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx12 Color not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->MotionVectors)
		{
			LOG("FeatureContext::XeSSExecuteDx12 MotionVectors exist..", spdlog::level::debug);
			params.pVelocityTexture = (ID3D12Resource*)initParams->MotionVectors;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx12 MotionVectors not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->Output)
		{
			LOG("FeatureContext::XeSSExecuteDx12 Output exist..", spdlog::level::debug);

			if (casActive)
			{
				if (casBuffer == nullptr)
				{
					if (!CreateCasBufferResource((ID3D12Resource*)initParams->Output))
					{
						LOG("FeatureContext::XeSSExecuteDx12 Can't create cas buffer!", spdlog::level::err);
						return false;
					}
				}

				params.pOutputTexture = casBuffer;
			}
			else
				params.pOutputTexture = (ID3D12Resource*)initParams->Output;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx12 Output not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->Depth)
		{
			LOG("FeatureContext::XeSSExecuteDx12 Depth exist..", spdlog::level::info);
			params.pDepthTexture = (ID3D12Resource*)initParams->Depth;
		}
		else
		{
			if (!instance->MyConfig->DisplayResolution.value_or(false))
				LOG("FeatureContext::XeSSExecuteDx12 Depth not exist!!", spdlog::level::err);
			else
				LOG("FeatureContext::XeSSExecuteDx12 Using high res motion vectors, depth is not needed!!", spdlog::level::info);

			params.pDepthTexture = nullptr;
		}

		if (!instance->MyConfig->AutoExposure.value_or(false))
		{
			if (initParams->ExposureTexture != nullptr)
			{
				LOG("FeatureContext::XeSSExecuteDx12 ExposureTexture exist..", spdlog::level::info);
				params.pExposureScaleTexture = (ID3D12Resource*)initParams->ExposureTexture;
			}
			else
				LOG("FeatureContext::XeSSExecuteDx12 AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!", spdlog::level::warn);
		}
		else
			LOG("FeatureContext::XeSSExecuteDx12 AutoExposure enabled!", spdlog::level::warn);

		if (!instance->MyConfig->DisableReactiveMask.value_or(true))
		{
			if (initParams->TransparencyMask != nullptr)
			{
				LOG("FeatureContext::XeSSExecuteDx12 TransparencyMask exist..", spdlog::level::info);
				params.pResponsivePixelMaskTexture = (ID3D12Resource*)initParams->TransparencyMask;
			}
			else
				LOG("FeatureContext::XeSSExecuteDx12 TransparencyMask not exist and its enabled in config, it may cause problems!!", spdlog::level::warn);
		}

		xessResult = xessSetVelocityScale(xessContext, initParams->MVScaleX, initParams->MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSExecuteDx12 xessSetVelocityScale : " + ResultToString(xessResult), spdlog::level::err);
			return false;
		}

		LOG("FeatureContext::XeSSExecuteDx12 Executing!!", spdlog::level::info);
		xessResult = xessD3D12Execute(xessContext, commandList, &params);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSExecuteDx12 xessD3D12Execute error: " + ResultToString(xessResult), spdlog::level::err);
			return false;
		}

		//apply cas
		if (!CasDispatch(commandList, casBuffer, (ID3D12Resource*)initParams->Output))
			return false;

		return true;
	}

	bool XeSSExecuteDx11(ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue,
		ID3D11Device* dx11device, ID3D11DeviceContext* deviceContext,
		const NvParameter* initParams)
	{
		if (!xessInit)
			return false;

		const auto instance = CyberXessContext::instance();

		ID3D12Fence* d3d12Fence;
		// Fence for syncing

		auto result = dx12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));

		if (result != S_OK)
		{
			LOG("FeatureContext::XeSSExecuteDx11 CreateFence d3d12fence error: " + int_to_hex(result), spdlog::level::debug);
			return false;
		}

		// creatimg params for XeSS
		xess_result_t xessResult;
		xess_d3d12_execute_params_t params{};

		params.jitterOffsetX = initParams->JitterOffsetX;
		params.jitterOffsetY = initParams->JitterOffsetY;

		params.exposureScale = initParams->ExposureScale;
		params.resetHistory = initParams->ResetRender;

		params.inputWidth = initParams->Width;
		params.inputHeight = initParams->Height;

#pragma region Texture copies

		D3D11_QUERY_DESC pQueryDesc;
		pQueryDesc.Query = D3D11_QUERY_EVENT;
		pQueryDesc.MiscFlags = 0;
		ID3D11Query* query1 = nullptr;
		result = dx11device->CreateQuery(&pQueryDesc, &query1);

		if (result != S_OK || query1 == nullptr || query1 == NULL)
		{
			LOG("FeatureContext::XeSSExecuteDx11 can't create query1!", spdlog::level::err);
			return false;
		}

		// Associate the query with the copy operation
		deviceContext->Begin(query1);

		if (initParams->Color != nullptr)
		{
			LOG("FeatureContext::XeSSExecuteDx11 Color exist..", spdlog::level::debug);
			if (CopyTextureFrom11To12((ID3D11Resource*)initParams->Color, &dx11Color.SharedTexture, &dx11Color.Desc) == NULL)
				return false;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx11 Color not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->MotionVectors)
		{
			LOG("FeatureContext::XeSSExecuteDx11 MotionVectors exist..", spdlog::level::debug);
			if (CopyTextureFrom11To12((ID3D11Resource*)initParams->MotionVectors, &dx11Mv.SharedTexture, &dx11Mv.Desc) == false)
				return false;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx11 MotionVectors not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->Output)
		{
			LOG("FeatureContext::XeSSExecuteDx11 Output exist..", spdlog::level::debug);
			if (CopyTextureFrom11To12((ID3D11Resource*)initParams->Output, &dx11Out.SharedTexture, &dx11Out.Desc, true) == false)
				return false;
		}
		else
		{
			LOG("FeatureContext::XeSSExecuteDx11 Output not exist!!", spdlog::level::err);
			return false;
		}

		if (initParams->Depth)
		{
			LOG("FeatureContext::XeSSExecuteDx11 Depth exist..", spdlog::level::debug);
			if (CopyTextureFrom11To12((ID3D11Resource*)initParams->Depth, &dx11Depth.SharedTexture, &dx11Depth.Desc) == false)
				return false;
		}
		else
			LOG("FeatureContext::XeSSExecuteDx11 Depth not exist!!", spdlog::level::err);

		if (!instance->MyConfig->AutoExposure.value_or(false))
		{
			if (initParams->ExposureTexture == nullptr)
				LOG("FeatureContext::XeSSExecuteDx11 AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!", spdlog::level::warn);
			else
			{
				LOG("FeatureContext::XeSSExecuteDx11 ExposureTexture exist..", spdlog::level::debug);
				if (CopyTextureFrom11To12((ID3D11Resource*)initParams->ExposureTexture, &dx11Exp.SharedTexture, &dx11Exp.Desc) == false)
					return false;
			}
		}
		else
			LOG("FeatureContext::XeSSExecuteDx11 AutoExposure enabled!", spdlog::level::warn);

		if (!instance->MyConfig->DisableReactiveMask.value_or(true))
		{
			if (initParams->TransparencyMask != nullptr)
			{
				LOG("FeatureContext::XeSSExecuteDx11 TransparencyMask exist..", spdlog::level::info);
				if (CopyTextureFrom11To12((ID3D11Resource*)initParams->TransparencyMask, &dx11Tm.SharedTexture, &dx11Tm.Desc) == false)
					return false;
			}
			else
				LOG("FeatureContext::XeSSExecuteDx11 TransparencyMask not exist and its enabled in config, it may cause problems!!", spdlog::level::warn);
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

		if (initParams->Color)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Color.Desc.handle, IID_PPV_ARGS(&params.pColorTexture));

			if (result != S_OK)
			{
				LOG("FeatureContext::XeSSExecuteDx11 Color OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
				return false;
			}
		}

		if (initParams->MotionVectors)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Mv.Desc.handle, IID_PPV_ARGS(&params.pVelocityTexture));

			if (result != S_OK)
			{
				LOG("FeatureContext::XeSSExecuteDx11 MotionVectors OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
				return false;
			}
		}

		if (initParams->Output)
		{
			if (casActive)
			{
				result = instance->Dx12Device->OpenSharedHandle(dx11Out.Desc.handle, IID_PPV_ARGS(&dx11OutputBuffer));
				dx11Out.SharedHandle = dx11Out.Desc.handle;

				if (result != S_OK)
				{
					LOG("FeatureContext::XeSSExecuteDx11 Output OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
					return false;
				}

				if (casBuffer == nullptr)
				{
					if (!CreateCasBufferResource(dx11OutputBuffer))
					{
						LOG("FeatureContext::XeSSExecuteDx11 Can't create cas buffer!", spdlog::level::err);
						return false;
					}
				}

				params.pOutputTexture = casBuffer;
			}
			else
			{
				result = instance->Dx12Device->OpenSharedHandle(dx11Out.Desc.handle, IID_PPV_ARGS(&params.pOutputTexture));
				dx11Out.SharedHandle = dx11Out.Desc.handle;

				if (result != S_OK)
				{
					LOG("FeatureContext::XeSSExecuteDx11 Output OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
					return false;
				}
			}
		}

		if (initParams->Depth)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Depth.Desc.handle, IID_PPV_ARGS(&params.pDepthTexture));

			if (result != S_OK)
			{
				LOG("FeatureContext::XeSSExecuteDx11 Depth OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
				return false;
			}
		}

		if (!instance->MyConfig->AutoExposure.value_or(false) && initParams->ExposureTexture != nullptr)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Exp.Desc.handle, IID_PPV_ARGS(&params.pExposureScaleTexture));

			if (result != S_OK)
			{
				LOG("FeatureContext::XeSSExecuteDx11 ExposureTexture OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
				return false;
			}
		}

		if (!instance->MyConfig->DisableReactiveMask.value_or(true) && initParams->TransparencyMask != nullptr)
		{
			result = instance->Dx12Device->OpenSharedHandle(dx11Tm.Desc.handle, IID_PPV_ARGS(&params.pResponsivePixelMaskTexture));

			if (result != S_OK)
			{
				LOG("FeatureContext::XeSSExecuteDx11 TransparencyMask OpenSharedHandle error: " + int_to_hex(result), spdlog::level::err);
				return false;
			}
		}

#pragma endregion

		xessResult = xessSetVelocityScale(xessContext, initParams->MVScaleX, initParams->MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSExecuteDx11 xessSetVelocityScale error: " + ResultToString(xessResult), spdlog::level::err);
			return false;
		}

		// Execute xess
		LOG("FeatureContext::XeSSExecuteDx11 Executing!!", spdlog::level::info);
		xessResult = xessD3D12Execute(xessContext, commandList, &params);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			LOG("FeatureContext::XeSSExecuteDx11 xessD3D12Execute error: " + ResultToString(xessResult), spdlog::level::info);
			return false;
		}

		//apply cas
		if (!CasDispatch(commandList, casBuffer, dx11OutputBuffer))
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
			d3d12Fence->SetEventOnCompletion(20, fenceEvent12);
			WaitForSingleObject(fenceEvent12, INFINITE);
			CloseHandle(fenceEvent12);
		}

		d3d12Fence->Release();

		//copy output back

		ID3D11Query* query2 = nullptr;
		result = dx11device->CreateQuery(&pQueryDesc, &query2);

		if (result != S_OK || query2 == nullptr || query2 == NULL)
		{
			LOG("FeatureContext::XeSSExecuteDx11 can't create query2!", spdlog::level::err);
			return false;
		}

		// Associate the query with the copy operation
		deviceContext->Begin(query2);
		deviceContext->CopyResource((ID3D11Resource*)initParams->Output, dx11Out.SharedTexture);
		// Execute dx11 commands 
		deviceContext->End(query2);
		instance->Dx11DeviceContext->Flush();

		// Wait for the query to be ready
		while (instance->Dx11DeviceContext->GetData(query2, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
			//
		}

		// Release the query
		query2->Release();

		return true;
	}
#pragma endregion 



};
