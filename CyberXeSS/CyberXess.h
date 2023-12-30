#pragma once
#include "pch.h"
#include "NvParameter.h"
#include "xess_d3d12.h"
#include "xess_debug.h"
#include "xess_d3d12_debug.h"
#include "dxgi1_6.h"

static unsigned int handleCounter = 1000;

class FeatureContext;

//Global Context
class CyberXessContext
{
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

	bool init = false;
	const NvParameter* CreateFeatureParams;

	// D3D12 stuff
	ID3D12Device* Dx12Device = nullptr;

	// D3D11 stuff
	ID3D11Device5* Dx11Device = nullptr;
	ID3D11DeviceContext4* Dx11DeviceContext = nullptr;

	// D3D11on12 stuff
	ID3D12CommandQueue* Dx12CommandQueue = nullptr;
	ID3D12CommandAllocator* Dx12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* Dx12CommandList = nullptr;

	// Vulkan stuff
	ID3D11DeviceContext* Dx11DeviceContext = nullptr;
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
			LOG("CreateDx12Device Can't create factory: " + int_to_hex(result), LEVEL_ERROR);
			return result;
		}

		IDXGIAdapter* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter, featureLevel, true);

		if (hardwareAdapter == nullptr)
		{
			LOG("CreateDx12Device Can't get hardwareAdapter!", LEVEL_ERROR);
			return E_NOINTERFACE;
		}

		result = D3D12CreateDevice(hardwareAdapter, featureLevel, IID_PPV_ARGS(&Dx12Device));

		if (result != S_OK)
		{
			LOG("CreateDx12Device Can't create device: " + int_to_hex(result), LEVEL_ERROR);
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
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandQueue result: " + int_to_hex(result), LEVEL_DEBUG);

		if (result != S_OK || Dx12CommandQueue == nullptr)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		return S_OK;
	}
};

class FeatureContext
{
public:
	NVSDK_NGX_Handle Handle;
	xess_context_handle_t XessContext = nullptr;

	unsigned int Width{};
	unsigned int Height{};
	unsigned int RenderWidth{};
	unsigned int RenderHeight{};
	NVSDK_NGX_PerfQuality_Value PerfQualityValue = NVSDK_NGX_PerfQuality_Value_Balanced;
	float Sharpness = 1.0f;
	float MVScaleX{};
	float MVScaleY{};
	float JitterOffsetX{};
	float JitterOffsetY{};
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

