#pragma once
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

inline static unsigned int handleCounter = 1000000;

class IFeature
{
private:
	NVSDK_NGX_Handle* _handle;

	bool _initParameters = false;
	bool _isInited = false;

	unsigned int _displayWidth = 0;
	unsigned int _displayHeight = 0;
	unsigned int _renderWidth = 0;
	unsigned int _renderHeight = 0;
	NVSDK_NGX_PerfQuality_Value _perfQualityValue;
	
	void SetHandle(unsigned int InHandleId);

protected:
	float _sharpness = 0;
	bool _hasColor = false;
	bool _hasDepth = false;
	bool _hasMV = false;
	bool _hasTM = false;
	bool _hasExposure = false;
	bool _hasOutput = false;
	int _initFlags = 0;

	bool SetInitParameters(const NVSDK_NGX_Parameter* InParameters);
	void GetRenderResolution(const NVSDK_NGX_Parameter* InParameters, unsigned int* OutWidth, unsigned int* OutHeight) const;
	virtual void SetInit(bool InValue) { _isInited = InValue; }

public:
	NVSDK_NGX_Handle* Handle() const { return _handle; };
	static unsigned int GetNextHandleId() { return handleCounter++; }

	unsigned int DisplayWidth() const { return _displayWidth; };
	unsigned int DisplayHeight() const { return _displayHeight; };
	unsigned int RenderWidth() const { return _renderWidth; };
	unsigned int RenderHeight() const { return _renderHeight; };
	NVSDK_NGX_PerfQuality_Value PerfQualityValue() const { return _perfQualityValue; }
	bool IsInitParameters() const { return _initParameters; };
	bool IsInited() const { return _isInited; }
	float Sharpness() const { return _sharpness; }
	bool HasColor() const { return _hasColor; }
	bool HasDepth() const { return _hasDepth; }
	bool HasMV() const { return _hasMV; }
	bool HasTM() const { return _hasTM; }
	bool HasExposure() const { return _hasExposure; }
	bool HasOutput() const { return _hasOutput; }
	int InitFlags() const { return _initFlags; }

	IFeature(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters)
	{
		SetHandle(InHandleId);
		_initParameters = SetInitParameters(InParameters);
	}

	virtual ~IFeature() {}
};