#pragma once

#include "IFeatureCreateParams.h"

#include "../Config.h"
#include "../NVNGX_Parameter.h"


class IFeatureCreateParams_DLSS : public IFeatureCreateParams
{
private:
	const NVNGX_Init_Info _initInfo;

	inline static CommonQualityPreset GetQualityPreset(int pqValue)
	{
		CommonQualityPreset result = Undefined;

		switch (pqValue)
		{
		case 0: // NVSDK_NGX_PerfQuality_Value_MaxPerf,
			result = Performance;
			break;

		case 1: // NVSDK_NGX_PerfQuality_Value_Balanced,
			result = Balanced;
			break;

		case 2: // NVSDK_NGX_PerfQuality_Value_MaxQuality,
			result = Quality;
			break;

		case 3: // NVSDK_NGX_PerfQuality_Value_UltraPerformance,
			result = UltraPerformance;
			break;

		case 4: // NVSDK_NGX_PerfQuality_Value_UltraQuality,
			result = UltraQuality;
			break;

		case 5: // NVSDK_NGX_PerfQuality_Value_DLAA,
			result = NativeAA;
			break;

		}

		return result;
	}

public:
	IFeatureCreateParams_DLSS(const NVSDK_NGX_Parameter* InParameters, const NVNGX_Init_Info InInitInfo) : _initInfo(InInitInfo)
	{
		if (InParameters == nullptr)
			return;

		NVSDK_NGX_Result result;

		unsigned int width = 0;
		unsigned int outWidth = 0;
		unsigned int height = 0;
		unsigned int outHeight = 0;
		int pqValue = 1;

		// Resolutions
		if (InParameters->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
			InParameters->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success &&
			InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
			InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success)
		{
			// Thanks to Crytek added these checks
			if (width > 16384 || width < 0)
				width = 0;

			if (height > 16384 || height < 0)
				height = 0;

			if (outWidth > 16384 || outWidth < 0)
				outWidth = 0;

			if (outHeight > 16384 || outHeight < 0)
				outHeight = 0;

			if (pqValue > 5 || pqValue < 0)
				pqValue = 1;

			_displayWidth = width > outWidth ? width : outWidth;
			_displayHeight = height > outHeight ? height : outHeight;
			_targetWidth = _displayWidth;
			_targetHeight = _displayHeight;

			if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, &_renderWidth) != NVSDK_NGX_Result_Success ||
				InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, &_renderHeight) != NVSDK_NGX_Result_Success)
			{
				_renderWidth = width < outWidth ? width : outWidth;
				_renderHeight = height < outHeight ? height : outHeight;
			}

			_pqValue = GetQualityPreset(pqValue);

			spdlog::info("IFeatureContext::SetInitParameters Render Resolution: {0}x{1}, Display Resolution {2}x{3}, Quality: {4}",
				_renderWidth, _renderHeight, _displayWidth, _displayHeight, pqValue);
		}
		else
		{
			spdlog::error("IFeatureCreateParams_DLSS::IFeatureCreateParams_DLSS Can't get resolution info from parameters!");
			return;
		}

		// Quality
		result = InParameters->Get(NVSDK_NGX_Parameter_PerfQualityValue, &pqValue);

		if(result != NVSDK_NGX_Result_Success)
			spdlog::warn("IFeatureCreateParams_DLSS::IFeatureCreateParams_DLSS Can't get quality profile from parameters!");

		_pqValue = GetQualityPreset(pqValue);

		// Init flags
		unsigned int createFlags = 0;
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &createFlags) != NVSDK_NGX_Result_Success)
		{
			spdlog::error("IFeatureCreateParams_DLSS::IFeatureCreateParams_DLSS Can't get create flags from parameters!");
			return;
		}

		_hdr = (createFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR);
		_displayResMV = !(createFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
		_jitterCancellation = (createFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered);
		_depthInverted = (createFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted);
		_sharpening = (createFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening);
		_autoExposure = (createFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure);

		// Output scaling
		if (Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(_displayResMV))
		{
			float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or(1.5f);

			if (ssMulti < 0.5f)
			{
				ssMulti = 0.5f;
				Config::Instance()->OutputScalingMultiplier = ssMulti;
			}
			else if (ssMulti > 3.0f)
			{
				ssMulti = 3.0f;
				Config::Instance()->OutputScalingMultiplier = ssMulti;
			}

			_targetWidth = _displayWidth * ssMulti;
			_targetHeight = _displayHeight * ssMulti;
		}
		else
		{
			_targetWidth = _displayWidth;
			_targetHeight = _displayHeight;
		}

		// dlss presets
		unsigned int RenderPresetDLAA = 0;
		unsigned int RenderPresetUltraQuality = 0;
		unsigned int RenderPresetQuality = 0;
		unsigned int RenderPresetBalanced = 0;
		unsigned int RenderPresetPerformance = 0;
		unsigned int RenderPresetUltraPerformance = 0;

		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, &RenderPresetDLAA);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, &RenderPresetUltraQuality);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, &RenderPresetQuality);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, &RenderPresetBalanced);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, &RenderPresetPerformance);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, &RenderPresetUltraPerformance);

		if (Config::Instance()->RenderPresetOverride.value_or(false))
		{
			Config::Instance()->RenderPresetDLAA = Config::Instance()->RenderPresetDLAA.value_or(RenderPresetDLAA);
			Config::Instance()->RenderPresetUltraQuality = Config::Instance()->RenderPresetUltraQuality.value_or(RenderPresetUltraQuality);
			Config::Instance()->RenderPresetQuality = Config::Instance()->RenderPresetQuality.value_or(RenderPresetQuality);
			Config::Instance()->RenderPresetBalanced = Config::Instance()->RenderPresetBalanced.value_or(RenderPresetBalanced);
			Config::Instance()->RenderPresetPerformance = Config::Instance()->RenderPresetPerformance.value_or(RenderPresetPerformance);
			Config::Instance()->RenderPresetUltraPerformance = Config::Instance()->RenderPresetUltraPerformance.value_or(RenderPresetUltraPerformance);
		}
		else
		{
			Config::Instance()->RenderPresetDLAA = RenderPresetDLAA;
			Config::Instance()->RenderPresetUltraQuality = RenderPresetUltraQuality;
			Config::Instance()->RenderPresetQuality = RenderPresetQuality;
			Config::Instance()->RenderPresetBalanced = RenderPresetBalanced;
			Config::Instance()->RenderPresetPerformance = RenderPresetPerformance;
			Config::Instance()->RenderPresetUltraPerformance = RenderPresetUltraPerformance;
		}

		_isInited = true;
	}

	NVNGX_Init_Info InitInfo() { return _initInfo; }
};