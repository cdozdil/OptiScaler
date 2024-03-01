#pragma once
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

inline static unsigned int handleCounter = 1000000;

class IFeature
{
private:
	NVSDK_NGX_Handle* _handle;

	bool _initParameters = false;

	unsigned int _displayWidth = 0;
	unsigned int _displayHeight = 0;
	unsigned int _renderWidth = 0;
	unsigned int _renderHeight = 0;
	NVSDK_NGX_PerfQuality_Value _perfQualityValue;

protected:
	void SetHandle(unsigned int handleId);

	bool SetInitParameters(const NVSDK_NGX_Parameter* InParameters);
	
	virtual void SetInit(bool value) = 0;

public:
	NVSDK_NGX_Handle* Handle() const { return _handle; };
	unsigned int DisplayWidth() const { return _displayWidth; };
	unsigned int DisplayHeight() const { return _displayHeight; };
	unsigned int RenderWidth() const { return _renderWidth; };
	unsigned int RenderHeight() const { return _renderHeight; };
	NVSDK_NGX_PerfQuality_Value PerfQualityValue() const { return _perfQualityValue; }
	bool IsInitParameters() const { return _initParameters; };
	static unsigned int GetNextHandleId() { return handleCounter++; }

	virtual void ReInit(const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool IsInited() = 0;

	IFeature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters)
	{
		SetHandle(handleId);
		_initParameters = SetInitParameters(InParameters);
	}

	virtual ~IFeature() {}
};