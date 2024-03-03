#include "pch.h"
#include "XeSSFeature_Dx11.h"
#include "Config.h"

#include "dxgi1_6.h"
#include <d3d12.h>

#define ASSIGN_DESC(dest, src) dest->Width = src.Width; dest->Height = src.Height; dest->Format = src.Format; dest->BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);		

bool XeSSFeatureDx11::CopyTextureFrom11To12(ID3D11Resource* InResource, ID3D11Texture2D** OutSharedResource, D3D11_TEXTURE2D_DESC_C* OutSharedDesc, bool InCopy = true)
{
	ID3D11Texture2D* originalTexture = nullptr;
	D3D11_TEXTURE2D_DESC desc{};

	auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

	if (result != S_OK)
		return false;

	originalTexture->GetDesc(&desc);

	if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
	{
		if (desc.Width != OutSharedDesc->Width || desc.Height != OutSharedDesc->Height ||
			desc.Format != OutSharedDesc->Format || desc.BindFlags != OutSharedDesc->BindFlags ||
			(*OutSharedResource) == nullptr)
		{
			if ((*OutSharedResource) != nullptr)
				(*OutSharedResource)->Release();

			ASSIGN_DESC(OutSharedDesc, desc);
			OutSharedDesc->pointer = nullptr;
			OutSharedDesc->handle = NULL;

			desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
			result = Dx11Device->CreateTexture2D(&desc, nullptr, OutSharedResource);

			IDXGIResource1* resource;

			result = (*OutSharedResource)->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("XeSSFeatureDx11::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			result = resource->GetSharedHandle(&OutSharedDesc->handle);

			if (result != S_OK)
			{
				spdlog::error("XeSSFeatureDx11::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();
			OutSharedDesc->pointer = (*OutSharedResource);
		}

		if (InCopy)
			Dx11DeviceContext->CopyResource(*OutSharedResource, InResource);
	}
	else
	{
		if (OutSharedDesc->pointer != InResource)
		{
			IDXGIResource1* resource;

			result = originalTexture->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("XeSSFeatureDx11::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			result = resource->GetSharedHandle(&OutSharedDesc->handle);

			if (result != S_OK)
			{
				spdlog::error("XeSSFeatureDx11::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();
			OutSharedDesc->pointer = InResource;
		}
	}

	originalTexture->Release();
	return true;
}

void XeSSFeatureDx11::ReleaseSharedResources()
{
	spdlog::debug("XeSSFeatureDx11::ReleaseSharedResources start!");

	SAFE_RELEASE(dx11Color.SharedTexture);
	SAFE_RELEASE(dx11Mv.SharedTexture);
	SAFE_RELEASE(dx11Out.SharedTexture);
	SAFE_RELEASE(dx11Depth.SharedTexture);
	SAFE_RELEASE(dx11Tm.SharedTexture);
	SAFE_RELEASE(dx11Exp.SharedTexture);
}

void XeSSFeatureDx11::GetHardwareAdapter(IDXGIFactory1* InFactory, IDXGIAdapter** InAdapter, D3D_FEATURE_LEVEL InFeatureLevel, bool InRequestHighPerformanceAdapter)
{
	spdlog::debug("XeSSFeatureDx11::GetHardwareAdapter");

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

HRESULT XeSSFeatureDx11::CreateDx12Device(D3D_FEATURE_LEVEL InFeatureLevel)
{
	spdlog::debug("XeSSFeatureDx11::CreateDx12Device");

	if (Dx12on11Device)
		return S_OK;

	HRESULT result;

	IDXGIFactory4* factory;
	result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::CreateDx12Device Can't create factory: {0:x}", result);
		return result;
	}

	IDXGIAdapter* hardwareAdapter = nullptr;
	GetHardwareAdapter(factory, &hardwareAdapter, InFeatureLevel, true);

	if (hardwareAdapter == nullptr)
	{
		spdlog::error("XeSSFeatureDx11::CreateDx12Device Can't get hardwareAdapter!");
		return E_NOINTERFACE;
	}

	result = D3D12CreateDevice(hardwareAdapter, InFeatureLevel, IID_PPV_ARGS(&Dx12on11Device));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::CreateDx12Device Can't create device: {0:x}", result);
		return result;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// CreateCommandQueue
	result = Dx12on11Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));

	if (result != S_OK || Dx12CommandQueue == nullptr)
	{
		spdlog::debug("XeSSFeatureDx11::CreateDx12Device CreateCommandQueue result: {0:x}", result);
		return E_NOINTERFACE;
	}

	result = Dx12on11Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Dx12CommandAllocator));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::CreateDx12Device CreateCommandAllocator error: {0:x}", result);
		return E_NOINTERFACE;
	}

	// CreateCommandList
	result = Dx12on11Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&Dx12CommandList));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::CreateDx12Device CreateCommandList error: {0:x}", result);
		return E_NOINTERFACE;
	}

	return S_OK;
}

