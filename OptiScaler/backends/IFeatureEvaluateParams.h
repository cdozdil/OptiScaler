#pragma once
#include "../pch.h"

class IFeatureEvaluateParams
{
private:
	inline static double _lastFrameTime = -1.0;

	inline static double MillisecondsNow()
	{
		LARGE_INTEGER s_frequency;
		bool s_use_qpc = QueryPerformanceFrequency(&s_frequency);
		double milliseconds = 0;

		if (s_use_qpc)
		{
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			milliseconds = double(1000.0 * now.QuadPart) / s_frequency.QuadPart;
		}
		else
		{
			milliseconds = double(GetTickCount64());
		}

		return milliseconds;
	}

protected:
	bool _isInited = false;

	void* _inputColor = nullptr;
	void* _inputDepth = nullptr;
	void* _inputMotion = nullptr;
	void* _inputExposure = nullptr;
	void* _inputMask = nullptr;
	void* _outputColor = nullptr;

	uint32_t _renderWidth = 0;
	uint32_t _renderHeight = 0;

	float _jitterOffsetX = 0.0f;
	float _jitterOffsetY = 0.0f;

	float _mvScaleX = 1.0f;
	float _mvScaleY = 1.0f;

	float _sharpness = 0.0f;
	float _exposureScale = 1.0f;
	float _preExposure = 1.0f;
	float _frameTimeDelta = 0.0f;

	bool _reset = false;

public:
	IFeatureEvaluateParams()
	{
		if (_lastFrameTime < 0)
		{
			_lastFrameTime = MillisecondsNow();
		}
		else
		{
			double currentTime = MillisecondsNow();
			_frameTimeDelta = currentTime - _lastFrameTime;
			_lastFrameTime = currentTime;
		}
	}

	virtual ParamterSource Source() { return Base; }

	bool IsInited() const { return _isInited; }

	void* InputColor() const { return _inputColor; }
	void* InputDepth() const { return _inputDepth; }
	void* InputMotion() const { return _inputMotion; }
	void* InputExposure() const { return _inputExposure; }
	void* InputMask() const { return _inputMask; }
	void* OutputColor() const { return _outputColor; }

	uint32_t RenderWidth() const { return _renderWidth; }
	uint32_t RenderHeight() const { return _renderHeight; }

	float JitterOffsetX() const { return _jitterOffsetX; }
	float JitterOffsetY() const { return _jitterOffsetY; }
	float MvScaleX() const { return _mvScaleX; }
	float MvScaleY() const { return _mvScaleY; }
	float Sharpness() const { return _sharpness; }
	float ExposureScale() const { return _exposureScale; }
	float FrameTimeDelta() const { return _frameTimeDelta; }

	bool Reset() const { return _reset; }
};
