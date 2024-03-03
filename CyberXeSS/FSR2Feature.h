#pragma once
#include "IFeature.h"

#include "FidelityFX/host/ffx_fsr2.h"

inline void FfxLogCallback(FfxMsgType type, const wchar_t* message)
{
	std::wstring string(message);
	std::string str(string.begin(), string.end());

	if (type == FFX_MESSAGE_TYPE_ERROR)
		spdlog::error("FSR2Feature::LogCallback FSR Runtime: {0}", str);
	else if (type == FFX_MESSAGE_TYPE_WARNING)
		spdlog::warn("FSR2Feature::LogCallback FSR Runtime: {0}", str);
	else
		spdlog::debug("FSR2Feature::LogCallback FSR Runtime: {0}", str);
}

class FSR2Feature : public virtual IFeature
{
private:
	double _lastFrameTime;
	bool _depthInverted = false;

protected:
	FfxFsr2Context _context = {};
	FfxFsr2ContextDescription _contextDesc = {};

	virtual bool InitFSR2(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters) = 0;
	
	double MillisecondsNow();
	double GetDeltaTime();
	void SetDepthInverted(bool InValue);

public:
	bool IsDepthInverted() const;

	FSR2Feature(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
		_lastFrameTime = MillisecondsNow();
	}

	~FSR2Feature();
};