#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"

inline void LogCallback(const char* Message, xess_logging_level_t Level)
{
	std::string s = Message;
	LOG("XeSS Runtime (" + std::to_string(Level) + ") : " + s, LEVEL_DEBUG);
}

static bool CreateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* handle)
{
	LOG("NVSDK_NGX_D3D12_CreateFeature Start!", LEVEL_INFO);

#pragma region Read XeSS Version

	xess_version_t ver;
	xess_result_t ret = xessGetVersion(&ver);

	if (ret != XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D12_CreateFeature error : " + ResultToString(ret), LEVEL_ERROR);

	char buf[128];
	sprintf_s(buf, "%u.%u.%u", ver.major, ver.minor, ver.patch);

	std::string m_VersionStr = buf;

	LOG("NVSDK_NGX_D3D12_CreateFeature XeSS Version - " + m_VersionStr, LEVEL_WARNING);

#pragma endregion

#pragma region Check for Dx12Device Device

	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		if (InCmdList == nullptr && CyberXessContext::instance()->Dx11Device != nullptr)
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature InCmdList is null!!!", LEVEL_ERROR);
			auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
			CyberXessContext::instance()->CreateDx12Device(fl);
		}
		else
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature CyberXessContext::instance()->Dx12Device is null trying to get from InCmdList!", LEVEL_WARNING);
			InCmdList->GetDevice(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));
		}

		if (CyberXessContext::instance()->Dx12Device == nullptr)
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature CyberXessContext::instance()->Dx12Device can't receive from InCmdList!", LEVEL_ERROR);
			return false;
		}
	}
	else
		LOG("NVSDK_NGX_D3D12_CreateFeature CyberXessContext::instance()->Dx12Device is OK!", LEVEL_DEBUG);

#pragma endregion

#pragma region Check for Dx12ProxyDevice Device
	//if (CyberXessContext::instance()->Dx12ProxyDevice != nullptr)
	//{
	//	LOG("NVSDK_NGX_D3D12_CreateFeature Dx12ProxyDevice proxy adapter disabling spoofing...", LEVEL_DEBUG);
	//	IDXGIProxyAdapter* pAdapter = nullptr;

	//	if (SUCCEEDED(CyberXessContext::instance()->Dx12ProxyDevice->GetProxyAdapter(&pAdapter)) && pAdapter != nullptr)
	//	{
	//		LOG("NVSDK_NGX_D3D12_CreateFeature Dx12ProxyDevice proxy adapter accuired...", LEVEL_DEBUG);
	//		pAdapter->Spoofing(false);
	//		LOG("NVSDK_NGX_D3D12_CreateFeature Dx12ProxyDevice proxy adapter spoofing disabled...", LEVEL_DEBUG);
	//	}
	//	else
	//		LOG("NVSDK_NGX_D3D12_CreateFeature Dx12ProxyDevice proxy adapter is null!!!", LEVEL_DEBUG);
	//}
#pragma endregion

	auto inParams = CyberXessContext::instance()->CreateFeatureParams;
	auto deviceContext = CyberXessContext::instance()->Contexts[handle->Id].get();

	if (deviceContext == nullptr)
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature deviceContext is null!", LEVEL_ERROR);
		return false;
	}

	LOG("NVSDK_NGX_D3D12_CreateFeature deviceContext ok, xessD3D12CreateContext start", LEVEL_DEBUG);

	if (deviceContext->XessContext != nullptr)
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature Destrying old XeSSContext", LEVEL_WARNING);
		ret = xessDestroyContext(deviceContext->XessContext);
		LOG("NVSDK_NGX_D3D12_CreateFeature xessDestroyContext result -> " + ResultToString(ret), LEVEL_WARNING);
	}

	if (CyberXessContext::instance()->Dx12Device == nullptr && CyberXessContext::instance()->Dx11Device != nullptr)
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature InCmdList is null!!!", LEVEL_ERROR);
		auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
		CyberXessContext::instance()->CreateDx12Device(fl);
	}

	ret = xessD3D12CreateContext(CyberXessContext::instance()->Dx12Device, &deviceContext->XessContext);
	LOG("NVSDK_NGX_D3D12_CreateFeature xessD3D12CreateContext result -> " + ResultToString(ret), LEVEL_INFO);

	ret = xessIsOptimalDriver(deviceContext->XessContext);
	LOG("NVSDK_NGX_D3D12_CreateFeature xessIsOptimalDriver : " + ResultToString(ret), LEVEL_DEBUG);

	ret = xessSetLoggingCallback(deviceContext->XessContext, XESS_LOGGING_LEVEL_DEBUG, LogCallback);
	LOG("NVSDK_NGX_D3D12_CreateFeature xessSetLoggingCallback : " + ResultToString(ret), LEVEL_DEBUG);

	ret = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);
	LOG("NVSDK_NGX_D3D12_CreateFeature xessSetVelocityScale : " + ResultToString(ret), LEVEL_DEBUG);