bool XeSSFeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Init!");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (!InContext)
	{
		spdlog::error("XeSSFeatureDx11::Init context is null!");
		return false;
	}

	auto contextResult = InContext->QueryInterface(IID_PPV_ARGS(&Dx11DeviceContext));
	if (contextResult != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::Init QueryInterface ID3D11DeviceContext4 result: {0:x}", contextResult);
		return false;
	}

	if (!InDevice)
		Dx11DeviceContext->GetDevice(&InDevice);

	auto dx11DeviceResult = InDevice->QueryInterface(IID_PPV_ARGS(&Dx11Device));

	if (dx11DeviceResult != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::Init QueryInterface ID3D11Device5 result: {0:x}", dx11DeviceResult);
		return false;
	}

	if (!Dx12on11Device)
	{
		auto fl = Dx11Device->GetFeatureLevel();
		auto result = CreateDx12Device(fl);

		if (result != S_OK || !Dx12on11Device)
		{
			spdlog::error("XeSSFeatureDx11::Init QueryInterface Dx12Device result: {0:x}", result);
			return false;
		}
	}

	spdlog::debug("XeSSFeatureDx11::Init calling InitXeSS");

	if (Dx12on11Device && !InitXeSS(Dx12on11Device, InParameters))
	{
		spdlog::error("XeSSFeatureDx11::Init InitXeSS fail!");
		return false;
	}

	return true;
}

bool XeSSFeatureDx11::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Evaluate");

	if (!IsInited() || !_xessContext)
		return false;

	ID3D11DeviceContext4* dc;
	auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	if (dc != Dx11DeviceContext)
	{
		spdlog::warn("XeSSFeatureDx11::Evaluate Dx11DeviceContext changed!");
		ReleaseSharedResources();
		Dx11DeviceContext = dc;
	}


	// Fence for syncing
	ID3D12Fence* d3d12Fence;
	result = Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	// creatimg params for XeSS
	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
		params.exposureScale = 1.0f;

	InParameters->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

	GetRenderResolution(InParameters, &params.inputWidth, &params.inputHeight);

	spdlog::debug("FeatureContext::XeSSInit Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

#pragma region Texture copies

	D3D11_QUERY_DESC pQueryDesc{};
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;
	ID3D11Query* query1 = nullptr;
	result = Dx11Device->CreateQuery(&pQueryDesc, &query1);

	if (result != S_OK || !query1)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate can't create query1!");
		return false;
	}

	// Associate the query with the copy operation
	Dx11DeviceContext->Begin(query1);

	ID3D11Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate Color exist..");
		if (CopyTextureFrom11To12(paramColor, &dx11Color.SharedTexture, &dx11Color.Desc) == NULL)
			return false;
	}
	else
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Color not exist!!");
		return false;
	}

	ID3D11Resource* paramMv;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMv) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramMv);

	if (paramMv)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate MotionVectors exist..");
		if (CopyTextureFrom11To12(paramMv, &dx11Mv.SharedTexture, &dx11Mv.Desc) == false)
			return false;
	}
	else
	{
		spdlog::error("XeSSFeatureDx11::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D11Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate Output exist..");
		if (CopyTextureFrom11To12(paramOutput, &dx11Out.SharedTexture, &dx11Out.Desc, false) == false)
			return false;
	}
	else
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Output not exist!!");
		return false;
	}

	ID3D11Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate Depth exist..");
		if (CopyTextureFrom11To12(paramDepth, &dx11Depth.SharedTexture, &dx11Depth.Desc) == false)
			return false;
	}
	else
		spdlog::error("XeSSFeatureDx11::Evaluate Depth not exist!!");

	ID3D11Resource* paramExposure = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExposure) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExposure);

		if (paramExposure)
		{
			spdlog::debug("XeSSFeatureDx11::Evaluate ExposureTexture exist..");

			if (CopyTextureFrom11To12(paramExposure, &dx11Exp.SharedTexture, &dx11Exp.Desc) == false)
				return false;
		}
		else
			spdlog::warn("XeSSFeatureDx11::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
	}
	else
		spdlog::debug("XeSSFeatureDx11::Evaluate AutoExposure enabled!");

	ID3D11Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
		{
			spdlog::debug("XeSSFeatureDx11::Evaluate Bias mask exist..");
			if (CopyTextureFrom11To12(paramMask, &dx11Tm.SharedTexture, &dx11Tm.Desc) == false)
				return false;
		}
		else
			spdlog::warn("XeSSFeatureDx11::Evaluate bias mask not exist and its enabled in config, it may cause problems!!");
	}

	// Execute dx11 commands 
	Dx11DeviceContext->End(query1);
	Dx11DeviceContext->Flush();

	// Wait for the query to be ready
	while (Dx11DeviceContext->GetData(query1, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
		//
	}

	// Release the query
	query1->Release();

	if (paramColor)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Color.Desc.handle, IID_PPV_ARGS(&params.pColorTexture));

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Color OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

	if (paramMv)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Mv.Desc.handle, IID_PPV_ARGS(&params.pVelocityTexture));

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate MotionVectors OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

	if (paramOutput)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Out.Desc.handle, IID_PPV_ARGS(&params.pOutputTexture));
		dx11Out.SharedHandle = dx11Out.Desc.handle;

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Output OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

	if (paramDepth)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Depth.Desc.handle, IID_PPV_ARGS(&params.pDepthTexture));

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Depth OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

	if (!Config::Instance()->AutoExposure.value_or(false) && paramExposure)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Exp.Desc.handle, IID_PPV_ARGS(&params.pExposureScaleTexture));

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate ExposureTexture OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true) && paramMask)
	{
		result = Dx12on11Device->OpenSharedHandle(dx11Tm.Desc.handle, IID_PPV_ARGS(&params.pResponsivePixelMaskTexture));

		if (result != S_OK)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate TransparencyMask OpenSharedHandle error: {0:x}", result);
			return false;
		}
	}

