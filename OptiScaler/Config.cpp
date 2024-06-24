#pragma once
#include "pch.h"
#include "Config.h"
#include "Util.h"

Config::Config(std::wstring fileName)
{
	absoluteFileName = Util::DllPath().parent_path() / fileName;
	Reload();
}

bool Config::Reload()
{
	if (ini.LoadFile(absoluteFileName.c_str()) == SI_OK)
	{
		// Upscalers
		Dx11Upscaler = readString("Upscalers", "Dx11Upscaler", true);
		Dx12Upscaler = readString("Upscalers", "Dx12Upscaler", true);
		VulkanUpscaler = readString("Upscalers", "VulkanUpscaler", true);
		OutputScalingEnabled = readBool("Upscalers", "OutputScalingEnabled");
		OutputScalingMultiplier = readFloat("Upscalers", "OutputScalingMultiplier");

		if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() < 0.5f)
			OutputScalingMultiplier = 0.5f;
		else if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() > 3.0f)
			OutputScalingMultiplier = 3.0f;

		// XeSS
		BuildPipelines = readBool("XeSS", "BuildPipelines");
		NetworkModel = readInt("XeSS", "NetworkModel");
		CreateHeaps = readBool("XeSS", "CreateHeaps");
		XeSSLibrary = readString("XeSS", "LibraryPath");

		// DLSS
		DLSSEnabled = readBool("DLSS", "Enabled");
		DLSSLibrary = readString("DLSS", "LibraryPath");
		RenderPresetOverride = readBool("DLSS", "RenderPresetOverride");
		RenderPresetDLAA = readInt("DLSS", "RenderPresetDLAA");
		RenderPresetUltraQuality = readInt("DLSS", "RenderPresetUltraQuality");
		RenderPresetQuality = readInt("DLSS", "RenderPresetQuality");
		RenderPresetBalanced = readInt("DLSS", "RenderPresetBalanced");
		RenderPresetPerformance = readInt("DLSS", "RenderPresetPerformance");
		RenderPresetUltraPerformance = readInt("DLSS", "RenderPresetUltraPerformance");

		if (RenderPresetDLAA.has_value() && (RenderPresetDLAA.value() < 0 || RenderPresetDLAA.value() > 7))
			RenderPresetDLAA.reset();

		if (RenderPresetUltraQuality.has_value() && (RenderPresetUltraQuality.value() < 0 || RenderPresetUltraQuality.value() > 7))
			RenderPresetUltraQuality.reset();

		if (RenderPresetQuality.has_value() && (RenderPresetQuality.value() < 0 || RenderPresetQuality.value() > 7))
			RenderPresetQuality.reset();

		if (RenderPresetBalanced.has_value() && (RenderPresetBalanced.value() < 0 || RenderPresetBalanced.value() > 7))
			RenderPresetBalanced.reset();

		if (RenderPresetPerformance.has_value() && (RenderPresetPerformance.value() < 0 || RenderPresetPerformance.value() > 7))
			RenderPresetPerformance.reset();

		if (RenderPresetUltraPerformance.has_value() && (RenderPresetUltraPerformance.value() < 0 || RenderPresetUltraPerformance.value() > 7))
			RenderPresetUltraPerformance.reset();

		// logging
		LoggingEnabled = readBool("Log", "LoggingEnabled");

		if (LoggingEnabled.value_or(true))
			LogLevel = readInt("Log", "LogLevel");
		else
			LogLevel = spdlog::level::off;

		LogToConsole = readBool("Log", "LogToConsole");
		LogToFile = readBool("Log", "LogToFile");
		LogToNGX = readBool("Log", "LogToNGX");
		OpenConsole = readBool("Log", "OpenConsole");
		LogFileName = readString("Log", "LogFile");

		if (!LogFileName.has_value())
		{
			auto logFile = Util::DllPath().parent_path() / "OptiScaler.log";
			LogFileName = logFile.string();
		}

		// Sharpness
		OverrideSharpness = readBool("Sharpness", "OverrideSharpness");
		Sharpness = readFloat("Sharpness", "Sharpness");

		// Menu
		MenuScale = readFloat("Menu", "Scale");
		OverlayMenu = readBool("Menu", "OverlayMenu");
		ShortcutKey = readInt("Menu", "ShortcutKey");
		ResetKey = readInt("Menu", "ResetKey");
		MenuInitDelay = readInt("Menu", "MenuInitDelay");

		// Hooks
		HookOriginalNvngxOnly = readBool("Hooks", "HookOriginalNvngxOnly");
		DisableEarlyHooking = readBool("Hooks", "DisableEarlyHooking");
		HookD3D12 = readBool("Hooks", "HookD3D12");
		HookSLProxy = readBool("Hooks", "HookSLProxy");
		HookFSR3Proxy = readBool("Hooks", "HookFSR3Proxy");

		// CAS
		RcasEnabled = readBool("CAS", "Enabled");
		MotionSharpnessEnabled = readBool("CAS", "MotionSharpnessEnabled");
		MotionSharpness = readFloat("CAS", "MotionSharpness");
		MotionSharpnessDebug = readBool("CAS", "MotionSharpnessDebug");
		MotionThreshold = readFloat("CAS", "MotionThreshold");
		MotionScaleLimit = readFloat("CAS", "MotionScaleLimit");

		if (Sharpness.has_value())
		{
			if (Sharpness.value() > 1.0f)
				Sharpness = 1.0f;
			else if (Sharpness.value() < 0.0f)
				Sharpness.reset();
		}

		if (MotionSharpness.has_value())
		{
			if (MotionSharpness.value() > 1.0f)
				MotionSharpness = 1.0f;
			else if (MotionSharpness.value() < 0.0f)
				MotionSharpness.reset();
		}

		if (MotionThreshold.has_value())
		{
			if (MotionThreshold.value() > 100.0f)
				MotionThreshold = 100.0f;
			else if (MotionThreshold.value() < 0.0f)
				MotionThreshold.reset();
		}

		if (MotionScaleLimit.has_value())
		{
			if (MotionScaleLimit.value() > 100.0f)
				MotionScaleLimit = 100.0f;
			else if (MotionScaleLimit.value() < 0.01f)
				MotionScaleLimit.reset();
		}

		// Depth
		DepthInverted = readBool("Depth", "DepthInverted");

		// Color
		AutoExposure = readBool("Color", "AutoExposure");
		HDR = readBool("Color", "HDR");

		// MotionVectors
		JitterCancellation = readBool("MotionVectors", "JitterCancellation");
		DisplayResolution = readBool("MotionVectors", "DisplayResolution");


		// DRS
		DrsMinOverrideEnabled = readBool("DRS", "DrsMinOverrideEnabled");
		DrsMaxOverrideEnabled = readBool("DRS", "DrsMaxOverrideEnabled");

		//Upscale Ratio Override
		UpscaleRatioOverrideEnabled = readBool("UpscaleRatio", "UpscaleRatioOverrideEnabled");
		UpscaleRatioOverrideValue = readFloat("UpscaleRatio", "UpscaleRatioOverrideValue");

		// Quality Overrides
		QualityRatioOverrideEnabled = readBool("QualityOverrides", "QualityRatioOverrideEnabled");
		QualityRatio_DLAA = readFloat("QualityOverrides", "QualityRatioDLAA");
		QualityRatio_UltraQuality = readFloat("QualityOverrides", "QualityRatioUltraQuality");
		QualityRatio_Quality = readFloat("QualityOverrides", "QualityRatioQuality");
		QualityRatio_Balanced = readFloat("QualityOverrides", "QualityRatioBalanced");
		QualityRatio_Performance = readFloat("QualityOverrides", "QualityRatioPerformance");
		QualityRatio_UltraPerformance = readFloat("QualityOverrides", "QualityRatioUltraPerformance");

		// hotfixes
		DisableReactiveMask = readBool("Hotfix", "DisableReactiveMask");
		RoundInternalResolution = readInt("Hotfix", "RoundInternalResolution");
		MipmapBiasOverride = readFloat("Hotfix", "MipmapBiasOverride");

		if (MipmapBiasOverride.has_value() && (MipmapBiasOverride.value() > 15.0 || MipmapBiasOverride.value() < -15.0))
			MipmapBiasOverride.reset();

		RestoreComputeSignature = readBool("Hotfix", "RestoreComputeSignature");
		RestoreGraphicSignature = readBool("Hotfix", "RestoreGraphicSignature");

		ColorResourceBarrier = readInt("Hotfix", "ColorResourceBarrier");
		MVResourceBarrier = readInt("Hotfix", "MotionVectorResourceBarrier");
		DepthResourceBarrier = readInt("Hotfix", "DepthResourceBarrier");
		MaskResourceBarrier = readInt("Hotfix", "ColorMaskResourceBarrier");
		ExposureResourceBarrier = readInt("Hotfix", "ExposureResourceBarrier");
		OutputResourceBarrier = readInt("Hotfix", "OutputResourceBarrier");

		// fsr
		FsrVerticalFov = readFloat("FSR", "VerticalFov");
		FsrHorizontalFov = readFloat("FSR", "HorizontalFov");

		// dx11wdx12
		TextureSyncMethod = readInt("Dx11withDx12", "TextureSyncMethod");
		CopyBackSyncMethod = readInt("Dx11withDx12", "CopyBackSyncMethod");
		Dx11DelayedInit = readInt("Dx11withDx12", "UseDelayedInit");
		SyncAfterDx12 = readInt("Dx11withDx12", "SyncAfterDx12");

		// nvapi
		OverrideNvapiDll = readBool("NvApi", "OverrideNvapiDll");
		NvapiDllPath = readString("NvApi", "NvapiDllPath", true);

		// spoofing
		DxgiSpoofing = readBool("Spoofing", "Dxgi");
		VulkanSpoofing = readBool("Spoofing", "Vulkan");		

		// plugins
		PluginPath = readString("Plugins", "Path", true);

		if (!PluginPath.has_value())
			PluginPath = (Util::DllPath().parent_path() / "plugins").string();

		return true;
	}

	return false;
}

