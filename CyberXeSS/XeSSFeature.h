#include "xess_d3d12.h"
#include "xess_debug.h"

#include "IFeature.h"

inline static std::string ResultToString(xess_result_t result)
{
	switch (result)
	{
	case XESS_RESULT_WARNING_NONEXISTING_FOLDER: return "Warning Nonexistent Folder";
	case XESS_RESULT_WARNING_OLD_DRIVER: return "Warning Old Driver";
	case XESS_RESULT_SUCCESS: return "Success";
	case XESS_RESULT_ERROR_UNSUPPORTED_DEVICE: return "Unsupported Device";
	case XESS_RESULT_ERROR_UNSUPPORTED_DRIVER: return "Unsupported Driver";
	case XESS_RESULT_ERROR_UNINITIALIZED: return "Uninitialized";
	case XESS_RESULT_ERROR_INVALID_ARGUMENT: return "Invalid Argument";
	case XESS_RESULT_ERROR_DEVICE_OUT_OF_MEMORY: return "Device Out of Memory";
	case XESS_RESULT_ERROR_DEVICE: return "Device Error";
	case XESS_RESULT_ERROR_NOT_IMPLEMENTED: return "Not Implemented";
	case XESS_RESULT_ERROR_INVALID_CONTEXT: return "Invalid Context";
	case XESS_RESULT_ERROR_OPERATION_IN_PROGRESS: return "Operation in Progress";
	case XESS_RESULT_ERROR_UNSUPPORTED: return "Unsupported";
	case XESS_RESULT_ERROR_CANT_LOAD_LIBRARY: return "Cannot Load Library";
	case XESS_RESULT_ERROR_UNKNOWN:
	default: return "Unknown";
	}
}

class XeSSFeature : public IFeature
{
private:
	bool _isInited = false;

protected:
	xess_context_handle_t _xessContext;

	bool InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);

	void SetRenderResolution(const NVSDK_NGX_Parameter* InParameters, xess_d3d12_execute_params_t* params) const;

	void SetInit(bool value) override;

public:

	XeSSFeature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters, Config* config) : IFeature(handleId, InParameters, config)
	{
	}

	bool IsInited() override;

	~XeSSFeature()
	{
		spdlog::debug("XeSSContext::Destroy!");

		if (!_xessContext)
			return;

		auto result = xessDestroyContext(_xessContext);

		if (result != XESS_RESULT_SUCCESS)
			spdlog::error("XeSSContext::Destroy xessDestroyContext error: {0}", ResultToString(result));
	}
};