#pragma endregion

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		xessResult = xessSetVelocityScale(_xessContext, MVScaleX, MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate xessSetVelocityScale error: {0}", ResultToString(xessResult));
			return false;
		}
	}
	else
		spdlog::warn("FeatureContext::XeSSExecuteDx12 Can't get motion vector scales!");

	// Execute xess
	spdlog::debug("XeSSFeatureDx11::Evaluate Executing!!");
	xessResult = xessD3D12Execute(_xessContext, Dx12CommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	// Execute dx12 commands to process xess
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	// xess done
	Dx12CommandQueue->Signal(d3d12Fence, 20);

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
	result = Dx11Device->CreateQuery(&pQueryDesc, &query2);

	if (result != S_OK || !query2)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate can't create query2!");
		return false;
	}

	// Associate the query with the copy operation
	Dx11DeviceContext->Begin(query2);
	Dx11DeviceContext->CopyResource(paramOutput, dx11Out.SharedTexture);

	// Execute dx11 commands 
	Dx11DeviceContext->End(query2);
	Dx11DeviceContext->Flush();

	// Wait for the query to be ready
	while (Dx11DeviceContext->GetData(query2, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
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

	if (!Config::Instance()->AutoExposure.value_or(false) && paramExposure)
		params.pExposureScaleTexture->Release();

	if (!Config::Instance()->DisableReactiveMask.value_or(true) && paramMask)
		params.pResponsivePixelMaskTexture->Release();

	if (paramOutput)
		params.pOutputTexture->Release();

	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

void XeSSFeatureDx11::ReInit(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::ReInit!");

	SetInit(false);

	if (_xessContext)
		xessDestroyContext(_xessContext);

	SetInit(Init(Device, DeviceContext, InParameters));
}

XeSSFeatureDx11::~XeSSFeatureDx11()
{
	spdlog::debug("XeSSFeatureDx11::XeSSFeatureDx11");

	if (Dx12on11Device && Dx12CommandQueue && Dx12CommandList)
	{
		ID3D12Fence* d3d12Fence;
		Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
		Dx12CommandQueue->Signal(d3d12Fence, 999);

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
		{
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}
		else
			spdlog::warn("XeSSFeatureDx11::XeSSFeatureDx11 can't get fenceEvent handle");

		d3d12Fence->Release();

		SAFE_RELEASE(Dx12CommandList);
		SAFE_RELEASE(Dx12CommandQueue);
		SAFE_RELEASE(Dx12CommandAllocator);
		SAFE_RELEASE(Dx12on11Device);
	}

	ReleaseSharedResources();
}

