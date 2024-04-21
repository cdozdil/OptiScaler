#include "IFeature_Dx11wDx12.h"
#include "../Config.h"

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; dest.Height = src.Height; dest.Format = src.Format; dest.BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0)	

void IFeature_Dx11wDx12::ResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	barrier.Transition.Subresource = 0;
	commandList->ResourceBarrier(1, &barrier);
}

bool IFeature_Dx11wDx12::CopyTextureFrom11To12(ID3D11Resource* InResource, D3D11_TEXTURE2D_RESOURCE_C* OutResource, bool InCopy, bool InDepth)
{
	ID3D11Texture2D* originalTexture = nullptr;
	D3D11_TEXTURE2D_DESC desc{};

	auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

	if (result != S_OK)
		return false;

	originalTexture->GetDesc(&desc);

	// check shared nt handle usage later
	if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 && (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_NTHANDLE) == 0 && !InDepth)
	{
		if (desc.Width != OutResource->Desc.Width || desc.Height != OutResource->Desc.Height ||
			desc.Format != OutResource->Desc.Format || desc.BindFlags != OutResource->Desc.BindFlags ||
			OutResource->SharedTexture == nullptr)
		{
			if (OutResource->SharedTexture != nullptr)
			{
				OutResource->SharedTexture->Release();

				if (OutResource->Dx11Handle != NULL)
					CloseHandle(OutResource->Dx11Handle);
			}

			ASSIGN_DESC(OutResource->Desc, desc);
			OutResource->Dx11Handle = NULL;
			OutResource->Dx12Handle = NULL;

			desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
			result = Dx11Device->CreateTexture2D(&desc, nullptr, &OutResource->SharedTexture);

			IDXGIResource1* resource;

			result = OutResource->SharedTexture->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			DWORD access = DXGI_SHARED_RESOURCE_READ;

			if (!InCopy)
				access |= DXGI_SHARED_RESOURCE_WRITE;

			result = resource->CreateSharedHandle(NULL, access, NULL, &OutResource->Dx11Handle);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();
		}

		if (InCopy && OutResource->SharedTexture != nullptr)
			Dx11DeviceContext->CopyResource(OutResource->SharedTexture, InResource);
	}
	else if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 && InDepth)
	{
		if (desc.Width != OutResource->Desc.Width || desc.Height != OutResource->Desc.Height ||
			desc.Format != OutResource->Desc.Format || desc.BindFlags != OutResource->Desc.BindFlags ||
			OutResource->SharedTexture == nullptr)
		{
			if (OutResource->SharedTexture != nullptr)
			{
				OutResource->SharedTexture->Release();

				if (OutResource->Dx11Handle != NULL)
					CloseHandle(OutResource->Dx11Handle);

				if (OutResource->Dx12Handle != NULL)
					CloseHandle(OutResource->Dx12Handle);
			}

			ASSIGN_DESC(OutResource->Desc, desc);
			OutResource->Dx11Handle = NULL;
			OutResource->Dx12Handle = NULL;

			desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
			result = Dx11Device->CreateTexture2D(&desc, nullptr, &OutResource->SharedTexture);

			IDXGIResource1* resource;

			result = OutResource->SharedTexture->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			result = resource->GetSharedHandle(&OutResource->Dx11Handle);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();
		}

		if (InCopy && OutResource->SharedTexture != nullptr)
			Dx11DeviceContext->CopyResource(OutResource->SharedTexture, InResource);
	}
	else
	{
		if (OutResource->SharedTexture != InResource)
		{
			IDXGIResource1* resource;

			result = originalTexture->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) != 0 && (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_NTHANDLE) != 0)
			{
				DWORD access = DXGI_SHARED_RESOURCE_READ;

				if (!InCopy)
					access |= DXGI_SHARED_RESOURCE_WRITE;

				result = resource->CreateSharedHandle(NULL, access, NULL, &OutResource->Dx11Handle);
			}
			else
			{
				result = resource->GetSharedHandle(&OutResource->Dx11Handle);
			}

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();

			OutResource->SharedTexture = (ID3D11Texture2D*)InResource;
		}
	}

	originalTexture->Release();
	return true;
}

