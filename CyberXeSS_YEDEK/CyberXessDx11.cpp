#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"
//#include "d3dx11tex.h"

static volatile int listCount = 0;



inline void LogCallback(const char* Message, xess_logging_level_t Level)
{
	std::string s = Message;
	LOG("XeSS Runtime (" + std::to_string(Level) + ") : " + s, LEVEL_DEBUG);
}

static bool CreateFeature11(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* handle)
{
	LOG("NVSDK_NGX_D3D11_CreateFeature Start!", LEVEL_INFO);

#pragma region Read XeSS Version

	xess_version_t ver;
	xess_result_t ret = xessGetVersion(&ver);

	if (ret != XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D11_CreateFeature error : " + ResultToString(ret), LEVEL_ERROR);

	char buf[128];
	sprintf_s(buf, "%u.%u.%u", ver.major, ver.minor, ver.patch);

	std::string m_VersionStr = buf;

	LOG("NVSDK_NGX_D3D11_CreateFeature XeSS Version - " + m_VersionStr, LEVEL_WARNING);

#pragma endregion

#pragma region Check for Dx12Device Device

	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		if (InCmdList == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature InCmdList is null!!!", LEVEL_ERROR);
			return false;
		}

		LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device is null trying to get from InCmdList!", LEVEL_WARNING);
		InCmdList->GetDevice(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));

		if (CyberXessContext::instance()->Dx12Device == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device can't receive from InCmdList!", LEVEL_ERROR);
			return false;
		}
		else
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device received from InCmdList!", LEVEL_WARNING);

			CyberXessContext::instance()->Dx12Device->QueryInterface(__uuidof(ID3D12ProxyDevice), (void**)&CyberXessContext::instance()->Dx12ProxyDevice);

			if (CyberXessContext::instance()->Dx12ProxyDevice != nullptr)
				LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice assigned...", LEVEL_DEBUG);
			else
				LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice not assigned...", LEVEL_DEBUG);
		}
	}
	else
		LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device is OK!", LEVEL_DEBUG);

#pragma endregion

#pragma region Check for Dx12ProxyDevice Device
	//if (CyberXessContext::instance()->Dx12ProxyDevice != nullptr)
	//{
	//	LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice proxy adapter disabling spoofing...", LEVEL_DEBUG);
	//	IDXGIProxyAdapter* pAdapter = nullptr;

	//	if (SUCCEEDED(CyberXessContext::instance()->Dx12ProxyDevice->GetProxyAdapter(&pAdapter)) && pAdapter != nullptr)
	//	{
	//		LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice proxy adapter accuired...", LEVEL_DEBUG);
	//		pAdapter->Spoofing(false);
	//		LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice proxy adapter spoofing disabled...", LEVEL_DEBUG);
	//	}
	//	else
	//		LOG("NVSDK_NGX_D3D11_CreateFeature Dx12ProxyDevice proxy adapter is null!!!", LEVEL_DEBUG);
	//}
