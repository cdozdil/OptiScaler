#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion,
	unsigned long long unknown0)
{
	LOG("NVSDK_NGX_D3D11_Init_Ext AppId:" + std::to_string(InApplicationId), LEVEL_INFO);
	LOG("NVSDK_NGX_D3D11_Init_Ext SDK:" + std::to_string(InSDKVersion), LEVEL_INFO);

	CyberXessContext::instance()->Dx11Device = InDevice;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init AppId:" + std::to_string(InApplicationId), LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init_ProjectID Init!", LEVEL_DEBUG);
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void)
{
	LOG("NVSDK_NGX_D3D11_Shutdown", LEVEL_INFO);

	CyberXessContext::instance()->Shutdown();

	CyberXessContext::instance()->NvParameterInstance->Params.clear();
	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
	LOG("NVSDK_NGX_D3D11_Shutdown1", LEVEL_INFO);

	CyberXessContext::instance()->Shutdown();

	CyberXessContext::instance()->NvParameterInstance->Params.clear();
	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetParameters", LEVEL_DEBUG);

	*OutParameters = CyberXessContext::instance()->NvParameterInstance->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation 
NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetCapabilityParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_AllocateParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	LOG("NVSDK_NGX_D3D11_DestroyParameters", LEVEL_DEBUG);

	NvParameter::instance()->DeleteParameters((NvParameter*)InParameters);
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId,
	const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800", LEVEL_WARNING);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	LOG("NVSDK_NGX_D3D11_CreateFeature");

	InDevCtx->GetDevice(&CyberXessContext::instance()->Dx11Device);

	// D3D11on12 Device
	auto result = CyberXessContext::instance()->Dx11Device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11on12Device));
	LOG("NVSDK_NGX_D3D11_CreateFeature query d3d11on12 result: " + int_to_hex(result));

	if (result != S_OK || CyberXessContext::instance()->Dx11on12Device == nullptr)
		return NVSDK_NGX_Result_FAIL_PlatformError;

	// D3D12 Device
	result = CyberXessContext::instance()->Dx11on12Device->GetD3D12Device(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));
	LOG("NVSDK_NGX_D3D11_CreateFeature query device12 result: " + int_to_hex(result));

	if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
		return NVSDK_NGX_Result_FAIL_PlatformError;

	auto cfResult = NVSDK_NGX_D3D12_CreateFeature(CyberXessContext::instance()->Dx12CommandList, InFeatureID, InParameters, OutHandle);
	LOG("NVSDK_NGX_D3D11_CreateFeature result: " + int_to_hex(cfResult));
	return cfResult;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	LOG("NVSDK_NGX_D3D11_ReleaseFeature", LEVEL_DEBUG);

	auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get();
	CyberXessContext::instance()->DeleteContext(InHandle);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
	LOG("NVSDK_NGX_D3D11_GetFeatureRequirements", LEVEL_DEBUG);

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.19045.2728");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device device!", LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_PlatformError;
	}

	CyberXessContext::instance()->Dx11DeviceContext = InDevCtx;

	HRESULT result;

	if (CyberXessContext::instance()->Dx12CommandQueue == nullptr)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

		// CreateCommandQueue
		result = CyberXessContext::instance()->Dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CyberXessContext::instance()->Dx12CommandQueue));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandQueue result: " + int_to_hex(result));

		if (result != S_OK || CyberXessContext::instance()->Dx12CommandQueue == nullptr)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		//	CreateCommandAllocator 
		result = CyberXessContext::instance()->Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&CyberXessContext::instance()->Dx12CommandAllocator));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator result: " + int_to_hex(result));

		if (FAILED(result))
			return NVSDK_NGX_Result_FAIL_PlatformError;

		// CreateCommandList
		result = CyberXessContext::instance()->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, CyberXessContext::instance()->Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&CyberXessContext::instance()->Dx12CommandList));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList result: " + int_to_hex(result));

		if (FAILED(result))
			return NVSDK_NGX_Result_FAIL_PlatformError;
	}
	else
	{
		CyberXessContext::instance()->Dx12CommandAllocator->Reset();
		CyberXessContext::instance()->Dx12CommandList->Reset(CyberXessContext::instance()->Dx12CommandAllocator, nullptr);
	}

	auto eResult = NVSDK_NGX_D3D12_EvaluateFeature(CyberXessContext::instance()->Dx12CommandList, InFeatureHandle, InParameters, InCallback);
	LOG("NVSDK_NGX_D3D11_EvaluateFeature result: " + int_to_hex(eResult));

	CyberXessContext::instance()->Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { CyberXessContext::instance()->Dx12CommandList };
	CyberXessContext::instance()->Dx12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	CyberXessContext::instance()->Dx12FenceValueCounter++;
	CyberXessContext::instance()->Dx12Fence->Release();

	return eResult;
}