void IFeature_Dx11wDx12::ReleaseSharedResources()
{
	SAFE_RELEASE(dx11Color.SharedTexture);
	SAFE_RELEASE(dx11Mv.SharedTexture);
	SAFE_RELEASE(dx11Out.SharedTexture);
	SAFE_RELEASE(dx11Depth.SharedTexture);
	SAFE_RELEASE(dx11Tm.SharedTexture);
	SAFE_RELEASE(dx11Exp.SharedTexture);
	SAFE_RELEASE(dx11Color.Dx12Resource);
	SAFE_RELEASE(dx11Mv.Dx12Resource);
	SAFE_RELEASE(dx11Out.Dx12Resource);
	SAFE_RELEASE(dx11Depth.Dx12Resource);
	SAFE_RELEASE(dx11Tm.Dx12Resource);
	SAFE_RELEASE(dx11Exp.Dx12Resource);

	SAFE_RELEASE(paramOutput);

	SAFE_RELEASE(Dx12CommandList);
	SAFE_RELEASE(Dx12CommandQueue);
	SAFE_RELEASE(Dx12CommandAllocator);
	SAFE_RELEASE(Dx12on11Device);
}

void IFeature_Dx11wDx12::GetHardwareAdapter(IDXGIFactory1* InFactory, IDXGIAdapter** InAdapter, D3D_FEATURE_LEVEL InFeatureLevel, bool InRequestHighPerformanceAdapter)
{
	spdlog::debug("IFeature_Dx11wDx12::GetHardwareAdapter");

	*InAdapter = nullptr;

	IDXGIAdapter1* adapter;

	IDXGIFactory6* factory6;
	if (SUCCEEDED(InFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				InRequestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
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

			auto result = D3D12CreateDevice(adapter, InFeatureLevel, _uuidof(ID3D12Device), nullptr);

			if (result == S_FALSE)
			{
				*InAdapter = adapter;
				break;
			}
		}
	}
	else
	{
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != InFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
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

			auto result = D3D12CreateDevice(adapter, InFeatureLevel, _uuidof(ID3D12Device), nullptr);

			if (result == S_FALSE)
			{
				*InAdapter = adapter;
				break;
			}
		}
	}
}

HRESULT IFeature_Dx11wDx12::CreateDx12Device(D3D_FEATURE_LEVEL InFeatureLevel)
{
	spdlog::debug("IFeature_Dx11wDx12::CreateDx12Device");

	HRESULT result;

	if (Dx12on11Device == nullptr)
	{
		IDXGIFactory4* factory;
		result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device Can't create factory: {0:x}", result);
			return result;
		}

		IDXGIAdapter* hardwareAdapter = nullptr;
		GetHardwareAdapter(factory, &hardwareAdapter, InFeatureLevel, true);

		if (hardwareAdapter == nullptr)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device Can't get hardwareAdapter!");
			return E_NOINTERFACE;
		}

		result = D3D12CreateDevice(hardwareAdapter, InFeatureLevel, IID_PPV_ARGS(&Dx12on11Device));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device Can't create device: {0:x}", result);
			return result;
		}
	}

	if (Dx12CommandQueue == nullptr)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// CreateCommandQueue
		result = Dx12on11Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));

		if (result != S_OK || Dx12CommandQueue == nullptr)
		{
			spdlog::debug("IFeature_Dx11wDx12::CreateDx12Device CreateCommandQueue result: {0:x}", result);
			return E_NOINTERFACE;
		}
	}

	if (Dx12CommandAllocator == nullptr)
	{
		result = Dx12on11Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Dx12CommandAllocator));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device CreateCommandAllocator error: {0:x}", result);
			return E_NOINTERFACE;
		}
	}

	if (Dx12CommandList == nullptr)
	{
		// CreateCommandList
		result = Dx12on11Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&Dx12CommandList));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device CreateCommandList error: {0:x}", result);
			return E_NOINTERFACE;
		}
	}

	return S_OK;
}

