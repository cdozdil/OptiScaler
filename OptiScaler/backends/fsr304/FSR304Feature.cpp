#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "FSR304Feature.h"

double FSR304Feature::GetDeltaTime()
{
	double currentTime = MillisecondsNow();
	double deltaTime = (currentTime - _lastFrameTime);
	_lastFrameTime = currentTime;
	return deltaTime;
}

void FSR304Feature::SetDepthInverted(bool InValue)
{
	_depthInverted = InValue;
}

bool FSR304Feature::IsDepthInverted() const
{
	return _depthInverted;
}

double FSR304Feature::MillisecondsNow()
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

FSR304Feature::~FSR304Feature()
{
	if (!IsInited())
		return;

	auto errorCode = Fsr304::ffxFsr3ContextDestroy(&_context);

	if (errorCode != Fsr304::FFX_OK)
		spdlog::error("FSR304Feature::~FSR304Feature ffxFsr3ContextDestroy error: {0:x}", errorCode);

	free(_contextDesc.backendInterfaceUpscaling.scratchBuffer);

	SetInit(false);
}


