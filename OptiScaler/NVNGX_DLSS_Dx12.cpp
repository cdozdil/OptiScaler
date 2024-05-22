#pragma once
#include "Config.h"
#include "NVNGX_Parameter.h"

#include "backends/dlss/DLSSFeature_Dx12.h"
#include "backends/fsr2/FSR2Feature_Dx12.h"
#include "backends/fsr2_212/FSR2Feature_Dx12_212.h"
#include "backends/xess/XeSSFeature_Dx12.h"

#include "imgui/imgui_overlay_dx12.h"

#include "detours/detours.h"
#include <ankerl/unordered_dense.h>
#include <dxgi1_4.h>

inline ID3D12Device* D3D12Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx12>> Dx12Contexts;
static inline NVSDK_NGX_Parameter* createParams = nullptr;
static inline int changeBackendCounter = 0;
static inline std::wstring appDataPath = L".";

#pragma region Hooks

typedef void(__fastcall* PFN_SetComputeRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);
typedef void(__fastcall* PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

static inline PFN_SetComputeRootSignature orgSetComputeRootSignature = nullptr;
static inline PFN_SetComputeRootSignature orgSetGraphicRootSignature = nullptr;
static inline PFN_CreateSampler orgCreateSampler = nullptr;

static inline ID3D12RootSignature* rootSigCompute = nullptr;
static inline ID3D12RootSignature* rootSigGraphic = nullptr;
static inline bool contextRendering = false;
static inline ULONGLONG computeTime = 0;
static inline ULONGLONG graphTime = 0;
static inline ULONGLONG lastEvalTime = 0;
inline static std::mutex sigatureMutex;

void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
	if (!contextRendering)
	{
		sigatureMutex.lock();
		rootSigCompute = pRootSignature;
		computeTime = GetTickCount64();
		sigatureMutex.unlock();
	}

	return orgSetComputeRootSignature(commandList, pRootSignature);
}

void hkSetGraphicRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
	if (!contextRendering)
	{
		sigatureMutex.lock();
		rootSigGraphic = pRootSignature;
		graphTime = GetTickCount64();
		sigatureMutex.unlock();
	}

	return orgSetGraphicRootSignature(commandList, pRootSignature);
}

void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	if (pDesc->MipLODBias < 0.0f && Config::Instance()->MipmapBiasOverride.has_value())
	{
		D3D12_SAMPLER_DESC newDesc{};

		newDesc.AddressU = pDesc->AddressU;
		newDesc.AddressV = pDesc->AddressV;
		newDesc.AddressW = pDesc->AddressW;
		newDesc.BorderColor[0] = pDesc->BorderColor[0];
		newDesc.BorderColor[1] = pDesc->BorderColor[1];
		newDesc.BorderColor[2] = pDesc->BorderColor[2];
		newDesc.BorderColor[3] = pDesc->BorderColor[3];
		newDesc.ComparisonFunc = pDesc->ComparisonFunc;
		newDesc.Filter = pDesc->Filter;
		newDesc.MaxAnisotropy = pDesc->MaxAnisotropy;
		newDesc.MaxLOD = pDesc->MaxLOD;
		newDesc.MinLOD = pDesc->MinLOD;
		newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();

		Config::Instance()->lastMipBias = newDesc.MipLODBias;

		return orgCreateSampler(device, &newDesc, DestDescriptor);
	}
	else if (pDesc->MipLODBias < 0.0f)
		Config::Instance()->lastMipBias = pDesc->MipLODBias;

	return orgCreateSampler(device, pDesc, DestDescriptor);
}

void HookToCommandList(ID3D12GraphicsCommandList* InCmdList)
{
	if (orgSetComputeRootSignature != nullptr || orgSetGraphicRootSignature != nullptr)
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

void HookToDevice(ID3D12Device* InDevice)
{
	if (orgCreateSampler != nullptr)
		return;

	// Get the vtable pointer
	PVOID* pVTable = *(PVOID**)InDevice;

	// Get the address of the SetComputeRootSignature function from the vtable
	orgCreateSampler = (PFN_CreateSampler)pVTable[22];

	// Apply the detour
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)orgCreateSampler, hkCreateSampler);
	DetourTransactionCommit();
}

void UnhookAll()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (orgSetComputeRootSignature != nullptr)
		DetourDetach(&(PVOID&)orgSetComputeRootSignature, hkSetComputeRootSignature);

	if (orgSetGraphicRootSignature != nullptr)
		DetourDetach(&(PVOID&)orgSetGraphicRootSignature, hkSetGraphicRootSignature);

	if (orgCreateSampler != nullptr)
		DetourDetach(&(PVOID&)orgCreateSampler, hkCreateSampler);

	DetourTransactionCommit();
}

