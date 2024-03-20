#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Dx11on12_212.h"

#include <dxgi1_6.h>
#include <d3d12.h>

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; dest.Height = src.Height; dest.Format = src.Format; dest.BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);		

bool FSR2FeatureDx11on12_212::CopyTextureFrom11To12(ID3D11Resource* InResource, D3D11_TEXTURE2D_RESOURCE_C* OutResource, bool InCopy = true)
{
	ID3D11Texture2D* originalTexture = nullptr;
	D3D11_TEXTURE2D_DESC desc{};

	auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

	if (result != S_OK)
		return false;

	originalTexture->GetDesc(&desc);

	if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
	{
		if (desc.Width != OutResource->Desc.Width || desc.Height != OutResource->Desc.Height ||
			desc.Format != OutResource->Desc.Format || desc.BindFlags != OutResource->Desc.BindFlags ||
			(OutResource->SharedTexture) == nullptr)
		{
			if (OutResource->SharedTexture != nullptr)
				OutResource->SharedTexture->Release();

			ASSIGN_DESC(OutResource->Desc, desc);
			OutResource->Dx11Handle = NULL;
			OutResource->Dx12Handle = NULL;

			desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
			result = Dx11Device->CreateTexture2D(&desc, nullptr, &OutResource->SharedTexture);

			IDXGIResource1* resource;

			result = OutResource->SharedTexture->QueryInterface(IID_PPV_ARGS(&resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			result = resource->GetSharedHandle(&OutResource->Dx11Handle);

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();
		}

		if (InCopy)
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
				spdlog::error("FSR2FeatureDx11on12::CopyTextureFrom11To12 QueryInterface(resource) error: {0:x}", result);
				return false;
			}

			// Get shared handle
			result = resource->GetSharedHandle(&OutResource->Dx11Handle);

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::CopyTextureFrom11To12 GetSharedHandle error: {0:x}", result);
				return false;
			}

			resource->Release();

			OutResource->SharedTexture = (ID3D11Texture2D*)InResource;
		}
	}

	originalTexture->Release();
	return true;
}

void FSR2FeatureDx11on12_212::ReleaseSharedResources()
{
	spdlog::debug("FSR2FeatureDx11on12::ReleaseSharedResources start!");

	SAFE_RELEASE(dx11Color.SharedTexture)
	SAFE_RELEASE(dx11Mv.SharedTexture)
	SAFE_RELEASE(dx11Out.SharedTexture)
	SAFE_RELEASE(dx11Depth.SharedTexture)
	SAFE_RELEASE(dx11Tm.SharedTexture)
	SAFE_RELEASE(dx11Exp.SharedTexture)
	SAFE_RELEASE(dx11Color.Dx12Resource)
	SAFE_RELEASE(dx11Mv.Dx12Resource)
	SAFE_RELEASE(dx11Out.Dx12Resource)
	SAFE_RELEASE(dx11Depth.Dx12Resource)
	SAFE_RELEASE(dx11Tm.Dx12Resource)
	SAFE_RELEASE(dx11Exp.Dx12Resource)
}

