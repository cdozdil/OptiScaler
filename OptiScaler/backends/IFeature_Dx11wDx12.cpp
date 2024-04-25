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

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyTextureFrom11To12 CreateTexture2D error: {0:x}", result);
				return false;
			}

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

	ReleaseSyncResources();

	SAFE_RELEASE(Dx12CommandList);
	SAFE_RELEASE(Dx12CommandQueue);
	SAFE_RELEASE(Dx12CommandAllocator);
	SAFE_RELEASE(Dx12Device);
}

void IFeature_Dx11wDx12::ReleaseSyncResources()
{
	SAFE_RELEASE(dx11FenceTextureCopy);
	SAFE_RELEASE(dx12FenceTextureCopy);
	SAFE_RELEASE(dx12FenceQuery);
	SAFE_RELEASE(dx11FenceCopySync);
	SAFE_RELEASE(dx12FenceCopySync);
	SAFE_RELEASE(dx11FenceCopyOutput);
	SAFE_RELEASE(dx12FenceCopyOutput);
	SAFE_RELEASE(queryTextureCopy);
	SAFE_RELEASE(queryCopyOutputFence);
	SAFE_RELEASE(queryCopyOutput);

	if (dx11SHForTextureCopy != NULL)
	{
		CloseHandle(dx11SHForTextureCopy);
		dx11SHForTextureCopy = NULL;
	}

	if (dx11SHForCopyOutput != NULL)
	{
		CloseHandle(dx11SHForCopyOutput);
		dx11SHForCopyOutput = NULL;
	}

	if (dx12SHForCopyOutput != NULL)
	{
		CloseHandle(dx12SHForCopyOutput);
		dx12SHForCopyOutput = NULL;
	}
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

	if (Dx12Device == nullptr)
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

		result = D3D12CreateDevice(hardwareAdapter, InFeatureLevel, IID_PPV_ARGS(&Dx12Device));

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
		result = Dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));

		if (result != S_OK || Dx12CommandQueue == nullptr)
		{
			spdlog::debug("IFeature_Dx11wDx12::CreateDx12Device CreateCommandQueue result: {0:x}", result);
			return E_NOINTERFACE;
		}
	}

	if (Dx12CommandAllocator == nullptr)
	{
		result = Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Dx12CommandAllocator));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CreateDx12Device CreateCommandAllocator error: {0:x}", result);
			return E_NOINTERFACE;
		}
	}

	if (Dx12CommandList == nullptr)
	{
		// CreateCommandList
		result = Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&Dx12CommandList));

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

	// Query only
	if (Config::Instance()->TextureSyncMethod.value_or(1) == 5 || _frameCount < 200)
	{
		if (queryTextureCopy == nullptr)
		{
			D3D11_QUERY_DESC pQueryDesc;
			pQueryDesc.Query = D3D11_QUERY_EVENT;
			pQueryDesc.MiscFlags = 0;

			result = Device->CreateQuery(&pQueryDesc, &queryTextureCopy);

			if (result != S_OK || queryTextureCopy == nullptr)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures can't create queryTextureCopy!");
				return false;
			}
		}

		// Associate the query with the copy operation
		DeviceContext->Begin(queryTextureCopy);
	}

#pragma region Texture copies

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

	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput[_frameCount % 2]) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput[_frameCount % 2]);

	if (paramOutput[_frameCount % 2])
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Output exist..");
		if (CopyTextureFrom11To12(paramOutput[_frameCount % 2], &dx11Out, false, false) == false)
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

	// query sync
	if (Config::Instance()->TextureSyncMethod.value_or(1) == 5 || _frameCount < 200)
	{
		spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Queries!");
		DeviceContext->End(queryTextureCopy);
		DeviceContext->Flush();

		// Wait for the query to be ready
		while (Dx11DeviceContext->GetData(queryTextureCopy, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);
	}
	else
	{
		if (dx11FenceTextureCopy == nullptr)
		{
			result = Dx11Device->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx11FenceTextureCopy));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create dx11FenceTextureCopy {0:x}", result);
				return false;
			}
		}

		if (dx11SHForTextureCopy == NULL)
		{
			result = dx11FenceTextureCopy->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &dx11SHForTextureCopy);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create sharedhandle for dx11FenceTextureCopy {0:x}", result);
				return false;
			}

			result = Dx12Device->OpenSharedHandle(dx11SHForTextureCopy, IID_PPV_ARGS(&dx12FenceTextureCopy));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Can't create open sharedhandle for dx12FenceTextureCopy {0:x}", result);
				return false;
			}
		}

		// Fence
		if (Config::Instance()->TextureSyncMethod.value_or(1) > 0)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Dx11 Signal & Dx12 Wait!");

			result = Dx11DeviceContext->Signal(dx11FenceTextureCopy, 10);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Dx11DeviceContext->Signal(dx11FenceTextureCopy, 10) : {0:x}!", result);
				return false;
			}
		}

		// Flush
		if (Config::Instance()->TextureSyncMethod.value_or(1) > 2)
		{
			spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures Dx11DeviceContext->Flush()!");
			Dx11DeviceContext->Flush();
		}

		// Gpu Sync
		if (Config::Instance()->TextureSyncMethod.value_or(1) == 1 || Config::Instance()->TextureSyncMethod.value_or(1) == 3)
		{
			result = Dx12CommandQueue->Wait(dx12FenceTextureCopy, 10);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Dx12CommandQueue->Wait(dx12fence_1, 10) : {0:x}!", result);
				return false;
			}
		}
		// Event Sync
		else if (Config::Instance()->TextureSyncMethod.value_or(1) != 0)
		{
			// wait for end of operation
			if (dx12FenceTextureCopy->GetCompletedValue() < 10)
			{
				auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

				if (fenceEvent12)
				{
					result = dx12FenceTextureCopy->SetEventOnCompletion(10, fenceEvent12);

					if (result != S_OK)
					{
						spdlog::error("IFeature_Dx11wDx12::CopyBackOutput dx12FenceTextureCopy->SetEventOnCompletion(10, fenceEvent12) : {0:x}!", result);
						return false;
					}

					WaitForSingleObject(fenceEvent12, INFINITE);
					CloseHandle(fenceEvent12);
				}
			}
		}
	}

