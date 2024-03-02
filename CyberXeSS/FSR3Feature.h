#pragma once
#include "IFeature.h"
#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
#include "FidelityFX/host/ffx_fsr3upscaler.h"


class FSR3Feature : public IFeature
{
private:
	bool _isInited = false;
	double _lastFrameTime;
	bool _depthInverted = false;

protected:

	FfxFsr3UpscalerContext _context = {};
	FfxFsr3UpscalerContextDescription _contextDesc = {};

	bool InitFSR3(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);
	void SetRenderResolution(const NVSDK_NGX_Parameter* InParameters, FfxFsr3UpscalerDispatchDescription* params) const;
	double MillisecondsNow();

	void SetInit(bool value) override;
	double GetDeltaTime();
	bool IsDepthInverted() const;

public:

	FSR3Feature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
	{
		_lastFrameTime = MillisecondsNow();
	}

	bool IsInited() override;

	~FSR3Feature();
};