bool Config::LoadFromPath(const wchar_t* InPath)
{
	std::filesystem::path iniPath(InPath);
	absoluteFileName = iniPath / L"nvngx.ini";
	return Reload();
}

std::string GetBoolValue(std::optional<bool> value)
{
	if (!value.has_value())
		return "auto";

	return value.value() ? "true" : "false";
}

std::string GetIntValue(std::optional<int> value)
{
	if (!value.has_value())
		return "auto";

	return std::to_string(value.value());
}

std::string GetFloatValue(std::optional<float> value)
{
	if (!value.has_value())
		return "auto";

	return std::to_string(value.value());
}

bool Config::SaveIni()
{
	// Upscalers 
	ini.SetValue("Upscalers", "Dx11Upscaler", Instance()->Dx11Upscaler.value_or("auto").c_str());
	ini.SetValue("Upscalers", "Dx12Upscaler", Instance()->Dx12Upscaler.value_or("auto").c_str());
	ini.SetValue("Upscalers", "VulkanUpscaler", Instance()->VulkanUpscaler.value_or("auto").c_str());
	ini.SetValue("Upscalers", "OutputScalingEnabled", GetBoolValue(Instance()->OutputScalingEnabled).c_str());
	ini.SetValue("Upscalers", "OutputScalingMultiplier", GetFloatValue(Instance()->OutputScalingMultiplier).c_str());

	// XeSS
	ini.SetValue("XeSS", "BuildPipelines", GetBoolValue(Instance()->BuildPipelines).c_str());
	ini.SetValue("XeSS", "CreateHeaps", GetBoolValue(Instance()->CreateHeaps).c_str());
	ini.SetValue("XeSS", "NetworkModel", GetIntValue(Instance()->NetworkModel).c_str());
	ini.SetValue("XeSS", "LibraryPath", Instance()->XeSSLibrary.value_or("auto").c_str());

	// DLSS
	ini.SetValue("DLSS", "Enabled", GetBoolValue(Instance()->DLSSEnabled).c_str());
	ini.SetValue("DLSS", "LibraryPath", Instance()->DLSSLibrary.value_or("auto").c_str());
	ini.SetValue("DLSS", "RenderPresetOverride", GetBoolValue(Instance()->RenderPresetOverride).c_str());
	ini.SetValue("DLSS", "RenderPresetDLAA", GetIntValue(Instance()->RenderPresetDLAA).c_str());
	ini.SetValue("DLSS", "RenderPresetUltraQuality", GetIntValue(Instance()->RenderPresetUltraQuality).c_str());
	ini.SetValue("DLSS", "RenderPresetQuality", GetIntValue(Instance()->RenderPresetQuality).c_str());
	ini.SetValue("DLSS", "RenderPresetBalanced", GetIntValue(Instance()->RenderPresetBalanced).c_str());
	ini.SetValue("DLSS", "RenderPresetPerformance", GetIntValue(Instance()->RenderPresetPerformance).c_str());
	ini.SetValue("DLSS", "RenderPresetUltraPerformance", GetIntValue(Instance()->RenderPresetUltraPerformance).c_str());

	// Sharpness
	ini.SetValue("Sharpness", "OverrideSharpness", GetBoolValue(Instance()->OverrideSharpness).c_str());
	ini.SetValue("Sharpness", "Sharpness", GetFloatValue(Instance()->Sharpness).c_str());

	// Ingame menu
	ini.SetValue("Menu", "Scale", GetFloatValue(Instance()->MenuScale).c_str());
	ini.SetValue("Menu", "OverlayMenu", GetBoolValue(Instance()->OverlayMenu).c_str());
	ini.SetValue("Menu", "ResetKey", GetIntValue(Instance()->ResetKey).c_str());
	ini.SetValue("Menu", "ShortcutKey", GetIntValue(Instance()->ShortcutKey).c_str());
	ini.SetValue("Menu", "MenuInitDelay", GetIntValue(Instance()->MenuInitDelay).c_str());

	// Hooks
	ini.SetValue("Hooks", "HookOriginalNvngxOnly", GetBoolValue(Instance()->HookOriginalNvngxOnly).c_str());
	ini.SetValue("Hooks", "DisableEarlyHooking", GetBoolValue(Instance()->DisableEarlyHooking).c_str());
	ini.SetValue("Hooks", "HookD3D12", GetBoolValue(Instance()->HookD3D12).c_str());
	ini.SetValue("Hooks", "HookSLProxy", GetBoolValue(Instance()->HookSLProxy).c_str());
	ini.SetValue("Hooks", "HookFSR3Proxy", GetBoolValue(Instance()->HookFSR3Proxy).c_str());

	// CAS
	ini.SetValue("CAS", "Enabled", GetBoolValue(Instance()->RcasEnabled).c_str());
	ini.SetValue("CAS", "MotionSharpnessEnabled", GetBoolValue(Instance()->MotionSharpnessEnabled).c_str());
	ini.SetValue("CAS", "MotionSharpnessDebug", GetBoolValue(Instance()->MotionSharpnessDebug).c_str());
	ini.SetValue("CAS", "MotionSharpness", GetFloatValue(Instance()->MotionSharpness).c_str());
	ini.SetValue("CAS", "MotionThreshold", GetFloatValue(Instance()->MotionThreshold).c_str());
	ini.SetValue("CAS", "MotionScaleLimit", GetFloatValue(Instance()->MotionScaleLimit).c_str());

	// Depth
	ini.SetValue("Depth", "DepthInverted", GetBoolValue(Instance()->DepthInverted).c_str());

	// Color
	ini.SetValue("Color", "AutoExposure", GetBoolValue(Instance()->AutoExposure).c_str());
	ini.SetValue("Color", "HDR", GetBoolValue(Instance()->HDR).c_str());

	// MotionVectors
	ini.SetValue("MotionVectors", "JitterCancellation", GetBoolValue(Instance()->JitterCancellation).c_str());
	ini.SetValue("MotionVectors", "DisplayResolution", GetBoolValue(Instance()->DisplayResolution).c_str());

	// Upscale Ratio Override
	ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideEnabled", GetBoolValue(Instance()->UpscaleRatioOverrideEnabled).c_str());
	ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideValue", GetFloatValue(Instance()->UpscaleRatioOverrideValue).c_str());

	// Quality Overrides
	ini.SetValue("QualityOverrides", "QualityRatioOverrideEnabled", GetBoolValue(Instance()->QualityRatioOverrideEnabled).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioDLAA", GetFloatValue(Instance()->QualityRatio_DLAA).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioUltraQuality", GetFloatValue(Instance()->QualityRatio_UltraQuality).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioQuality", GetFloatValue(Instance()->QualityRatio_Quality).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioBalanced", GetFloatValue(Instance()->QualityRatio_Balanced).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioPerformance", GetFloatValue(Instance()->QualityRatio_Performance).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioUltraPerformance", GetFloatValue(Instance()->QualityRatio_UltraPerformance).c_str());

	// Hotfixes
	ini.SetValue("Hotfix", "DisableReactiveMask", GetBoolValue(Instance()->DisableReactiveMask).c_str());
	ini.SetValue("Hotfix", "MipmapBiasOverride", GetFloatValue(Instance()->MipmapBiasOverride).c_str());
	ini.SetValue("Hotfix", "RoundInternalResolution", GetIntValue(Instance()->RoundInternalResolution).c_str());
	
	ini.SetValue("Hotfix", "RestoreComputeSignature", GetBoolValue(Instance()->RestoreComputeSignature).c_str());
	ini.SetValue("Hotfix", "RestoreGraphicSignature", GetBoolValue(Instance()->RestoreGraphicSignature).c_str());

	ini.SetValue("Hotfix", "ColorResourceBarrier", GetIntValue(Instance()->ColorResourceBarrier).c_str());
	ini.SetValue("Hotfix", "MotionVectorResourceBarrier", GetIntValue(Instance()->MVResourceBarrier).c_str());
	ini.SetValue("Hotfix", "DepthResourceBarrier", GetIntValue(Instance()->DepthResourceBarrier).c_str());
	ini.SetValue("Hotfix", "ColorMaskResourceBarrier", GetIntValue(Instance()->MaskResourceBarrier).c_str());
	ini.SetValue("Hotfix", "ExposureResourceBarrier", GetIntValue(Instance()->ExposureResourceBarrier).c_str());
	ini.SetValue("Hotfix", "OutputResourceBarrier", GetIntValue(Instance()->OutputResourceBarrier).c_str());

	// FSR
	ini.SetValue("FSR", "VerticalFov", GetFloatValue(Instance()->FsrVerticalFov).c_str());
	ini.SetValue("FSR", "HorizontalFov", GetFloatValue(Instance()->FsrHorizontalFov).c_str());

	// Dx11 with Dx12
	ini.SetValue("Dx11withDx12", "TextureSyncMethod", GetIntValue(Instance()->TextureSyncMethod).c_str());
	ini.SetValue("Dx11withDx12", "CopyBackSyncMethod", GetIntValue(Instance()->CopyBackSyncMethod).c_str());
	ini.SetValue("Dx11withDx12", "SyncAfterDx12", GetBoolValue(Instance()->SyncAfterDx12).c_str());
	ini.SetValue("Dx11withDx12", "UseDelayedInit", GetBoolValue(Instance()->Dx11DelayedInit).c_str());
	ini.SetValue("Dx11withDx12", "DontUseNTShared", GetBoolValue(Instance()->DontUseNTShared).c_str());

	// Logging
	ini.SetValue("Log", "LoggingEnabled", GetBoolValue(Instance()->LoggingEnabled).c_str());
	ini.SetValue("Log", "LogLevel", GetIntValue(Instance()->LogLevel).c_str());
	ini.SetValue("Log", "LogToConsole", GetBoolValue(Instance()->LogToConsole).c_str());
	ini.SetValue("Log", "LogToFile", GetBoolValue(Instance()->LogToFile).c_str());
	ini.SetValue("Log", "LogToNGX", GetBoolValue(Instance()->LogToNGX).c_str());
	ini.SetValue("Log", "OpenConsole", GetBoolValue(Instance()->OpenConsole).c_str());
	ini.SetValue("Log", "LogFile", Instance()->LogFileName.value_or("auto").c_str());

	// nvapi
	ini.SetValue("NvApi", "OverrideNvapiDll", GetBoolValue(Instance()->OverrideNvapiDll).c_str());
	ini.SetValue("NvApi", "NvapiDllPath", Instance()->NvapiDllPath.value_or("auto").c_str());

	// DRS
	ini.SetValue("DRS", "DrsMinOverrideEnabled", GetBoolValue(Instance()->DrsMinOverrideEnabled).c_str());
	ini.SetValue("DRS", "DrsMaxOverrideEnabled", GetBoolValue(Instance()->DrsMaxOverrideEnabled).c_str());

	// Spoofing
	ini.SetValue("Spoofing", "Dxgi", GetBoolValue(Instance()->DxgiSpoofing).c_str());
	ini.SetValue("Spoofing", "Vulkan", GetBoolValue(Instance()->VulkanSpoofing).c_str());

	// Plugins
	ini.SetValue("Plugins", "Path", Instance()->PluginPath.value_or("auto").c_str());


	return ini.SaveFile(absoluteFileName.wstring().c_str()) >= 0;
}

