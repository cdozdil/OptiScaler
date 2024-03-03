#pragma once
#include "IFeature.h"
#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
#include "FidelityFX/host/ffx_fsr2.h"


class FSR2Feature : public virtual IFeature
{
private:
	double _lastFrameTime;
	bool _depthInverted = false;

protected:
	FfxFsr2Context _context = {};
	FfxFsr2ContextDescription _contextDesc = {};

	bool InitFSR2(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters);
	double MillisecondsNow();

	double GetDeltaTime();
	bool IsDepthInverted() const;

public:

	FSR2Feature(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
		_lastFrameTime = MillisecondsNow();
	}

	~FSR2Feature();
};