#pragma endregion

	auto inParams = CyberXessContext::instance()->CreateFeatureParams;
	auto deviceContext = CyberXessContext::instance()->Contexts[handle->Id].get();

	if (deviceContext == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature deviceContext is null!", LEVEL_ERROR);
		return false;
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature deviceContext ok, xessD3D12CreateContext start", LEVEL_DEBUG);

	if (deviceContext->XessContext != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature Destrying old XeSSContext", LEVEL_WARNING);
		ret = xessDestroyContext(deviceContext->XessContext);
		LOG("NVSDK_NGX_D3D11_CreateFeature xessDestroyContext result -> " + ResultToString(ret), LEVEL_WARNING);
	}

	ret = xessD3D12CreateContext(CyberXessContext::instance()->Dx12Device, &deviceContext->XessContext);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12CreateContext result -> " + ResultToString(ret), LEVEL_INFO);

	ret = xessSetLoggingCallback(deviceContext->XessContext, XESS_LOGGING_LEVEL_DEBUG, LogCallback);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessSetLoggingCallback : " + ResultToString(ret), LEVEL_DEBUG);

	ret = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessSetVelocityScale : " + ResultToString(ret), LEVEL_DEBUG);

#pragma region Create Parameters for XeSS

	xess_d3d12_init_params_t initParams{};

	LOG("NVSDK_NGX_D3D11_CreateFeature Params Init!", LEVEL_DEBUG);
	initParams.outputResolution.x = inParams->OutWidth;
	LOG("NVSDK_NGX_D3D11_CreateFeature initParams.outputResolution.x : " + std::to_string(initParams.outputResolution.x), LEVEL_DEBUG);
	initParams.outputResolution.y = inParams->OutHeight;
	LOG("NVSDK_NGX_D3D11_CreateFeature initParams.outputResolution.y : " + std::to_string(initParams.outputResolution.y), LEVEL_DEBUG);

	switch (inParams->PerfQualityValue)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
		initParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		initParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		initParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		initParams.qualitySetting = XESS_QUALITY_SETTING_QUALITY;
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		initParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;
		break;
	default:
		initParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing fsr ultra quality mode
		break;
	}

	initParams.initFlags = XESS_INIT_FLAG_NONE;

	if (CyberXessContext::instance()->MyConfig->DepthInverted.value_or(inParams->DepthInverted))
	{
		initParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
		CyberXessContext::instance()->MyConfig->DepthInverted = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (DepthInverted) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->AutoExposure.value_or(inParams->AutoExposure))
	{
		initParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		CyberXessContext::instance()->MyConfig->AutoExposure = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	else
	{
		initParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (!AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->HDR.value_or(!inParams->Hdr))
	{
		initParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		CyberXessContext::instance()->MyConfig->HDR = false;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (HDR) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->JitterCancellation.value_or(inParams->JitterMotion))
	{
		initParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		CyberXessContext::instance()->MyConfig->JitterCancellation = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (JitterCancellation) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->DisplayResolution.value_or(!inParams->LowRes))
	{
		initParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		CyberXessContext::instance()->MyConfig->DisplayResolution = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (LowRes) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(true))
	{
		initParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (DisableReactiveMask) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature Params done!", LEVEL_DEBUG);

#pragma endregion

#pragma region Build Pipelines

	if (CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true))
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12BuildPipelines start!", LEVEL_DEBUG);

		ret = xessD3D12BuildPipelines(deviceContext->XessContext, NULL, false, initParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12BuildPipelines error : -> " + ResultToString(ret), LEVEL_ERROR);
			return false;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature skipping xessD3D12BuildPipelines!", LEVEL_DEBUG);
	}

#pragma endregion

#pragma region Select Network Model

	auto model = static_cast<xess_network_model_t>(CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0));

	LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel trying to set value to " + std::to_string(model), LEVEL_DEBUG);

	ret = xessSelectNetworkModel(deviceContext->XessContext, model);

	if (ret == XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel set to " + std::to_string(model), LEVEL_DEBUG);
	else
		LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel(" + std::to_string(model) + ") error : " + ResultToString(ret), LEVEL_ERROR);

#pragma endregion


	LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12Init start!", LEVEL_DEBUG);

	ret = xessD3D12Init(deviceContext->XessContext, &initParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12Init error: " + ResultToString(ret), LEVEL_ERROR);
		CyberXessContext::instance()->init = false;
		return false;
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature End!", LEVEL_DEBUG);

	CyberXessContext::instance()->init = true;
	return true;
}

HANDLE CopyTextureFrom11To12(ID3D11Resource* d3d11texture, ID3D11Texture2D** pSharedTexture, bool copyTexture = true, bool save = false)
{
	ID3D11Texture2D* originalTexture = nullptr;
	ID3D11Texture2D* sharedTexture = nullptr;
	HANDLE handle;
	HRESULT result;

	// Get texture
	result = d3d11texture->QueryInterface(IID_PPV_ARGS(&originalTexture));

	if (result != S_OK)
	{
		LOG("CopyTextureFrom11To12 QueryInterface(texture2d) result: " + int_to_hex(result), LEVEL_DEBUG);
		return NULL;
	}

	// Get desc of original texture and create shared desc
	D3D11_TEXTURE2D_DESC desc;
	originalTexture->GetDesc(&desc);

	// if we are lucky maybe it's a shared texture
	if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
	{
		desc.CPUAccessFlags = 0;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

		if (!copyTexture)
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		// Create shared texture
		result = CyberXessContext::instance()->Dx11Device->CreateTexture2D(&desc, nullptr, &sharedTexture);

		if (result != S_OK)
		{
			LOG("CopyTextureFrom11To12 CreateTexture2D result: " + int_to_hex(result), LEVEL_DEBUG);
			return NULL;
		}

		if (copyTexture)
			CyberXessContext::instance()->Dx11DeviceContext->CopyResource(sharedTexture, originalTexture);

		// Query resource
		IDXGIResource1* resource;
		result = sharedTexture->QueryInterface(IID_PPV_ARGS(&resource));

		if (result != S_OK)
		{
			LOG("CopyTextureFrom11To12 QueryInterface(resource) result: " + int_to_hex(result), LEVEL_DEBUG);
			return NULL;
		}

		// Get shared handle
		result = resource->GetSharedHandle(&handle);

		if (result != S_OK)
		{
			LOG("CopyTextureFrom11To12 GetSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NULL;
		}

		resource->Release();

		*pSharedTexture = sharedTexture;
	return handle;
	}
	else
	{
		*pSharedTexture = originalTexture;
		return nullptr;
	}
}

FeatureContext* CreateContext11(NVSDK_NGX_Handle** OutHandle)
{
	auto deviceContext = CyberXessContext::instance()->CreateContext();
	*OutHandle = &deviceContext->Handle;
	return deviceContext;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	LOG("NVSDK_NGX_D3D11_Init_Ext AppId:" + std::to_string(InApplicationId), LEVEL_INFO);
	LOG("NVSDK_NGX_D3D11_Init_Ext SDK:" + std::to_string(InSDKVersion), LEVEL_INFO);

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

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800", LEVEL_WARNING);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	LOG("NVSDK_NGX_D3D11_CreateFeature");

	HRESULT result;

	ID3D11Device* device;
	InDevCtx->GetDevice(&device);

	result = device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11Device));

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature QueryInterface ID3D11Device5 result: " + int_to_hex(result));
		return NVSDK_NGX_Result_FAIL_PlatformError;
	}

#ifdef D3D11on12
	// D3D11on12 Device
	result = CyberXessContext::instance()->Dx11Device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11on12Device));
	LOG("NVSDK_NGX_D3D11_CreateFeature query d3d11on12 result: " + int_to_hex(result));

	if (result != S_OK || CyberXessContext::instance()->Dx11on12Device == nullptr)
		return NVSDK_NGX_Result_FAIL_PlatformError;

	// D3D12 Device
	result = CyberXessContext::instance()->Dx11on12Device->GetD3D12Device(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));
	LOG("NVSDK_NGX_D3D11_CreateFeature query device12 result: " + int_to_hex(result));
