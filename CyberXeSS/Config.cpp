#include "pch.h"
#include "Config.h"
#include "Util.h"

Config::Config(std::wstring fileName)
{
	absoluteFileName = Util::DllPath().parent_path() / fileName;
	Reload();
}

void Config::Reload()
{
	LogLevel = 2;
	LoggingEnabled = false;

	NetworkModel = 0;
	BuildPipelines = true;
	DelayedInit = false;

	if (ini.LoadFile(absoluteFileName.c_str()) == SI_OK)
	{
		DelayedInit = readBool("XeSS", "DelayedInit");
		BuildPipelines = readBool("XeSS", "BuildPipelines");
		NetworkModel = readInt("XeSS", "NetworkModel");

		LoggingEnabled = readBool("Log", "LoggingEnabled");

		if (LoggingEnabled.value_or(false))
		{
			LogLevel = readInt("Log", "LogLevel");
			LogToConsole = readBool("Log", "LogToConsole");

			if (!LogToConsole.value_or(false))
			{
				LogFileName = readString("Log", "LogFile");

				if (!LogFileName.has_value())
				{
					const auto now = std::chrono::system_clock::now();
					auto str = std::format("{:%d%m%Y_%H%M%OS}", now);

					auto path = Util::DllPath().parent_path();
					auto logFile = path.string() + "/log_xess_" + str + ".log";

					LogFileName = logFile;
				}
			}
		}
		else
			LogLevel = 6;



		// CAS
		CasEnabled = readBool("CAS", "Enabled");
		CasSharpness = readFloat("CAS", "Sharpness");
		ColorSpaceConversion = readInt("CAS", "ColorSpaceConversion");

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
		UpscaleRatioOverrideValue = readFloat("UpscaleRatio", "UpscaleRatioOverrideValue");

		// Quality Overrides
		QualityRatioOverrideEnabled = readBool("QualityOverrides", "QualityRatioOverrideEnabled");
		if (QualityRatioOverrideEnabled) {
			QualityRatio_UltraQuality = readFloat("QualityOverrides", "QualityRatioUltraQuality");
			QualityRatio_Quality = readFloat("QualityOverrides", "QualityRatioQuality");
			QualityRatio_Balanced = readFloat("QualityOverrides", "QualityRatioBalanced");
			QualityRatio_Performance = readFloat("QualityOverrides", "QualityRatioPerformance");
			QualityRatio_UltraPerformance = readFloat("QualityOverrides", "QualityRatioUltraPerformance");
		}

		DisableReactiveMask = readBool("Hotfix", "DisableReactiveMask");
	}
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

