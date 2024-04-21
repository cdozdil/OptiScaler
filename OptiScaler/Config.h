#pragma once
#include "pch.h"
#include <optional>
#include <filesystem>
#include <SimpleIni.h>
#include "backends/IFeature.h"

typedef enum NVNGX_Api
{
	NVNGX_NOT_SELECTED = 0,
	NVNGX_DX11,
	NVNGX_DX12,
	NVNGX_VULKAN,
} NVNGX_Api;

class Config
{
public:
	Config(std::wstring fileName);

	// Depth
	std::optional<bool> DepthInverted;

	// Color
	std::optional<bool> AutoExposure;
	std::optional<bool> HDR;

	// Motion
	std::optional<bool> JitterCancellation;
	std::optional<bool> DisplayResolution;

	//Logging
	std::optional<bool> LoggingEnabled;
	std::optional<bool> LogToFile;
	std::optional<bool> LogToConsole;
	std::optional<bool> LogToNGX;
	std::optional<bool> OpenConsole;
	std::optional<int> LogLevel;
	std::optional<std::string> LogFileName;

	// XeSS
	std::optional<bool> BuildPipelines;
	std::optional<int32_t> NetworkModel;

	// CAS
	std::optional<bool> CasEnabled;
	std::optional<int> CasColorSpaceConversion;

	//Sharpness 
	std::optional<bool> OverrideSharpness;
	std::optional<float> Sharpness;

	// Upscale Ratio Override
	std::optional<bool> UpscaleRatioOverrideEnabled;
	std::optional<bool> DrsMaxOverrideEnabled;
	std::optional<float> UpscaleRatioOverrideValue;
	
	// Quality Overrides
	std::optional<bool> QualityRatioOverrideEnabled;
	std::optional<float> QualityRatio_UltraQuality;
	std::optional<float> QualityRatio_Quality;
	std::optional<float> QualityRatio_Balanced;
	std::optional<float> QualityRatio_Performance;
	std::optional<float> QualityRatio_UltraPerformance;

	//Hotfixes
	std::optional<bool> DisableReactiveMask;
	std::optional<float> MipmapBiasOverride;
	std::optional<bool> RestoreComputeSignature;
	std::optional<bool> RestoreGraphicSignature;
	std::optional<bool> UWPTempFolder;
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
	
	std::optional<bool> SuperSamplingEnabled;
	std::optional<float> SuperSamplingMultiplier;

	// fsr
	std::optional<float> FsrVerticalFov;
	std::optional<float> FsrHorizontalFov;

	// dx11wdx12
	std::optional<int> UseSafeSyncQueries;
	std::optional<bool> Dx11DelayedInit;

	// nvngx info
	NVSDK_NGX_EngineType NVNGX_Engine = NVSDK_NGX_ENGINE_TYPE_CUSTOM;
	bool NVNGX_EngineVersion5 = false;
	NVNGX_Api Api = NVNGX_NOT_SELECTED;
	NVSDK_NGX_LoggingInfo NVSDK_Logger{ nullptr, NVSDK_NGX_LOGGING_LEVEL_OFF, false };
	IFeature* CurrentFeature = nullptr;
	int ActiveFeatureCount = 0;

	// for realtime changes
	bool changeBackend = false;
	bool changeCAS = false;
	std::string newBackend = "";
	bool xessDebug = false;
	int xessDebugFrames = 5;
	float lastMipBias = 0.0f;
	
	void Reload();
	bool SaveIni();

	static Config* Instance();

private:
	inline static Config* _config;

	CSimpleIniA ini;
	std::filesystem::path absoluteFileName;

	std::optional<std::string> readString(std::string section, std::string key, bool lowercase = false);
	std::optional<float> readFloat(std::string section, std::string key);
	std::optional<int> readInt(std::string section, std::string key);
	std::optional<bool> readBool(std::string section, std::string key);
};