#else
	auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();

	result = CyberXessContext::instance()->CreateDx12Device(fl);
#endif // D3D11on12

	if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
		return NVSDK_NGX_Result_FAIL_PlatformError;

	auto cfResult = NVSDK_NGX_D3D12_CreateFeature(nullptr, InFeatureID, InParameters, OutHandle);
	LOG("NVSDK_NGX_D3D11_CreateFeature result: " + int_to_hex(cfResult));

	return cfResult;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	LOG("NVSDK_NGX_D3D11_ReleaseFeature", LEVEL_DEBUG);


	auto cfResult = NVSDK_NGX_D3D12_ReleaseFeature(InHandle);
	LOG("NVSDK_NGX_D3D11_ReleaseFeature result: " + int_to_hex(cfResult));

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
	auto instance = CyberXessContext::instance();

	// No D3D12 device!
	if (instance->Dx12Device == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device device!", LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_PlatformError;
	}

	HRESULT result;

	InDevCtx->GetDevice((ID3D11Device**)&instance->Dx11Device);

	result = InDevCtx->QueryInterface(IID_PPV_ARGS(&instance->Dx11DeviceContext));

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature no ID3D11DeviceContext4 interface!", LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_PlatformError;
	}

#ifdef D3D11on12
	if (instance->Dx12CommandQueue == nullptr)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		//queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

		// CreateCommandQueue
		result = instance->Dx12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&instance->Dx12CommandQueue));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandQueue result: " + int_to_hex(result));

		if (result != S_OK || instance->Dx12CommandQueue == nullptr)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		//	CreateCommandAllocator 
		result = instance->Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&instance->Dx12CommandAllocator));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator result: " + int_to_hex(result));

		if (result != S_OK)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		// CreateCommandList
		result = instance->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, instance->Dx12CommandAllocator, nullptr, IID_PPV_ARGS(&instance->Dx12CommandList));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList result: " + int_to_hex(result));

		if (result != S_OK)
			return NVSDK_NGX_Result_FAIL_PlatformError;
	}
	else
	{
		instance->Dx12CommandAllocator->Reset();
		instance->Dx12CommandList->Reset(instance->Dx12CommandAllocator, nullptr);
	}

	auto eResult = NVSDK_NGX_D3D11_EvaluateFeature(instance->Dx12CommandList, InFeatureHandle, InParameters, InCallback);
	LOG("NVSDK_NGX_D3D11_EvaluateFeature result: " + int_to_hex(eResult));

	instance->Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { instance->Dx12CommandList };
	instance->Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	instance->Dx12FenceValueCounter++;
	instance->Dx12Fence->Release();

	return NVSDK_NGX_Result_Success;
