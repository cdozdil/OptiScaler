#pragma once

#include <upscalers/IFeature.h>

#include <string>

#include <XeSS_Proxy.h>

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

class XeSSFeature : public virtual IFeature
{
private:
	std::string _version = "1.3.0";

	ID3D12PipelineLibrary* _localPipeline = nullptr;
	ID3D12Heap* _localBufferHeap = nullptr;
	ID3D12Heap* _localTextureHeap = nullptr;

protected:
	xess_context_handle_t _xessContext = nullptr;
	
	int dumpCount = 0;

	bool InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);

public:
	feature_version Version() final { return feature_version{ XeSSProxy::Version().major, XeSSProxy::Version().minor, XeSSProxy::Version().patch}; }
	const char* Name() override { return "XeSS"; }

	XeSSFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters);

	~XeSSFeature();
};