bool IFeature_Dx11wDx12::ProcessDx11Textures(const NVSDK_NGX_Parameter* InParameters)
{
	HRESULT result;

	// fence operation results
	HRESULT fr;
	HANDLE dx11_sharedHandle;

	D3D11_QUERY_DESC pQueryDesc;
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;
	ID3D11Query* query0 = nullptr;

	// 3 is query sync
	if (Config::Instance()->UseSafeSyncQueries.value_or(3) < 4 && _frameCount > 20)
	{
		fr = Dx11Device->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx11fence_1));

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create dx11fence_1 {0:x}", fr);
			return false;
		}

		fr = dx11fence_1->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &dx11_sharedHandle);

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create sharedhandle for dx11fence_1 {0:x}", fr);
			return false;
		}

		fr = Dx12on11Device->OpenSharedHandle(dx11_sharedHandle, IID_PPV_ARGS(&dx12fence_1));

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create open sharedhandle for dx12fence_1 {0:x}", fr);
			return false;
		}
	}
	else
	{
		result = Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12fence_1));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures CreateFence d3d12fence error: {0:x}", result);
			return false;
		}

		result = Device->CreateQuery(&pQueryDesc, &query0);

		if (result != S_OK || query0 == nullptr)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures can't create query1!");
			return false;
		}

		// Associate the query with the copy operation
		DeviceContext->Begin(query0);
	}

#pragma region Texture copies

	auto frame = _frameCount % 2;

	ID3D11Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Color exist..");
		if (CopyTextureFrom11To12(paramColor, &dx11Color, true, false) == NULL)
			return false;
	}
	else
	{
		spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Color not exist!!");
		return false;
	}

	ID3D11Resource* paramMv;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMv) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramMv);

	if (paramMv)
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures MotionVectors exist..");
		if (CopyTextureFrom11To12(paramMv, &dx11Mv, true, false) == false)
			return false;
	}
	else
	{
		spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures MotionVectors not exist!!");
		return false;
	}

	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Output exist..");
		if (CopyTextureFrom11To12(paramOutput, &dx11Out, false, false) == false)
			return false;
	}
	else
	{
		spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Output not exist!!");
		return false;
	}

	ID3D11Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Depth exist..");
		if (CopyTextureFrom11To12(paramDepth, &dx11Depth, true, true) == false)
			return false;
	}
	else
		spdlog::error("IFeature_Dx11wDx12::Evaluate Depth not exist!!");

	ID3D11Resource* paramExposure = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExposure) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExposure);

		if (paramExposure)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures ExposureTexture exist..");

			if (CopyTextureFrom11To12(paramExposure, &dx11Exp, true, false) == false)
				return false;
		}
		else
		{
			spdlog::warn("IFeature_Dx11wDx12::ProcessDx11Textures AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
			Config::Instance()->AutoExposure = true;
			Config::Instance()->changeBackend = true;
		}
	}
	else
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures AutoExposure enabled!");

	ID3D11Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Bias mask exist..");

			if (CopyTextureFrom11To12(paramMask, &dx11Tm, true, false) == false)
				return false;
		}
		else
		{
			spdlog::warn("IFeature_Dx11wDx12::ProcessDx11Textures bias mask not exist and its enabled in config, it may cause problems!!");
			Config::Instance()->DisableReactiveMask = true;
			Config::Instance()->changeBackend = true;
		}
	}
	else
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures DisableReactiveMask enabled!");

#pragma endregion

	// 3 is query sync
	if (Config::Instance()->UseSafeSyncQueries.value_or(3) < 4 && _frameCount > 20)
	{
		if (Config::Instance()->UseSafeSyncQueries.value_or(3) > 1)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Dx11DeviceContext->Flush()!");
			Dx11DeviceContext->Flush();
		}

		if (Config::Instance()->UseSafeSyncQueries.value_or(3) > 0)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Dx11 Signal & Dx12 Wait!");

			fr = Dx11DeviceContext->Signal(dx11fence_1, 10);

			if (fr != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Dx11DeviceContext->Signal(dx11fence_1, 10) : {0:x}!", fr);
				return false;
			}

			fr = Dx12CommandQueue->Wait(dx12fence_1, 10);

			if (fr != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Dx12CommandQueue->Wait(dx12fence_1, 10) : {0:x}!", fr);
				return false;
			}
		}
	}
	else
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Queries!");
		DeviceContext->End(query0);
		DeviceContext->Flush();

		// Wait for the query to be ready
		while (Dx11DeviceContext->GetData(query0, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);

		// Release the query
		query0->Release();
	}