#else
	auto listIndex = listCount % 2;

	// Command allocator & command list
	if (instance->Dx12CommandAllocator[listIndex] == nullptr)
	{
		//	CreateCommandAllocator 
		result = instance->Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&instance->Dx12CommandAllocator[listIndex]));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator result: " + int_to_hex(result));

		if (result != S_OK)
			return NVSDK_NGX_Result_FAIL_PlatformError;

		// CreateCommandList
		result = instance->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, instance->Dx12CommandAllocator[listIndex], nullptr, IID_PPV_ARGS(&instance->Dx12CommandList[listIndex]));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList result: " + int_to_hex(result));
	}

	// get params from dlss
	const auto inParams = static_cast<const NvParameter*>(InParameters);

	// init check
	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature init is false, calling CreateFeature!", LEVEL_WARNING);
		instance->init = CreateFeature11(nullptr, InFeatureHandle);
	}

	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature init still is null CreateFeature failed!", LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	// Get device context
	auto deviceContext = instance->Contexts[InFeatureHandle->Id].get();

	//if (listCount == 1150)
	//{
	//	xess_dump_parameters_t dumpParams = {};
	//	dumpParams.frame_count = 20;
	//	dumpParams.frame_idx = 1;
	//	dumpParams.path = "D:\\dmp\\";
	//	xessStartDump(deviceContext->XessContext, &dumpParams);
	//}

	// Fence for syncing
	ID3D12Fence* d3d12Fence;
	result = instance->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence d3d12fence result: " + int_to_hex(result), LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	HANDLE fenceHandle = NULL;
	result = instance->Dx12Device->CreateSharedHandle(d3d12Fence, NULL, GENERIC_ALL, nullptr, &fenceHandle);

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence fenceHandle result: " + int_to_hex(result), LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	ID3D11Fence* d3d11Fence;
	result = instance->Dx11Device->OpenSharedFence(fenceHandle, IID_PPV_ARGS(&d3d11Fence));

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence d3d11fence result: " + int_to_hex(result), LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	instance->Dx12FenceValueCounter++;

	// creatimg params for XeSS
	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	params.jitterOffsetX = inParams->JitterOffsetX;
	params.jitterOffsetY = inParams->JitterOffsetY;

	params.exposureScale = inParams->ExposureScale;
	params.resetHistory = inParams->ResetRender;

	params.inputWidth = inParams->Width;
	params.inputHeight = inParams->Height;
	LOG("NVSDK_NGX_D3D11_EvaluateFeature inp width: " + std::to_string(inParams->Width) + " height: " + std::to_string(inParams->Height), LEVEL_DEBUG);

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Dx11Device CreateQuery result: " + int_to_hex(result), LEVEL_DEBUG);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}


