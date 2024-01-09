#include "pch.h"
#include "Config.h"
#include "Util.h"
#include "NvParameter.h"
#include "CyberXess.h"

void NvParameter::Set(const char* InName, unsigned long long InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvULL);
}

void NvParameter::Set(const char* InName, float InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvFloat);
}

void NvParameter::Set(const char* InName, double InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvDouble);
}

void NvParameter::Set(const char* InName, unsigned int InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvUInt);
}

void NvParameter::Set(const char* InName, int InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvInt);
}

void NvParameter::Set(const char* InName, ID3D11Resource* InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvD3D11Resource);
}

void NvParameter::Set(const char* InName, ID3D12Resource* InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvD3D12Resource);
}

void NvParameter::Set(const char* InName, void* InValue)
{
	auto value = (unsigned long long*) & InValue;
	Set_Internal(InName, *value, NvVoidPtr);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, unsigned long long* OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvULL);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, float* OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvFloat);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, double* OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvDouble);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, unsigned int* OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvUInt);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, int* OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvInt);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, ID3D11Resource** OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvD3D11Resource);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, ID3D12Resource** OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvD3D12Resource);
}

NVSDK_NGX_Result NvParameter::Get(const char* InName, void** OutValue) const
{
	return Get_Internal(InName, (unsigned long long*)OutValue, NvVoidPtr);
}

void NvParameter::Reset()
{
}

