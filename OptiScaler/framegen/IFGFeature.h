#pragma once
#include <pch.h>

#include <State.h>
#include <Config.h>
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

    UINT64 _frameCount = 0;

    bool _isActive = false;
    UINT64 _targetFrame = 10;
    
    int GetIndex();

public:
    OwnedMutex Mutex;

    UINT64 UpscaleStart();
    virtual void UpscaleEnd(float lastFrameTime) = 0;

    void SetJitter(float x, float y);
    void SetMVScale(float x, float y);
    void SetCameraValues(float nearValue, float farValue, float vFov, float meterFactor = 0.0f);
    void SetFrameTimeDelta(float delta);
    void SetReset(UINT reset);

    virtual feature_version Version() = 0;
    virtual const char* Name() = 0;

    virtual bool Dispatch(UINT64 frameId, double frameTime) = 0;

    virtual void ReleaseObjects() = 0;
    virtual void StopAndDestroyContext(bool destroy, bool shutDown, bool useMutex) = 0;
};