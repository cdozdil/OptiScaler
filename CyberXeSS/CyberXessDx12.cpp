#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	LOG("NVSDK_NGX_D3D12_Init_Ext AppId:" + std::to_string(InApplicationId), spdlog::level::info);
	LOG("NVSDK_NGX_D3D12_Init_Ext SDK:" + std::to_string(InSDKVersion), spdlog::level::info);

	LOG("NVSDK_NGX_D3D12_Init_Ext DelayedInit: " + std::to_string(CyberXessContext::instance()->MyConfig->DelayedInit.value_or(false)), spdlog::level::info);
	LOG("NVSDK_NGX_D3D12_Init_Ext BuildPipelines: " + std::to_string(CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true)), spdlog::level::info); 
	LOG("NVSDK_NGX_D3D12_Init_Ext NetworkModel: " + std::to_string(CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0)), spdlog::level::info);
	LOG("NVSDK_NGX_D3D12_Init_Ext LogLevel: " + std::to_string(CyberXessContext::instance()->MyConfig->LogLevel.value_or(1)), spdlog::level::info);

	CyberXessContext::instance()->Shutdown(true, true);

	if (InDevice)
		CyberXessContext::instance()->Dx12Device = InDevice;
	else
		LOG("NVSDK_NGX_D3D12_Init_Ext InDevice is null!!!!", spdlog::level::err); 

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D12_Init AppId:" + std::to_string(InApplicationId), spdlog::level::debug);
	LOG("NVSDK_NGX_D3D12_Init SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D12_Init_ProjectID : " + pId, spdlog::level::debug);
	LOG("NVSDK_NGX_D3D12_Init_ProjectID SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D12_Init_with_ProjectID : " + pId, spdlog::level::debug);
	LOG("NVSDK_NGX_D3D12_Init_with_ProjectID SDK:" + std::to_string(InSDKVersion), spdlog::level::debug);

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region DLSS Shutdown Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown(void)
{
	LOG("NVSDK_NGX_D3D12_Shutdown", spdlog::level::info);

	CyberXessContext::instance()->Shutdown(false, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

	for (auto const& [key, val] : CyberXessContext::instance()->Contexts)
	{
		NVSDK_NGX_D3D12_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
	LOG("NVSDK_NGX_D3D12_Shutdown1", spdlog::level::info);

	CyberXessContext::instance()->Shutdown(false, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

	for (auto const& [key, val] : CyberXessContext::instance()->Contexts)
	{
		NVSDK_NGX_D3D12_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Parameter Calls

//currently it's kind of hack but better than what it was previously -- External Memory Tracking
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_GetParameters", spdlog::level::debug);

	*OutParameters = CyberXessContext::instance()->NvParameterInstance->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation 
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_GetCapabilityParameters", spdlog::level::debug);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D12_AllocateParameters", spdlog::level::debug);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	
	LOG("NVSDK_NGX_D3D12_DestroyParameters", spdlog::level::debug);
	NvParameter::instance()->DeleteParameters((NvParameter*)InParameters);
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Feature Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	LOG("NVSDK_NGX_D3D12_CreateFeature", spdlog::level::info);

	auto deviceContext = CyberXessContext::instance()->CreateContext();
	*OutHandle = &deviceContext->Handle;

	CyberXessContext::instance()->CreateFeatureParams = static_cast<const NvParameter*>(InParameters);

#pragma region Check for Dx12Device Device

	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		if (InCmdList == nullptr && CyberXessContext::instance()->Dx11Device != nullptr)
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature InCmdList is null!!!", spdlog::level::err);
			auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
			CyberXessContext::instance()->CreateDx12Device(fl);
		}
		else
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature CyberXessContext::instance()->Dx12Device is null trying to get from InCmdList!", spdlog::level::warn); 
			InCmdList->GetDevice(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));
		}

		if (CyberXessContext::instance()->Dx12Device == nullptr)
		{
			LOG("NVSDK_NGX_D3D12_CreateFeature CyberXessContext::instance()->Dx12Device can't receive from InCmdList!", spdlog::level::err);
			return NVSDK_NGX_Result_Fail;
		}
	}

#pragma endregion


	if (deviceContext->XeSSInit(CyberXessContext::instance()->Dx12Device, CyberXessContext::instance()->CreateFeatureParams))
		return NVSDK_NGX_Result_Success;

	LOG("NVSDK_NGX_D3D12_CreateFeature: CreateFeature failed", spdlog::level::err);
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	LOG("NVSDK_NGX_D3D12_ReleaseFeature", spdlog::level::info);

	if (InHandle == nullptr || InHandle == NULL)
		return NVSDK_NGX_Result_Success;

	if (auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get(); deviceContext != nullptr)
		deviceContext->XeSSDestroy();

	CyberXessContext::instance()->DeleteContext(InHandle);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	NVSDK_NGX_FeatureRequirement* OutSupported)
{
	LOG("NVSDK_NGX_D3D12_GetFeatureRequirements!", spdlog::level::debug);

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.19045.2728");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	LOG("NVSDK_NGX_D3D12_EvaluateFeature init!", spdlog::level::debug);


	if (!InCmdList)
	{
		LOG("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!", spdlog::level::err);
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		LOG("NVSDK_NGX_D3D12_EvaluateFeature callback exist", spdlog::level::warn);

	const auto inParams = static_cast<const NvParameter*>(InParameters);
	const auto deviceContext = CyberXessContext::instance()->Contexts[InFeatureHandle->Id].get();

	if (!deviceContext->XeSSIsInited())
	{
		deviceContext->XeSSInit(CyberXessContext::instance()->Dx12Device, CyberXessContext::instance()->CreateFeatureParams);

		if (!deviceContext->XeSSIsInited())
		{
			LOG("NVSDK_NGX_D3D12_EvaluateFeature deviceContext.XessInit is false, init failed!", spdlog::level::err);
			return NVSDK_NGX_Result_Fail;
		}
	}

	//dumpParams.frame_count = 1;
	//dumpParams.frame_idx = cnt++;
	//dumpParams.path = "D:\\dmp\\";
	//xessStartDump(deviceContext->XessContext, &dumpParams);

	if (deviceContext->XeSSExecuteDx12(InCmdList, inParams))
		return NVSDK_NGX_Result_Success;
	else
		return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID)
{
	LOG("NVSDK_NGX_UpdateFeature -> " + std::to_string(FeatureID), spdlog::level::debug);
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D12_GetScratchBufferSize -> 52428800", spdlog::level::warn);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

