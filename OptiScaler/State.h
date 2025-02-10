#pragma once
#include "pch.h"
#include <deque>
#include "upscalers/IFeature.h"

typedef enum API
{
	NotSelected = 0,
	DX11,
	DX12,
	Vulkan,
} API;

typedef enum GameQuirk
{
	Other,
	Cyberpunk,
	FMF2,
	RDR1,
	Banishers
} GameQuirk;

typedef enum FGType : uint32_t
{
	NoFG,
	OptiFG,
	Nukems
} FGType;

class State {
public:
    static State& Instance() {
        static State instance;
        return instance;
    }

	// DLSSG
	GameQuirk gameQuirk = GameQuirk::Other;
    bool NukemsFilesAvailable = false;
	bool DLSSGDebugView = false;
	bool DLSSGInterpolatedOnly = false;

	// FSR Common
	float lastFsrCameraNear = 0.0f;
	float lastFsrCameraFar = 0.0f;

	// Frame Generation
	FGType activeFgType = FGType::NoFG;

	// OptiFG
	bool FGonlyGenerated = false;
	bool FGchanged = false;
	bool SCchanged = false;
	bool skipHeapCapture = false;
	bool useThreadingForHeaps = false;

	bool FGcaptureResources = false;
	int FGcapturedResourceCount = false;
	bool FGresetCapturedResources = false;
	bool FGonlyUseCapturedResources = false;

	// NVNGX init parameters
	uint64_t NVNGX_ApplicationId = 1337;
	std::wstring NVNGX_ApplicationDataPath;
	std::string NVNGX_ProjectId;
	NVSDK_NGX_Version NVNGX_Version{};
	const NVSDK_NGX_FeatureCommonInfo* NVNGX_FeatureInfo = nullptr;
	std::vector<std::wstring> NVNGX_FeatureInfo_Paths;
	NVSDK_NGX_LoggingInfo NVNGX_Logger{ nullptr, NVSDK_NGX_LOGGING_LEVEL_OFF, false };
	NVSDK_NGX_EngineType NVNGX_Engine = NVSDK_NGX_ENGINE_TYPE_CUSTOM;
	std::string NVNGX_EngineVersion;

	// NGX OTA
	std::string NGX_OTA_Dlss;
	std::string NGX_OTA_Dlssd;

	API api = API::NotSelected;
	API swapchainApi = API::NotSelected;

	// DLSS Enabler
	bool enablerAvailable = false;

	// Framerate
	bool reflexLimitsFps = false;
	bool reflexShowWarning = false;

	// for realtime changes
	bool changeBackend = false;
	std::string newBackend = "";

	// XeSS debug stuff
	bool xessDebug = false;
	int xessDebugFrames = 5;
	float lastMipBias = 100.0f;
	float lastMipBiasMax = -100.0f;

	// DLSS
	bool upscalerDisableHook = false;
	bool dlssPresetsOverriddenExternally = false;
	bool dlssdPresetsOverriddenExternally = false;

	// Spoofing
	bool skipSpoofing = false;
	bool skipDllLoadChecks = false;
	// For DXVK, it calls DXGI which cause softlock
	bool skipDxgiLoadChecks = false;

	// FSR3.x
	std::vector<const char*> fsr3xVersionNames;
	std::vector<uint64_t> fsr3xVersionIds;

	// Linux check
	bool isRunningOnLinux = false;
	bool isRunningOnDXVK = false;
	bool isRunningOnNvidia = false;

	bool isDxgiMode = false;
	bool isWorkingAsNvngx = false;

	// Vulkan stuff
	bool vulkanCreatingSC = false;
	bool vulkanSkipHooks = false;
	bool renderMenu = true;

	// Framegraph
	std::deque<float> upscaleTimes;
	std::deque<float> frameTimes;

	// Swapchain info
	float screenWidth = 800.0;
	float screenHeight = 450.0;

	// HDR
	std::vector<IUnknown*> SCbuffers;
	bool isHdrActive = false;

	std::string setInputApiName;
	std::string currentInputApiName;

	bool isShuttingDown = false;

	// menu warnings
	bool showRestartWarning = false;
	bool nvngxIniDetected = false;

	bool nvngxExists = false;
	bool libxessExists = false;
	bool fsrHooks = false;

	IFeature* currentFeature = nullptr;

	std::vector<ID3D12Device*> d3d12Devices;
	std::vector<ID3D11Device*> d3d11Devices;
	std::map<UINT64, std::string> adapterDescs;

private:
    State() = default;
};