void FSR2FeatureDx11on12_212::GetHardwareAdapter(IDXGIFactory1* InFactory, IDXGIAdapter** InAdapter, D3D_FEATURE_LEVEL InFeatureLevel, bool InRequestHighPerformanceAdapter)
{
	spdlog::debug("FSR2FeatureDx11on12::GetHardwareAdapter");

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

HRESULT FSR2FeatureDx11on12_212::CreateDx12Device(D3D_FEATURE_LEVEL InFeatureLevel)
{
	spdlog::debug("FSR2FeatureDx11on12::CreateDx12Device");

	if (Dx12on11Device)
		return S_OK;

	HRESULT result;

	IDXGIFactory4* factory;
	result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::CreateDx12Device Can't create factory: {0:x}", result);
		return result;
	}

	IDXGIAdapter* hardwareAdapter = nullptr;
	GetHardwareAdapter(factory, &hardwareAdapter, InFeatureLevel, true);

	if (hardwareAdapter == nullptr)
	{
		spdlog::error("FSR2FeatureDx11on12::CreateDx12Device Can't get hardwareAdapter!");
		return E_NOINTERFACE;
	}

	result = D3D12CreateDevice(hardwareAdapter, InFeatureLevel, IID_PPV_ARGS(&Dx12on11Device));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::CreateDx12Device Can't create device: {0:x}", result);
		return result;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// CreateCommandQueue
	result = Dx12on11Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Dx12CommandQueue));

	if (result != S_OK || Dx12CommandQueue == nullptr)
	{
		spdlog::debug("FSR2FeatureDx11on12::CreateDx12Device CreateCommandQueue result: {0:x}", result);
		return E_NOINTERFACE;
	}

	result = Dx12on11Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Dx12CommandAllocator));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::CreateDx12Device CreateCommandAllocator error: {0:x}", result);
		return E_NOINTERFACE;
	}

	// CreateCommandList
	result = Dx12on11Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&Dx12CommandList));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::CreateDx12Device CreateCommandList error: {0:x}", result);
		return E_NOINTERFACE;
	}

	return S_OK;
}

bool FSR2FeatureDx11on12_212::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12::Init!");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (!InContext)
	{
		spdlog::error("FSR2FeatureDx11on12::Init context is null!");
		return false;
	}

	auto contextResult = InContext->QueryInterface(IID_PPV_ARGS(&Dx11DeviceContext));
	if (contextResult != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::Init QueryInterface ID3D11DeviceContext4 result: {0:x}", contextResult);
		return false;
	}

	if (!InDevice)
		Dx11DeviceContext->GetDevice(&InDevice);

	auto dx11DeviceResult = InDevice->QueryInterface(IID_PPV_ARGS(&Dx11Device));

	if (dx11DeviceResult != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::Init QueryInterface ID3D11Device5 result: {0:x}", dx11DeviceResult);
		return false;
	}

	if (!Dx12on11Device)
	{
		auto fl = Dx11Device->GetFeatureLevel();
		auto result = CreateDx12Device(fl);

		if (result != S_OK || !Dx12on11Device)
		{
			spdlog::error("FSR2FeatureDx11on12::Init QueryInterface Dx12Device result: {0:x}", result);
			return false;
		}
	}

	spdlog::debug("FSR2FeatureDx11on12::Init calling InitXeSS");

	if (Dx12on11Device && !InitFSR2(InParameters))
	{
		spdlog::error("FSR2FeatureDx11on12::Init InitFSR2 fail!");
		return false;
	}

	Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
	return true;
}

bool FSR2FeatureDx11on12_212::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12::Evaluate");

	if (!IsInited())
		return false;

	ID3D11DeviceContext4* dc;
	auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	if (dc != Dx11DeviceContext)
	{
		spdlog::warn("FSR2FeatureDx11on12::Evaluate Dx11DeviceContext changed!");
		ReleaseSharedResources();
		Dx11DeviceContext = dc;
	}

	Fsr212::FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureDx11on12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = Fsr212::ffxGetCommandListDX12_212(Dx12CommandList);

	// fence operation results
	HRESULT fr;
	ID3D11Fence* dx11fence_1 = nullptr;
	ID3D12Fence* dx12fence_1 = nullptr;
	HANDLE dx11_sharedHandle;

	D3D11_QUERY_DESC pQueryDesc;
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;
	ID3D11Query* query0 = nullptr;

	// 3 is query sync
	if (Config::Instance()->UseSafeSyncQueries.value_or(0) < 3)
	{
		fr = Dx11Device->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx11fence_1));

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create dx11fence_1 {0:x}", fr);
			return false;
		}

		fr = dx11fence_1->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &dx11_sharedHandle);

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create sharedhandle for dx11fence_1 {0:x}", fr);
			return false;
		}

		fr = Dx12on11Device->OpenSharedHandle(dx11_sharedHandle, IID_PPV_ARGS(&dx12fence_1));

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create open sharedhandle for dx12fence_1 {0:x}", fr);
			return false;
		}
	}
	else
	{
		auto result = Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12fence_1));

		if (result != S_OK)
		{
			spdlog::debug("FeatureContext::FSR2FeatureDx11on12 CreateFence d3d12fence error: {0:x}", result);
			return false;
		}

		result = Device->CreateQuery(&pQueryDesc, &query0);

		if (result != S_OK || query0 == nullptr)
		{
			spdlog::error("FeatureContext::FSR2FeatureDx11on12 can't create query1!");
			return false;
		}

		// Associate the query with the copy operation
		DeviceContext->Begin(query0);
	}


