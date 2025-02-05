#pragma once
#include <pch.h>

#include <OwnedMutex.h>

#include <dxgi.h>

class IFGFeature
{
protected:
    float _jitterX = 0.0;
    float _jitterY = 0.0;
    float _mvScaleX = 0.0;
    float _mvScaleY = 0.0;
    float _cameraNear = 0.0;
    float _cameraFar = 0.0;
    float _cameraVFov = 0.0;
    float _meterFactor = 0.0;
    float _ftDelta = 0.0;
    UINT _reset = 0;

    bool _isActive = false;
    UINT64 _targetFrame = 10;
    bool _skipWrapping = false;

    OwnedMutex _mutex;

public:
    void SetJitter(float x, float y);
    void SetMVScale(float x, float y);
    void SetCameraValues(float nearValue, float farValue, float vFov, float meterFactor = 0.0f);
    void SetFrameTimeDelta(float delta);
    void SetReset(UINT reset);

    virtual feature_version Version() = 0;
    virtual const char* Name() = 0;

};