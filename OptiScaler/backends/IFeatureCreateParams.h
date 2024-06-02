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
	bool IsInited() { return _isInited; }
	uint32_t RenderWidth() { return _renderWidth; }
	uint32_t RenderHeight() { return _renderHeight; }
	uint32_t DisplayWidth() { return _displayWidth; }
	uint32_t DisplayHeight() { return _displayHeight; }
	uint32_t TargetWidth() { return _targetWidth; }
	uint32_t TargetHeight() { return _targetHeight; }
	CommonQualityPreset QualityPreset() { return _pqValue; }
	bool AutoExposure() { return _autoExposure; }
	bool Hdr() { return _hdr; }
	bool Sharpening() { return _sharpening; }
	bool DisableReactiveMask() { return _disableReactiveMask; }
	bool JitterCancellation() { return _jitterCancellation; }
	bool DisplayResolutionMV() { return _displayResMV; }
	bool DepthInverted() { return _depthInverted; }

};