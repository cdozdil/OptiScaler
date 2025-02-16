#include "IFGFeature.h"

#include <Config.h>

int IFGFeature::GetIndex()
{
    return (_frameCount % BUFFER_COUNT);
}

bool IFGFeature::CheckForRealObject(std::string functionName, IUnknown* pObject, IUnknown** ppRealObject)
{
    //return false;

    if (streamlineRiid.Data1 == 0)
    {
        auto iidResult = IIDFromString(L"{ADEC44E2-61F0-45C3-AD9F-1B37379284FF}", &streamlineRiid);

        if (iidResult != S_OK)
            return false;
    }

    auto qResult = pObject->QueryInterface(streamlineRiid, (void**)ppRealObject);

    if (qResult == S_OK && *ppRealObject != nullptr)
    {
        LOG_INFO("{} Streamline proxy found!", functionName);
        (*ppRealObject)->Release();
        return true;
    }

    return false;
}

bool IFGFeature::IsActive()
{
    return _isActive;
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
    _targetFrame = 0;
}

void IFGFeature::UpdateTarget()
{
    _targetFrame = _frameCount + 10;
    LOG_DEBUG("Current frame: {} target frame: {}", _frameCount, _targetFrame);
}

UINT64 IFGFeature::FrameCount()
{
    return _frameCount;
}

UINT64 IFGFeature::TargetFrame()
{
    return _targetFrame;
}

