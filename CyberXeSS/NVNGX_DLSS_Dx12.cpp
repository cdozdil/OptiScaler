#pragma once
#include "pch.h"

#include <ankerl/unordered_dense.h>
#include <dxgi1_4.h>
#include "imgui/imgui/imgui_impl_dx12.h"
#include "detours/detours.h"

#include "Config.h"
#include "backends/xess/XeSSFeature_Dx12.h"
#include "backends/fsr2/FSR2Feature_Dx12.h"
#include "backends/fsr2_212/FSR2Feature_Dx12_212.h"
#include "NVNGX_Parameter.h"

typedef void(__fastcall* PFN_SetComputeRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);

static inline PFN_SetComputeRootSignature orgSetComputeRootSignature = nullptr;
static inline PFN_SetComputeRootSignature orgSetGraphicRootSignature = nullptr;
static inline ID3D12RootSignature* rootSigCompute = nullptr;
static inline ID3D12RootSignature* rootSigGraphic = nullptr;
static inline bool contextRendering = false;
static inline ULONGLONG computeTime = 0;
static inline ULONGLONG graphTime = 0;
static inline ULONGLONG lastEvalTime = 0;
static inline NVSDK_NGX_Parameter* createParams;
static inline int changeBackendCounter = 0;

inline ID3D12Device* D3D12Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx12>> Dx12Contexts;

inline static std::mutex RootSigatureMutex;
inline HWND currentHwnd = nullptr;

void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
	if (!contextRendering)
	{
		RootSigatureMutex.lock();
		rootSigCompute = pRootSignature;
		computeTime = GetTickCount64();
		RootSigatureMutex.unlock();
	}

	return orgSetComputeRootSignature(commandList, pRootSignature);
}

void hkSetGraphicRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
	if (!contextRendering)
	{
		RootSigatureMutex.lock();
		rootSigGraphic = pRootSignature;
		graphTime = GetTickCount64();
		RootSigatureMutex.unlock();
	}

	return orgSetGraphicRootSignature(commandList, pRootSignature);
}

void HookToCommandList(ID3D12GraphicsCommandList* InCmdList)
{
	if (orgSetComputeRootSignature != nullptr)
		return;

	// Get the vtable pointer
	PVOID* pVTable = *(PVOID**)InCmdList;

	// Get the address of the SetComputeRootSignature function from the vtable
	orgSetComputeRootSignature = (PFN_SetComputeRootSignature)pVTable[29];
	orgSetGraphicRootSignature = (PFN_SetComputeRootSignature)pVTable[30];

	// Apply the detour
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)orgSetComputeRootSignature, hkSetComputeRootSignature);
	DetourAttach(&(PVOID&)orgSetGraphicRootSignature, hkSetGraphicRootSignature);
	DetourTransactionCommit();
}

void UnhookSetComputeRootSignature()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)orgSetComputeRootSignature, hkSetComputeRootSignature);
	DetourDetach(&(PVOID&)orgSetGraphicRootSignature, hkSetGraphicRootSignature);
	DetourTransactionCommit();
}

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

	D3D12Device = InDevice;

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

	UnhookSetComputeRootSignature();

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
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.2.1 feature");
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, InParameters);
	}
	else if (upscalerChoice == 2)
	{
		Config::Instance()->Dx12Upscaler = "fsr21";
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.1.2 feature");
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, InParameters);
	}
	else
	{
		Config::Instance()->Dx12Upscaler = "xess";
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new XeSS feature");
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
		HookToCommandList(InCmdList);
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
	auto evaluateStart = GetTickCount64();

	spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature init!");

	if (!InCmdList)
	{
		spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	ID3D12RootSignature* orgComputeRootSig = nullptr;
	ID3D12RootSignature* orgGraphicRootSig = nullptr;

	RootSigatureMutex.lock();
	orgComputeRootSig = rootSigCompute;
	orgGraphicRootSig = rootSigGraphic;
	RootSigatureMutex.unlock();

	if (InCallback)
		spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature callback exist");

	IFeature_Dx12* deviceContext = nullptr;
	auto handleId = InFeatureHandle->Id;

	if (Config::Instance()->changeBackend)
	{
		// first release everything
		if (Dx12Contexts.contains(handleId) && changeBackendCounter == 0)
		{
			auto dc = Dx12Contexts[handleId].get();

			createParams = GetNGXParameters();
			createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
			createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
			createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
			createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
			createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
			createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

			dc = nullptr;

			spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature sleeping before reset of current feature for 1000ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			Dx12Contexts[handleId].reset();
			auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx12Contexts.erase(it);

			contextRendering = false;
			lastEvalTime = evaluateStart;

			rootSigCompute = nullptr;
			rootSigGraphic = nullptr;

			return NVSDK_NGX_Result_Success;
		}

		changeBackendCounter++;

		if (changeBackendCounter == 1)
		{
			// next frame create context
			if (Config::Instance()->newBackend == "")
				Config::Instance()->newBackend = Config::Instance()->Dx12Upscaler.value_or("xess");

			// prepare new upscaler
			if (Config::Instance()->newBackend == "fsr22")
			{
				Config::Instance()->Dx12Upscaler = "fsr22";
				spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.2.1 feature");
				Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, createParams);
			}
			else if (Config::Instance()->newBackend == "fsr21")
			{
				Config::Instance()->Dx12Upscaler = "fsr21";
				spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.1.2 feature");
				Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, createParams);
			}
			else
			{
				Config::Instance()->Dx12Upscaler = "xess";
				spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature creating new XeSS feature");
				Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, createParams);
			}

			return NVSDK_NGX_Result_Success;
		}

		if (changeBackendCounter == 2)
		{
			auto initResult = Dx12Contexts[handleId]->Init(D3D12Device, createParams);

			Config::Instance()->newBackend = "";
			Config::Instance()->changeBackend = false;
			free(createParams);
			createParams = nullptr;
			changeBackendCounter = 0;

			if (!initResult)
				return NVSDK_NGX_Result_Fail;
		}

		return NVSDK_NGX_Result_Success;
	}

	deviceContext = Dx12Contexts[handleId].get();
	Config::Instance()->CurrentFeature = deviceContext;

	if (deviceContext == nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
		return NVSDK_NGX_Result_Success;
	}

	contextRendering = true;

	bool evalResult = deviceContext->Evaluate(InCmdList, InParameters);

	if (computeTime != 0 && computeTime > lastEvalTime && computeTime < evaluateStart && orgComputeRootSig != nullptr)
		orgSetComputeRootSignature(InCmdList, orgComputeRootSig);

	if (graphTime != 0 && graphTime > lastEvalTime && graphTime < evaluateStart && orgGraphicRootSig != nullptr)
		orgSetGraphicRootSignature(InCmdList, orgGraphicRootSig);

	contextRendering = false;
	lastEvalTime = evaluateStart;

	rootSigCompute = nullptr;
	rootSigGraphic = nullptr;

	if (evalResult)
		return NVSDK_NGX_Result_Success;
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

