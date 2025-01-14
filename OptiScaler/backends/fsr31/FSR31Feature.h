#pragma once
#include "../../FfxApi_Proxy.h"
#include "ffx_upscale.h"

#include "../IFeature.h"
#include "../../detours/detours.h"

inline static void FfxLogCallback(uint32_t type, const wchar_t* message)
{
    std::wstring string(message);

    //if (type == FFX_API_MESSAGE_TYPE_ERROR)
    //	LOG_ERROR("FSR Runtime: {0}", str);
    //else if (type == FFX_API_MESSAGE_TYPE_WARNING)
    //	LOG_WARN("FSR Runtime: {0}", str);
    //else
    LOG_DEBUG("FSR Runtime: {0}", wstring_to_string(string));
}

class FSR31Feature : public virtual IFeature
{
private:
    double _lastFrameTime;
    bool _depthInverted = false;
    unsigned int _lastWidth = 0;
    unsigned int _lastHeight = 0;
    static inline feature_version _version{ 3, 1, 2 };

protected:
    std::string _name = "FSR 3.X";

    ffxContext _context = nullptr;
    ffxCreateContextDescUpscale _contextDesc = {};

    virtual bool InitFSR3(const NVSDK_NGX_Parameter* InParameters) = 0;

    double MillisecondsNow();
    double GetDeltaTime();
    void SetDepthInverted(bool InValue);

    static inline void parse_version(const char* version_str) 
    {
        if (sscanf_s(version_str, "%u.%u.%u", &_version.major, &_version.minor, &_version.patch) != 3)
            LOG_WARN("can't parse {0}", version_str);
    }

    float _velocity = 0.0f;

public:
    bool IsDepthInverted() const;
    feature_version Version() final { return _version; }
    const char* Name() override { return _name.c_str(); }

    FSR31Feature(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
    {
        _lastFrameTime = MillisecondsNow();
    }

    ~FSR31Feature();
};