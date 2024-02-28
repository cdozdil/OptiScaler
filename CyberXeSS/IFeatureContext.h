#pragma once

#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

#define SPDLOG_USE_STD_FORMAT
#include "spdlog/spdlog.h"

inline static unsigned int handleCounter = 1000000;

class IFeatureContext
{
private:
	unsigned int _displayWidth = 0;
	unsigned int _displayHeight = 0;
	unsigned int _renderWidth = 0;
	unsigned int _renderHeight = 0;
	NVSDK_NGX_PerfQuality_Value _perfQualityValue;
	unsigned int _handle;

protected:
	bool _isInited = false;

	void SetHandle()
	{
		_handle = handleCounter++;
		spdlog::info("IFeatureContext::SetHandle Handle: {0}", _handle);
	}

	bool SetInitParameters(const NVSDK_NGX_Parameter* InParameters)
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

public:
	unsigned int Handle() const { return _handle; };
	unsigned int DisplayWidth() const { return _displayWidth; };
	unsigned int DisplayHeight() const { return _displayHeight; };
	unsigned int RenderWidth() const { return _renderWidth; };
	unsigned int RenderHeight() const { return _renderHeight; };
	NVSDK_NGX_PerfQuality_Value PerfQualityValue() const { return _perfQualityValue; }
	bool IsInited() const { return _isInited; };

	explicit IFeatureContext(const NVSDK_NGX_Parameter* InParameters)
	{
		SetHandle();
		SetInitParameters(InParameters);
	}

	virtual ~IFeatureContext() {}
};