#pragma endregion

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext SDK: {0:x}", (unsigned int)InSDKVersion);
	appDataPath = InApplicationDataPath;
	std::string str(appDataPath.begin(), appDataPath.end());
	spdlog::info("NVSDK_NGX_D3D12_Init_Ext InApplicationDataPath {0}", str);

	Config::Instance()->NVNGX_ApplicationId = InApplicationId;
	Config::Instance()->NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
	Config::Instance()->NVNGX_Version = InSDKVersion;
	Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;

	if (InFeatureInfo != nullptr)
	{
		
		if (InSDKVersion > 0x0000013)
		{
			Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

			if (Config::Instance()->NVNGX_Logger.LoggingCallback != nullptr)
				spdlog::info("NVSDK_NGX_D3D12_Init_Ext NVSDK Logging callback received!");
		}

		Config::Instance()->NVNGX_FeatureInfo_Paths.clear();

		for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
		{
			const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];

			Config::Instance()->NVNGX_FeatureInfo_Paths.push_back(std::wstring(path));

			std::wstring iniPathW(path);
			std::string iniPath(iniPathW.begin(), iniPathW.end());

			spdlog::debug("NVSDK_NGX_D3D12_Init_Ext InApplicationDataPath checking nvngx.ini file in: {0}", iniPath);

			if (Config::Instance()->LoadFromPath(path))
				spdlog::info("NVSDK_NGX_D3D12_Init_Ext InApplicationDataPath nvngx.ini file reloaded from: {0}", iniPath);
		}
	}

	D3D12Device = InDevice;

	if (D3D12Device != nullptr)
		HookToDevice(D3D12Device);

	Config::Instance()->Api = NVNGX_DX12;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::debug("NVSDK_NGX_D3D12_Init");
	return NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InEngineType: {0}", (int)InEngineType);
	spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InEngineVersion: {0}", InEngineVersion);

	Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	else
		Config::Instance()->NVNGX_EngineVersion5 = false;

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineType: {0}", (int)InEngineType);
	spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);

	Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	else
		Config::Instance()->NVNGX_EngineVersion5 = false;

	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
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

	Config::Instance()->CurrentFeature = nullptr;

	UnhookAll();

	DLSSFeatureDx12::Shutdown(D3D12Device);

	if (Config::Instance()->OverlayMenu.value_or(true) && ImGuiOverlayDx12::IsInitedDx12())
		ImGuiOverlayDx12::ShutdownDx12();

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
	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't create this feature ({0})!", (int)InFeatureID);
		return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
	}

	// DLSS Enabler check
	int deAvail;
	if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
	{
		spdlog::info("NVSDK_NGX_D3D12_CreateFeature DLSSEnabler.Available: {0}", deAvail);
		Config::Instance()->DE_Available = (deAvail > 0);
	}

	// Create feature
	auto handleId = IFeature::GetNextHandleId();
	spdlog::info("NVSDK_NGX_D3D12_CreateFeature HandleId: {0}", handleId);

	// backend selection
	// 0 : XeSS
	// 1 : FSR2.2
	// 2 : FSR2.1
	// 3 : DLSS
	int upscalerChoice = 0; // Default XeSS

	// ini first
	if (InParameters->Get("DLSSEnabler.Dx12Backend", &upscalerChoice) != NVSDK_NGX_Result_Success &&
		Config::Instance()->Dx12Upscaler.has_value())
	{
		if (Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr22")
			upscalerChoice = 1;
		else if (Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr21")
			upscalerChoice = 2;
		else if (Config::Instance()->Dx12Upscaler.value_or("xess") == "dlss")
			upscalerChoice = 3;
	}

	if (upscalerChoice == 3)
	{
		Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, InParameters);

		if (!Dx12Contexts[handleId]->ModuleLoaded())
		{
			spdlog::error("NVSDK_NGX_D3D12_CreateFeature can't create new DLSS feature, Fallback to XeSS!");

			Dx12Contexts[handleId].reset();
			auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx12Contexts.erase(it);

			upscalerChoice = 0;
		}
		else
		{
			Config::Instance()->Dx12Upscaler = "dlss";
			spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new DLSS feature");
		}
	}

	if (upscalerChoice == 0)
	{
		Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, InParameters);

		if (!Dx12Contexts[handleId]->ModuleLoaded())
		{
			spdlog::error("NVSDK_NGX_D3D12_CreateFeature can't create new XeSS feature, Fallback to FSR2.1!");

			Dx12Contexts[handleId].reset();
			auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx12Contexts.erase(it);

			upscalerChoice = 2;
		}
		else
		{
			Config::Instance()->Dx12Upscaler = "xess";
			spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new XeSS feature");
		}
	}

	if (upscalerChoice == 1)
	{
		Config::Instance()->Dx12Upscaler = "fsr22";
		spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new FSR 2.2.1 feature");
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, InParameters);
	}
	else if (upscalerChoice == 2)
	{
		Config::Instance()->Dx12Upscaler = "fsr21";
		spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new FSR 2.1.2 feature");
		Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, InParameters);
	}

	// nvsdk logging - ini first
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

		HookToDevice(D3D12Device);
	}

