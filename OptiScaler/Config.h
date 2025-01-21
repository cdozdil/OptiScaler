#pragma once
#include "pch.h"
#include "State.h"
#include <optional>
#include <filesystem>
#include <SimpleIni.h>
#include <map>

template <class T, bool HasDefaultValue = true>
class CustomOptional : public std::optional<T> {
private:
	T _defaultValue;
	std::optional<T> _configIni;
	bool _volatile;

public:
	CustomOptional(T defaultValue = T{}) requires (HasDefaultValue)
		: std::optional<T>(), _defaultValue(std::move(defaultValue)), _configIni(std::nullopt), _volatile(false) {}

	CustomOptional() requires (!HasDefaultValue)
		: std::optional<T>(), _defaultValue(T{}), _configIni(std::nullopt), _volatile(false) {}

	// Prevents a change from being saved to ini
	constexpr void set_volatile_value(const T& value) {
		if (!_volatile) { // make sure the previously set value is saved
			if (this->has_value())
				_configIni = this->value();
			else
				_configIni = std::nullopt;
		}
		_volatile = true;
		std::optional<T>::operator=(value);
	}
	
	// Use this when first setting a CustomOptional
	constexpr void set_from_config(const std::optional<T>& opt) {
		if (!this->has_value()) {
			_configIni = opt;
			std::optional<T>::operator=(opt);
		}
	}

	constexpr CustomOptional& operator=(const T& value) {
		_volatile = false;
		std::optional<T>::operator=(value);
		return *this;
	}

	constexpr CustomOptional& operator=(T&& value) {
		_volatile = false;
		std::optional<T>::operator=(std::move(value));
		return *this;
	}

	constexpr CustomOptional& operator=(const std::optional<T>& opt) {
		_volatile = false;
		std::optional<T>::operator=(opt);
		return *this;
	}

	constexpr CustomOptional& operator=(std::optional<T>&& opt) {
		_volatile = false;
		std::optional<T>::operator=(std::move(opt));
		return *this;
	}

	// Needed for string literals for some reason
	constexpr CustomOptional& operator=(const char* value) requires std::same_as<T, std::string> {
		_volatile = false;
		std::optional<T>::operator=(T(value));
		return *this;
	}

	constexpr T value_or_default() const& requires (HasDefaultValue) {
		return this->has_value() ? this->value() : _defaultValue;
	}

	constexpr T value_or_default()&& requires (HasDefaultValue) {
		return this->has_value() ? std::move(this->value()) : std::move(_defaultValue);
	}

	constexpr std::optional<T> value_for_config() {
		if (_volatile) {
			if (_configIni != _defaultValue)
				return _configIni;
			else
				return std::nullopt;
		}
		else if (!this->has_value() || *this == _defaultValue) {
			return std::nullopt;
		}
		else {
			return this->value();
		}
	}

	constexpr T value_for_config_or(T other) {
		auto option = value_for_config();

		if (option.has_value())
			return option.value();
		else
			return other;
	}
};

class Config
{
public:
	Config();

	// Init flags
	std::optional<bool> DepthInverted;
	std::optional<bool> AutoExposure;
	std::optional<bool> HDR;
	std::optional<bool> JitterCancellation;
	std::optional<bool> DisplayResolution;
	CustomOptional<bool, false> DisableReactiveMask;
	CustomOptional<float> DlssReactiveMaskBias{ 0.45f };

	// Logging
	CustomOptional<bool> LogToFile{ false };
	CustomOptional<bool> LogToConsole{ false };
	CustomOptional<bool> LogToNGX{ false };
	CustomOptional<bool> OpenConsole{ false };
	CustomOptional<bool> DebugWait{ false }; // not in ini
	CustomOptional<int> LogLevel{ 2 };
	std::optional<std::wstring> LogFileName;
	CustomOptional<bool> LogSingleFile{ true };

	// XeSS
	CustomOptional<bool> BuildPipelines{ true };
	CustomOptional<int32_t> NetworkModel{ 0 };
	CustomOptional<bool> CreateHeaps{ true };
	std::optional<std::wstring> XeSSLibrary;

	// DLSS
	CustomOptional<bool> DLSSEnabled{ true };
	std::optional<std::wstring> DLSSLibrary;
	std::optional<std::wstring> NVNGX_DLSS_Library;
	CustomOptional<bool> RenderPresetOverride{ false };
	CustomOptional<uint32_t> RenderPresetDLAA{ 0 };
	CustomOptional<uint32_t> RenderPresetUltraQuality{ 0 };
	CustomOptional<uint32_t> RenderPresetQuality{ 0 };
	CustomOptional<uint32_t> RenderPresetBalanced{ 0 };
	CustomOptional<uint32_t> RenderPresetPerformance{ 0 };
	CustomOptional<uint32_t> RenderPresetUltraPerformance{ 0 };

	// DLSSG
	CustomOptional<bool> SpoofHAGS{ false };
	CustomOptional<bool> DLSSGMod{ false };

