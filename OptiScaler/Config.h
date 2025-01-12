#pragma once
#include "pch.h"
#include <optional>
#include <filesystem>
#include <SimpleIni.h>
#include "backends/IFeature.h"
#include <deque>
#include <map>

typedef enum NVNGX_Api
{
	NVNGX_NOT_SELECTED = 0,
	NVNGX_DX11,
	NVNGX_DX12,
	NVNGX_VULKAN,
} NVNGX_Api;

typedef enum GameQuirk 
{
	Other,
	Cyberpunk
} GameQuirk;

class Config
{
public:
	Config(std::wstring fileName);

	// Init flags
	std::optional<bool> DepthInverted;
	std::optional<bool> AutoExposure;
	std::optional<bool> HDR;
	std::optional<bool> JitterCancellation;
	std::optional<bool> DisplayResolution;
	std::optional<bool> DisableReactiveMask;
	std::optional<float> DlssReactiveMaskBias;

	// Logging
	std::optional<bool> LogToFile;
	std::optional<bool> LogToConsole;
	std::optional<bool> LogToNGX;
	std::optional<bool> OpenConsole;
	std::optional<bool> DebugWait;
	std::optional<int> LogLevel;
	std::optional<std::wstring> LogFileName;
	std::optional<bool> LogSingleFile;

	// XeSS
	std::optional<bool> BuildPipelines;
	std::optional<int32_t> NetworkModel;
	std::optional<bool> CreateHeaps;
	std::optional<std::wstring> XeSSLibrary;

	//DLSS
	std::optional<bool> DLSSEnabled;
	std::optional<std::wstring> DLSSLibrary;
	std::optional<std::wstring> NVNGX_DLSS_Library;
	std::optional<bool> RenderPresetOverride;
	std::optional<uint32_t> RenderPresetDLAA;
	std::optional<uint32_t> RenderPresetUltraQuality;
	std::optional<uint32_t> RenderPresetQuality;
	std::optional<uint32_t> RenderPresetBalanced;
	std::optional<uint32_t> RenderPresetPerformance;
	std::optional<uint32_t> RenderPresetUltraPerformance;

	// DLSSG
	std::optional<bool> SpoofHAGS;
	std::optional<bool> DLSSGMod;
	GameQuirk gameQuirk = Other;

	// CAS
	std::optional<bool> RcasEnabled;
	std::optional<bool> MotionSharpnessEnabled;
	std::optional<bool> MotionSharpnessDebug;
	std::optional<float> MotionSharpness;
	std::optional<float> MotionThreshold;
	std::optional<float> MotionScaleLimit;

	//Sharpness 
	std::optional<bool> OverrideSharpness;
	std::optional<float> Sharpness;

	// menu
	std::optional<float> MenuScale;
	std::optional<bool> OverlayMenu;
	std::optional<int> ShortcutKey;
	std::optional<bool> AdvancedSettings;
	std::optional<bool> ExtendedLimits;
    std::optional<bool> ShowFps;
    std::optional<int> FpsOverlayPos; // 0 Top Left, 1 Top Right, 2 Bottom Left, 3 Bottom Right
    std::optional<int> FpsOverlayType; // 0 Only FPS, 1 +Frame Time, 2 +Upscaler Time, 3 +Frame Time Graph, 4 +Upscaler Time Graph
	std::optional<int> FpsShortcutKey;
	std::optional<int> FpsCycleShortcutKey;
    std::optional<bool> FpsOverlayHorizontal;
	std::optional<float> FpsOverlayAlpha;

	// hooks
	std::optional<bool> HookOriginalNvngxOnly;

	// Upscale Ratio Override
	std::optional<bool> UpscaleRatioOverrideEnabled;
	std::optional<float> UpscaleRatioOverrideValue;

	// DRS
	std::optional<bool> DrsMinOverrideEnabled;
	std::optional<bool> DrsMaxOverrideEnabled;

	// Quality Overrides
	std::optional<bool> QualityRatioOverrideEnabled;
	std::optional<float> QualityRatio_DLAA;
	std::optional<float> QualityRatio_UltraQuality;
	std::optional<float> QualityRatio_Quality;
	std::optional<float> QualityRatio_Balanced;
	std::optional<float> QualityRatio_Performance;
	std::optional<float> QualityRatio_UltraPerformance;

