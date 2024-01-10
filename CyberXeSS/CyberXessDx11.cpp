#define HOOKING_DISABLED

#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"


#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	LOG("NVSDK_NGX_D3D11_Init_Ext AppId:" + std::to_string(InApplicationId), spdlog::level::info);
	LOG("NVSDK_NGX_D3D11_Init_Ext SDK:" + std::to_string(InSDKVersion), spdlog::level::info);

	CyberXessContext::instance()->Shutdown(true, true);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init AppId:" + std::to_string(InApplicationId), spdlog::level::debug);
	LOG("NVSDK_NGX_D3D11_Init SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init_ProjectID Init!", spdlog::level::debug);
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_ProjectID : " + pId, spdlog::level::debug);
	LOG("NVSDK_NGX_D3D11_Init_ProjectID SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID : " + pId, spdlog::level::debug);
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void)
{
	LOG("NVSDK_NGX_D3D11_Shutdown", spdlog::level::info);

	// Close D3D12 device
	if (CyberXessContext::instance()->Dx12Device != nullptr &&
		CyberXessContext::instance()->Dx12CommandQueue != nullptr &&
		CyberXessContext::instance()->Dx12CommandList != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 resources", spdlog::level::debug);
		ID3D12Fence* d3d12Fence;
		CyberXessContext::instance()->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
		CyberXessContext::instance()->Dx12CommandQueue->Signal(d3d12Fence, 999);
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 fence created and signalled", spdlog::level::debug);

		CyberXessContext::instance()->Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { CyberXessContext::instance()->Dx12CommandList };
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 command list executing", spdlog::level::debug);
		CyberXessContext::instance()->Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 waiting signal", spdlog::level::debug);
		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		d3d12Fence->SetEventOnCompletion(999, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
		d3d12Fence->Release();
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 release done", spdlog::level::debug);
	}

	CyberXessContext::instance()->Shutdown(true, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

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
	LOG("NVSDK_NGX_D3D11_Shutdown1", spdlog::level::info);

	NVSDK_NGX_D3D11_Shutdown();

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetParameters", spdlog::level::debug);

	*OutParameters = CyberXessContext::instance()->NvParameterInstance->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation 
NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetCapabilityParameters", spdlog::level::debug);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_AllocateParameters", spdlog::level::debug);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	LOG("NVSDK_NGX_D3D11_DestroyParameters", spdlog::level::debug);

	NvParameter::instance()->DeleteParameters((NvParameter*)InParameters);
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800", spdlog::level::warn);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Feature

NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	LOG("NVSDK_NGX_D3D11_CreateFeature", spdlog::level::info);

	if (CyberXessContext::instance()->Dx11Device == nullptr)
	{
		ID3D11Device* device;
		InDevCtx->GetDevice(&device);

		auto result = device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11Device));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature QueryInterface ID3D11Device5 result: " + int_to_hex(result), spdlog::level::err);
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
			LOG("NVSDK_NGX_D3D11_CreateFeature QueryInterface Dx12Device result: " + int_to_hex(result), spdlog::level::err);
			return NVSDK_NGX_Result_Fail;
		}
	}

	auto result = NVSDK_NGX_D3D12_CreateFeature(nullptr, InFeatureID, InParameters, OutHandle);

	if (result != NVSDK_NGX_Result_Success)
		LOG("NVSDK_NGX_D3D11_CreateFeature NVSDK_NGX_D3D12_CreateFeature error: " + int_to_hex(result), spdlog::level::err);

	return result;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (InHandle == nullptr || InHandle == NULL)
		return NVSDK_NGX_Result_Success;

	LOG("NVSDK_NGX_D3D11_ReleaseFeature Handle: " + std::to_string(InHandle->Id), spdlog::level::info);

	if (auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get(); deviceContext != nullptr)
		deviceContext->XeSSDestroy();

	CyberXessContext::instance()->DeleteContext(InHandle);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
	LOG("NVSDK_NGX_D3D11_GetFeatureRequirements", spdlog::level::debug);

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.19045.2728");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	LOG("NVSDK_NGX_D3D11_EvaluateFeature Handle: " + std::to_string(InFeatureHandle->Id), spdlog::level::debug);

	auto instance = CyberXessContext::instance();
	auto deviceContext = instance->Contexts[InFeatureHandle->Id].get();

	if (deviceContext == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success", spdlog::level::debug);
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
			LOG("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device device!", spdlog::level::debug);

			auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
			result = CyberXessContext::instance()->CreateDx12Device(fl);

			if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
				return NVSDK_NGX_Result_Fail;

			LOG("NVSDK_NGX_D3D11_EvaluateFeature Dx12Device created!", spdlog::level::debug);
		}
	}

	// if no context
	if (instance->Dx11DeviceContext == nullptr)
	{
		// get context
		result = InDevCtx->QueryInterface(IID_PPV_ARGS(&instance->Dx11DeviceContext));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature no ID3D11DeviceContext4 interface!", spdlog::level::debug);
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
			LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator error: " + int_to_hex(result), spdlog::level::err);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}

		// CreateCommandList
		result = instance->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, instance->Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&instance->Dx12CommandList));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList error: " + int_to_hex(result), spdlog::level::err);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}
	}

	if (!deviceContext->XeSSIsInited())
	{
		deviceContext->XeSSInit(CyberXessContext::instance()->Dx12Device, CyberXessContext::instance()->CreateFeatureParams);

		if (!deviceContext->XeSSIsInited())
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature deviceContext.XessInit is false, init failed!", spdlog::level::err);
			return NVSDK_NGX_Result_Fail;
		}
	}

	// get params from dlss
	const auto inParams = static_cast<const NvParameter*>(InParameters);

	NVSDK_NGX_Result evResult = NVSDK_NGX_Result_Success;
	if (!deviceContext->XeSSExecuteDx11(instance->Dx12CommandList, instance->Dx12CommandQueue, instance->Dx11Device, InDevCtx, inParams))
		evResult = NVSDK_NGX_Result_Fail;

	instance->Dx12CommandAllocator->Reset();
	instance->Dx12CommandList->Reset(instance->Dx12CommandAllocator, nullptr);

	return evResult;
}

#pragma endregion