#pragma region shared handles

	spdlog::debug("IFeature_Dx11wDx12::ProcessDx11Textures SharedHandles start!");

	if (paramColor && dx11Color.Dx12Handle != dx11Color.Dx11Handle)
	{
		if (dx11Color.Dx12Handle != NULL)
			CloseHandle(dx11Color.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Color.Dx11Handle, IID_PPV_ARGS(&dx11Color.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Color OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Color.Dx12Handle = dx11Color.Dx11Handle;
	}

	if (paramMv && dx11Mv.Dx12Handle != dx11Mv.Dx11Handle)
	{
		if (dx11Mv.Dx12Handle != NULL)
			CloseHandle(dx11Mv.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Mv.Dx11Handle, IID_PPV_ARGS(&dx11Mv.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures MotionVectors OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Mv.Dx11Handle = dx11Mv.Dx12Handle;
	}

	if (paramOutput[_frameCount % 2] && dx11Out.Dx12Handle != dx11Out.Dx11Handle)
	{
		if (dx11Out.Dx12Handle != NULL)
			CloseHandle(dx11Out.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Out.Dx11Handle, IID_PPV_ARGS(&dx11Out.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Output OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Out.Dx12Handle = dx11Out.Dx11Handle;
	}

	if (paramDepth && dx11Depth.Dx12Handle != dx11Depth.Dx11Handle)
	{
		if (dx11Depth.Dx12Handle != NULL)
			CloseHandle(dx11Depth.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Depth.Dx11Handle, IID_PPV_ARGS(&dx11Depth.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures Depth OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Depth.Dx12Handle = dx11Depth.Dx11Handle;
	}

	if (!Config::Instance()->AutoExposure.value_or(false) && paramExposure && dx11Exp.Dx12Handle != dx11Exp.Dx11Handle)
	{
		if (dx11Exp.Dx12Handle != NULL)
			CloseHandle(dx11Exp.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Exp.Dx11Handle, IID_PPV_ARGS(&dx11Exp.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures ExposureTexture OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Exp.Dx12Handle = dx11Exp.Dx11Handle;
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true) && paramMask && dx11Tm.Dx12Handle != dx11Tm.Dx11Handle)
	{
		if (dx11Tm.Dx12Handle != NULL)
			CloseHandle(dx11Tm.Dx12Handle);

		result = Dx12Device->OpenSharedHandle(dx11Tm.Dx11Handle, IID_PPV_ARGS(&dx11Tm.Dx12Resource));

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::ProcessDx11Textures TransparencyMask OpenSharedHandle error: {0:x}", result);
			return false;
		}

		dx11Tm.Dx12Handle = dx11Tm.Dx11Handle;
	}

#pragma endregion

	return true;
}

bool IFeature_Dx11wDx12::CopyBackOutput()
{
	HRESULT result;

	// No sync
	if (Config::Instance()->CopyBackSyncMethod.value_or(5) == 0 && _frameCount >= 200)
	{
		Dx11DeviceContext->CopyResource(paramOutput[_frameCount % 2], dx11Out.SharedTexture);
		return true;
	}

	//Fence ones
	if (Config::Instance()->CopyBackSyncMethod.value_or(5) != 5 && _frameCount >= 200)
	{
		if (dx12FenceCopySync == nullptr)
		{
			result = Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx12FenceCopySync));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create dx12FenceCopySync {0:x}", result);
				return false;
			}
		}

		if (dx12SHForCopyOutput == NULL)
		{
			result = Dx12Device->CreateSharedHandle(dx12FenceCopySync, nullptr, GENERIC_ALL, nullptr, &dx12SHForCopyOutput);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create sharedhandle for dx12FenceCopySync {0:x}", result);
				return false;
			}

			result = Dx11Device->OpenSharedFence(dx12SHForCopyOutput, IID_PPV_ARGS(&dx11FenceCopySync));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create open sharedhandle for dx11FenceCopySync {0:x}", result);
				return false;
			}
		}

		Dx12CommandQueue->Signal(dx12FenceCopySync, 20);

		if (Config::Instance()->CopyBackSyncMethod.value_or(5) == 1 || Config::Instance()->CopyBackSyncMethod.value_or(5) == 3)
		{
			// wait for fsr on dx12
			Dx11DeviceContext->Wait(dx11FenceCopySync, 20);
		}
		else
		{
			// wait for end of operation
			if (dx12FenceCopySync->GetCompletedValue() < 20)
			{
				auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

				if (fenceEvent12)
				{
					result = dx12FenceCopySync->SetEventOnCompletion(20, fenceEvent12);

					if (result != S_OK)
					{
						spdlog::error("IFeature_Dx11wDx12::CopyBackOutput dx12FenceCopySync->SetEventOnCompletion(20, fenceEvent12) : {0:x}!", result);
						return false;
					}

					WaitForSingleObject(fenceEvent12, INFINITE);
					CloseHandle(fenceEvent12);
				}
			}
		}

		if (dx11FenceCopyOutput == nullptr)
		{
			result = Dx11Device->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx11FenceCopyOutput));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create dx11FenceCopyOutput {0:x}", result);
				return false;
			}
		}

		if (dx11SHForCopyOutput == NULL)
		{
			result = dx11FenceCopyOutput->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &dx11SHForCopyOutput);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create sharedhandle for dx11FenceTextureCopy {0:x}", result);
				return false;
			}

			result = Dx12Device->OpenSharedHandle(dx11SHForCopyOutput, IID_PPV_ARGS(&dx12FenceCopyOutput));

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Can't create open sharedhandle for dx12FenceTextureCopy {0:x}", result);
				return false;
			}
		}

		// Copy Back
		Dx11DeviceContext->CopyResource(paramOutput[_frameCount % 2], dx11Out.SharedTexture);

		result = Dx11DeviceContext->Signal(dx11FenceCopyOutput, 30);

		if (result != S_OK)
		{
			spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Dx11DeviceContext->Signal(dx11FenceTextureCopy, 30) : {0:x}!", result);
			return false;
		}

		// fence + flush
		if (Config::Instance()->CopyBackSyncMethod.value_or(5) > 2)
			Dx11DeviceContext->Flush();

		if (Config::Instance()->CopyBackSyncMethod.value_or(5) == 1 || Config::Instance()->CopyBackSyncMethod.value_or(5) == 3)
		{
			result = Dx12CommandQueue->Wait(dx12FenceCopyOutput, 30);

			if (result != S_OK)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput Dx12CommandQueue->Wait(dx12FenceTextureCopy, 30) : {0:x}!", result);
				return false;
			}
		}
		else
		{
			// wait for end of operation
			if (dx12FenceCopyOutput->GetCompletedValue() < 30)
			{
				auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

				if (fenceEvent12)
				{
					result = dx12FenceCopyOutput->SetEventOnCompletion(30, fenceEvent12);

					if (result != S_OK)
					{
						spdlog::error("IFeature_Dx11wDx12::CopyBackOutput dx12FenceCopyOutput->SetEventOnCompletion(30, fenceEvent12) : {0:x}!", result);
						return false;
					}

					WaitForSingleObject(fenceEvent12, INFINITE);
					CloseHandle(fenceEvent12);
				}
			}

		}
	}
	// query	
	else
	{
		if (queryCopyOutputFence == nullptr)
		{
			D3D11_QUERY_DESC pQueryDesc;
			pQueryDesc.Query = D3D11_QUERY_EVENT;
			pQueryDesc.MiscFlags = 0;


			result = Dx11Device->CreateQuery(&pQueryDesc, &queryCopyOutputFence);

			if (result != S_OK || !queryCopyOutputFence)
			{
				spdlog::error("IFeature_Dx11wDx12::CopyBackOutput can't create queryCopyOutputFence!");
				return false;
			}
		}

		Dx11DeviceContext->Begin(queryCopyOutputFence);

		// copy back output
		Dx11DeviceContext->CopyResource(paramOutput[_frameCount % 2], dx11Out.SharedTexture);

		Dx11DeviceContext->End(queryCopyOutputFence);

		// Execute dx11 commands 
		Dx11DeviceContext->Flush();

		// wait for completion
		while (Dx11DeviceContext->GetData(queryCopyOutputFence, nullptr, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);
	}

	ReleaseSyncResources();

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

	if (Dx12Device == nullptr)
	{
		auto fl = Dx11Device->GetFeatureLevel();
		auto result = CreateDx12Device(fl);

		//spdlog::trace("IFeature_Dx11wDx12::BaseInit sleeping after CreateDx12Device for 500ms");
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		if (result != S_OK || Dx12Device == nullptr)
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
	if (Dx12Device && Dx12CommandQueue && Dx12CommandList)
	{
		ID3D12Fence* d3d12Fence = nullptr;

		do
		{
			if (Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
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