	//Hotfixes
	std::optional<float> MipmapBiasOverride;
	std::optional<bool> MipmapBiasFixedOverride;
	std::optional<bool> MipmapBiasScaleOverride;
	std::optional<bool> MipmapBiasOverrideAll;
	std::optional<int> AnisotropyOverride;
	std::optional<int> RoundInternalResolution;

	std::optional<bool> RestoreComputeSignature;
	std::optional<bool> RestoreGraphicSignature;
	std::optional<int> SkipFirstFrames;
	
	std::optional<bool> UsePrecompiledShaders;

	std::optional<bool> UseGenericAppIdWithDlss;
	std::optional<bool> PreferDedicatedGpu;
	std::optional<bool> PreferFirstDedicatedGpu;

	std::optional<int32_t> ColorResourceBarrier;
	std::optional<int32_t> MVResourceBarrier;
	std::optional<int32_t> DepthResourceBarrier;
	std::optional<int32_t> ExposureResourceBarrier;
	std::optional<int32_t> MaskResourceBarrier;
	std::optional<int32_t> OutputResourceBarrier;

	// Upscalers
	std::optional<std::string> Dx11Upscaler;
	std::optional<std::string> Dx12Upscaler;
	std::optional<std::string> VulkanUpscaler;

	// Output Scaling
	std::optional<bool> OutputScalingEnabled;
	std::optional<float> OutputScalingMultiplier;
	std::optional<bool> OutputScalingUseFsr;
	std::optional<uint32_t> OutputScalingDownscaler;

	// fsr
	std::optional<bool> FsrDebugView;
	std::optional<int> Fsr3xIndex;
	std::optional<bool> FsrUseMaskForTransparency;
	std::optional<float> FsrVelocity;

	// fsr common
	std::optional<float> FsrVerticalFov;
	std::optional<float> FsrHorizontalFov;
	std::optional<float> FsrCameraNear;
	std::optional<float> FsrCameraFar;
	std::optional<bool> FsrUseFsrInputValues;
	float LastFsrCameraNear = 0.0f;
	float LastFsrCameraFar = 0.0f;

	// dx11wdx12
	std::optional<int> TextureSyncMethod;
	std::optional<int> CopyBackSyncMethod;
	std::optional<bool> Dx11DelayedInit;
	std::optional<bool> SyncAfterDx12;
	std::optional<bool> DontUseNTShared;

	// nvapi override
	std::optional<bool> OverrideNvapiDll;
	std::optional<std::wstring> NvapiDllPath;

	// spoofing
	std::optional<bool> DxgiSpoofing;
	std::optional<std::string> DxgiBlacklist;
	std::optional<int> DxgiVRAM;
	std::optional<bool> VulkanSpoofing;
	std::optional<bool> VulkanExtensionSpoofing;
	std::optional<std::wstring> SpoofedGPUName;
	std::optional<int> VulkanVRAM;

	// plugins
	std::optional<std::wstring> PluginPath;
	std::optional<bool> LoadSpecialK;
	std::optional<bool> LoadReShade;

	// fg
	std::optional<bool> FGUseFGSwapChain;
	std::optional<bool> FGEnabled;
	std::optional<bool> FGHighPriority;
	std::optional<bool> FGDebugView;
	std::optional<bool> FGAsync;
	std::optional<bool> FGHUDFix;
	std::optional<bool> FGHUDFixExtended;
	std::optional<bool> FGImmediateCapture;
	std::optional<int> FGHUDLimit;
	std::optional<int> FGRectLeft;
	std::optional<int> FGRectTop;
	std::optional<int> FGRectWidth;
	std::optional<int> FGRectHeight;
	std::optional<bool> FGDisableOverlays;
	std::optional<bool> FGAlwaysTrackHeaps;
	std::optional<bool> FGHybridSpin;
	bool FGOnlyGenerated = false;
	bool FGChanged = false;
	bool SCChanged = false;
	bool SkipHeapCapture = false;
	bool UseThreadingForHeaps = false;

	bool FGCaptureResources = false;
	int FGCapturedResourceCount = false;
	bool FGResetCapturedResources = false;
	bool FGOnlyUseCapturedResources = false;