void NvParameter::Set_Internal(const char* InName, unsigned long long InValue, NvParameterType ParameterType)
{
	auto inValueFloat = (float*)&InValue;
	auto inValueInt = (int*)&InValue;
	auto inValueDouble = (double*)&InValue;
	auto inValueUInt = (unsigned int*)&InValue;
	//Includes DirectX Resources
	auto inValuePtr = (void*)InValue;

	std::string s;
	s = InName;

	LOG("Set_Internal : " + s + " - f:" + std::to_string(*inValueFloat) + " - d:" + std::to_string(*inValueDouble) + " - i:" + std::to_string(*inValueInt) + " - u:" + std::to_string(*inValueUInt), spdlog::level::debug);

	switch (Util::NvParameterToEnum(InName))
	{
	case Util::NvParameter::MV_Scale_X:
		MVScaleX = *inValueFloat;
		break;
	case Util::NvParameter::MV_Scale_Y:
		MVScaleY = *inValueFloat;
		break;
	case Util::NvParameter::Jitter_Offset_X:
		JitterOffsetX = *inValueFloat;
		break;
	case Util::NvParameter::Jitter_Offset_Y:
		JitterOffsetY = *inValueFloat;
		break;
	case Util::NvParameter::Sharpness:
		Sharpness = *inValueFloat;
		break;
	case Util::NvParameter::Width:
		Width = *inValueInt;
		break;
	case Util::NvParameter::Height:
		Height = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Render_Subrect_Dimensions_Width:
		Width = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Render_Subrect_Dimensions_Height:
		Height = *inValueInt;
		break;
	case Util::NvParameter::PerfQualityValue:
		PerfQualityValue = static_cast<NVSDK_NGX_PerfQuality_Value>(*inValueInt);
		break;
	case Util::NvParameter::RTXValue:
		RTXValue = *inValueInt;
		break;
	case Util::NvParameter::FreeMemOnReleaseFeature:
		FreeMemOnReleaseFeature = *inValueInt;
		break;
	case Util::NvParameter::CreationNodeMask:
		CreationNodeMask = *inValueInt;
		break;
	case Util::NvParameter::VisibilityNodeMask:
		VisibilityNodeMask = *inValueInt;
		break;
	case Util::NvParameter::Reset:
		ResetRender = *inValueInt;
		break;
	case Util::NvParameter::OutWidth:
		OutWidth = *inValueInt;
		break;
	case Util::NvParameter::OutHeight:
		OutHeight = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Balanced:
		BalancedPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_DLAA:
		DLAAPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Performance:
		PerfPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Quality:
		QualityPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_UltraQuality:
		UltraQualityPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_UltraPerformance:
		UltraPerfPreset = *inValueInt;
		break;
	case Util::NvParameter::DLSS_Feature_Create_Flags:
		Hdr = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
		EnableSharpening = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
		DepthInverted = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
		JitterMotion = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
		LowRes = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
		AutoExposure = *inValueInt & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
		break;
	case Util::NvParameter::DLSS_Input_Bias_Current_Color_Mask:
		InputBiasCurrentColorMask = inValuePtr;
		break;
	case Util::NvParameter::Color:
		Color = inValuePtr;
		break;
	case Util::NvParameter::Depth:
		Depth = inValuePtr;
		break;
	case Util::NvParameter::MotionVectors:
		MotionVectors = inValuePtr;
		break;
	case Util::NvParameter::Output:
		Output = inValuePtr;
		break;
	case Util::NvParameter::TransparencyMask:
		TransparencyMask = inValuePtr;
		break;
	case Util::NvParameter::ExposureTexture:
		ExposureTexture = inValuePtr;
		break;
	case Util::NvParameter::Exposure_Scale:
		ExposureScale = *inValueFloat;
		break;
	default:
		LOG("Set_Internal Not Implemented : " + s, spdlog::level::debug);

	}
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetOptimalSettingsCallback(NVSDK_NGX_Parameter* InParams);
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetStatsCallback(NVSDK_NGX_Parameter* InParams);

NVSDK_NGX_Result NvParameter::Get_Internal(const char* InName, unsigned long long* OutValue, NvParameterType ParameterType) const
{
	auto outValueFloat = (float*)OutValue;
	auto outValueInt = (int*)OutValue;
	auto outValueDouble = (double*)OutValue;
	auto outValueUInt = (unsigned int*)OutValue;
	auto outValueULL = (unsigned long long*)OutValue;
	//Includes DirectX Resources
	auto outValuePtr = (void**)OutValue;
	const auto params = CyberXessContext::instance()->CreateFeatureParams;

	std::string s;
	s = InName;

	switch (Util::NvParameterToEnum(InName))
	{
	case Util::NvParameter::Sharpness:
		*outValueFloat = Sharpness;
		break;
	case Util::NvParameter::SuperSampling_Available:
		*outValueInt = true;
		break;
	case Util::NvParameter::SuperSampling_FeatureInitResult:
		*outValueInt = NVSDK_NGX_Result_Success;
		break;
	case Util::NvParameter::SuperSampling_NeedsUpdatedDriver:
		*outValueInt = 0;
		break;
	case Util::NvParameter::SuperSampling_MinDriverVersionMinor:
	case Util::NvParameter::SuperSampling_MinDriverVersionMajor:
		*outValueInt = 0;
		break;
	case Util::NvParameter::DLSS_Render_Subrect_Dimensions_Width:
		*outValueInt = Width;
		break;
	case Util::NvParameter::DLSS_Render_Subrect_Dimensions_Height:
		*outValueInt = Height;
		break;
	case Util::NvParameter::OutWidth:
		*outValueInt = OutWidth;
		break;
	case Util::NvParameter::OutHeight:
		*outValueInt = OutHeight;
		break;
	case Util::NvParameter::DLSS_Get_Dynamic_Max_Render_Width:
		*outValueInt = Width;
		break;
	case Util::NvParameter::DLSS_Get_Dynamic_Max_Render_Height:
		*outValueInt = Height;
		break;
	case Util::NvParameter::DLSS_Get_Dynamic_Min_Render_Width:
		*outValueInt = OutWidth;
		break;
	case Util::NvParameter::DLSS_Get_Dynamic_Min_Render_Height:
		*outValueInt = OutHeight;
		break;
	case Util::NvParameter::DLSSOptimalSettingsCallback:
		*outValuePtr = NVSDK_NGX_DLSS_GetOptimalSettingsCallback;
		break;
	case Util::NvParameter::DLSSGetStatsCallback:
		*outValuePtr = NVSDK_NGX_DLSS_GetStatsCallback;
		break;
	case Util::NvParameter::SizeInBytes:
		*outValueULL = params->OutHeight * params->OutWidth * 31; //Dummy value
		break;
	case Util::NvParameter::OptLevel:
		*outValueInt = 0; //Dummy value
		break;
	case Util::NvParameter::IsDevSnippetBranch:
		*outValueInt = 0; //Dummy value
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Balanced:
		*outValueInt = BalancedPreset;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_DLAA:
		*outValueInt = DLAAPreset;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Performance:
		*outValueInt = PerfPreset;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_Quality:
		*outValueInt = QualityPreset;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_UltraQuality:
		*outValueInt = UltraQualityPreset;
		break;
	case Util::NvParameter::DLSS_Hint_Render_Preset_UltraPerformance:
		*outValueInt = UltraPerfPreset;
		break;
	default:
		LOG("Get_Internal Not Implemented : " + s, spdlog::level::debug);
		return NVSDK_NGX_Result_Fail;
	}

	LOG("Get_Internal : " + s + " - f:" + std::to_string(*outValueFloat) + " - d:" + std::to_string(*outValueDouble) + " - i:" + std::to_string(*outValueInt) + " - u:" + std::to_string(*outValueUInt) + " - ul:" + std::to_string(*outValueULL), spdlog::level::debug);
	return NVSDK_NGX_Result_Success;
}

// EvaluateRenderScale helper
inline xess_quality_settings_t DLSS2XeSSQualityTable(const NVSDK_NGX_PerfQuality_Value input)
{
	xess_quality_settings_t output;

	switch (input)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
		output = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		output = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		output = XESS_QUALITY_SETTING_BALANCED;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		output = XESS_QUALITY_SETTING_QUALITY;
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		output = XESS_QUALITY_SETTING_ULTRA_QUALITY;
		break;
	default:
		output = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing fsr ultra quality mode
		break;
	}

	return output;
}

