#pragma once
#include "../../fsr2_212/include/ffx_fsr2.h"

#include "../IFeature.h"

inline static std::string ResultToString212(Fsr212::FfxErrorCode result)
{
	switch (result)
	{
	case Fsr212::FFX_OK: return "The operation completed successfully.";
	case Fsr212::FFX_ERROR_INVALID_POINTER: return "The operation failed due to an invalid pointer.";
	case Fsr212::FFX_ERROR_INVALID_ALIGNMENT: return "The operation failed due to an invalid alignment.";
	case Fsr212::FFX_ERROR_INVALID_SIZE: return "The operation failed due to an invalid size.";
	case Fsr212::FFX_EOF: return "The end of the file was encountered.";
	case Fsr212::FFX_ERROR_INVALID_PATH: return "The operation failed because the specified path was invalid.";
	case Fsr212::FFX_ERROR_EOF: return "The operation failed because end of file was reached.";
	case Fsr212::FFX_ERROR_MALFORMED_DATA: return "The operation failed because of some malformed data.";
	case Fsr212::FFX_ERROR_OUT_OF_MEMORY: return "The operation failed because it ran out memory.";
	case Fsr212::FFX_ERROR_INCOMPLETE_INTERFACE: return "The operation failed because the interface was not fully configured.";
	case Fsr212::FFX_ERROR_INVALID_ENUM: return "The operation failed because of an invalid enumeration value.";
	case Fsr212::FFX_ERROR_INVALID_ARGUMENT: return "The operation failed because an argument was invalid.";
	case Fsr212::FFX_ERROR_OUT_OF_RANGE: return "The operation failed because a value was out of range.";
	case Fsr212::FFX_ERROR_NULL_DEVICE: return "The operation failed because a device was null.";
	case Fsr212::FFX_ERROR_BACKEND_API_ERROR: return "The operation failed because the backend API returned an error code.";
	case Fsr212::FFX_ERROR_INSUFFICIENT_MEMORY: return "The operation failed because there was not enough memory.";
	default: return "Unknown";
	}
}

class FSR2Feature212 : public virtual IFeature
{
private:
	double _lastFrameTime;
	bool _depthInverted = false;
	unsigned int _lastWidth = 0;
	unsigned int _lastHeight = 0;
	static inline feature_version _version{ 2, 1, 2 };

protected:
	Fsr212::FfxFsr2Context _context = {};
	Fsr212::FfxFsr2ContextDescription _contextDesc = {};

	virtual bool InitFSR2(const NVSDK_NGX_Parameter* InParameters) = 0;

	double MillisecondsNow();
	double GetDeltaTime();
	void SetDepthInverted(bool InValue);

public:
	bool IsDepthInverted() const;
	feature_version Version() final { return _version; }
	const char* Name() override { return "FSR"; }

	FSR2Feature212(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
		_moduleLoaded = true;
		_lastFrameTime = MillisecondsNow();
	}

	~FSR2Feature212();
};