	// nvngx init parameters
	unsigned long long NVNGX_ApplicationId = 1337;
	std::wstring NVNGX_ApplicationDataPath;
	std::string NVNGX_ProjectId;
	NVSDK_NGX_Version NVNGX_Version{};
	const NVSDK_NGX_FeatureCommonInfo* NVNGX_FeatureInfo = nullptr;
	std::vector<std::wstring> NVNGX_FeatureInfo_Paths;
	NVSDK_NGX_LoggingInfo NVNGX_Logger{ nullptr, NVSDK_NGX_LOGGING_LEVEL_OFF, false };
	NVSDK_NGX_EngineType NVNGX_Engine = NVSDK_NGX_ENGINE_TYPE_CUSTOM;
	std::string NVNGX_EngineVersion;
	
	NVNGX_Api Api = NVNGX_NOT_SELECTED;
	
	// dlss enabler
	bool DE_Available = false;
	std::optional<int> DE_FramerateLimit;			// off - vsync - number
	std::optional<bool> DE_FramerateLimitVsync;
	std::optional<int> DE_DynamicLimitAvailable;	// DFG.Available
	std::optional<int> DE_DynamicLimitEnabled;		// DFG.Enabled
	std::optional<std::string> DE_Generator;		// auto - fsr30 - fsr31 - dlssg
	std::optional<std::string> DE_Reflex;			// on - boost - off
	std::optional<std::string> DE_ReflexEmu;		// auto - on - off

	// fakenvapi
	std::optional<bool> FN_EnableLogs;
	std::optional<bool> FN_EnableTraceLogs;
	std::optional<bool> FN_ForceLatencyFlex;
	std::optional<uint32_t> FN_LatencyFlexMode;		// conservative - aggressive - reflex ids
	std::optional<uint32_t> FN_ForceReflex;			// in-game - force disable - force enable

	// inputs
	std::optional<bool> DlssInputs;
	std::optional<bool> XeSSInputs;
	std::optional<bool> Fsr2Inputs;
	std::optional<bool> Fsr3Inputs;
	std::optional<bool> FfxInputs;

	// framerate
	bool ReflexAvailable = false;
	std::optional<float> FramerateLimit;

	// for realtime changes
	bool changeBackend = false;
	std::string newBackend = "";

	// XeSS debug stuff
	bool xessDebug = false;
	int xessDebugFrames = 5;
	float lastMipBias = 100.0f;
	float lastMipBiasMax = -100.0f;

	// dlss hook
	bool upscalerDisableHook = false;

	// spoofing
	bool skipSpoofing = false;
	bool skipDllLoadChecks = false;
	// for DXVK, it calls DXGI which cause softlock
	bool skipDxgiLoadChecks = false;

	// fsr3.x
	std::vector<const char*> fsr3xVersionNames;
	std::vector<uint64_t> fsr3xVersionIds;

	// linux check
	bool IsRunningOnLinux = false;
	bool IsRunningOnDXVK = false;
	bool IsRunningOnNvidia = false;

	bool IsDxgiMode = false;
	bool WorkingAsNvngx = false;

	// vulkan stuff
	bool VulkanCreatingSC = false;
	bool VulkanSkipHooks = false;
	bool RenderMenu = true;

	// framegraph
	std::deque<float> upscaleTimes;
	std::deque<float> frameTimes;

	// swapchain info
	float ScreenWidth = 800.0;
	float ScreenHeight = 450.0;

	// hdr
	std::optional<bool> forceHdr;
	std::optional<bool> useHDR10;
	std::vector<IUnknown*> scBuffers;
	bool isHdrActive = false;

	std::string setInputApiName;
	std::string currentInputApiName;

	bool IsShuttingDown = false;

	bool nvngxExist = false;
	bool libxessExist = false;
	bool fsrHooks = false;

	IFeature* CurrentFeature = nullptr;

    std::vector<ID3D12Device*> d3d12Devices;
    std::vector<ID3D11Device*> d3d11Devices;
    std::map<UINT64, std::string> adapterDescs;

	bool Reload(std::filesystem::path iniPath);
	bool LoadFromPath(const wchar_t* InPath);
	bool SaveIni();

	bool ReloadFakenvapi();
	bool SaveFakenvapiIni();

	void CheckUpscalerFiles();

	static Config* Instance();

private:
	inline static Config* _config;

	CSimpleIniA ini;
	CSimpleIniA fakenvapiIni;
	std::filesystem::path absoluteFileName;

	std::optional<std::string> readString(std::string section, std::string key, bool lowercase = false);
	std::optional<float> readFloat(std::string section, std::string key);
	std::optional<int> readInt(std::string section, std::string key);
	std::optional<uint32_t> readUInt(std::string section, std::string key);
	std::optional<bool> readBool(std::string section, std::string key);
};