// EvaluateRenderScale helper
inline std::optional<float> GetQualityOverrideRatio(const NVSDK_NGX_PerfQuality_Value input)
{
	std::optional<float> output;

	if (!(CyberXessContext::instance()->MyConfig->QualityRatioOverrideEnabled.has_value() && CyberXessContext::instance()->MyConfig->QualityRatioOverrideEnabled))
		return output; // override not enabled

	switch (input)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
		output = CyberXessContext::instance()->MyConfig->QualityRatio_UltraPerformance;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		output = CyberXessContext::instance()->MyConfig->QualityRatio_Performance;
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		output = CyberXessContext::instance()->MyConfig->QualityRatio_Balanced;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		output = CyberXessContext::instance()->MyConfig->QualityRatio_Quality;
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		output = CyberXessContext::instance()->MyConfig->QualityRatio_UltraQuality;
		break;
	default:
		LOG("GetQualityOverrideRatio: Unknown quality : " + std::to_string(input), spdlog::level::warn);
		output = CyberXessContext::instance()->MyConfig->QualityRatio_Balanced;
		break;
	}
	return output;
}

void NvParameter::EvaluateRenderScale()
{
	LOG("EvaluateRenderScale start :" + std::to_string(Width) + "x" + std::to_string(Height) + " o:" + std::to_string(OutWidth) + "x" + std::to_string(OutHeight), spdlog::level::debug);

	const std::optional<float> QualityRatio = GetQualityOverrideRatio(PerfQualityValue);

	if (QualityRatio.has_value()) {
		OutHeight = (unsigned int)((float)Height / QualityRatio.value());
		OutWidth = (unsigned int)((float)Width / QualityRatio.value());
	}
	else {
		const xess_quality_settings_t xessQualityMode = DLSS2XeSSQualityTable(PerfQualityValue);

		LOG("EvaluateRenderScale Quality : " + std::to_string(PerfQualityValue), spdlog::level::debug);

		switch (PerfQualityValue)
		{
		case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
			OutHeight = (unsigned int)((float)Height / 3.0);
			OutWidth = (unsigned int)((float)Width / 3.0);
			break;
		case NVSDK_NGX_PerfQuality_Value_MaxPerf:
			OutHeight = (unsigned int)((float)Height / 2.0);
			OutWidth = (unsigned int)((float)Width / 2.0);
			break;
		case NVSDK_NGX_PerfQuality_Value_Balanced:
			OutHeight = (unsigned int)((float)Height / 1.699115044247788);
			OutWidth = (unsigned int)((float)Width / 1.699115044247788);
			break;
		case NVSDK_NGX_PerfQuality_Value_MaxQuality:
			OutHeight = (unsigned int)((float)Height / 1.5);
			OutWidth = (unsigned int)((float)Width / 1.5);
			break;
		case NVSDK_NGX_PerfQuality_Value_UltraQuality:
			OutHeight = (unsigned int)((float)Height / 1.299932295192959);
			OutWidth = (unsigned int)((float)Width / 1.299932295192959);
			break;
		default:
			OutHeight = (unsigned int)((float)Height / 1.699115044247788);
			OutWidth = (unsigned int)((float)Width / 1.699115044247788);
			break;
		}
	}

	LOG("EvaluateRenderScale end :" + std::to_string(Width) + "x" + std::to_string(Height) + " o:" + std::to_string(OutWidth) + "x" + std::to_string(OutHeight), spdlog::level::debug);
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetOptimalSettingsCallback(NVSDK_NGX_Parameter* InParams)
{
	auto params = static_cast<NvParameter*>(InParams);
	params->EvaluateRenderScale();
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetStatsCallback(NVSDK_NGX_Parameter* InParams)
{
	//TODO: Somehow check for allocated memory
	//Then set values: SizeInBytes, OptLevel, IsDevSnippetBranch
	return NVSDK_NGX_Result_Success;
}