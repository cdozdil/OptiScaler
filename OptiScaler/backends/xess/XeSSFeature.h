#pragma once
#include <backends/IFeature.h>

#include <xess_d3d12.h>
#include <xess_debug.h>

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

typedef xess_result_t(*PFN_xessD3D12CreateContext)(ID3D12Device* pDevice, xess_context_handle_t* phContext);
typedef xess_result_t(*PFN_xessD3D12BuildPipelines)(xess_context_handle_t hContext, ID3D12PipelineLibrary* pPipelineLibrary, bool blocking, uint32_t initFlags);
typedef xess_result_t(*PRN_xessD3D12Init)(xess_context_handle_t hContext, const xess_d3d12_init_params_t* pInitParams);
typedef xess_result_t(*PFN_xessD3D12Execute)(xess_context_handle_t hContext, ID3D12GraphicsCommandList* pCommandList, const xess_d3d12_execute_params_t* pExecParams);
typedef xess_result_t(*PFN_xessSelectNetworkModel)(xess_context_handle_t hContext, xess_network_model_t network);
typedef xess_result_t(*PFN_xessStartDump)(xess_context_handle_t hContext, const xess_dump_parameters_t* dump_parameters);
typedef xess_result_t(*PRN_xessGetVersion)(xess_version_t* pVersion);
typedef xess_result_t(*PFN_xessIsOptimalDriver)(xess_context_handle_t hContext);
typedef xess_result_t(*PFN_xessSetLoggingCallback)(xess_context_handle_t hContext, xess_logging_level_t loggingLevel, xess_app_log_callback_t loggingCallback);
typedef xess_result_t(*PFN_xessGetProperties)(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_properties_t* pBindingProperties);
typedef xess_result_t(*PFN_xessDestroyContext)(xess_context_handle_t hContext);
typedef xess_result_t(*PFN_xessSetVelocityScale)(xess_context_handle_t hContext, float x, float y);

class XeSSFeature : public virtual IFeature
{
private:
	std::string _version = "1.3.0";
	xess_version_t _xessVersion{};

	ID3D12PipelineLibrary* _localPipeline = nullptr;
	ID3D12Heap* _localBufferHeap = nullptr;
	ID3D12Heap* _localTextureHeap = nullptr;

	PFN_xessD3D12CreateContext _xessD3D12CreateContext = nullptr;
	PFN_xessD3D12BuildPipelines _xessD3D12BuildPipelines = nullptr;
	PRN_xessD3D12Init _xessD3D12Init = nullptr;
	PFN_xessD3D12Execute _xessD3D12Execute = nullptr;
	PFN_xessSelectNetworkModel _xessSelectNetworkModel = nullptr;
	PFN_xessStartDump _xessStartDump = nullptr;
	PRN_xessGetVersion _xessGetVersion = nullptr;
	PFN_xessIsOptimalDriver _xessIsOptimalDriver = nullptr;
	PFN_xessSetLoggingCallback _xessSetLoggingCallback = nullptr;
	PFN_xessGetProperties _xessGetProperties = nullptr;
	PFN_xessDestroyContext _xessDestroyContext = nullptr;
	PFN_xessSetVelocityScale _xessSetVelocityScale = nullptr;

	HMODULE _libxess = nullptr;

protected:
	xess_context_handle_t _xessContext = nullptr;
	
	int dumpCount = 0;

	bool InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);

	PFN_xessD3D12CreateContext D3D12CreateContext() const { return _xessD3D12CreateContext; }
	PFN_xessD3D12BuildPipelines D3D12BuildPipelines() const { return _xessD3D12BuildPipelines; }
	PRN_xessD3D12Init D3D12Init() const { return _xessD3D12Init; }
	PFN_xessD3D12Execute D3D12Execute() const { return _xessD3D12Execute; }
	PFN_xessSelectNetworkModel SelectNetworkModel() const { return _xessSelectNetworkModel; }
	PFN_xessStartDump StartDump() const { return _xessStartDump; }
	PRN_xessGetVersion GetVersion() const { return _xessGetVersion; }
	PFN_xessIsOptimalDriver IsOptimalDriver() const { return _xessIsOptimalDriver; }
	PFN_xessSetLoggingCallback SetLoggingCallback() const { return _xessSetLoggingCallback; }
	PFN_xessGetProperties GetProperties() const { return _xessGetProperties; }
	PFN_xessDestroyContext DestroyContext() const { return _xessDestroyContext; }
	PFN_xessSetVelocityScale SetVelocityScale() const { return _xessSetVelocityScale; }

public:
	feature_version Version() final { return feature_version{ _xessVersion.major, _xessVersion.minor, _xessVersion.patch }; }
	const char* Name() override { return "XeSS"; }

	XeSSFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters);

	~XeSSFeature();
};