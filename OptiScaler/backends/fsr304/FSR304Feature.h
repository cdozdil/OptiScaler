#pragma once
#include <string>
#include "../../Fsr304/include/ffx_fsr3.h"
#include "../../Fsr304/include/ffx_types.h"
#include "../../Fsr304/include/ffx_error.h"

#include "../IFeature.h"
#include "../../detours/detours.h"

class FSR304Feature : public virtual IFeature
{
private:
    double _lastFrameTime;
    bool _depthInverted = false;
    unsigned int _lastWidth = 0;
    unsigned int _lastHeight = 0;
    static inline feature_version _version{ 3, 0, 4 };

protected:
    std::string _name = "FSR 3.0.4";

    Fsr304::FfxFsr3Context _context = {};
    Fsr304::FfxFsr3ContextDescription _contextDesc = {};

    virtual bool InitFSR3(const NVSDK_NGX_Parameter* InParameters) = 0;

    double MillisecondsNow();
    double GetDeltaTime();
    void SetDepthInverted(bool InValue);

    static inline void parse_version(const char* version_str) {
        if (sscanf_s(version_str, "%u.%u.%u", &_version.major, &_version.minor, &_version.patch) != 3)
            LOG_WARN("can't parse {0}", version_str);
    }

public:
    bool IsDepthInverted() const;
    feature_version Version() final { return _version; }
    const char* Name() override { return _name.c_str(); }

    FSR304Feature(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
    {
        _lastFrameTime = MillisecondsNow();
    }

    ~FSR304Feature();
};