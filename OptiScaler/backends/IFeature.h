#pragma once
#include "../pch.h"

#include "IFeatureCreateParams.h"
#include "IFeatureEvaluateParams.h"

#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

typedef struct _feature_version
{
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
} FeatureVersion;


class IFeature
{
private:
	bool _isInited = false;
	inline static unsigned int handleCounter = 1000000;

protected:
	unsigned int _handleId = 0;
	IFeatureCreateParams _createParams;

	bool _hasColor = false;
	bool _hasDepth = false;
	bool _hasMV = false;
	bool _hasTM = false;
	bool _hasExposure = false;
	bool _hasOutput = false;

	unsigned int _renderWidth = 0;
	unsigned int _renderHeight = 0;
	unsigned int _targetWidth = 0;
	unsigned int _targetHeight = 0;
	unsigned int _displayWidth = 0;
	unsigned int _displayHeight = 0;

	float _sharpness = 0;

	long _frameCount = 0;
	bool _moduleLoaded = false;

	void SetHandle(unsigned int InHandleId)
	{
		_handleId = InHandleId;
		spdlog::info("IFeatureContext::SetHandle Handle: {0}", _handleId);
	}

	void SetInit(bool InValue)
	{
		_isInited = InValue;
	}

public:
	static unsigned int GetNextHandleId() { return handleCounter++; }

	unsigned int DisplayWidth() const { return _displayWidth; };
	unsigned int DisplayHeight() const { return _displayHeight; };
	unsigned int TargetWidth() const { return _targetWidth; };
	unsigned int TargetHeight() const { return _targetHeight; };
	unsigned int RenderWidth() const { return _renderWidth; };
	unsigned int RenderHeight() const { return _renderHeight; };
	CommonQualityPreset PerfQualityValue() const { return _createParams.QualityPreset(); }

	const IFeatureCreateParams CreateParams() { return _createParams; }

	bool IsInited() const { return _isInited; }

	float Sharpness() const { return _sharpness; }
	bool HasColor() const { return _hasColor; }
	bool HasDepth() const { return _hasDepth; }
	bool HasMV() const { return _hasMV; }
	bool HasTM() const { return _hasTM; }
	bool HasExposure() const { return _hasExposure; }
	bool HasOutput() const { return _hasOutput; }

	virtual FeatureVersion Version() { return {}; }
	virtual const char* Name() { return "Base"; }

	bool ModuleLoaded() const { return _moduleLoaded; }
	long FrameCount() const { return _frameCount; }

	virtual bool UseRcas() const { return true; }

	IFeature(unsigned int InHandleId, const IFeatureCreateParams InParams) : _createParams(InParams), _handleId(InHandleId)
	{
		_renderWidth = InParams.RenderWidth();
		_renderHeight = InParams.RenderHeight();
		_displayWidth = InParams.DisplayWidth();
		_displayHeight = InParams.DisplayHeight();
		_targetWidth = InParams.TargetWidth();
		_targetHeight = InParams.TargetHeight();
	}

	virtual void Shutdown() = 0;
	virtual ~IFeature() {}
};