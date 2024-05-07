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
		SuperSamplingEnabled = readBool("Upscalers", "SuperSamplingEnabled");
		SuperSamplingMultiplier = readFloat("Upscalers", "SuperSamplingMultiplier");

		// XeSS
		BuildPipelines = readBool("XeSS", "BuildPipelines");
		NetworkModel = readInt("XeSS", "NetworkModel");
		CreateHeaps = readBool("XeSS", "CreateHeaps");
		XeSSLibrary = readString("XeSS", "LibraryPath");
		
		// DLSS
		DLSSLibrary = readString("DLSS", "LibraryPath");

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

		// CAS
		CasEnabled = readBool("CAS", "Enabled");
		CasColorSpaceConversion = readInt("CAS", "ColorSpaceConversion");

		// Depth
		DepthInverted = readBool("Depth", "DepthInverted");

		// Color
		AutoExposure = readBool("Color", "AutoExposure");
		HDR = readBool("Color", "HDR");

		// MotionVectors
		JitterCancellation = readBool("MotionVectors", "JitterCancellation");
		DisplayResolution = readBool("MotionVectors", "DisplayResolution");

		//Upscale Ratio Override
		UpscaleRatioOverrideEnabled = readBool("UpscaleRatio", "UpscaleRatioOverrideEnabled");
		DrsMaxOverrideEnabled = readBool("UpscaleRatio", "DrsMaxOverrideEnabled");
		UpscaleRatioOverrideValue = readFloat("UpscaleRatio", "UpscaleRatioOverrideValue");

		// Quality Overrides
		QualityRatioOverrideEnabled = readBool("QualityOverrides", "QualityRatioOverrideEnabled");
		QualityRatio_DLAA = readFloat("QualityOverrides", "QualityRatioDLAA");
			QualityRatio_UltraQuality = readFloat("QualityOverrides", "QualityRatioUltraQuality");
			QualityRatio_Quality = readFloat("QualityOverrides", "QualityRatioQuality");
			QualityRatio_Balanced = readFloat("QualityOverrides", "QualityRatioBalanced");
			QualityRatio_Performance = readFloat("QualityOverrides", "QualityRatioPerformance");
			QualityRatio_UltraPerformance = readFloat("QualityOverrides", "QualityRatioUltraPerformance");
		}

		// hotfixes
		DisableReactiveMask = readBool("Hotfix", "DisableReactiveMask");
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
	ini.SetValue("Upscalers", "SuperSamplingEnabled", GetBoolValue(Instance()->SuperSamplingEnabled).c_str());
	ini.SetValue("Upscalers", "SuperSamplingMultiplier", GetFloatValue(Instance()->SuperSamplingMultiplier).c_str());

	// XeSS
	ini.SetValue("XeSS", "BuildPipelines", GetBoolValue(Instance()->BuildPipelines).c_str());
	ini.SetValue("XeSS", "CreateHeaps", GetBoolValue(Instance()->CreateHeaps).c_str());
	ini.SetValue("XeSS", "NetworkModel", GetIntValue(Instance()->NetworkModel).c_str());
	ini.SetValue("XeSS", "LibraryPath", Instance()->XeSSLibrary.value_or("auto").c_str());
	
	// DLSS
	ini.SetValue("DLSS", "LibraryPath", Instance()->DLSSLibrary.value_or("auto").c_str());

	// Sharpness
	ini.SetValue("Sharpness", "OverrideSharpness", GetBoolValue(Instance()->OverrideSharpness).c_str());
	ini.SetValue("Sharpness", "Sharpness", GetFloatValue(Instance()->Sharpness).c_str());
	
	// ingame menu
	ini.SetValue("Menu", "Scale", GetFloatValue(Instance()->MenuScale).c_str());

	// CAS
	ini.SetValue("CAS", "Enabled", GetBoolValue(Instance()->CasEnabled).c_str());
	ini.SetValue("CAS", "ColorSpaceConversion", GetIntValue(Instance()->CasColorSpaceConversion).c_str());

	// Depth
	ini.SetValue("Depth", "DepthInverted", GetBoolValue(Instance()->DepthInverted).c_str());

	// Color
	ini.SetValue("Color", "AutoExposure", GetBoolValue(Instance()->AutoExposure).c_str());
	ini.SetValue("Color", "HDR", GetBoolValue(Instance()->HDR).c_str());

	// MotionVectors
	ini.SetValue("MotionVectors", "JitterCancellation", GetBoolValue(Instance()->JitterCancellation).c_str());
	ini.SetValue("MotionVectors", "DisplayResolution", GetBoolValue(Instance()->DisplayResolution).c_str());

	//Upscale Ratio Override
	ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideEnabled", GetBoolValue(Instance()->UpscaleRatioOverrideEnabled).c_str());
	ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideValue", GetFloatValue(Instance()->UpscaleRatioOverrideValue).c_str());

	// Quality Overrides
	ini.SetValue("QualityOverrides", "QualityRatioOverrideEnabled", GetBoolValue(Instance()->QualityRatioOverrideEnabled).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioUltraQuality", GetFloatValue(Instance()->QualityRatio_UltraQuality).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioQuality", GetFloatValue(Instance()->QualityRatio_Quality).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioBalanced", GetFloatValue(Instance()->QualityRatio_Balanced).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioPerformance", GetFloatValue(Instance()->QualityRatio_Performance).c_str());
	ini.SetValue("QualityOverrides", "QualityRatioUltraPerformance", GetFloatValue(Instance()->QualityRatio_UltraPerformance).c_str());

	// hotfixes
	ini.SetValue("Hotfix", "DisableReactiveMask", GetBoolValue(Instance()->DisableReactiveMask).c_str());
	ini.SetValue("Hotfix", "MipmapBiasOverride", GetBoolValue(Instance()->MipmapBiasOverride).c_str());

	ini.SetValue("Hotfix", "RestoreComputeSignature", GetBoolValue(Instance()->RestoreComputeSignature).c_str());
	ini.SetValue("Hotfix", "RestoreGraphicSignature", GetBoolValue(Instance()->RestoreGraphicSignature).c_str());

	ini.SetValue("Hotfix", "ColorResourceBarrier", GetIntValue(Instance()->ColorResourceBarrier).c_str());
	ini.SetValue("Hotfix", "MotionVectorResourceBarrier", GetIntValue(Instance()->MVResourceBarrier).c_str());
	ini.SetValue("Hotfix", "DepthResourceBarrier", GetIntValue(Instance()->DepthResourceBarrier).c_str());
	ini.SetValue("Hotfix", "ColorMaskResourceBarrier", GetIntValue(Instance()->MaskResourceBarrier).c_str());
	ini.SetValue("Hotfix", "ExposureResourceBarrier", GetIntValue(Instance()->ExposureResourceBarrier).c_str());
	ini.SetValue("Hotfix", "OutputResourceBarrier", GetIntValue(Instance()->OutputResourceBarrier).c_str());

	// fsr
	ini.SetValue("FSR", "VerticalFov", GetFloatValue(Instance()->FsrVerticalFov).c_str());
	ini.SetValue("FSR", "HorizontalFov", GetFloatValue(Instance()->FsrHorizontalFov).c_str());

	// dx11wdx12
	ini.SetValue("Dx11withDx12", "TextureSyncMethod", GetIntValue(Instance()->TextureSyncMethod).c_str());
	ini.SetValue("Dx11withDx12", "CopyBackSyncMethod", GetIntValue(Instance()->CopyBackSyncMethod).c_str());
	ini.SetValue("Dx11withDx12", "SyncAfterDx12", GetBoolValue(Instance()->SyncAfterDx12).c_str());
	ini.SetValue("Dx11withDx12", "UseDelayedInit", GetBoolValue(Instance()->Dx11DelayedInit).c_str());

	// log
	ini.SetValue("Log", "LoggingEnabled", GetBoolValue(Instance()->LoggingEnabled).c_str());
	ini.SetValue("Log", "LogLevel", GetIntValue(Instance()->LogLevel).c_str());
	ini.SetValue("Log", "LogToConsole", GetBoolValue(Instance()->LogToConsole).c_str());
	ini.SetValue("Log", "LogToFile", GetBoolValue(Instance()->LogToFile).c_str());
	ini.SetValue("Log", "LogToNGX", GetBoolValue(Instance()->LogToNGX).c_str());
	ini.SetValue("Log", "OpenConsole", GetBoolValue(Instance()->OpenConsole).c_str());
	ini.SetValue("Log", "LogFile", Instance()->LogFileName.value_or("auto").c_str());

	auto fileName = absoluteFileName.wstring().c_str();
	return ini.SaveFile(fileName) >= 0;
}

std::optional<std::string> Config::readString(std::string section, std::string key, bool lowercase)
{
	std::string value = ini.GetValue(section.c_str(), key.c_str(), "auto");

	std::string lower = value;
	std::transform(
		lower.begin(), lower.end(),
		lower.begin(),
		[](unsigned char c)
		{
			return std::tolower(c);
		}
	);

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