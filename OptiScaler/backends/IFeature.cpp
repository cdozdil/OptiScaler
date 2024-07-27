#pragma once
#include "../pch.h"
#include "IFeature.h"
#include "../Config.h"

void IFeature::SetHandle(unsigned int InHandleId)
{
	_handle = new NVSDK_NGX_Handle{ InHandleId };
	LOG_INFO("Handle: {0}", _handle->Id);
}

bool IFeature::SetInitParameters(NVSDK_NGX_Parameter* InParameters)
{
	unsigned int width = 0;
	unsigned int outWidth = 0;
	unsigned int height = 0;
	unsigned int outHeight = 0;
	int pqValue = 0;

	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &_featureFlags);

	if (InParameters->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_PerfQualityValue, &pqValue) == NVSDK_NGX_Result_Success)
	{
		// Thanks to Crytek added these checks
		if (width > 16384 || width < 20)
			width = 0;

		if (height > 16384 || height < 20)
			height = 0;

		if (outWidth > 16384 || outWidth < 20)
			outWidth = 0;

		if (outHeight > 16384 || outHeight < 20)
			outHeight = 0;

		if (pqValue > 5 || pqValue < 0)
			pqValue = 1;

		// When using extended limits render res might be bigger than display res
		// it might create rendering issues but extending limits is an advanced option after all
		if (!Config::Instance()->ExtendedLimits.value_or(false))
		{
			_displayWidth = width > outWidth ? width : outWidth;
			_displayHeight = height > outHeight ? height : outHeight;
			_targetWidth = _displayWidth;
			_targetHeight = _displayHeight;
			_renderWidth = width < outWidth ? width : outWidth;
			_renderHeight = height < outHeight ? height : outHeight;
		}
		else
		{
			_displayWidth = outWidth;
			_displayHeight = outHeight;
			_targetWidth = _displayWidth;
			_targetHeight = _displayHeight;
			_renderWidth = width;
			_renderHeight = height;
		}


		_perfQualityValue = (NVSDK_NGX_PerfQuality_Value)pqValue;

		LOG_INFO("Render Resolution: {0}x{1}, Display Resolution {2}x{3}, Quality: {4}",
			_renderWidth, _renderHeight, _displayWidth, _displayHeight, pqValue);

		return true;
	}

	LOG_ERROR("Can't set parameters!");
	return false;
}

void IFeature::GetRenderResolution(NVSDK_NGX_Parameter* InParameters, unsigned int* OutWidth, unsigned int* OutHeight)
{
	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width, OutWidth) != NVSDK_NGX_Result_Success ||
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height, OutHeight) != NVSDK_NGX_Result_Success)
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
						*OutWidth = width;
						*OutHeight = height;
						break;
					}

					*OutWidth = outWidth;
					*OutHeight = outHeight;
				}
				else
				{
					if (width < RenderWidth())
					{
						*OutWidth = width;
						*OutHeight = height;
						break;
					}

					*OutWidth = RenderWidth();
					*OutHeight = RenderHeight();
					return;
				}
			}

			*OutWidth = RenderWidth();
			*OutHeight = RenderHeight();

		} while (false);
	}

	_renderWidth = *OutWidth;
	_renderHeight = *OutHeight;

	//Should not be needed but who knows
	//if (_renderHeight == _displayHeight && _renderWidth == _displayWidth && _perfQualityValue != NVSDK_NGX_PerfQuality_Value_DLAA)
	//{
	//	InParameters->Set(NVSDK_NGX_Parameter_PerfQualityValue, 5);
	//	InParameters->Set(NVSDK_NGX_Parameter_Scale, 1.0f);
	//	InParameters->Set(NVSDK_NGX_Parameter_SuperSampling_ScaleFactor, 1.0f);
	//}
}