#pragma region Create Parameters for XeSS

	xess_d3d12_init_params_t initParams{};

	LOG("NVSDK_NGX_D3D12_CreateFeature Params Init!", LEVEL_DEBUG);
	initParams.outputResolution.x = inParams->OutWidth;
	LOG("NVSDK_NGX_D3D12_CreateFeature initParams.outputResolution.x : " + std::to_string(initParams.outputResolution.x), LEVEL_DEBUG);
	initParams.outputResolution.y = inParams->OutHeight;
	LOG("NVSDK_NGX_D3D12_CreateFeature initParams.outputResolution.y : " + std::to_string(initParams.outputResolution.y), LEVEL_DEBUG);

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
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (DepthInverted) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->AutoExposure.value_or(inParams->AutoExposure))
	{
		initParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		CyberXessContext::instance()->MyConfig->AutoExposure = true;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	else
	{
		initParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (!AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->HDR.value_or(!inParams->Hdr))
	{
		initParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		CyberXessContext::instance()->MyConfig->HDR = false;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (HDR) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->JitterCancellation.value_or(inParams->JitterMotion))
	{
		initParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		CyberXessContext::instance()->MyConfig->JitterCancellation = true;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (JitterCancellation) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->DisplayResolution.value_or(!inParams->LowRes))
	{
		initParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		CyberXessContext::instance()->MyConfig->DisplayResolution = true;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (LowRes) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(true))
	{
		initParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		LOG("NVSDK_NGX_D3D12_CreateFeature initParams.initFlags (DisableReactiveMask) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	LOG("NVSDK_NGX_D3D12_CreateFeature Params done!", LEVEL_DEBUG);

#pragma endregion

#pragma region Build Pipelines

	if (CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true))
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature xessD3D12BuildPipelines start!", LEVEL_DEBUG);

		ret = xessD3D12BuildPipelines(deviceContext->XessContext, NULL, true, initParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature xessD3D12BuildPipelines error : -> " + ResultToString(ret), LEVEL_ERROR);
			return false;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature skipping xessD3D12BuildPipelines!", LEVEL_DEBUG);
	}

#pragma endregion

#pragma region Select Network Model

	auto model = static_cast<xess_network_model_t>(CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0));

	LOG("NVSDK_NGX_D3D12_CreateFeature xessSelectNetworkModel trying to set value to " + std::to_string(model), LEVEL_DEBUG);

	ret = xessSelectNetworkModel(deviceContext->XessContext, model);

	if (ret == XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D12_CreateFeature xessSelectNetworkModel set to " + std::to_string(model), LEVEL_DEBUG);
	else
		LOG("NVSDK_NGX_D3D12_CreateFeature xessSelectNetworkModel(" + std::to_string(model) + ") error : " + ResultToString(ret), LEVEL_ERROR);

#pragma endregion

	LOG("NVSDK_NGX_D3D12_CreateFeature xessD3D12Init start!", LEVEL_DEBUG);

	ret = xessD3D12Init(deviceContext->XessContext, &initParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D12_CreateFeature xessD3D12Init error: " + ResultToString(ret), LEVEL_ERROR);
		CyberXessContext::instance()->init = false;
		return false;
	}

	LOG("NVSDK_NGX_D3D12_CreateFeature End!", LEVEL_DEBUG);

	CyberXessContext::instance()->init = true;

	return true;
}

FeatureContext* CreateContext(NVSDK_NGX_Handle** OutHandle)
{
	auto deviceContext = CyberXessContext::instance()->CreateContext();
	*OutHandle = &deviceContext->Handle;
	return deviceContext;
}

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{

	LOG("XeSS DelayedInit : " + std::to_string(CyberXessContext::instance()->MyConfig->DelayedInit.value_or(false)), LEVEL_INFO);
	LOG("XeSS BuildPipelines : " + std::to_string(CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true)), LEVEL_INFO);
	LOG("XeSS NetworkModel : " + std::to_string(CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0)), LEVEL_INFO);
	LOG("XeSS LogFile : " + CyberXessContext::instance()->MyConfig->LogFile.value_or(""), LEVEL_INFO);
	LOG("XeSS LogLevel : " + std::to_string(CyberXessContext::instance()->MyConfig->LogLevel.value_or(1)), LEVEL_INFO);
	LOG("XeSS XeSSLogging : " + std::to_string(CyberXessContext::instance()->MyConfig->XeSSLogging.value_or(true)), LEVEL_INFO);

	LOG("NVSDK_NGX_D3D12_Init_Ext AppId:" + std::to_string(InApplicationId), LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D12_Init_Ext SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	CyberXessContext::instance()->init = false;
	CyberXessContext::instance()->Shutdown(true, true);

	if (InDevice)
	{
		CyberXessContext::instance()->Dx12Device = InDevice;
		LOG("NVSDK_NGX_D3D12_Init_Ext Dx12Device assigned...", LEVEL_DEBUG);
	}
	else
		LOG("NVSDK_NGX_D3D12_Init_Ext InDevice is null!!!!", LEVEL_ERROR);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D12_Init AppId:" + std::to_string(InApplicationId), LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D12_Init SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D12_Init_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D12_Init_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D12_Init_with_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D12_Init_with_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region DLSS Shutdown Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown(void)
{
	LOG("NVSDK_NGX_D3D12_Shutdown", LEVEL_DEBUG);

	CyberXessContext::instance()->Shutdown(false, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

	for (auto const& [key, val] : CyberXessContext::instance()->Contexts) {
		NVSDK_NGX_D3D12_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
	LOG("NVSDK_NGX_D3D12_Shutdown1", LEVEL_DEBUG);

	CyberXessContext::instance()->Shutdown(false, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();
	CyberXessContext::instance()->Contexts.clear();

	for (auto const& [key, val] : CyberXessContext::instance()->Contexts) {
		NVSDK_NGX_D3D12_ReleaseFeature(&val->Handle);
	}

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Parameter Calls

//currently it's kind of hack but better than what it was previously -- External Memory Tracking
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_GetParameters", LEVEL_DEBUG);

	*OutParameters = CyberXessContext::instance()->NvParameterInstance->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation 
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_GetCapabilityParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_AllocateParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	LOG("NVSDK_NGX_D3D12_DestroyParameters", LEVEL_DEBUG);

	NvParameter::instance()->DeleteParameters((NvParameter*)InParameters);
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Feature Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	auto context = CreateContext(OutHandle);
	CyberXessContext::instance()->CreateFeatureParams = static_cast<const NvParameter*>(InParameters);

	if (CyberXessContext::instance()->MyConfig->DelayedInit.value_or(false))
		return NVSDK_NGX_Result_Success;

	if (CreateFeature(InCmdList, &context->Handle))
		return NVSDK_NGX_Result_Success;

	LOG("NVSDK_NGX_D3D12_CreateFeature: CreateFeature failed", LEVEL_ERROR);

	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	LOG("NVSDK_NGX_D3D12_ReleaseFeature!", LEVEL_DEBUG);

	if (auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get(); deviceContext->XessContext != nullptr)
	{
		auto result = xessDestroyContext(deviceContext->XessContext);
		deviceContext->XessContext = nullptr;
		LOG("NVSDK_NGX_D3D11_ReleaseFeature: xessDestroyContext result: " + ResultToString(result), LEVEL_DEBUG);
	}

	CyberXessContext::instance()->DeleteContext(InHandle);

	if (CyberXessContext::instance()->Contexts.empty())
		CyberXessContext::instance()->Shutdown(false, true);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	NVSDK_NGX_FeatureRequirement* OutSupported)
{
	LOG("NVSDK_NGX_D3D12_GetFeatureRequirements!", LEVEL_DEBUG);

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.19045.2728");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	LOG("NVSDK_NGX_D3D12_EvaluateFeature init!", LEVEL_DEBUG);

	if (!InCmdList)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		LOG("NVSDK_NGX_D3D12_EvaluateFeature callback exist", LEVEL_WARNING);

	const auto inParams = static_cast<const NvParameter*>(InParameters);
	const auto instance = CyberXessContext::instance();

	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature init is false, calling CreateFeature!", LEVEL_WARNING);
		instance->init = CreateFeature(InCmdList, InFeatureHandle);
	}

	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature init still is null CreateFeature failed!", LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	auto deviceContext = instance->Contexts[InFeatureHandle->Id].get();

	//dumpParams.frame_count = 1;
	//dumpParams.frame_idx = cnt++;
	//dumpParams.path = "D:\\dmp\\";
	//xessStartDump(deviceContext->XessContext, &dumpParams);

	// creatimg params for XeSS
	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	params.jitterOffsetX = inParams->JitterOffsetX;
	params.jitterOffsetY = inParams->JitterOffsetY;

	params.exposureScale = inParams->ExposureScale;
	params.resetHistory = inParams->ResetRender;

	params.inputWidth = inParams->Width;
	params.inputHeight = inParams->Height;

	LOG("NVSDK_NGX_D3D12_EvaluateFeature inp width: " + std::to_string(inParams->Width) + " height: " + std::to_string(inParams->Height), LEVEL_DEBUG);

	if (inParams->Color != nullptr)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature Color exist..", LEVEL_DEBUG);
		params.pColorTexture = (ID3D12Resource*)inParams->Color;
	}
	else
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature Color not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->MotionVectors)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature MotionVectors exist..", LEVEL_DEBUG);
		params.pVelocityTexture = (ID3D12Resource*)inParams->MotionVectors;
	}
	else
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature MotionVectors not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Output)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature Output exist..", LEVEL_DEBUG);
		params.pOutputTexture = (ID3D12Resource*)inParams->Output;
	}
	else
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature Output not exist!!", LEVEL_ERROR);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Depth)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature Depth exist..", LEVEL_INFO);
		params.pDepthTexture = (ID3D12Resource*)inParams->Depth;
	}
	else
	{
		if (!instance->MyConfig->DisplayResolution.value_or(false))
			LOG("NVSDK_NGX_D3D12_EvaluateFeature Depth not exist!!", LEVEL_ERROR);
		else
			LOG("NVSDK_NGX_D3D12_EvaluateFeature Using high res motion vectors, depth is not needed!!", LEVEL_INFO);

		params.pDepthTexture = nullptr;
	}

	if (!instance->MyConfig->AutoExposure.value_or(false))
	{
		if (inParams->ExposureTexture == nullptr)
		{
			LOG("NVSDK_NGX_D3D12_EvaluateFeature AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!", LEVEL_WARNING);
			params.pExposureScaleTexture = nullptr;
		}
		else
		{
			LOG("NVSDK_NGX_D3D12_EvaluateFeature ExposureTexture exist..", LEVEL_INFO);
			params.pExposureScaleTexture = (ID3D12Resource*)inParams->ExposureTexture;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature AutoExposure enabled!", LEVEL_WARNING);
		params.pExposureScaleTexture = nullptr;
	}

	if (!instance->MyConfig->DisableReactiveMask.value_or(true))
	{
		if (inParams->TransparencyMask != nullptr)
		{
			LOG("NVSDK_NGX_D3D12_EvaluateFeature TransparencyMask exist..", LEVEL_INFO);
			params.pResponsivePixelMaskTexture = (ID3D12Resource*)inParams->TransparencyMask;
		}
		else
		{
			LOG("NVSDK_NGX_D3D12_EvaluateFeature TransparencyMask not exist and its enabled in config, it may cause problems!!", LEVEL_WARNING);
			params.pResponsivePixelMaskTexture = nullptr;
		}
	}
	else
	{
		params.pResponsivePixelMaskTexture = nullptr;
	}

	LOG("NVSDK_NGX_D3D12_EvaluateFeature mvscale x: " + std::to_string(inParams->MVScaleX) + " y: " + std::to_string(inParams->MVScaleY), LEVEL_DEBUG);
	xessResult = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature xessSetVelocityScale : " + ResultToString(xessResult), LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	LOG("NVSDK_NGX_D3D12_EvaluateFeature Executing!!", LEVEL_INFO);
	xessResult = xessD3D12Execute(deviceContext->XessContext, InCmdList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		LOG("xessD3D12Execute error : -> " + ResultToString(xessResult), LEVEL_ERROR);
		return NVSDK_NGX_Result_Fail;
	}

	LOG("NVSDK_NGX_D3D12_EvaluateFeature End!", LEVEL_DEBUG);
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID)
{
	LOG("NVSDK_NGX_UpdateFeature -> " + std::to_string(FeatureID), LEVEL_DEBUG);
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D12_GetScratchBufferSize -> 52428800", LEVEL_WARNING);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