	// CAS
	CustomOptional<bool> RcasEnabled{ false };
	CustomOptional<bool> MotionSharpnessEnabled{ false };
	CustomOptional<bool> MotionSharpnessDebug{ false };
	CustomOptional<float> MotionSharpness{ 0.4f };
	CustomOptional<float> MotionThreshold{ 0.0f };
	CustomOptional<float> MotionScaleLimit{ 10.0f };

	// Sharpness 
	CustomOptional<bool> OverrideSharpness{ false };
	CustomOptional<float> Sharpness{ 0.3f };

	// Menu
	CustomOptional<float> MenuScale{ 1.0f };
	CustomOptional<bool> OverlayMenu{ true };
	CustomOptional<int> ShortcutKey{ VK_INSERT };
	CustomOptional<bool> AdvancedSettings{ false };
	CustomOptional<bool> ExtendedLimits{ false };
	CustomOptional<bool> ShowFps{ false };
	CustomOptional<int> FpsOverlayPos{ 0 }; // 0 Top Left, 1 Top Right, 2 Bottom Left, 3 Bottom Right
	CustomOptional<int> FpsOverlayType{ 0 }; // 0 Only FPS, 1 +Frame Time, 2 +Upscaler Time, 3 +Frame Time Graph, 4 +Upscaler Time Graph
	CustomOptional<int> FpsShortcutKey{ VK_PRIOR };
	CustomOptional<int> FpsCycleShortcutKey{ VK_NEXT };
	CustomOptional<bool> FpsOverlayHorizontal{ false };
	CustomOptional<float> FpsOverlayAlpha{ 0.4f };

	// Hooks
	CustomOptional<bool> HookOriginalNvngxOnly{ false };

	// Upscale Ratio Override
	CustomOptional<bool> UpscaleRatioOverrideEnabled{ false };
	CustomOptional<float> UpscaleRatioOverrideValue{ 1.3f };

	// DRS
	CustomOptional<bool> DrsMinOverrideEnabled{ false };
	CustomOptional<bool> DrsMaxOverrideEnabled{ false };

	// Quality Overrides
	CustomOptional<bool> QualityRatioOverrideEnabled{ false };
	CustomOptional<float> QualityRatio_DLAA{ 1.0f };
	CustomOptional<float> QualityRatio_UltraQuality{ 1.3f };
	CustomOptional<float> QualityRatio_Quality{ 1.5f };
	CustomOptional<float> QualityRatio_Balanced{ 1.7f };
	CustomOptional<float> QualityRatio_Performance{ 2.0f };
	CustomOptional<float> QualityRatio_UltraPerformance{ 3.0f };

	// Hotfixes
	std::optional<float> MipmapBiasOverride; // disabled by default
	CustomOptional<bool> MipmapBiasFixedOverride{ false };
	CustomOptional<bool> MipmapBiasScaleOverride{ false };
	CustomOptional<bool> MipmapBiasOverrideAll{ false };
	std::optional<int> AnisotropyOverride; // disabled by default
	std::optional<int> RoundInternalResolution; // disabled by default

	CustomOptional<bool> RestoreComputeSignature{ false };
	CustomOptional<bool> RestoreGraphicSignature{ false };
	std::optional<int> SkipFirstFrames; // disabled by default

	CustomOptional<bool> UsePrecompiledShaders{ true };

	CustomOptional<bool> UseGenericAppIdWithDlss{ false };
	CustomOptional<bool> PreferDedicatedGpu{ false };
	CustomOptional<bool> PreferFirstDedicatedGpu{ false };

	std::optional<int32_t> ColorResourceBarrier; // disabled by default
	std::optional<int32_t> MVResourceBarrier; // disabled by default
	std::optional<int32_t> DepthResourceBarrier; // disabled by default
	std::optional<int32_t> ExposureResourceBarrier; // disabled by default
	std::optional<int32_t> MaskResourceBarrier; // disabled by default
	std::optional<int32_t> OutputResourceBarrier; // disabled by default

	// Upscalers
	CustomOptional<std::string> Dx11Upscaler{ "fsr22" };
	CustomOptional<std::string> Dx12Upscaler{ "xess" };
	CustomOptional<std::string> VulkanUpscaler{ "fsr21" };

	// Output Scaling
	CustomOptional<bool> OutputScalingEnabled{ false };
	CustomOptional<float> OutputScalingMultiplier{ 1.5f };
	CustomOptional<bool> OutputScalingUseFsr{ true };
	CustomOptional<uint32_t> OutputScalingDownscaler{ 0 }; // 0 = Bicubic | 1 = Lanczos | 2 = Catmull-Rom | 3 = MAGC

	// FSR
	CustomOptional<bool> FsrDebugView{ false };
	CustomOptional<int> Fsr3xIndex{ 0 };
	CustomOptional<bool> FsrUseMaskForTransparency{ true };
	CustomOptional<float> FsrVelocity{ 1.0f };

