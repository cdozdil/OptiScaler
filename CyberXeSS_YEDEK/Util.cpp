#include "pch.h"
#include "Config.h"
#include "Util.h"

namespace fs = std::filesystem;

extern HMODULE dllModule;

fs::path Util::DllPath()
{
	static fs::path dll;
	if (dll.empty())
	{
		wchar_t dllPath[MAX_PATH];
		GetModuleFileNameW(dllModule, dllPath, MAX_PATH);
		dll = fs::path(dllPath);
	}
	return dll;
}

fs::path Util::ExePath()
{
	static fs::path exe;
	if (exe.empty())
	{
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		exe = fs::path(exePath);
	}
	return exe;
}

Util::NvParameter Util::NvParameterToEnum(const char* name)
{
	static ankerl::unordered_dense::map<std::string, NvParameter> NvParamTranslation = {
		{"SuperSampling.ScaleFactor", NvParameter::SuperSampling_ScaleFactor},
		{"SuperSampling.Available", NvParameter::SuperSampling_Available},
		{"SuperSampling.MinDriverVersionMajor", NvParameter::SuperSampling_MinDriverVersionMajor},
		{"SuperSampling.MinDriverVersionMinor", NvParameter::SuperSampling_MinDriverVersionMinor},
		{"SuperSampling.FeatureInitResult", NvParameter::SuperSampling_FeatureInitResult},
		{"SuperSampling.NeedsUpdatedDriver", NvParameter::SuperSampling_NeedsUpdatedDriver},
		{"#\x01", NvParameter::SuperSampling_Available},

		{"Width", NvParameter::Width},
		{"Height", NvParameter::Height},
		{"PerfQualityValue", NvParameter::PerfQualityValue},
		{"RTXValue", NvParameter::RTXValue},
		{"NVSDK_NGX_Parameter_FreeMemOnReleaseFeature", NvParameter::FreeMemOnReleaseFeature},

		{"OutWidth", NvParameter::OutWidth},
		{"OutHeight", NvParameter::OutHeight},

		{"DLSS.Render.Subrect.Dimensions.Width", NvParameter::DLSS_Render_Subrect_Dimensions_Width},
		{"DLSS.Render.Subrect.Dimensions.Height", NvParameter::DLSS_Render_Subrect_Dimensions_Height},
		{"DLSS.Get.Dynamic.Max.Render.Width", NvParameter::DLSS_Get_Dynamic_Max_Render_Width},
		{"DLSS.Get.Dynamic.Max.Render.Height", NvParameter::DLSS_Get_Dynamic_Max_Render_Height},
		{"DLSS.Get.Dynamic.Min.Render.Width", NvParameter::DLSS_Get_Dynamic_Min_Render_Width},
		{"DLSS.Get.Dynamic.Min.Render.Height", NvParameter::DLSS_Get_Dynamic_Min_Render_Height},
		{"Sharpness", NvParameter::Sharpness},

		{"DLSSOptimalSettingsCallback", NvParameter::DLSSOptimalSettingsCallback},
		{"DLSSGetStatsCallback", NvParameter::DLSSGetStatsCallback},

		{"CreationNodeMask", NvParameter::CreationNodeMask},
		{"VisibilityNodeMask", NvParameter::VisibilityNodeMask},
		{"DLSS.Feature.Create.Flags", NvParameter::DLSS_Feature_Create_Flags},
		{"DLSS.Enable.Output.Subrects", NvParameter::DLSS_Enable_Output_Subrects},

		{"Color", NvParameter::Color},
		{"MotionVectors", NvParameter::MotionVectors},
		{"Depth", NvParameter::Depth},
		{"Output", NvParameter::Output},
		{"TransparencyMask", NvParameter::TransparencyMask},
		{"ExposureTexture", NvParameter::ExposureTexture},
		{"DLSS.Input.Bias.Current.Color.Mask", NvParameter::DLSS_Input_Bias_Current_Color_Mask},

		{"DLSS.Pre.Exposure", NvParameter::Pre_Exposure},
		{"DLSS.Exposure.Scale", NvParameter::Exposure_Scale},

		{"Reset", NvParameter::Reset},
		{"MV.Scale.X", NvParameter::MV_Scale_X},
		{"MV.Scale.Y", NvParameter::MV_Scale_Y},
		{"Jitter.Offset.X", NvParameter::Jitter_Offset_X},
		{"Jitter.Offset.Y", NvParameter::Jitter_Offset_Y},

		{"SizeInBytes", NvParameter::SizeInBytes},
		{"Snippet.OptLevel", NvParameter::OptLevel},
		{"#\x44", NvParameter::OptLevel},
		{"Snippet.IsDevBranch", NvParameter::IsDevSnippetBranch},
		{"#\x45", NvParameter::IsDevSnippetBranch},
		{"DLSS.Hint.Render.Preset.DLAA", NvParameter::DLSS_Hint_Render_Preset_DLAA},
		{"DLSS.Hint.Render.Preset.UltraQuality", NvParameter::DLSS_Hint_Render_Preset_UltraQuality},
		{"DLSS.Hint.Render.Preset.Quality", NvParameter::DLSS_Hint_Render_Preset_Quality},
		{"DLSS.Hint.Render.Preset.Balanced", NvParameter::DLSS_Hint_Render_Preset_Balanced},
		{"DLSS.Hint.Render.Preset.Performance", NvParameter::DLSS_Hint_Render_Preset_Performance},
		{"DLSS.Hint.Render.Preset.UltraPerformance", NvParameter::DLSS_Hint_Render_Preset_UltraPerformance},
	};

	return NvParamTranslation[std::string(name)];
}
