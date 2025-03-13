#pragma once
#include <pch.h>
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

#define DLSS_MOD_ID_OFFSET 1000000

inline static unsigned int handleCounter = DLSS_MOD_ID_OFFSET;

class IFeature
{
private:
	bool _initParameters = false;
	bool _isInited = false;
	int _featureFlags = 0;

	NVSDK_NGX_PerfQuality_Value _perfQualityValue;

protected:
	NVSDK_NGX_Handle* _handle = nullptr;

	float _sharpness = 0;
	bool _hasColor = false;
	bool _hasDepth = false;
	bool _hasMV = false;
	bool _hasTM = false;
	bool _accessToReactiveMask = false;
	bool _hasExposure = false;
	bool _hasOutput = false;

	unsigned int _renderWidth = 0;
	unsigned int _renderHeight = 0;
	unsigned int _targetWidth = 0;
	unsigned int _targetHeight = 0;
	unsigned int _displayWidth = 0;
	unsigned int _displayHeight = 0;

	long _frameCount = 0;
	bool _moduleLoaded = false;

	void SetHandle(unsigned int InHandleId);
	bool SetInitParameters(NVSDK_NGX_Parameter* InParameters);
	void GetRenderResolution(NVSDK_NGX_Parameter* InParameters, unsigned int* OutWidth, unsigned int* OutHeight);
	void GetDynamicOutputResolution(NVSDK_NGX_Parameter* InParameters, unsigned int* width, unsigned int* height);
	float GetSharpness(const NVSDK_NGX_Parameter* InParameters);

	virtual void SetInit(bool InValue) { _isInited = InValue; }

public:
	NVSDK_NGX_Handle* Handle() const { return _handle; };
	static unsigned int GetNextHandleId() { return handleCounter++; }

	int GetFeatureFlags() const { return _featureFlags; }

	bool UpdateOutputResolution(const NVSDK_NGX_Parameter* InParameters);
	unsigned int DisplayWidth() const { return _displayWidth; };
	unsigned int DisplayHeight() const { return _displayHeight; };
	unsigned int TargetWidth() const { return _targetWidth; };
	unsigned int TargetHeight() const { return _targetHeight; };
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
	bool AccessToReactiveMask() const { return _accessToReactiveMask; }
	bool HasExposure() const { return _hasExposure; }
	bool HasOutput() const { return _hasOutput; }
	virtual feature_version Version() = 0;
	virtual const char* Name() = 0;
	bool ModuleLoaded() const { return _moduleLoaded; }
	long FrameCount() { return _frameCount; }

	IFeature(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters)
	{
		SetHandle(InHandleId);
		_initParameters = SetInitParameters(InParameters);
	}

	virtual void Shutdown() = 0;
	virtual ~IFeature() {}
};