#pragma endregion

	if (deviceContext->Init(D3D12Device, InCmdList, InParameters))
	{
		Config::Instance()->CurrentFeature = deviceContext;
		HookToCommandList(InCmdList);

		return NVSDK_NGX_Result_Success;
	}

	spdlog::error("NVSDK_NGX_D3D12_CreateFeature: CreateFeature failed");
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (!InHandle)
		return NVSDK_NGX_Result_Success;

	auto handleId = InHandle->Id;
	spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature releasing feature with id {0}", handleId);

	if (auto deviceContext = Dx12Contexts[handleId].get(); deviceContext)
	{
		if (deviceContext == Config::Instance()->CurrentFeature)
		{
			Config::Instance()->CurrentFeature = nullptr;
			deviceContext->Shutdown();
		}

		Dx12Contexts[handleId].reset();
		auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
		Dx12Contexts.erase(it);
	}
	else
		spdlog::error("NVSDK_NGX_D3D12_ReleaseFeature can't release feature with id {0}!", handleId);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo,
	NVSDK_NGX_FeatureRequirement* OutSupported)
{
	spdlog::debug("NVSDK_NGX_D3D12_GetFeatureRequirements!");

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling) ?
		NVSDK_NGX_FeatureSupportResult_Supported : NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
	OutSupported->MinHWArchitecture = 0;

	//Some old windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	auto evaluateStart = GetTickCount64();

	spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature init!");

	if (!InCmdList)
	{
		spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!");
		return NVSDK_NGX_Result_Fail;
	}

	if (Config::Instance()->OverlayMenu.value_or(true) &&
		Config::Instance()->CurrentFeature != nullptr && Config::Instance()->CurrentFeature->FrameCount() > Config::Instance()->MenuInitDelay.value_or(90) &&
		!ImGuiOverlayDx12::IsInitedDx12())
	{
		auto hwnd = Util::GetProcessWindow();

		HWND consoleWindow = GetConsoleWindow();
		bool consoleAllocated = false;

		if (consoleWindow == NULL)
		{
			AllocConsole();
			consoleWindow = GetConsoleWindow();
			consoleAllocated = true;

			ShowWindow(consoleWindow, SW_HIDE);
		}

		SetForegroundWindow(hwnd);
		SetFocus(hwnd);

		ImGuiOverlayDx12::InitDx12(consoleWindow, D3D12Device);

		if (consoleAllocated)
			FreeConsole();
	}

	// Check window recreation
	if (Config::Instance()->OverlayMenu.value_or(true))
	{
		HWND currentHandle = Util::GetProcessWindow();
		if (ImGuiOverlayDx12::IsInitedDx12() && ImGuiOverlayDx12::Handle() != currentHandle)
			ImGuiOverlayDx12::ReInitDx12(currentHandle);
	}

	if (InCallback)
		spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature callback exist");

	IFeature_Dx12* deviceContext = nullptr;
	auto handleId = InFeatureHandle->Id;

	int limit;
	if (InParameters->Get("FramerateLimit", &limit) == NVSDK_NGX_Result_Success)
	{
		if (Config::Instance()->DE_FramerateLimit.has_value())
		{
			if (Config::Instance()->DE_FramerateLimit.value() != limit)
			{
				spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler FramerateLimit initial new value: {0}", Config::Instance()->DE_FramerateLimit.value());
				InParameters->Set("FramerateLimit", Config::Instance()->DE_FramerateLimit.value());
			}
		}
		else
		{
			spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler FramerateLimit initial value: {0}", limit);
			Config::Instance()->DE_FramerateLimit = limit;
		}
	}

	if (Config::Instance()->changeBackend)
	{
		if (Config::Instance()->newBackend == "")
			Config::Instance()->newBackend = Config::Instance()->Dx12Upscaler.value_or("xess");

		changeBackendCounter++;

		// first release everything
		if (changeBackendCounter == 1)
		{
			if (Dx12Contexts.contains(handleId))
			{
				spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature changing backend to {0}", Config::Instance()->newBackend);

				auto dc = Dx12Contexts[handleId].get();

				createParams = GetNGXParameters();
				createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
				createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
				createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
				createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
				createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
				createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

				dc = nullptr;

				spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature sleeping before reset of current feature for 1000ms");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));

				Dx12Contexts[handleId].reset();
				auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
				Dx12Contexts.erase(it);

				Config::Instance()->CurrentFeature = nullptr;

				contextRendering = false;
				lastEvalTime = evaluateStart;

				rootSigCompute = nullptr;
				rootSigGraphic = nullptr;
			}
			else
			{
				spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature can't find handle {0} in Dx12Contexts!", handleId);

				Config::Instance()->newBackend = "";
				Config::Instance()->changeBackend = false;

				if (createParams != nullptr)
				{
					free(createParams);
					createParams = nullptr;
				}

				changeBackendCounter = 0;
			}

			return NVSDK_NGX_Result_Success;
		}

		if (changeBackendCounter == 2)
		{
			// backend selection
			// 0 : XeSS
			// 1 : FSR2.2
			// 2 : FSR2.1
			// 3 : DLSS
			int upscalerChoice = 0;

			// prepare new upscaler
			if (Config::Instance()->newBackend == "fsr22")
			{
				Config::Instance()->Dx12Upscaler = "fsr22";
				spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.2.1 feature");
				Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, createParams);
				upscalerChoice = 1;
			}
			else if (Config::Instance()->newBackend == "fsr21")
			{
				Config::Instance()->Dx12Upscaler = "fsr21";
				spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.1.2 feature");
				Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, createParams);
				upscalerChoice = 2;
			}
			else if (Config::Instance()->newBackend == "dlss")
			{
				Config::Instance()->Dx12Upscaler = "dlss";
				spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new DLSS feature");
				Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, createParams);
				upscalerChoice = 3;
			}
			else
			{
				Config::Instance()->Dx12Upscaler = "xess";
				spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new XeSS feature");
				Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, createParams);
			}

			InParameters->Set("DLSSEnabler.Dx12Backend", upscalerChoice);

			return NVSDK_NGX_Result_Success;
		}

		if (changeBackendCounter == 3)
		{
			// next frame create context
			auto initResult = Dx12Contexts[handleId]->Init(D3D12Device, InCmdList, createParams);

			Config::Instance()->newBackend = "";
			Config::Instance()->changeBackend = false;
			free(createParams);
			createParams = nullptr;
			changeBackendCounter = 0;

			if (!initResult)
			{
				spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature init failed with {0} feature", Config::Instance()->newBackend);

				if (Config::Instance()->Dx12Upscaler == "dlss")
				{
					Config::Instance()->newBackend = "xess";
					InParameters->Set("DLSSEnabler.Dx12Backend", 0);
				}
				else
				{
					Config::Instance()->newBackend = "fsr21";
					InParameters->Set("DLSSEnabler.Dx12Backend", 2);
				}

				Config::Instance()->changeBackend = true;
			}
		}

		// if initial feature can't be inited
		Config::Instance()->CurrentFeature = Dx12Contexts[handleId].get();

		return NVSDK_NGX_Result_Success;
	}

	deviceContext = Dx12Contexts[handleId].get();
	Config::Instance()->CurrentFeature = deviceContext;

	if (deviceContext == nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
		return NVSDK_NGX_Result_Success;
	}

	if (!deviceContext->IsInited() && (deviceContext->Name() == "XeSS" || deviceContext->Name() == "DLSS"))
	{
		Config::Instance()->newBackend = "fsr21";
		Config::Instance()->changeBackend = true;
		return NVSDK_NGX_Result_Success;
	}

	ID3D12RootSignature* orgComputeRootSig = nullptr;
	ID3D12RootSignature* orgGraphicRootSig = nullptr;

	sigatureMutex.lock();
	orgComputeRootSig = rootSigCompute;
	orgGraphicRootSig = rootSigGraphic;
	sigatureMutex.unlock();

	contextRendering = true;

	bool evalResult = deviceContext->Evaluate(InCmdList, InParameters);

	if (Config::Instance()->RestoreComputeSignature.value_or(false) && computeTime != 0 && computeTime > lastEvalTime && computeTime < evaluateStart && orgComputeRootSig != nullptr)
		orgSetComputeRootSignature(InCmdList, orgComputeRootSig);

	if (Config::Instance()->RestoreGraphicSignature.value_or(false) && graphTime != 0 && graphTime > lastEvalTime && graphTime < evaluateStart && orgGraphicRootSig != nullptr)
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

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	spdlog::warn("NVSDK_NGX_D3D12_GetScratchBufferSize -> 52428800");
	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