#pragma region Texture copies

	HANDLE colorHandle = NULL;
	HANDLE mvHandle = NULL;
	HANDLE depthHandle = NULL;
	HANDLE outHandle = NULL;
	HANDLE tmHandle = NULL;
	HANDLE expHandle = NULL;
	ID3D11Texture2D* colorShared = nullptr;
	ID3D11Texture2D* mvShared = nullptr;
	ID3D11Texture2D* depthShared = nullptr;
	ID3D11Texture2D* outShared = nullptr;
	ID3D11Texture2D* tmShared = nullptr;
	ID3D11Texture2D* expShared = nullptr;

	if (inParams->Color != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Color exist..", LEVEL_DEBUG);
		colorHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Color, &colorShared, true, true);

		if (colorHandle == NULL)
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Color not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->MotionVectors)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors exist..", LEVEL_DEBUG);
		mvHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->MotionVectors, &mvShared);

		if (colorHandle == NULL)
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Output)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Output exist..", LEVEL_DEBUG);
		outHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Output, &outShared, false);

		if (outHandle == NULL)
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Output not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Depth && !instance->MyConfig->DisplayResolution.value_or(false))
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth exist..", LEVEL_DEBUG);
		depthHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Depth, &depthShared);

		if (depthHandle == NULL)
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}
	else
	{
		if (!instance->MyConfig->DisplayResolution.value_or(false))
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth not exist!!", LEVEL_ERROR);
		else
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Using high res motion vectors, depth is not needed!!", LEVEL_INFO);

		params.pDepthTexture = nullptr;
	}

	if (!instance->MyConfig->AutoExposure.value_or(false))
	{
		if (inParams->ExposureTexture == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!", LEVEL_WARNING);
			params.pExposureScaleTexture = nullptr;
		}
		else
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature ExposureTexture exist..", LEVEL_DEBUG);
			expHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->ExposureTexture, &expShared, &params.pExposureScaleTexture);

			if (expHandle == NULL)
				return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature AutoExposure enabled!", LEVEL_WARNING);
		params.pExposureScaleTexture = nullptr;
	}

	if (!instance->MyConfig->DisableReactiveMask.value_or(true))
	{
		if (inParams->TransparencyMask != nullptr)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask exist..", LEVEL_INFO);

			tmHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->TransparencyMask, &tmShared);

			if (tmHandle == NULL)
				return NVSDK_NGX_Result_FAIL_InvalidParameter;

		}
		else
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask not exist and its enabled in config, it may cause problems!!", LEVEL_WARNING);
			params.pResponsivePixelMaskTexture = nullptr;
		}
	}
	else
	{
		params.pResponsivePixelMaskTexture = nullptr;
	}

	// Signal #1
	instance->Dx11DeviceContext->Signal(d3d11Fence, instance->Dx12FenceValueCounter); 

	// for fences

	// Wait for Signal #1
	instance->Dx12CommandQueue->Wait(d3d12Fence, instance->Dx12FenceValueCounter);
	instance->Dx12FenceValueCounter++;

	if (inParams->Color)
	{
		result = instance->Dx12Device->OpenSharedHandle(colorHandle, IID_PPV_ARGS(&params.pColorTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Color OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (inParams->MotionVectors)
	{
		result = instance->Dx12Device->OpenSharedHandle(mvHandle, IID_PPV_ARGS(&params.pVelocityTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}


	if (inParams->Output)
	{
		result = instance->Dx12Device->OpenSharedHandle(outHandle, IID_PPV_ARGS(&params.pOutputTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Output OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (inParams->Depth && !instance->MyConfig->DisplayResolution.value_or(false))
	{
		result = instance->Dx12Device->OpenSharedHandle(depthHandle, IID_PPV_ARGS(&params.pDepthTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (!instance->MyConfig->AutoExposure.value_or(false) && inParams->ExposureTexture != nullptr)
	{
		result = instance->Dx12Device->OpenSharedHandle(expHandle, IID_PPV_ARGS(&params.pExposureScaleTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature ExposureTexture OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (!instance->MyConfig->DisableReactiveMask.value_or(true) && inParams->TransparencyMask != nullptr)
	{
		result = instance->Dx12Device->OpenSharedHandle(tmHandle, IID_PPV_ARGS(&params.pResponsivePixelMaskTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

#pragma endregion

	LOG("NVSDK_NGX_D3D11_EvaluateFeature mvscale x: " + std::to_string(inParams->MVScaleX) + " y: " + std::to_string(inParams->MVScaleY), LEVEL_DEBUG);
	xessResult = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature xessSetVelocityScale : " + ResultToString(xessResult), LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	// Transition render targets D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE for XeSS
	std::vector<D3D12_RESOURCE_BARRIER> transitions = {};

	if (params.pColorTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pColorTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	if (params.pVelocityTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pVelocityTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	if (params.pDepthTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pDepthTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	if (params.pExposureScaleTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pExposureScaleTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	if (params.pResponsivePixelMaskTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pResponsivePixelMaskTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	if (params.pOutputTexture != nullptr)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pOutputTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		transitions.push_back(barrier);
	}

	instance->Dx12CommandList[listIndex]->ResourceBarrier((UINT)transitions.size(), transitions.data());

	// Execute xess
	LOG("NVSDK_NGX_D3D11_EvaluateFeature Executing!!", LEVEL_INFO);
	xessResult = xessD3D12Execute(deviceContext->XessContext, instance->Dx12CommandList[listIndex], &params);

	NVSDK_NGX_Result evaluateResult = NVSDK_NGX_Result_FAIL_InvalidParameter;

	if (xessResult != XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D11_EvaluateFeature xessD3D12Execute result: " + ResultToString(xessResult), LEVEL_INFO);
	else
	{
		evaluateResult = NVSDK_NGX_Result_Success;

		// switch back to common
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = params.pOutputTexture;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		instance->Dx12CommandList[listIndex]->ResourceBarrier(1, &barrier);

		// signal for xess complete to d3d11 for copy back - Signal #2
		instance->Dx12CommandQueue->Signal(d3d12Fence, instance->Dx12FenceValueCounter);

		// Execute dx12 commands to process xess
		instance->Dx12CommandList[listIndex]->Close();
		ID3D12CommandList* ppCommandLists[] = { instance->Dx12CommandList[listIndex] };
		instance->Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		// copy back output texture - Signal #2
		instance->Dx11DeviceContext->Wait(d3d11Fence, instance->Dx12FenceValueCounter);

		// copy output back
		instance->Dx11DeviceContext->CopyResource((ID3D11Resource*)inParams->Output, outShared);

		instance->Dx12FenceValueCounter++;

		// Signal #3
		instance->Dx11DeviceContext->Signal(d3d11Fence, instance->Dx12FenceValueCounter);

		// Execute dx11 commands 
		instance->Dx11DeviceContext->Flush();

		// Wait for Signal #3
		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		d3d11Fence->SetEventOnCompletion(instance->Dx12FenceValueCounter, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}


	if (colorShared != nullptr)
	{
		params.pColorTexture->Release();
		colorShared->Release();
		colorShared = nullptr;
	}

	if (mvShared != nullptr)
	{
		params.pVelocityTexture->Release();
		mvShared->Release();
		mvShared = nullptr;
	}

	if (expShared != nullptr)
	{
		params.pExposureScaleTexture->Release();
		expShared->Release();
		expShared = nullptr;
	}

	if (depthShared != nullptr)
	{
		params.pDepthTexture->Release();
		depthShared->Release();
		depthShared = nullptr;
	}

	if (tmShared != nullptr)
	{
		params.pResponsivePixelMaskTexture->Release();
		tmShared->Release();
		tmShared = nullptr;
	}

	if (outShared != nullptr)
	{
		params.pOutputTexture->Release();
		outShared->Release();
		outShared = nullptr;
	}

	d3d12Fence->Release();
	d3d11Fence->Release();
	CloseHandle(fenceHandle);

	instance->Dx12CommandAllocator[listIndex]->Reset();
	instance->Dx12CommandList[listIndex]->Reset(instance->Dx12CommandAllocator[listIndex], nullptr);

	listCount++;

	return evaluateResult;
#endif
}

