#pragma once
#include "pch.h"

#include <ankerl/unordered_dense.h>
#include "Config.h"

#include "backends/xess/XeSSFeature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11On12.h"
#include "backends/fsr2_212/FSR2Feature_Dx11On12_212.h"
#include "NVNGX_Parameter.h"

inline ID3D11Device* D3D11Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx11>> Dx11Contexts;

#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext SDK: {0:x}", (int)InSDKVersion);
	std::wstring string(InApplicationDataPath);
	std::string str(string.begin(), string.end());
	spdlog::debug("NVSDK_NGX_D3D11_Init_Ext InApplicationDataPath {0}", str);

	if (InDevice)
		D3D11Device = InDevice;

	//if (InFeatureInfo)
	//	Config::Instance()->NVSDK_Logger = InFeatureInfo->LoggingInfo;

	Config::Instance()->Api = NVNGX_DX11;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init");
	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID InEngineType: {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_D3D11_Init_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D11_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::debug("NVSDK_NGX_D3D11_Init_with_ProjectID InEngineType: {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_D3D11_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown(void)
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown");

	for (auto const& [key, val] : Dx11Contexts)
		NVSDK_NGX_D3D11_ReleaseFeature(val->Handle());

	Dx11Contexts.clear();
	D3D11Device = nullptr;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown1");
	return NVSDK_NGX_D3D11_Shutdown();
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_GetParameters exception: {0}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetCapabilityParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_GetCapabilityParameters exception: {0}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_AllocateParameters");

	try
	{
		*OutParameters = new NVNGX_Parameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D11_AllocateParameters exception: {0}", ex.what());
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

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_DestroyParameters");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	delete InParameters;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::warn("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800");
	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Feature

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_D3D11_CreateFeature");

	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_D3D11_CreateFeature Can't create this feature ({0})!", (int)InFeatureID);
		return NVSDK_NGX_Result_Fail;
	}

	// Create feature
	auto handleId = IFeature::GetNextHandleId();

	if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "xess")
		Dx11Contexts[handleId] = std::make_unique<XeSSFeatureDx11>(handleId, InParameters);
	else if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22_12")
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12>(handleId, InParameters);
	else if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr21_12")
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12_212>(handleId, InParameters);
	else
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11>(handleId, InParameters);

	auto deviceContext = Dx11Contexts[handleId].get();
	*OutHandle = deviceContext->Handle();

	if (!D3D11Device)
	{
		spdlog::debug("NVSDK_NGX_D3D12_CreateFeature Get Dx11Device from InDevCtx!");
		InDevCtx->GetDevice(&D3D11Device);

		if (!D3D11Device)
		{
			spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't get Dx11Device from InDevCtx!");
			return NVSDK_NGX_Result_Fail;
		}
	}

	if (deviceContext->Init(D3D11Device, InDevCtx, InParameters))
		return NVSDK_NGX_Result_Success;

	spdlog::error("NVSDK_NGX_D3D11_CreateFeature: CreateFeature failed");
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	spdlog::info("NVSDK_NGX_D3D11_ReleaseFeature");

	if (!InHandle)
		return NVSDK_NGX_Result_Success;

	auto handleId = InHandle->Id;

	if (auto deviceContext = Dx11Contexts[handleId].get(); deviceContext)
	{
		Dx11Contexts[handleId].reset();
		auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
		Dx11Contexts.erase(it);
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetFeatureRequirements");

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;

	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature Handle: {0}", InFeatureHandle->Id);

	if (!InDevCtx)
	{
		spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		spdlog::warn("NVSDK_NGX_D3D11_EvaluateFeature callback exist");

	IFeature_Dx11* deviceContext = nullptr;

	if (Config::Instance()->changeBackend)
	{
		// first release everything
		auto handleId = InFeatureHandle->Id;

		if (auto dc = Dx11Contexts[handleId].get(); dc)
		{
			Dx11Contexts[handleId].reset();
			auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx11Contexts.erase(it);
		}

		// next prepare stuff
		if (Config::Instance()->newBackend == "xess")
		{
			Config::Instance()->Dx11Upscaler = "xess";
			Dx11Contexts[InFeatureHandle->Id] = std::make_unique<XeSSFeatureDx11>(InFeatureHandle->Id, InParameters);
		}
		else if (Config::Instance()->newBackend == "fsr21_12")
		{
			Config::Instance()->Dx11Upscaler = "fsr21_12";
			Dx11Contexts[InFeatureHandle->Id] = std::make_unique<FSR2FeatureDx11on12_212>(InFeatureHandle->Id, InParameters);
		}
		else if (Config::Instance()->newBackend == "fsr22_12")
		{
			Config::Instance()->Dx11Upscaler = "fsr22_12";
			Dx11Contexts[InFeatureHandle->Id] = std::make_unique<FSR2FeatureDx11on12>(InFeatureHandle->Id, InParameters);
		}
		else
		{
			Config::Instance()->Dx11Upscaler = "fsr22";
			Dx11Contexts[InFeatureHandle->Id] = std::make_unique<FSR2FeatureDx11>(InFeatureHandle->Id, InParameters);
		}

		if (Dx11Contexts[InFeatureHandle->Id].get()->Init(D3D11Device, InDevCtx, InParameters))
		{
			Config::Instance()->changeBackend = false;
			//return NVSDK_NGX_Result_Success;
		}
		else
			return NVSDK_NGX_Result_Fail;
	}

	deviceContext = Dx11Contexts[InFeatureHandle->Id].get();
	Config::Instance()->CurrentFeature = deviceContext;

	if (deviceContext == nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
		return NVSDK_NGX_Result_Success;
	}

	NVSDK_NGX_Result evResult = NVSDK_NGX_Result_Success;
	if (!deviceContext->Evaluate(InDevCtx, InParameters))
		evResult = NVSDK_NGX_Result_Fail;

	return evResult;
}

#pragma endregion
