#define HOOKING_DISABLED

#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"

#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext SDK: {0}", (int)InSDKVersion);
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext BuildPipelines: {0}", CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true));
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext NetworkModel: {0}", CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0));
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext LogLevel: {0}", CyberXessContext::instance()->MyConfig->LogLevel.value_or(1));

	CyberXessContext::instance()->Shutdown(true, true);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init AppId: {0}", InApplicationId);
	spdlog::debug("NVSDK_NGX_D3D11_Init SDK: {0}", (int)InSDKVersion);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID Init!");
	spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID SDK: {0}", (int)InSDKVersion);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init_with_ProjectID: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_D3D11_Init_with_ProjectID SDK: {0}", (int)InSDKVersion);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void)
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown");

	// Close D3D12 device
	if (CyberXessContext::instance()->Dx12Device != nullptr &&
		CyberXessContext::instance()->Dx12CommandQueue != nullptr &&
		CyberXessContext::instance()->Dx12CommandList != nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 resources");
		ID3D12Fence* d3d12Fence;
		CyberXessContext::instance()->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
		CyberXessContext::instance()->Dx12CommandQueue->Signal(d3d12Fence, 999);
		spdlog::debug("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 fence created and signalled");

		CyberXessContext::instance()->Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { CyberXessContext::instance()->Dx12CommandList };
		spdlog::debug("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 command list executing");
		CyberXessContext::instance()->Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		spdlog::debug("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 waiting signal");
		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		d3d12Fence->SetEventOnCompletion(999, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
		d3d12Fence->Release();
		spdlog::debug("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 release done");
	}

	CyberXessContext::instance()->Shutdown(true, true);

	// close all xess contexts
	for (auto const& [key, val] : CyberXessContext::instance()->Contexts) {
		val->XeSSDestroy();
		NVSDK_NGX_D3D11_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown1");

	NVSDK_NGX_D3D11_Shutdown();

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_GetParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetCapabilityParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_GetCapabilityParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_AllocateParameters");

	try
	{
		*OutParameters = new NGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_AllocateParameters exception: {}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_PopulateParameters_Impl");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	InitNGXParameters(InParameters);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_DestroyParameters");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	delete InParameters;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::warn("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800");
	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Feature

NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_D3D11_CreateFeature");

	if (CyberXessContext::instance()->Dx11Device == nullptr)
	{
		ID3D11Device* device;
		InDevCtx->GetDevice(&device);

		auto result = device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11Device));

		if (result != S_OK)
		{
			spdlog::error("NVSDK_NGX_D3D11_CreateFeature QueryInterface ID3D11Device5 result: {0:x}", result);
			return NVSDK_NGX_Result_Fail;
		}
	}

	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		// create d3d12 device
		auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
		auto result = CyberXessContext::instance()->CreateDx12Device(fl);

		if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
		{
			spdlog::error("NVSDK_NGX_D3D11_CreateFeature QueryInterface Dx12Device result: {0:x}", result);
			return NVSDK_NGX_Result_Fail;
		}
	}

	auto result = NVSDK_NGX_D3D12_CreateFeature(nullptr, InFeatureID, InParameters, OutHandle);

	if (result != NVSDK_NGX_Result_Success)
		spdlog::error("NVSDK_NGX_D3D11_CreateFeature NVSDK_NGX_D3D12_CreateFeature error: {0:x}", (int)result);

	return result;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (InHandle == nullptr || InHandle == NULL)
		return NVSDK_NGX_Result_Success;

	spdlog::info("NVSDK_NGX_D3D11_ReleaseFeature Handle: {0}", InHandle->Id);

	if (auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get(); deviceContext != nullptr)
		deviceContext->XeSSDestroy();

	CyberXessContext::instance()->DeleteContext(InHandle);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetFeatureRequirements");

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature Handle: {0}", InFeatureHandle->Id);

	auto instance = CyberXessContext::instance();
	auto deviceContext = instance->Contexts[InFeatureHandle->Id].get();

	if (deviceContext == nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
		return NVSDK_NGX_Result_Success;
	}

	HRESULT result;

	// if no d3d11device
	if (instance->Dx11Device == nullptr)
	{
		// get d3d11device
		InDevCtx->GetDevice((ID3D11Device**)&instance->Dx11Device);

		// No D3D12 device!
		if (instance->Dx12Device == nullptr)
		{
			spdlog::warn("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device device!");

			auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
			result = CyberXessContext::instance()->CreateDx12Device(fl);

			if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
				return NVSDK_NGX_Result_Fail;

			spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature Dx12Device created!");
		}
	}

	// if no context
	if (instance->Dx11DeviceContext == nullptr)
	{
		// get context
		result = InDevCtx->QueryInterface(IID_PPV_ARGS(&instance->Dx11DeviceContext));

		if (result != S_OK)
		{
			spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature no ID3D11DeviceContext4 interface!");
			return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
		}
	}

	// Command allocator & command list
	if (instance->Dx12CommandAllocator == nullptr)
	{
		//	CreateCommandAllocator 
		result = instance->Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&instance->Dx12CommandAllocator));

		if (result != S_OK)
		{
			spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator error: {0:x}", result);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}

		// CreateCommandList
		result = instance->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, instance->Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&instance->Dx12CommandList));

		if (result != S_OK)
		{
			spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList error: {0:x}", result);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}
	}

	unsigned int width, outWidth, height, outHeight;
	InParameters->Get(NVSDK_NGX_Parameter_Width, &width);
	InParameters->Get(NVSDK_NGX_Parameter_Height, &height);
	InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth);
	InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight);
	width = width > outWidth ? width : outWidth;
	height = height > outHeight ? height : outHeight;

	if (deviceContext->XeSSIsInited() && (deviceContext->DisplayHeight != height || deviceContext->DisplayWidth != width))
		deviceContext->XeSSDestroy();

	if (!CyberXessContext::instance()->MyConfig->DisableReactiveMaskSetFromIni)
	{
		ID3D11Resource* paramMask = nullptr;
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask != nullptr && !deviceContext->XeSSMaskEnabled())
		{
			spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature bias mask exist, enabling reactive mask!");
			CyberXessContext::instance()->MyConfig->DisableReactiveMask = false;
			deviceContext->XeSSDestroy();
		}
		else if (paramMask == nullptr && deviceContext->XeSSMaskEnabled())
		{
			spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature bias mask does not exist, disabling reactive mask!");
			CyberXessContext::instance()->MyConfig->DisableReactiveMask = true;
			deviceContext->XeSSDestroy();
		}
	}

	if (!deviceContext->XeSSIsInited())
	{
		deviceContext->XeSSInit(CyberXessContext::instance()->Dx12Device, InParameters);

		if (!deviceContext->XeSSIsInited())
		{
			spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature deviceContext.XessInit is false, init failed!");
			return NVSDK_NGX_Result_Fail;
		}
	}
	
	NVSDK_NGX_Result evResult = NVSDK_NGX_Result_Success;
	if (!deviceContext->XeSSExecuteDx11(instance->Dx12CommandList, instance->Dx12CommandQueue, instance->Dx11Device, InDevCtx, InParameters))
		evResult = NVSDK_NGX_Result_Fail;

	instance->Dx12CommandAllocator->Reset();
	instance->Dx12CommandList->Reset(instance->Dx12CommandAllocator, nullptr);

	return evResult;
}

#pragma endregion