#pragma region shared handles

	spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures SharedHandles start!");

	if (paramColor)
	{
		if (dx11Color.Dx12Handle != dx11Color.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Color.Dx11Handle, IID_PPV_ARGS(&dx11Color.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Color OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Color.Dx12Handle = dx11Color.Dx11Handle;
		}
	}

	if (paramMv)
	{
		if (dx11Mv.Dx12Handle != dx11Mv.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Mv.Dx11Handle, IID_PPV_ARGS(&dx11Mv.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures MotionVectors OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Mv.Dx11Handle = dx11Mv.Dx12Handle;
		}
	}

	if (paramOutput)
	{
		if (dx11Out.Dx12Handle != dx11Out.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Out.Dx11Handle, IID_PPV_ARGS(&dx11Out.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Output OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Out.Dx12Handle = dx11Out.Dx11Handle;
		}
	}

	if (paramDepth)
	{
		if (dx11Depth.Dx12Handle != dx11Depth.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Depth.Dx11Handle, IID_PPV_ARGS(&dx11Depth.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Depth OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Depth.Dx12Handle = dx11Depth.Dx11Handle;
		}
	}

	if (!Config::Instance()->AutoExposure.value_or(false) && paramExposure)
	{
		if (dx11Exp.Dx12Handle != dx11Exp.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Exp.Dx11Handle, IID_PPV_ARGS(&dx11Exp.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures ExposureTexture OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Exp.Dx12Handle = dx11Exp.Dx11Handle;
		}
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true) && paramMask)
	{
		if (dx11Tm.Dx12Handle != dx11Tm.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Tm.Dx11Handle, IID_PPV_ARGS(&dx11Tm.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures TransparencyMask OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Tm.Dx12Handle = dx11Tm.Dx11Handle;
		}
	}

#pragma endregion

	return true;
}

bool IFeature_Dx11wDx12::CopyBackOutput()
{
	HRESULT result;
	HANDLE dx12_sharedHandle;

	D3D11_QUERY_DESC pQueryDesc;
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;

	auto frame = _frameCount % 2;

	// dispatch fences
	if (Config::Instance()->UseSafeSyncQueries.value_or(3) < 4 && _frameCount > 20)
	{
		auto fr = Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx12fence_2));

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create dx12fence_2 {0:x}", fr);
			return false;
		}

		fr = Dx12on11Device->CreateSharedHandle(dx12fence_2, nullptr, GENERIC_ALL, nullptr, &dx12_sharedHandle);

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create sharedhandle for dx12fence_2 {0:x}", fr);
			return false;
		}

		fr = Dx11Device->OpenSharedFence(dx12_sharedHandle, IID_PPV_ARGS(&dx11fence_2));

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create open sharedhandle for dx11fence_2 {0:x}", fr);
			return false;
		}

		if (Config::Instance()->UseSafeSyncQueries.value_or(3) > 0)
		{
			Dx12CommandQueue->Signal(dx12fence_2, 20);

			// wait for fsr on dx12
			Dx11DeviceContext->Wait(dx11fence_2, 20);
		}

		// copy back output
		if (Config::Instance()->UseSafeSyncQueries.value_or(3) > 2)
		{
			ID3D11Query* query1 = nullptr;
			result = Dx11Device->CreateQuery(&pQueryDesc, &query1);

			if (result != S_OK || !query1)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput can't create query1!");
				return false;
			}

			Dx11DeviceContext->Begin(query1);

			// copy back output
			Dx11DeviceContext->CopyResource(paramOutput, dx11Out.SharedTexture);

			Dx11DeviceContext->End(query1);

			// Execute dx11 commands 
			Dx11DeviceContext->Flush();

			// wait for completion
			while (Dx11DeviceContext->GetData(query1, nullptr, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);

			// Release the query
			query1->Release();
		}
		else
			Dx11DeviceContext->CopyResource(paramOutput, dx11Out.SharedTexture);
	}
	else
	{
		auto fr = Dx12CommandQueue->Signal(dx12fence_1, 20);

		if (fr != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Dx12CommandQueue->Signal(dx12fence_1, 20) : {0:x}!", fr);
			return false;
		}

		// wait for end of operation
		if (dx12fence_1->GetCompletedValue() < 20)
		{
			auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent12)
			{
				fr = dx12fence_1->SetEventOnCompletion(20, fenceEvent12);

				if (fr != S_OK)
				{
					spdlog::error("IFeature_Dx11wDx12::CopyBackOutput dx12fence_1->SetEventOnCompletion(20, fenceEvent12) : {0:x}!", fr);
					return false;
				}

				WaitForSingleObject(fenceEvent12, INFINITE);
				CloseHandle(fenceEvent12);
			}
		}

		ID3D11Query* query2 = nullptr;
		result = Dx11Device->CreateQuery(&pQueryDesc, &query2);

		if (result != S_OK || query2 == nullptr)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput can't create query2!");
			return false;
		}

		// Associate the query with the copy operation
		DeviceContext->Begin(query2);

		//copy output back
		DeviceContext->CopyResource(paramOutput, dx11Out.SharedTexture);

		// Execute dx11 commands 
		DeviceContext->End(query2);
		Dx11DeviceContext->Flush();

		// Wait for the query to be ready
		while (Dx11DeviceContext->GetData(query2, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);

		// Release the query
		query2->Release();
	}

	// release fences
	if (dx11fence_1)
		dx11fence_1->Release();

	if (dx11fence_2)
		dx11fence_2->Release();

	if (dx12fence_1)
		dx12fence_1->Release();

	if (dx12fence_2)
		dx12fence_2->Release();

	return true;
}

