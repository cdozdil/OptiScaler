#pragma once
#include <pch.h>

#include <OwnedMutex.h>

#include <dxgi1_6.h>

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
    UINT64 _targetFrame = 0;
    
    int GetIndex();

public:
    OwnedMutex Mutex;

    bool IsActive();

    virtual UINT64 UpscaleStart() = 0;
    virtual void UpscaleEnd() = 0;

    void SetJitter(float x, float y);
    void SetMVScale(float x, float y);
    void SetCameraValues(float nearValue, float farValue, float vFov, float meterFactor = 0.0f);
    void SetFrameTimeDelta(float delta);
    void SetReset(UINT reset);

    virtual feature_version Version() = 0;
    virtual const char* Name() = 0;

    virtual void ReleaseObjects() = 0;
    virtual void StopAndDestroyContext(bool destroy, bool shutDown, bool useMutex) = 0;

    void ResetCounters();
    void UpdateTarget();
    
    UINT64 FrameCount();
    UINT64 TargetFrame();

    IFGFeature() = default;
};