#pragma region Texture copies


	ID3D11Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureDx11on12::Evaluate Color exist..");
		if (CopyTextureFrom11To12(paramColor, &dx11Color) == NULL)
			return false;
	}
	else
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate Color not exist!!");
		return false;
	}

	ID3D11Resource* paramMv;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMv) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramMv);

	if (paramMv)
	{
		spdlog::debug("FSR2FeatureDx11on12::Evaluate MotionVectors exist..");
		if (CopyTextureFrom11To12(paramMv, &dx11Mv) == false)
			return false;
	}
	else
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D11Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR2FeatureDx11on12::Evaluate Output exist..");
		if (CopyTextureFrom11To12(paramOutput, &dx11Out, false) == false)
			return false;
	}
	else
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate Output not exist!!");
		return false;
	}

	ID3D11Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureDx11on12::Evaluate Depth exist..");
		if (CopyTextureFrom11To12(paramDepth, &dx11Depth) == false)
			return false;
	}
	else
		spdlog::error("FSR2FeatureDx11on12::Evaluate Depth not exist!!");

	ID3D11Resource* paramExposure = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExposure) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExposure);

		if (paramExposure)
		{
			spdlog::debug("FSR2FeatureDx11on12::Evaluate ExposureTexture exist..");

			if (CopyTextureFrom11To12(paramExposure, &dx11Exp) == false)
				return false;
		}
		else
			spdlog::warn("FSR2FeatureDx11on12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
	}
	else
		spdlog::debug("FSR2FeatureDx11on12::Evaluate AutoExposure enabled!");

	ID3D11Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
		{
			spdlog::debug("FSR2FeatureDx11on12::Evaluate Bias mask exist..");
			if (CopyTextureFrom11To12(paramMask, &dx11Tm) == false)
				return false;
		}
		else
			spdlog::warn("FSR2FeatureDx11on12::Evaluate bias mask not exist and its enabled in config, it may cause problems!!");
	}
