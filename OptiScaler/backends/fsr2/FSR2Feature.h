#pragma once
#include <fsr2/ffx_fsr2.h>

#include <backends/IFeature.h>

inline static std::string ResultToString(FfxErrorCode result)
{
	switch (result)
	{
	case FFX_OK: return "The operation completed successfully.";
	case FFX_ERROR_INVALID_POINTER: return "The operation failed due to an invalid pointer.";
	case FFX_ERROR_INVALID_ALIGNMENT: return "The operation failed due to an invalid alignment.";
	case FFX_ERROR_INVALID_SIZE: return "The operation failed due to an invalid size.";
	case FFX_EOF: return "The end of the file was encountered.";
	case FFX_ERROR_INVALID_PATH: return "The operation failed because the specified path was invalid.";
	case FFX_ERROR_EOF: return "The operation failed because end of file was reached.";
	case FFX_ERROR_MALFORMED_DATA: return "The operation failed because of some malformed data.";
	case FFX_ERROR_OUT_OF_MEMORY: return "The operation failed because it ran out memory.";
	case FFX_ERROR_INCOMPLETE_INTERFACE: return "The operation failed because the interface was not fully configured.";
	case FFX_ERROR_INVALID_ENUM: return "The operation failed because of an invalid enumeration value.";
	case FFX_ERROR_INVALID_ARGUMENT: return "The operation failed because an argument was invalid.";
	case FFX_ERROR_OUT_OF_RANGE: return "The operation failed because a value was out of range.";
	case FFX_ERROR_NULL_DEVICE: return "The operation failed because a device was null.";
	case FFX_ERROR_BACKEND_API_ERROR: return "The operation failed because the backend API returned an error code.";
	case FFX_ERROR_INSUFFICIENT_MEMORY: return "The operation failed because there was not enough memory.";
	default: return "Unknown";
	}
}

inline static void FfxLogCallback(FfxFsr2MsgType type, const wchar_t* message)
{
	std::wstring string(message);

	//if (type == FFX_FSR2_MESSAGE_TYPE_ERROR)
	//	LOG_ERROR("FSR Runtime: {0}", str);
	//else if (type == FFX_FSR2_MESSAGE_TYPE_WARNING)
	//	LOG_WARN("FSR Runtime: {0}", str);
	//else
	LOG_DEBUG("FSR Runtime: {0}", wstring_to_string(string));
}

class FSR2Feature : public virtual IFeature
{
private:
	double _lastFrameTime;
	bool _depthInverted = false;
	unsigned int _lastWidth = 0;
	unsigned int _lastHeight = 0;
	static inline feature_version _version{ 2, 2, 1 };

protected:
	FfxFsr2Context _context = {};
	FfxFsr2ContextDescription _contextDesc = {};

	virtual bool InitFSR2(const NVSDK_NGX_Parameter* InParameters) = 0;

	double MillisecondsNow();
	double GetDeltaTime();
	void SetDepthInverted(bool InValue);

public:
	bool IsDepthInverted() const;
	feature_version Version() final { return _version; }
	const char* Name() override { return "FSR 2.2"; }

	FSR2Feature(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
		_moduleLoaded = true;
		_lastFrameTime = MillisecondsNow();
	}

	~FSR2Feature();
};