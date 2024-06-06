#pragma once
#include "../pch.h"

enum CommonQualityPreset
{
	NativeAA,
	UltraQuality,
	Quality,
	Balanced,
	Performance,
	UltraPerformance,
	Undefined
};

class IFeatureCreateParams
{
protected:
	bool _isInited = false;

	uint32_t _renderWidth;
	uint32_t _renderHeight;
	uint32_t _displayWidth;
	uint32_t _displayHeight;
	uint32_t _targetWidth;
	uint32_t _targetHeight;

	CommonQualityPreset _pqValue = Undefined;

	bool _autoExposure = false;
	bool _hdr = false;
	bool _sharpening = false;
	bool _disableReactiveMask = false;
	bool _jitterCancellation = false;
	bool _displayResMV = false;
	bool _depthInverted = false;

public:
	bool IsInited() const { return _isInited; }

	uint32_t RenderWidth() const { return _renderWidth; }
	uint32_t RenderHeight() const { return _renderHeight; }
	uint32_t DisplayWidth() const { return _displayWidth; }
	uint32_t DisplayHeight() const { return _displayHeight; }
	uint32_t TargetWidth() const { return _targetWidth; }
	uint32_t TargetHeight() const { return _targetHeight; }

	CommonQualityPreset QualityPreset() const { return _pqValue; }

	bool AutoExposure() const { return _autoExposure; }
	bool Hdr() const { return _hdr; }
	bool Sharpening() const { return _sharpening; }
	bool DisableReactiveMask() const { return _disableReactiveMask; }
	bool JitterCancellation() const { return _jitterCancellation; }
	bool DisplayResolutionMV() const { return _displayResMV; }
	bool DepthInverted() const { return _depthInverted; }
};