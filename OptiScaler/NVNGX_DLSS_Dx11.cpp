#pragma once
#include "pch.h"

#include "Config.h"
#include "Util.h"

#include "NVNGX_Parameter.h"
#include "NVNGX_Proxy.h"

#include "backends/dlss/DLSSFeature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11On12.h"
#include "backends/fsr2_212/FSR2Feature_Dx11On12_212.h"
#include "backends/xess/XeSSFeature_Dx11.h"

#include "imgui/imgui_overlay_dx11.h"
#include "imgui/imgui_overlay_dx12.h"

#include <ankerl/unordered_dense.h>

inline ID3D11Device* D3D11Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx11>> Dx11Contexts;
static inline NVSDK_NGX_Parameter* createParams;
static inline int changeBackendCounter = 0;

#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice,
	NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	Config::Instance()->NVNGX_ApplicationId = InApplicationId;
	Config::Instance()->NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
	Config::Instance()->NVNGX_Version = InSDKVersion;
	Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;

	if (InFeatureInfo != nullptr && InSDKVersion > 0x0000013)
		Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

	spdlog::info("NVSDK_NGX_D3D11_Init_Ext AppId: {0}", InApplicationId);
	spdlog::info("NVSDK_NGX_D3D11_Init_Ext SDK: {0:x}", (int)InSDKVersion);
	std::wstring string(InApplicationDataPath);

	std::string str(string.length(), 0);
	std::transform(string.begin(), string.end(), str.begin(), [](wchar_t c) { return (char)c; });
	
	spdlog::debug("NVSDK_NGX_D3D11_Init_Ext InApplicationDataPath {0}", str);

	Config::Instance()->NVNGX_FeatureInfo_Paths.clear();

	if (InFeatureInfo != nullptr)
	{
		for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
		{
			const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];
			std::wstring iniPathW(path);

			std::string iniPath(iniPathW.length(), 0);
			std::transform(iniPathW.begin(), iniPathW.end(), iniPath.begin(), [](wchar_t c) { return (char)c; });

			Config::Instance()->NVNGX_FeatureInfo_Paths.push_back(iniPathW);
			spdlog::debug("NVSDK_NGX_D3D11_Init_Ext PathListInfo[{0}]: {1}", i, iniPath);

		}
	}

	if (InDevice)
		D3D11Device = InDevice;

	Config::Instance()->Api = NVNGX_DX11;

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
	spdlog::debug("NVSDK_NGX_D3D11_Init was called NVSDK_NGX_D3D11_Init_Ext");
	return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
	auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

	spdlog::info("NVSDK_NGX_D3D11_Init_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D11_Init_ProjectID InEngineType: {0}", (int)InEngineType);
	spdlog::info("NVSDK_NGX_D3D11_Init_ProjectID InEngineVersion: {0}", InEngineVersion);

	Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	else
		Config::Instance()->NVNGX_EngineVersion5 = false;

	return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
	const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

	spdlog::info("NVSDK_NGX_D3D11_Init_with_ProjectID InProjectId: {0}", InProjectId);
	spdlog::info("NVSDK_NGX_D3D11_Init_with_ProjectID InEngineType: {0}", (int)InEngineType);
	spdlog::info("NVSDK_NGX_D3D11_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);

	Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
	Config::Instance()->NVNGX_Engine = InEngineType;
	Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL && InEngineVersion)
		Config::Instance()->NVNGX_EngineVersion5 = InEngineVersion[0] == '5';
	else
		Config::Instance()->NVNGX_EngineVersion5 = false;

	return result;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown()
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown");

	for (auto const& [key, val] : Dx11Contexts)
		NVSDK_NGX_D3D11_ReleaseFeature(val->Handle());

	Dx11Contexts.clear();
	D3D11Device = nullptr;
	Config::Instance()->CurrentFeature = nullptr;

	DLSSFeatureDx11::Shutdown(D3D11Device);

	if (Config::Instance()->OverlayMenu.value_or(true) && ImGuiOverlayDx11::IsInitedDx11())
		ImGuiOverlayDx11::ShutdownDx11();

	if (NVNGXProxy::D3D11_Shutdown() != nullptr)
	{
		auto result = NVNGXProxy::D3D11_Shutdown()();
		spdlog::info("NVSDK_NGX_D3D11_Shutdown D3D11_Shutdown result: {0:X}", (UINT)result);
	}

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
	spdlog::info("NVSDK_NGX_D3D11_Shutdown1");

	if (NVNGXProxy::D3D11_Shutdown1() != nullptr)
	{
		auto result = NVNGXProxy::D3D11_Shutdown1()(InDevice);
		spdlog::info("NVSDK_NGX_D3D11_Shutdown1 D3D11_Shutdown1 result: {0:X}", (UINT)result);
	}

	return NVSDK_NGX_D3D11_Shutdown();
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	spdlog::debug("NVSDK_NGX_D3D11_GetParameters");

	try
	{
		*OutParameters = GetNGXParameters("OptiDx11");
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
		*OutParameters = GetNGXParameters("OptiDx11");
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
	// FeatureId check
	if (InFeatureID != NVSDK_NGX_Feature_SuperSampling)
	{
		if (NVNGXProxy::InitDx11(D3D11Device) && NVNGXProxy::D3D11_CreateFeature() != nullptr)
		{
			auto result = NVNGXProxy::D3D11_CreateFeature()(InDevCtx, InFeatureID, InParameters, OutHandle);
			spdlog::info("NVSDK_NGX_D3D11_CreateFeature D3D11_CreateFeature result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
			return result;
		}
		else
		{
			spdlog::error("NVSDK_NGX_D3D11_CreateFeature Can't create this feature ({0})!", (int)InFeatureID);
			return NVSDK_NGX_Result_Fail;
		}
	}

	// CreateFeature
	auto handleId = IFeature::GetNextHandleId();
	spdlog::info("NVSDK_NGX_D3D11_CreateFeature HandleId: {0}", handleId);

	if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "xess")
	{
		Dx11Contexts[handleId] = std::make_unique<XeSSFeatureDx11>(handleId, InParameters);


		if (!Dx11Contexts[handleId]->ModuleLoaded())
		{
			spdlog::error("NVSDK_NGX_D3D11_CreateFeature can't create new XeSS with Dx12 feature, Fallback to FSR2.2!");

			Dx11Contexts[handleId].reset();
			auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx11Contexts.erase(it);

			Config::Instance()->Dx11Upscaler = "fsr22";
		}
		else
		{
			spdlog::info("NVSDK_NGX_D3D11_CreateFeature creating new XeSS with Dx12 feature");
		}
	}

	if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "dlss")
	{
		Dx11Contexts[handleId] = std::make_unique<DLSSFeatureDx11>(handleId, InParameters);


		if (!Dx11Contexts[handleId]->ModuleLoaded())
		{
			spdlog::error("NVSDK_NGX_D3D11_CreateFeature can't create new DLSS feature, Fallback to FSR2.2!");

			Dx11Contexts[handleId].reset();
			auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
			Dx11Contexts.erase(it);

			Config::Instance()->Dx11Upscaler = "fsr22";
		}
		else
		{
			spdlog::info("NVSDK_NGX_D3D11_CreateFeature creating new DLSS feature");
		}
	}

	if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22_12")
	{
		spdlog::info("NVSDK_NGX_D3D11_CreateFeature creating new FSR 2.2.1 with Dx12 feature");
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12>(handleId, InParameters);
	}
	else if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr21_12")
	{
		spdlog::info("NVSDK_NGX_D3D11_CreateFeature creating new FSR 2.1.2 with Dx12 feature");
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12_212>(handleId, InParameters);
	}
	else if (Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22")
	{
		spdlog::info("NVSDK_NGX_D3D11_CreateFeature creating new native FSR 2.2.1 feature");
		Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11>(handleId, InParameters);
	}

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

	if (deviceContext->Init(D3D11Device, InDevCtx, InParameters) && Dx11Contexts[handleId]->ModuleLoaded())
	{
		Config::Instance()->CurrentFeature = deviceContext;
		return NVSDK_NGX_Result_Success;
	}

	spdlog::error("NVSDK_NGX_D3D11_CreateFeature: CreateFeature failed");
	return NVSDK_NGX_Result_Fail;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (!InHandle)
		return NVSDK_NGX_Result_Success;
	
	auto handleId = InHandle->Id;
	if (handleId < 1000000)
	{
		if (NVNGXProxy::D3D11_ReleaseFeature() != nullptr)
		{
			auto result = NVNGXProxy::D3D11_ReleaseFeature()(InHandle);
			spdlog::info("NVSDK_NGX_D3D11_ReleaseFeature D3D11_ReleaseFeature result for ({0}): {1:X}", handleId, (UINT)result);
			return result;
		}
		else
		{
			return NVSDK_NGX_Result_FAIL_FeatureNotFound;
		}
	}

	spdlog::info("NVSDK_NGX_D3D11_ReleaseFeature releasing feature with id {0}", handleId);

	if (auto deviceContext = Dx11Contexts[handleId].get(); deviceContext != nullptr)
	{

		spdlog::trace("NVSDK_NGX_D3D11_ReleaseFeature sleeping for 250ms before reset()!");
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		if (deviceContext == Config::Instance()->CurrentFeature)
		{
			deviceContext->Shutdown();
			Config::Instance()->CurrentFeature = nullptr;
		}

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
	OutSupported->FeatureSupported = (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling) ?
		NVSDK_NGX_FeatureSupportResult_Supported : NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
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
	
	auto handleId = InFeatureHandle->Id;
	if (handleId < 1000000)
	{
		if (NVNGXProxy::D3D11_EvaluateFeature() != nullptr)
		{
			auto result = NVNGXProxy::D3D11_EvaluateFeature()(InDevCtx, InFeatureHandle, InParameters, InCallback);
			spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature D3D11_EvaluateFeature result for ({0}): {1:X}", handleId, (UINT)result);
			return result;
		}
		else
		{
			return NVSDK_NGX_Result_FAIL_FeatureNotFound;
		}
	}

	// DLSS Enabler check
	int deAvail;
	if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
	{
		if (Config::Instance()->DE_Available != (deAvail > 0))
			spdlog::info("NVSDK_NGX_D3D11_CreateFeature DLSSEnabler.Available: {0}", deAvail);

		Config::Instance()->DE_Available = (deAvail > 0);
	}

	if (Config::Instance()->OverlayMenu.value_or(true) &&
		Config::Instance()->CurrentFeature != nullptr && Config::Instance()->CurrentFeature->FrameCount() > Config::Instance()->MenuInitDelay.value_or(90) &&
		!ImGuiOverlayDx11::IsInitedDx11())
	{
		ImGuiOverlayDx12::ShutdownDx12();

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

		ImGuiOverlayDx11::InitDx11(consoleWindow);

		if (consoleAllocated)
			FreeConsole();
	}

	// Check window recreation
	HWND currentHandle = Util::GetProcessWindow();
	if (ImGuiOverlayDx11::IsInitedDx11() && ImGuiOverlayDx11::Handle() != currentHandle)
		ImGuiOverlayDx11::ReInitDx11(currentHandle);

	if (InCallback)
		spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature callback exist");

	IFeature_Dx11* deviceContext = nullptr;


	if (Config::Instance()->changeBackend)
	{
		if (Config::Instance()->newBackend == "")
			Config::Instance()->newBackend = Config::Instance()->Dx11Upscaler.value_or("fsr22");

		changeBackendCounter++;

		// first release everything
		if (changeBackendCounter == 1)
		{
			if (Dx11Contexts.contains(handleId))
			{
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature changing backend to {0}", Config::Instance()->newBackend);

				auto dc = Dx11Contexts[handleId].get();

				createParams = GetNGXParameters("OptiDx11");
				createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
				createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
				createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
				createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
				createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
				createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

				spdlog::trace("NVSDK_NGX_D3D11_EvaluateFeature sleeping before reset of current feature for 1000ms");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));

				Dx11Contexts[handleId].reset();
				auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
				Dx11Contexts.erase(it);

				Config::Instance()->CurrentFeature = nullptr;
			}
			else
			{
				spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature can't find handle {0} in Dx11Contexts!", handleId);

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
			// next frame prepare stuff
			if (Config::Instance()->newBackend == "xess")
			{
				Config::Instance()->Dx11Upscaler = "xess";
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature creating new XeSS with Dx12 feature");
				Dx11Contexts[handleId] = std::make_unique<XeSSFeatureDx11>(handleId, createParams);
			}
			else if (Config::Instance()->newBackend == "dlss")
			{
				Config::Instance()->Dx11Upscaler = "dlss";
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature creating new DLSS feature");
				Dx11Contexts[handleId] = std::make_unique<DLSSFeatureDx11>(handleId, createParams);
			}
			else if (Config::Instance()->newBackend == "fsr21_12")
			{
				Config::Instance()->Dx11Upscaler = "fsr21_12";
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature creating new FSR 2.1.2 with Dx12 feature");
				Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12_212>(handleId, createParams);
			}
			else if (Config::Instance()->newBackend == "fsr22_12")
			{
				Config::Instance()->Dx11Upscaler = "fsr22_12";
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature creating new FSR 2.2.1 with Dx12 feature");
				Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12>(handleId, createParams);
			}
			else if (Config::Instance()->newBackend == "fsr22")
			{
				Config::Instance()->Dx11Upscaler = "fsr22";
				spdlog::info("NVSDK_NGX_D3D11_EvaluateFeature creating new native FSR 2.2.1 feature");
				Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11>(handleId, createParams);
			}

			if (Config::Instance()->Dx11DelayedInit.value_or(false) && Config::Instance()->newBackend != "fsr22" && Config::Instance()->newBackend != "dlss")
			{
				spdlog::trace("NVSDK_NGX_D3D11_EvaluateFeature sleeping after new creation of new feature for 1000ms");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			return NVSDK_NGX_Result_Success;
		}

		if (changeBackendCounter == 3)
		{
			// then init and continue
			auto initResult = Dx11Contexts[handleId]->Init(D3D11Device, InDevCtx, createParams);

			if (Config::Instance()->Dx11DelayedInit.value_or(false))
			{
				spdlog::trace("NVSDK_NGX_D3D11_EvaluateFeature sleeping after new Init of new feature for 1000ms");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			free(createParams);
			createParams = nullptr;
			Config::Instance()->changeBackend = false;
			changeBackendCounter = 0;

			if (!initResult || !Dx11Contexts[handleId]->ModuleLoaded())
			{
				spdlog::error("NVSDK_NGX_D3D11_EvaluateFeature init failed with {0} feature", Config::Instance()->newBackend);

				Config::Instance()->newBackend = "fsr22";
				Config::Instance()->changeBackend = true;
			}
		}

		// if initial feature can't be inited
		Config::Instance()->CurrentFeature = Dx11Contexts[handleId].get();

		return NVSDK_NGX_Result_Success;
	}

	deviceContext = Dx11Contexts[handleId].get();
	Config::Instance()->CurrentFeature = deviceContext;

	if (deviceContext == nullptr)
	{
		spdlog::debug("NVSDK_NGX_D3D11_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
		return NVSDK_NGX_Result_Success;
	}

	if (!deviceContext->IsInited() && (deviceContext->Name() == "XeSS" || deviceContext->Name() == "DLSS"))
	{
		Config::Instance()->newBackend = "fsr22";
		Config::Instance()->changeBackend = true;
		return NVSDK_NGX_Result_Success;
	}

	if (!deviceContext->Evaluate(InDevCtx, InParameters))
		return NVSDK_NGX_Result_Fail;

	return NVSDK_NGX_Result_Success;
}

#pragma endregion
