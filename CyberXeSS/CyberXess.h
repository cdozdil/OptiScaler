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

public:
	std::shared_ptr<Config> MyConfig;

	bool init = false;
	const NvParameter* CreateFeatureParams;
	ID3D12Device* Dx12Device = nullptr;	
	WrappedD3D12Device* Dx12ProxyDevice = nullptr;	
	ID3D11Device* Dx11Device = nullptr;
	ID3D11DeviceContext* Dx11DeviceContext = nullptr;
	VkDevice VulkanDevice = nullptr;
	VkInstance VulkanInstance = nullptr;
	VkPhysicalDevice VulkanPhysicalDevice = nullptr;

	// D3D11on12 stuff
	ID3D11On12Device2* Dx11on12Device = nullptr;
	ID3D12CommandAllocator* Dx12CommandAllocator = nullptr;
	ID3D12CommandQueue* Dx12CommandQueue = nullptr;
	ID3D12GraphicsCommandList* Dx12CommandList = nullptr;
	ID3D12Fence* Dx12Fence = nullptr;
	UINT64 Dx12FenceValueCounter = 0;

	std::shared_ptr<NvParameter> NvParameterInstance = NvParameter::instance();

	ankerl::unordered_dense::map <unsigned int, std::unique_ptr<FeatureContext>> Contexts;
	FeatureContext* CreateContext();
	void DeleteContext(NVSDK_NGX_Handle* handle);

	static std::shared_ptr<CyberXessContext> instance()
	{
		static std::shared_ptr<CyberXessContext> INSTANCE{ std::make_shared<CyberXessContext>(CyberXessContext()) };
		return INSTANCE;
	}

	void Shutdown()
	{
		CyberXessContext::instance()->Dx12FenceValueCounter = 0;

		if (CyberXessContext::instance()->Dx12Fence != nullptr)
		{
			CyberXessContext::instance()->Dx12Fence->Release();
			CyberXessContext::instance()->Dx12Fence = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandList != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandList->Release();
			CyberXessContext::instance()->Dx12CommandList = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandQueue != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandQueue->Release();
			CyberXessContext::instance()->Dx12CommandQueue = nullptr;
		}

		if (CyberXessContext::instance()->Dx12CommandAllocator != nullptr)
		{
			CyberXessContext::instance()->Dx12CommandAllocator->Release();
			CyberXessContext::instance()->Dx12CommandAllocator = nullptr;
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