	// FSR Common
	CustomOptional<float> FsrVerticalFov{ 60.0f };
	std::optional<float> FsrHorizontalFov; // off by default
	CustomOptional<float> FsrCameraNear{ 0.1f };
	CustomOptional<float> FsrCameraFar{ 100000.0f };
	CustomOptional<bool> FsrUseFsrInputValues{ true };

	// dx11wdx12
	// Valid values are;
	//	0 - No syncing(fastest, most prone to errors)
	//	1 - Fence
	//	2 - Fences + Flush
	//	3 - Fences + Event
	//	4 - Fences + Flush + Event
	//	5 - Query Only
	CustomOptional<int> TextureSyncMethod{ 1 };
	CustomOptional<int> CopyBackSyncMethod{ 5 };
	CustomOptional<bool> Dx11DelayedInit{ false };
	CustomOptional<bool> SyncAfterDx12{ true };
	CustomOptional<bool> DontUseNTShared{ false };

	// NVAPI Override
	CustomOptional<bool> OverrideNvapiDll{ false };
	std::optional<std::wstring> NvapiDllPath;

	// Spoofing
	std::optional<bool> DxgiSpoofing; // it's not always true by default
	std::optional<std::string> DxgiBlacklist; // disabled by default
	std::optional<int> DxgiVRAM; // disabled by default
	CustomOptional<bool> VulkanSpoofing{ false };
	CustomOptional<bool> VulkanExtensionSpoofing{ false };
	CustomOptional<std::wstring> SpoofedGPUName{ L"NVIDIA GeForce RTX 4090" };
	std::optional<int> VulkanVRAM; // disabled by default

	// Plugins
	CustomOptional<std::wstring> PluginPath{ L"plugins" };
	CustomOptional<bool> LoadSpecialK{ false };
	CustomOptional<bool> LoadReShade{ false };

	// FG
	CustomOptional<bool> FGUseFGSwapChain{ true };
	CustomOptional<bool> FGEnabled{ false };
	CustomOptional<bool> FGHighPriority{ true };
	CustomOptional<bool> FGDebugView{ false };
	CustomOptional<bool> FGAsync{ false };
	CustomOptional<bool> FGHUDFix{ false };
	CustomOptional<bool> FGHUDFixExtended{ false };
	CustomOptional<bool> FGImmediateCapture{ false };
	CustomOptional<int> FGHUDLimit{ 1 };
	std::optional<int> FGRectLeft;
	std::optional<int> FGRectTop;
	std::optional<int> FGRectWidth;
	std::optional<int> FGRectHeight;
	CustomOptional<bool> FGDisableOverlays{ true };
	CustomOptional<bool> FGAlwaysTrackHeaps{ false };
	CustomOptional<bool> FGHybridSpin{ false };

	// DLSS Enabler
	std::optional<int> DE_FramerateLimit;			// off - vsync - number
	std::optional<bool> DE_FramerateLimitVsync;
	std::optional<int> DE_DynamicLimitAvailable;	// DFG.Available
	std::optional<int> DE_DynamicLimitEnabled;		// DFG.Enabled
	std::optional<std::string> DE_Generator;		// auto - fsr30 - fsr31 - dlssg // "auto"
	std::optional<std::string> DE_Reflex;			// on - boost - off // "on"
	std::optional<std::string> DE_ReflexEmu;		// auto - on - off // "auto"

	// fakenvapi
	CustomOptional<bool> FN_EnableLogs{ true };
	CustomOptional<bool> FN_EnableTraceLogs{ false };
	CustomOptional<bool> FN_ForceLatencyFlex{ false };
	CustomOptional<uint32_t> FN_LatencyFlexMode{ 0 };		// conservative - aggressive - reflex ids
	CustomOptional<uint32_t> FN_ForceReflex{ 0 };			// in-game - force disable - force enable

	// Inputs
	CustomOptional<bool> DlssInputs{ true };
	CustomOptional<bool> XeSSInputs{ true };
	CustomOptional<bool> Fsr2Inputs{ true };
	CustomOptional<bool> Fsr2Pattern{ false };
	CustomOptional<bool> Fsr3Inputs{ true };
	CustomOptional<bool> FfxInputs{ true };

	// Framerate
	CustomOptional<float> FramerateLimit{ 0.0f };

	// HDR
	CustomOptional<bool> ForceHDR{ false };
	CustomOptional<bool> UseHDR10{ false };

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
	std::wstring fileName = L"nvngx.ini";

	bool Reload(std::filesystem::path iniPath);

	std::optional<std::string> readString(std::string section, std::string key, bool lowercase = false);
	std::optional<std::wstring> readWString(std::string section, std::string key, bool lowercase = false);
	std::optional<float> readFloat(std::string section, std::string key);
	std::optional<int> readInt(std::string section, std::string key);
	std::optional<uint32_t> readUInt(std::string section, std::string key);
	std::optional<bool> readBool(std::string section, std::string key);
};