bool IFeature_Dx11wDx12::BaseInit(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("IFeature_Dx11wDx12::BaseInit!");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (!InContext)
	{
		spdlog::error("IFeature_Dx11wDx12::BaseInit context is null!");
		return false;
	}

	auto contextResult = InContext->QueryInterface(IID_PPV_ARGS(&Dx11DeviceContext));
	if (contextResult != S_OK)
	{
		spdlog::error("IFeature_Dx11wDx12::BaseInit QueryInterface ID3D11DeviceContext4 result: {0:x}", contextResult);
		return false;
	}

	if (!InDevice)
		Dx11DeviceContext->GetDevice(&InDevice);

	auto dx11DeviceResult = InDevice->QueryInterface(IID_PPV_ARGS(&Dx11Device));

	if (dx11DeviceResult != S_OK)
	{
		spdlog::error("IFeature_Dx11wDx12::BaseInit QueryInterface ID3D11Device5 result: {0:x}", dx11DeviceResult);
		return false;
	}

	if (Dx12on11Device == nullptr)
	{
		auto fl = Dx11Device->GetFeatureLevel();
		auto result = CreateDx12Device(fl);

		//spdlog::trace("IFeature_Dx11wDx12::BaseInit sleeping after CreateDx12Device for 500ms");
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		if (result != S_OK || Dx12on11Device == nullptr)
		{
			spdlog::error("IFeature_Dx11wDx12::BaseInit QueryInterface Dx12Device result: {0:x}", result);
			return false;
		}
	}

	return true;
}

IFeature_Dx11wDx12::IFeature_Dx11wDx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters)
{
}

IFeature_Dx11wDx12::~IFeature_Dx11wDx12()
{
	if (Dx12on11Device && Dx12CommandQueue && Dx12CommandList)
	{
		ID3D12Fence* d3d12Fence = nullptr;

		do
		{
			if (Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
				break;

			if (Dx12CommandList->Close() != S_OK)
				break;

			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
			Dx12CommandQueue->Signal(d3d12Fence, 999);

			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
			{
				WaitForSingleObject(fenceEvent, INFINITE);
				CloseHandle(fenceEvent);
			}

		} while (false);

		if (d3d12Fence != nullptr)
		{
			d3d12Fence->Release();
			d3d12Fence = nullptr;
		}
	}

	ReleaseSharedResources();

	//spdlog::trace("IFeature_Dx11wDx12::~IFeature_Dx11wDx12 sleeping for 500ms");
	//std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
