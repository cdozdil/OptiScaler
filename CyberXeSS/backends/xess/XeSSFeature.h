#pragma once
#include "../IFeature.h"
#include "xess_d3d12.h"
#include "xess_debug.h"
#include <string>

#include "../../cas/include/ffx_cas.h"
#include "../../cas/include/backends/dx12/ffx_dx12.h"

//dumpParams.frame_count = 1;
//dumpParams.frame_idx = cnt++;
//dumpParams.path = "F:\\";
//xessStartDump(deviceContext->XessContext, &dumpParams);

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

protected:
	xess_context_handle_t _xessContext = nullptr;

	// cas
	FfxCas::FfxCasContext casContext;
	bool casInit = false;
	bool casContextCreated = false;
	bool casUpscale = false;
	float casSharpness = 0.3f;
	ID3D12Resource* casBuffer = nullptr;
	FfxCas::FfxCasContextDescription casContextDesc = {};

	//rec709
	bool _recInit = false;
	ID3D12RootSignature* _recRootSignatureEncode = nullptr;
	ID3D12RootSignature* _recRootSignatureDecode = nullptr;
	ID3D12PipelineState* _recPSOEncode = nullptr;
	ID3D12PipelineState* _recPSODecode = nullptr;
	ID3D12Resource* _recBufferEncode = nullptr;
	ID3D12Resource* _recBufferDecode = nullptr;

	bool InitXeSS(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters);

	void CasInit();
	bool CreateCasContext(ID3D12Device* device);
	void DestroyCasContext();
	bool CreateCasBufferResource(ID3D12Resource* source, ID3D12Device* device);
	bool CasDispatch(ID3D12CommandList* commandList, const NVSDK_NGX_Parameter* initParams, ID3D12Resource* input, ID3D12Resource* output);

	bool RecInit(ID3D12Device* InDevice);
	bool RecDecode(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* input, ID3D12Resource* output);
	bool RecEncode(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* input, ID3D12Resource* output);
	bool CreateRecBufferResource(ID3D12Resource* source, ID3D12Device* device, ID3D12Resource** output);

public:

	XeSSFeature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(handleId, InParameters)
	{
	}

	~XeSSFeature();
};