#pragma once
#include "pch.h"
#include "NvParameter.h"
#include "xess_d3d12.h"
#include "xess_debug.h"
#include "WrappedD3D12Device.h"
#include "d3d11on12.h"
#include "d3dx12.h"

class FeatureContext;

//Global Context
class CyberXessContext
{
	CyberXessContext();

	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL featureLevel, bool requestHighPerformanceAdapter) const
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
					break;
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
					break;
			}
		}

		*ppAdapter = adapter;
	}


public:
	std::shared_ptr<Config> MyConfig;

	bool init = false;
	const NvParameter* CreateFeatureParams;

	// D3D12 stuff
	ID3D12Device* Dx12Device = nullptr;
	WrappedD3D12Device* Dx12ProxyDevice = nullptr;

	// D3D11 stuff
	ID3D11Device5* Dx11Device = nullptr;
	ID3D11DeviceContext4* Dx11DeviceContext = nullptr;

	// D3D11on12 stuff
	ID3D11On12Device2* Dx11on12Device = nullptr;
	ID3D12CommandQueue* Dx12CommandQueue = nullptr;
	ID3D12CommandAllocator* Dx12CommandAllocator[2] = { nullptr, nullptr };
	ID3D12GraphicsCommandList* Dx12CommandList[2] = { nullptr, nullptr };
	ID3D12Fence* Dx12Fence = nullptr;
	volatile UINT64 Dx12FenceValueCounter = 0;

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

	void Shutdown() const
	{
		CyberXessContext::instance()->Dx12FenceValueCounter = 0;

		if (CyberXessContext::instance()->Dx12Fence != nullptr)
		{
			CyberXessContext::instance()->Dx12Fence->Release();
			CyberXessContext::instance()->Dx12Fence = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandList[0] != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandList[0]->Release();
			CyberXessContext::instance()->Dx12CommandList[0] = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandList[1] != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandList[1]->Release();
			CyberXessContext::instance()->Dx12CommandList[1] = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandQueue != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandQueue->Release();
			CyberXessContext::instance()->Dx12CommandQueue = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandAllocator[0] != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandAllocator[0]->Release();
			CyberXessContext::instance()->Dx12CommandAllocator[0] = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandAllocator[1] != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandAllocator[1]->Release();
			CyberXessContext::instance()->Dx12CommandAllocator[1] = nullptr;
		}

		if (CyberXessContext::instance()->Dx12ProxyDevice != nullptr)
		{
			CyberXessContext::instance()->Dx12ProxyDevice->Release();
			CyberXessContext::instance()->Dx12ProxyDevice = nullptr;
		}

		if (CyberXessContext::instance()->Dx12Device != nullptr)
		{
			CyberXessContext::instance()->Dx12Device->Release();
			CyberXessContext::instance()->Dx12Device = nullptr;
		}

		if (CyberXessContext::instance()->Dx11on12Device != nullptr)
		{
			CyberXessContext::instance()->Dx11on12Device->Release();
			CyberXessContext::instance()->Dx11on12Device = nullptr;
		}

		if (CyberXessContext::instance()->Dx11Device != nullptr)
		{
			CyberXessContext::instance()->Dx11Device->Release();
			CyberXessContext::instance()->Dx11Device = nullptr;
		}

		if (CyberXessContext::instance()->Dx11DeviceContext != nullptr)
		{
			CyberXessContext::instance()->Dx11DeviceContext->Release();
			CyberXessContext::instance()->Dx11DeviceContext = nullptr;
		}

		if (CyberXessContext::instance()->VulkanInstance != nullptr)
			CyberXessContext::instance()->VulkanInstance = nullptr;

		if (CyberXessContext::instance()->VulkanDevice != nullptr)
			CyberXessContext::instance()->VulkanDevice = nullptr;

		if (CyberXessContext::instance()->VulkanPhysicalDevice != nullptr)
			CyberXessContext::instance()->VulkanPhysicalDevice = nullptr;
	}

	HRESULT CreateDx12Device(D3D_FEATURE_LEVEL featureLevel)
	{
		HRESULT result;

		IDXGIFactory4* factory;
		result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

		if (result != S_OK)
		{
			LOG("CreateDx12Device Can't create factory: " + int_to_hex(result));
			return result;
		}

		IDXGIAdapter1* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter, featureLevel, true);

		if (hardwareAdapter == nullptr)
		{
			LOG("CreateDx12Device Can't get hardwareAdapter!");
			return E_NOINTERFACE;
		}

		result = D3D12CreateDevice(hardwareAdapter, featureLevel, IID_PPV_ARGS(&Dx12Device));

		if (result != S_OK)
		{
			LOG("CreateDx12Device Can't create device: " + int_to_hex(result));
			return result;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

		// CreateCommandQueue
		result = Dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandQueue result: " + int_to_hex(result));

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

	unsigned int Width{}, Height{}, RenderWidth{}, RenderHeight{};
	NVSDK_NGX_PerfQuality_Value PerfQualityValue = NVSDK_NGX_PerfQuality_Value_Balanced;
	float Sharpness = 1.0f;
	float MVScaleX{}, MVScaleY{};
	float JitterOffsetX{}, JitterOffsetY{};
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