#include "IFGFeature.h"

int IFGFeature::GetIndex()
{
    return (_frameCount % BUFFER_COUNT);
}

bool IFGFeature::IsActive()
{
    return _isActive && _frameCount > _targetFrame;
}

void IFGFeature::SetJitter(float x, float y)
{
    _jitterX = x;
    _jitterY = y;
}

void IFGFeature::SetMVScale(float x, float y)
{
    _mvScaleX = x;
    _mvScaleY = y;
}

void IFGFeature::SetCameraValues(float nearValue, float farValue, float vFov, float meterFactor)
{
    _cameraFar = farValue;
    _cameraNear = nearValue;
    _cameraVFov = vFov;
    _meterFactor = meterFactor;
}

void IFGFeature::SetFrameTimeDelta(float delta)
{
    _ftDelta = delta;
}

void IFGFeature::SetReset(UINT reset)
{
    _reset = reset;
}

void IFGFeature::ResetCounters()
{
    _frameCount = 0;
    _targetFrame = 10;
}

void IFGFeature::UpdateTarget()
{
    _targetFrame = _frameCount + 10;
}

