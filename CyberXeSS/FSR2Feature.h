#pragma once
#include "IFeature.h"
#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
#include "FidelityFX/host/ffx_fsr2.h"


class FSR2Feature : public IFeature
{
private:
	bool _isInited = false;
	double _lastFrameTime;
	bool _depthInverted = false;

protected:

	FfxFsr2Context _context = {};
	FfxFsr2ContextDescription _contextDesc = {};

	bool InitFSR2(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);
	void SetRenderResolution(const NVSDK_NGX_Parameter* InParameters, FfxFsr2DispatchDescription* params) const;
	double MillisecondsNow();

	void SetInit(bool value) override;
	double GetDeltaTime();
	bool IsDepthInverted() const;

public:

	FSR2Feature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
	{
		_lastFrameTime = MillisecondsNow();
	}

	bool IsInited() override;

	~FSR2Feature();
};