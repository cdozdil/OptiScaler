#pragma once

#include "IFeatureEvaluateParams.h"

class IFeatureEvaluateParams_DLSS : public IFeatureEvaluateParams
{
private:
	double _lastFrameTime = 0.0;

	const NVSDK_NGX_Parameter* _nvParameter = nullptr;

	void GetRenderResolution(const NVSDK_NGX_Parameter* InParameters)
	{
		uint32_t OutWidth;
		uint32_t OutHeight;

		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, &OutWidth) != NVSDK_NGX_Result_Success ||
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, &OutHeight) != NVSDK_NGX_Result_Success)
		{
			unsigned int width;
			unsigned int height;
			unsigned int outWidth;
			unsigned int outHeight;

			do
			{
				if (InParameters->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
					InParameters->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success)
				{
					if (InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
						InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success)
					{
						if (width < outWidth)
						{
							OutWidth = width;
							OutHeight = height;
							break;
						}

						OutWidth = outWidth;
						OutHeight = outHeight;
					}
					else
					{
						if (width < RenderWidth())
						{
							OutWidth = width;
							OutHeight = height;
							break;
						}

						OutWidth = -1;
						OutHeight = -1;
						return;
					}
				}

				OutWidth = -1;
				OutHeight = -1;

			} while (false);
		}

		_renderWidth = OutWidth;
		_renderHeight = OutHeight;
	}

public:
	IFeatureEvaluateParams_DLSS(const NVSDK_NGX_Parameter* InParameters) : IFeatureEvaluateParams()
	{
		_nvParameter = InParameters;

		InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &_jitterOffsetX);
		InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &_jitterOffsetY);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &_exposureScale);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &_preExposure);
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &_mvScaleX);
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &_mvScaleY);
		InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &_sharpness);

		int reset = 0;
		InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
		_reset = reset != 0;

		InParameters->Get(NVSDK_NGX_Parameter_Color, &_inputColor);
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &_inputMotion);
		InParameters->Get(NVSDK_NGX_Parameter_Depth, &_inputDepth);
		InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &_inputExposure);
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &_inputMask);
		InParameters->Get(NVSDK_NGX_Parameter_Output, &_outputColor);
		

		float ftDelta = 0.0f;
		if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &ftDelta) == NVSDK_NGX_Result_Success || ftDelta > 1.0f)
			_frameTimeDelta = ftDelta;

		GetRenderResolution(InParameters);
	}

	ParamterSource Source() final { return DLSS; }
};