std::optional<std::string> Config::readString(std::string section, std::string key, bool lowercase)
{
	std::string value = ini.GetValue(section.c_str(), key.c_str(), "auto");

	std::string lower = value;
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

	if (lower == "auto")
	{
		return std::nullopt;
	}
	return lowercase ? lower : value;
}

std::optional<float> Config::readFloat(std::string section, std::string key)
{
	auto value = readString(section, key);
	try
	{
		return std::stof(value.value());
	}
	catch (const std::bad_optional_access&) // missing or auto value
	{
		return std::nullopt;
	}
	catch (const std::invalid_argument&) // invalid float string for std::stof
	{
		return std::nullopt;
	}
	catch (const std::out_of_range&) // out of range for 32 bit float
	{
		return std::nullopt;
	}
}

std::optional<int> Config::readInt(std::string section, std::string key)
{
	auto value = readString(section, key);
	try
	{
		return std::stoi(value.value());
	}
	catch (const std::bad_optional_access&) // missing or auto value
	{
		return std::nullopt;
	}
	catch (const std::invalid_argument&) // invalid float string for std::stof
	{
		return std::nullopt;
	}
	catch (const std::out_of_range&) // out of range for 32 bit float
	{
		return std::nullopt;
	}
}

std::optional<bool> Config::readBool(std::string section, std::string key)
{
	auto value = readString(section, key, true);
	if (value == "true")
	{
		return true;
	}
	else if (value == "false")
	{
		return false;
	}

	return std::nullopt;
}

Config* Config::Instance()
{
	if (!_config)
		_config = new Config(L"nvngx.ini");

	return _config;
}