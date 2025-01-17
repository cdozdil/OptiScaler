#include "../../pch.h"
#include "../../Config.h"
#include "FSR2Feature_212.h"

double FSR2Feature212::GetDeltaTime()
{
	double currentTime = MillisecondsNow();
	double deltaTime = (currentTime - _lastFrameTime);
	_lastFrameTime = currentTime;
	return deltaTime;
}

void FSR2Feature212::SetDepthInverted(bool InValue)
{
	_depthInverted = InValue;
}

bool FSR2Feature212::IsDepthInverted() const
{
	return _depthInverted;
}

double FSR2Feature212::MillisecondsNow()
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

FSR2Feature212::~FSR2Feature212()
{
	if (!IsInited())
		return;

	auto errorCode = Fsr212::ffxFsr2ContextDestroy212(&_context);

	free(_contextDesc.callbacks.scratchBuffer);

	SetInit(false);
}


