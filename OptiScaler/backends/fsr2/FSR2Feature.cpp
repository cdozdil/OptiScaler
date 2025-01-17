#include "../../pch.h"
#include "FSR2Feature.h"

double FSR2Feature::GetDeltaTime()
{
	double currentTime = MillisecondsNow();
	double deltaTime = (currentTime - _lastFrameTime);
	_lastFrameTime = currentTime;
	return deltaTime;
}

void FSR2Feature::SetDepthInverted(bool InValue)
{
	_depthInverted = InValue;
}

bool FSR2Feature::IsDepthInverted() const
{
	return _depthInverted;
}

double FSR2Feature::MillisecondsNow()
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

FSR2Feature::~FSR2Feature()
{
	if (!IsInited())
		return;

	auto errorCode = ffxFsr2ContextDestroy(&_context);

	free(_contextDesc.callbacks.scratchBuffer);

	SetInit(false);
}


