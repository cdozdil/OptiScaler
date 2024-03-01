#include "pch.h"
#include "IFeature.h"

void IFeature::SetHandle(unsigned int handleId)
{
	_handle = new NVSDK_NGX_Handle{ handleId };
	spdlog::info("IFeatureContext::SetHandle Handle: {0}", _handle->Id);
}

bool IFeature::SetInitParameters(const NVSDK_NGX_Parameter* InParameters)
{
	unsigned int width, outWidth, height, outHeight;
	int pqValue;

	if (InParameters->Get(NVSDK_NGX_Parameter_Width, &width) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_Height, &height) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_OutWidth, &outWidth) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_OutHeight, &outHeight) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_PerfQualityValue, &pqValue) == NVSDK_NGX_Result_Success)
	{
		_displayWidth = width > outWidth ? width : outWidth;
		_displayHeight = height > outHeight ? height : outHeight;
		_renderWidth = width < outWidth ? width : outWidth;
		_renderHeight = height < outHeight ? height : outHeight;
		_perfQualityValue = (NVSDK_NGX_PerfQuality_Value)pqValue;

		spdlog::info("IFeatureContext::SetInitParameters Render Resolution: {0}x{1}, Display Resolution {2}x{3}, Quality: {4}",
			_renderWidth, _renderHeight, _displayWidth, _displayHeight, pqValue);

		return true;
	}

	spdlog::error("IFeatureContext::SetInitParameters Can't set parameters!");
	return false;
}