#pragma endregion

	// 3 is query sync
	if (Config::Instance()->UseSafeSyncQueries.value_or(0) < 3)
	{
		if (Config::Instance()->UseSafeSyncQueries.value_or(0) > 0)
			Dx11DeviceContext->Flush();

		Dx11DeviceContext->Signal(dx11fence_1, 10);
		Dx12CommandQueue->Wait(dx12fence_1, 10);
	}
	else
	{
		DeviceContext->End(query0);
		DeviceContext->Flush();

		// Wait for the query to be ready
		while (Dx11DeviceContext->GetData(query0, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE);

		// Release the query
		query0->Release();
	}

#pragma region shared handles

	if (paramColor)
	{
		if (dx11Color.Dx12Handle != dx11Color.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Color.Dx11Handle, IID_PPV_ARGS(&dx11Color.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate Color OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Color.Dx12Handle = dx11Color.Dx11Handle;
		}

		params.color = Fsr212::ffxGetResourceDX12_212(&_context, dx11Color.Dx12Resource, (wchar_t*)L"FSR2_Color", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}

	if (paramMv)
	{
		if (dx11Mv.Dx12Handle != dx11Mv.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Mv.Dx11Handle, IID_PPV_ARGS(&dx11Mv.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate MotionVectors OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Mv.Dx12Handle = dx11Mv.Dx11Handle;
		}

		params.motionVectors = Fsr212::ffxGetResourceDX12_212(&_context, dx11Mv.Dx12Resource, (wchar_t*)L"FSR2_Motion", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}

	if (paramOutput)
	{
		if (dx11Out.Dx12Handle != dx11Out.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Out.Dx11Handle, IID_PPV_ARGS(&dx11Out.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate Output OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Out.Dx12Handle = dx11Out.Dx11Handle;
		}

		params.output = Fsr212::ffxGetResourceDX12_212(&_context, dx11Out.Dx12Resource, (wchar_t*)L"FSR2_Out", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	if (paramDepth)
	{
		if (dx11Depth.Dx12Handle != dx11Depth.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Depth.Dx11Handle, IID_PPV_ARGS(&dx11Depth.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate Depth OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Depth.Dx12Handle = dx11Depth.Dx11Handle;
		}

		params.depth = Fsr212::ffxGetResourceDX12_212(&_context, dx11Depth.Dx12Resource, (wchar_t*)L"FSR2_Depth", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}

	if (!Config::Instance()->AutoExposure.value_or(false) && paramExposure)
	{
		if (dx11Exp.Dx12Handle != dx11Exp.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Exp.Dx11Handle, IID_PPV_ARGS(&dx11Exp.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate ExposureTexture OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Exp.Dx12Handle = dx11Exp.Dx11Handle;
		}

		params.exposure = Fsr212::ffxGetResourceDX12_212(&_context, dx11Exp.Dx12Resource, (wchar_t*)L"FSR2_Exp", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true) && paramMask)
	{
		if (dx11Tm.Dx12Handle != dx11Tm.Dx11Handle)
		{
			result = Dx12on11Device->OpenSharedHandle(dx11Tm.Dx11Handle, IID_PPV_ARGS(&dx11Tm.Dx12Resource));

			if (result != S_OK)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate TransparencyMask OpenSharedHandle error: {0:x}", result);
				return false;
			}

			dx11Tm.Dx12Handle = dx11Tm.Dx11Handle;
		}

		params.reactive = Fsr212::ffxGetResourceDX12_212(&_context, dx11Tm.Dx12Resource, (wchar_t*)L"FSR2_Reactive", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}

#pragma endregion

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		params.motionVectorScale.x = MVScaleX;
		params.motionVectorScale.y = MVScaleY;
	}
	else
		spdlog::warn("FSR2FeatureDx11on12::Evaluate Can't get motion vector scales!");

	if (Config::Instance()->OverrideSharpness.value_or(false))
	{
		params.enableSharpening = true;
		params.sharpness = Config::Instance()->Sharpness.value_or(0.3);
	}
	else
	{
		float shapness = 0.0f;
		if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &shapness) == NVSDK_NGX_Result_Success)
		{
			params.enableSharpening = !(shapness == 0.0f || shapness == -1.0f);

			if (params.enableSharpening)
			{
				if (shapness < 0)
					params.sharpness = (shapness + 1.0f) / 2.0f;
				else
					params.sharpness = shapness;
			}
		}
	}

	if (IsDepthInverted())
	{
		params.cameraFar = 0.0f;
		params.cameraNear = 1.0f;
	}
	else
	{
		params.cameraFar = 1.0f;
		params.cameraNear = 0.0f;
	}

	if (Config::Instance()->FsrVerticalFov.has_value())
		params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.01745329251f;
	else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
		params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.01745329251f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
	else
		params.cameraFovAngleVertical = 1.047198f;

	if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
		params.frameTimeDelta = (float)GetDeltaTime();

	params.preExposure = 1.0f;

	spdlog::debug("FSR2FeatureDx11on12::Evaluate Dispatch!!");
	auto ffxresult = Fsr212::ffxFsr2ContextDispatch212(&_context, &params);

	if (ffxresult != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx12::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString212(ffxresult));
		return false;
	}

	ID3D12Fence* dx12fence_2 = nullptr;
	ID3D11Fence* dx11fence_2 = nullptr;
	HANDLE dx12_sharedHandle;

	// dispatch fences
	if (Config::Instance()->UseSafeSyncQueries.value_or(0) < 3)
	{
		fr = Dx12on11Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&dx12fence_2));

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create dx12fence_2 {0:x}", fr);
			return false;
		}

		fr = Dx12on11Device->CreateSharedHandle(dx12fence_2, nullptr, GENERIC_ALL, nullptr, &dx12_sharedHandle);

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create sharedhandle for dx12fence_2 {0:x}", fr);
			return false;
		}

		fr = Dx11Device->OpenSharedFence(dx12_sharedHandle, IID_PPV_ARGS(&dx11fence_2));

		if (fr != S_OK)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Can't create open sharedhandle for dx11fence_2 {0:x}", fr);
			return false;
		}
	}

	// Execute dx12 commands to process fsr
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	// fsr done
	if (Config::Instance()->UseSafeSyncQueries.value_or(0) < 3)
	{
		Dx12CommandQueue->Signal(dx12fence_2, 20);

		// wait for fsr on dx12
		Dx11DeviceContext->Wait(dx11fence_2, 20);

		// copy back output
		if (Config::Instance()->UseSafeSyncQueries.value_or(0) > 1)
		{
			ID3D11Query* query1 = nullptr;
			result = Dx11Device->CreateQuery(&pQueryDesc, &query1);

			if (result != S_OK || !query1)
			{
				spdlog::error("FSR2FeatureDx11on12::Evaluate can't create query1!");
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
		Dx12CommandQueue->Signal(dx12fence_1, 20);

		// wait for end of operation
		if (dx12fence_1->GetCompletedValue() < 20)
		{
			auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent12)
			{
				dx12fence_1->SetEventOnCompletion(20, fenceEvent12);
				WaitForSingleObject(fenceEvent12, INFINITE);
				CloseHandle(fenceEvent12);
			}
		}

		ID3D11Query* query2 = nullptr;
		result = Dx11Device->CreateQuery(&pQueryDesc, &query2);

		if (result != S_OK || query2 == nullptr)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate can't create query2!");
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

	// imgui
	if (Imgui)
	{
		if (Imgui->IsHandleDifferent())
			Imgui.reset();
		else
			Imgui->Render(InDeviceContext, paramOutput);
	}
	else
		Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);

	// release fences
	if (dx11fence_1)
		dx11fence_1->Release();

	if (dx11fence_2)
		dx11fence_2->Release();

	if (dx12fence_1)
		dx12fence_1->Release();

	if (dx12fence_2)
		dx12fence_2->Release();

	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

FSR2FeatureDx11on12_212::~FSR2FeatureDx11on12_212()
{
	spdlog::debug("FSR2FeatureDx11on12::~FSR2FeatureDx11on12");

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
			spdlog::warn("FSR2FeatureDx11on12::FSR2FeatureDx11on12 can't get fenceEvent handle");

		d3d12Fence->Release();

		SAFE_RELEASE(Dx12CommandList);
		SAFE_RELEASE(Dx12CommandQueue);
		SAFE_RELEASE(Dx12CommandAllocator);
		SAFE_RELEASE(Dx12on11Device);
	}

	ReleaseSharedResources();

	if (Imgui)
		Imgui.reset();
}

bool FSR2FeatureDx11on12_212::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12_212::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = Fsr212::ffxFsr2GetScratchMemorySizeDX12_212();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = Fsr212::ffxFsr2GetInterfaceDX12_212(&_contextDesc.callbacks, Dx12on11Device, scratchBuffer, scratchBufferSize);

	if (errorCode != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString212(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = Fsr212::ffxGetDeviceDX12_212(Dx12on11Device);
	_contextDesc.maxRenderSize.width = DisplayWidth();
	_contextDesc.maxRenderSize.height = DisplayHeight();
	_contextDesc.displaySize.width = DisplayWidth();
	_contextDesc.displaySize.height = DisplayHeight();

	_contextDesc.flags = 0;

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
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DEPTH_INVERTED;
		SetDepthInverted(true);
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	spdlog::debug("FSR2FeatureDx11on12_212::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = Fsr212::ffxFsr2ContextCreate212(&_context, &_contextDesc);

	if (ret != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString212(ret));
		return false;
	}

	SetInit(true);

	return true;
}