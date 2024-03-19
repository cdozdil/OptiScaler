#pragma once
#include "pch.h"

#include <ankerl/unordered_dense.h>
#include <dxgi1_4.h>
#include "imgui/imgui/imgui_impl_dx12.h"

#include "Config.h"
#include "backends/xess/XeSSFeature_Dx12.h"
#include "backends/fsr2/FSR2Feature_Dx12.h"
#include "backends/fsr2_212/FSR2Feature_Dx12_212.h"
#include "NVNGX_Parameter.h"

inline ID3D12Device* D3D12Device = nullptr;
//inline std::unique_ptr<Imgui_Dx12> ImguiDx12 = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx12>> Dx12Contexts;
inline HWND currentHwnd = nullptr;

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext SDK: {0:x}", (int)InSDKVersion);
	std::wstring string(InApplicationDataPath);
	std::string str(string.begin(), string.end());
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext InApplicationDataPath {0}", str);

	//if (InFeatureInfo)
	//	Config::Instance()->NVSDK_Logger = InFeatureInfo->LoggingInfo;

	Config::Instance()->Api = NVNGX_DX12;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D12_Init");
	return NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InEngineType: {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_D3D12_Init_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineType: {0}", (int)InEngineType);

	Config::Instance()->NVNGX_Engine = InEngineType;

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
	{
		spdlog::debug("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	}

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region DLSS Shutdown Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown(void)
{
	spdlog::info("NVSDK_NGX_D3D12_Shutdown");

	for (auto const& [key, val] : Dx12Contexts)
		NVSDK_NGX_D3D12_ReleaseFeature(val->Handle());

	Dx12Contexts.clear();

	D3D12Device = nullptr;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
	spdlog::info("NVSDK_NGX_D3D12_Shutdown1");
	return NVSDK_NGX_D3D12_Shutdown();
}

#pragma endregion

#pragma region DLSS Parameter Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D12_GetParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D12_GetParameters exception: {0}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D12_GetCapabilityParameters");

	try
	{
		*OutParameters = GetNGXParameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D12_GetCapabilityParameters exception: {0}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D12_AllocateParameters");

	try
	{
		*OutParameters = new NVNGX_Parameters();
		return NVSDK_NGX_Result_Success;
	}
	catch (const std::exception& ex)
	{
		spdlog::error("NVSDK_NGX_D3D12_AllocateParameters exception: {0}", ex.what());
		return NVSDK_NGX_Result_Fail;
	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("NVSDK_NGX_D3D12_PopulateParameters_Impl");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	InitNGXParameters(InParameters);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{

	spdlog::debug("NVSDK_NGX_D3D12_DestroyParameters");

	if (InParameters == nullptr)
		return NVSDK_NGX_Result_Fail;

	delete InParameters;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Feature Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	spdlog::info("NVSDK_NGX_D3D12_CreateFeature");

	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't create this feature ({0})!", (int)InFeatureID);
		return NVSDK_NGX_Result_Fail;
	}

	// Create feature
	auto handleId = IFeature::GetNextHandleId();

	// backend selection
	// 0 : XeSS
	// 1 : FSR2.2
	// 2 : FSR2.1
	int upscalerChoice = 0; // Default XeSS

	// ini first
	if (Config::Instance()->Dx12Upscaler.has_value())
	{
		if (Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr22")
			upscalerChoice = 1;
		else if (Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr21")
			upscalerChoice = 2;
		else
			Config::Instance()->Dx12Upscaler = "xess";
	}
	else if (InParameters)
	{
		InParameters->Get("DLSSEnabler.Dx12Backend", &upscalerChoice);
	}

	if (upscalerChoice == 1)
	{
		Config::Instance()->Dx12Upscaler = "fsr22";
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, InParameters);
	}
	else if (upscalerChoice == 2)
	{
		Config::Instance()->Dx12Upscaler = "fsr21";
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, InParameters);
	}
	else
	{
		Config::Instance()->Dx12Upscaler = "xess";
		Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, InParameters);
	}

	// nvsdk logging
	// ini first
	if (!Config::Instance()->LogToNGX.has_value())
	{
		int nvsdkLogging = 0;
		InParameters->Get("DLSSEnabler.Logging", &nvsdkLogging);

		Config::Instance()->LogToNGX = nvsdkLogging > 0;
	}

	auto deviceContext = Dx12Contexts[handleId].get();
	*OutHandle = deviceContext->Handle();

#pragma region Check for Dx12Device Device

	if (!D3D12Device)
	{
		spdlog::debug("NVSDK_NGX_D3D12_CreateFeature Get D3d12 device from InCmdList!");
		auto deviceResult = InCmdList->GetDevice(IID_PPV_ARGS(&D3D12Device));

		if (deviceResult != S_OK || !D3D12Device)
		{
			spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't get Dx12Device from InCmdList!");
			return NVSDK_NGX_Result_Fail;
		}
	}

#pragma endregion

	if (deviceContext->Init(D3D12Device, InParameters))
	{
		//if (ImguiDx12)
		//	ImguiDx12.reset();

		return NVSDK_NGX_Result_Success;
	}

	spdlog::error("NVSDK_NGX_D3D12_CreateFeature: CreateFeature failed");
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature");

	if (!InHandle)
		return NVSDK_NGX_Result_Success;

	auto handleId = InHandle->Id;

	if (auto deviceContext = Dx12Contexts[handleId].get(); deviceContext)
	{
		Dx12Contexts[handleId].reset();
		auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
		Dx12Contexts.erase(it);
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	NVSDK_NGX_FeatureRequirement* OutSupported)
{
	spdlog::debug("NVSDK_NGX_D3D12_GetFeatureRequirements!");

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;

	//Some old windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature init!");

	if (!InCmdList)
	{
		spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	if (InCallback)
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature callback exist");

	IFeature_Dx12* deviceContext = nullptr;

	if (Config::Instance()->changeBackend)
	{
		// first release everything
		auto handleId = InFeatureHandle->Id;

		if (auto dc = Dx12Contexts[handleId].get(); dc)
		{
			Dx12Contexts[handleId].reset();
			auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx12Contexts.erase(it);

			//if (ImguiDx12)
			//	ImguiDx12.reset();

			return NVSDK_NGX_Result_Success;
		}

		// next frame prepare stuff
		if (Config::Instance()->newBackend == "fsr22")
		{
			Config::Instance()->Dx12Upscaler = "fsr22";
			Dx12Contexts[InFeatureHandle->Id] = std::make_unique<FSR2FeatureDx12>(InFeatureHandle->Id, InParameters);
		}
		else if (Config::Instance()->newBackend == "fsr21")
		{
			Config::Instance()->Dx12Upscaler = "fsr21";
			Dx12Contexts[InFeatureHandle->Id] = std::make_unique<FSR2FeatureDx12_212>(InFeatureHandle->Id, InParameters);
		}
		else
		{
			Config::Instance()->Dx12Upscaler = "xess";
			Dx12Contexts[InFeatureHandle->Id] = std::make_unique<XeSSFeatureDx12>(InFeatureHandle->Id, InParameters);
		}

		if (Dx12Contexts[InFeatureHandle->Id].get()->Init(D3D12Device, InParameters))
		{
			// next frame we will start rendering again
			Config::Instance()->changeBackend = false;
			return NVSDK_NGX_Result_Success;
		}
		else
			return NVSDK_NGX_Result_Fail;
	}

	deviceContext = Dx12Contexts[InFeatureHandle->Id].get();

	if (deviceContext->Evaluate(InCmdList, InParameters))
	{
		//if (ImguiDx12 == nullptr)
		//{
		//	if (currentHwnd == nullptr)
		//		currentHwnd = Util::GetProcessWindow();

		//	if (currentHwnd)
		//		ImguiDx12 = std::make_unique<Imgui_Dx12>(currentHwnd, D3D12Device);
		//}

		//if (ImguiDx12)
		//{
		//	ID3D12Resource* out;
		//	InParameters->Get(NVSDK_NGX_Parameter_Output, &out);

		//	ImguiDx12->Render(InCmdList, out);
		//}

		return NVSDK_NGX_Result_Success;
	}
	else
		return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_UpdateFeature(const NVSDK_NGX_Application_Identifier* ApplicationId, const NVSDK_NGX_Feature FeatureID)
{
	spdlog::debug("NVSDK_NGX_UpdateFeature -> {0}", (int)FeatureID);
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::warn("NVSDK_NGX_D3D12_GetScratchBufferSize -> 52428800");
	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

