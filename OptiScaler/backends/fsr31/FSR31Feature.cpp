#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "FSR31Feature.h"

double FSR31Feature::GetDeltaTime()
{
	double currentTime = MillisecondsNow();
	double deltaTime = (currentTime - _lastFrameTime);
	_lastFrameTime = currentTime;
	return deltaTime;
}

void FSR31Feature::SetDepthInverted(bool InValue)
{
	_depthInverted = InValue;
}

bool FSR31Feature::IsDepthInverted() const
{
	return _depthInverted;
}

double FSR31Feature::MillisecondsNow()
{
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
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

FSR31Feature::~FSR31Feature()
{
	if (!IsInited())
		return;

	_destroyContext(&_context, NULL);

	SetInit(false);
}


