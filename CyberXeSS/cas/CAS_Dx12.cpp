#include "CAS_Dx12.h"

inline static std::string ResultToString(FfxCas::FfxErrorCode result)
{
	switch (result)
	{
	case FfxCas::FFX_OK: return "The operation completed successfully.";
	case FfxCas::FFX_ERROR_INVALID_POINTER: return "The operation failed due to an invalid pointer.";
	case FfxCas::FFX_ERROR_INVALID_ALIGNMENT: return "The operation failed due to an invalid alignment.";
	case FfxCas::FFX_ERROR_INVALID_SIZE: return "The operation failed due to an invalid size.";
	case FfxCas::FFX_EOF: return "The end of the file was encountered.";
	case FfxCas::FFX_ERROR_INVALID_PATH: return "The operation failed because the specified path was invalid.";
	case FfxCas::FFX_ERROR_EOF: return "The operation failed because end of file was reached.";
	case FfxCas::FFX_ERROR_MALFORMED_DATA: return "The operation failed because of some malformed data.";
	case FfxCas::FFX_ERROR_OUT_OF_MEMORY: return "The operation failed because it ran out memory.";
	case FfxCas::FFX_ERROR_INCOMPLETE_INTERFACE: return "The operation failed because the interface was not fully configured.";
	case FfxCas::FFX_ERROR_INVALID_ENUM: return "The operation failed because of an invalid enumeration value.";
	case FfxCas::FFX_ERROR_INVALID_ARGUMENT: return "The operation failed because an argument was invalid.";
	case FfxCas::FFX_ERROR_OUT_OF_RANGE: return "The operation failed because a value was out of range.";
	case FfxCas::FFX_ERROR_NULL_DEVICE: return "The operation failed because a device was null.";
	case FfxCas::FFX_ERROR_BACKEND_API_ERROR: return "The operation failed because the backend API returned an error code.";
	case FfxCas::FFX_ERROR_INSUFFICIENT_MEMORY: return "The operation failed because there was not enough memory.";
	default: return "Unknown";
	}
}

inline static void FfxCasLogCallback(const char* message)
{
	spdlog::debug("XeSSFeature::LogCallback FSR Runtime: {0}", message);
}

inline static FfxCas::FfxSurfaceFormat ffxGetSurfaceFormatDX12(DXGI_FORMAT format)
{
	switch (format) {

	case(DXGI_FORMAT_R32G32B32A32_TYPELESS):
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
	case(DXGI_FORMAT_R32G32B32A32_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return FfxCas::FFX_SURFACE_FORMAT_R32G32B32A32_UINT;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case(DXGI_FORMAT_R16G16B16A16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
		return FfxCas::FFX_SURFACE_FORMAT_R32G32_FLOAT;

	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return FfxCas::FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return FfxCas::FFX_SURFACE_FORMAT_R32_UINT;

	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return FfxCas::FFX_SURFACE_FORMAT_R8_UINT;

	case (DXGI_FORMAT_R11G11B10_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R11G11B10_FLOAT;

	case (DXGI_FORMAT_R8G8B8A8_TYPELESS):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
	case (DXGI_FORMAT_R8G8B8A8_UNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
	case (DXGI_FORMAT_R8G8B8A8_UNORM_SRGB):
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;

	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return FfxCas::FFX_SURFACE_FORMAT_R8G8B8A8_SNORM;

	case DXGI_FORMAT_R16G16_TYPELESS:
	case (DXGI_FORMAT_R16G16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16_FLOAT;

	case (DXGI_FORMAT_R16G16_UINT):
		return FfxCas::FFX_SURFACE_FORMAT_R16G16_UINT;

	case DXGI_FORMAT_R32_TYPELESS:
	case(DXGI_FORMAT_D32_FLOAT):
	case(DXGI_FORMAT_R32_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R8G8_TYPELESS:
		return FfxCas::FFX_SURFACE_FORMAT_R8G8_UNORM;

	case DXGI_FORMAT_R16_TYPELESS:
	case (DXGI_FORMAT_R16_FLOAT):
		return FfxCas::FFX_SURFACE_FORMAT_R16_FLOAT;
	case (DXGI_FORMAT_R16_UINT):
		return FfxCas::FFX_SURFACE_FORMAT_R16_UINT;
	case DXGI_FORMAT_D16_UNORM:
	case (DXGI_FORMAT_R16_UNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R16_UNORM;
	case (DXGI_FORMAT_R16_SNORM):
		return FfxCas::FFX_SURFACE_FORMAT_R16_SNORM;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_A8_UNORM:
		return FfxCas::FFX_SURFACE_FORMAT_R8_UNORM;

	case(DXGI_FORMAT_UNKNOWN):
		return FfxCas::FFX_SURFACE_FORMAT_UNKNOWN;

	default:
		return FfxCas::FFX_SURFACE_FORMAT_UNKNOWN;
	}
}

inline static bool IsDepthDX12(DXGI_FORMAT format)
{
	return (format == DXGI_FORMAT_D16_UNORM) ||
		(format == DXGI_FORMAT_D32_FLOAT) ||
		(format == DXGI_FORMAT_D24_UNORM_S8_UINT) ||
		(format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
}

inline static FfxCas::FfxResourceDescription GetFfxResourceDescriptionDX12(ID3D12Resource* pResource)
{
	FfxCas::FfxResourceDescription resourceDescription = {};

	// This is valid
	if (!pResource)
		return resourceDescription;

	if (pResource)
	{
		D3D12_RESOURCE_DESC desc = pResource->GetDesc();

		// Set flags properly for resource registration
		resourceDescription.flags = FfxCas::FFX_RESOURCE_FLAGS_NONE;
		resourceDescription.usage = IsDepthDX12(desc.Format) ? FfxCas::FFX_RESOURCE_USAGE_DEPTHTARGET : FfxCas::FFX_RESOURCE_USAGE_READ_ONLY;
		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			resourceDescription.usage = (FfxCas::FfxResourceUsage)(resourceDescription.usage | FfxCas::FFX_RESOURCE_USAGE_UAV);

		resourceDescription.width = desc.Width;
		resourceDescription.height = desc.Height;
		resourceDescription.depth = desc.DepthOrArraySize;
		resourceDescription.mipCount = desc.MipLevels;
		resourceDescription.format = ffxGetSurfaceFormatDX12(desc.Format);

		resourceDescription.type = FfxCas::FFX_RESOURCE_TYPE_TEXTURE2D;
	}

	return resourceDescription;
}

bool CAS_Dx12::CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState)
{
	if (InDevice == nullptr || InSource == nullptr)
		return false;

	D3D12_RESOURCE_DESC texDesc = InSource->GetDesc();

	if (_buffer != nullptr)
	{
		auto bufDesc = _buffer->GetDesc();

		if (bufDesc.Width != texDesc.Width || bufDesc.Height != texDesc.Height || bufDesc.Format != texDesc.Format)
		{
			_buffer->Release();
			_buffer = nullptr;
		}
		else
			return true;
	}

	spdlog::debug("CAS_Dx12::CreateCasBufferResource Start!");

	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags;
	HRESULT hr = InSource->GetHeapProperties(&heapProperties, &heapFlags);

	if (hr != S_OK)
	{
		spdlog::error("CAS_Dx12::CreateBufferResource GetHeapProperties result: {0:x}", hr);
		return false;
	}

	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

	hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(&_buffer));

	if (hr != S_OK)
	{
		spdlog::error("CAS_Dx12::CreateBufferResource CreateCommittedResource result: {0:x}", hr);
		return false;
	}

	_bufferState = InState;

	return true;
}

bool CAS_Dx12::Dispatch(ID3D12CommandList* InCommandList, float InSharpness, ID3D12Resource* InResource, ID3D12Resource* OutResource)
{
	if (!_init || InCommandList == nullptr || InResource == nullptr || OutResource == nullptr || InSharpness <= 0.0f)
		return false;

	spdlog::debug("CAS_Dx12::CasDispatch Start!");

	FfxCas::FfxCasDispatchDescription dispatchParameters = {};
	dispatchParameters.commandList = FfxCas::ffxGetCommandListDX12Cas(InCommandList);
	dispatchParameters.renderSize = { _width, _height };
	dispatchParameters.sharpness = InSharpness;
	dispatchParameters.color = FfxCas::ffxGetResourceDX12Cas(InResource, GetFfxResourceDescriptionDX12(InResource), nullptr, FfxCas::FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
	dispatchParameters.output = FfxCas::ffxGetResourceDX12Cas(OutResource, GetFfxResourceDescriptionDX12(OutResource), nullptr, FfxCas::FFX_RESOURCE_STATE_UNORDERED_ACCESS);

	if (auto errorCode = FfxCas::ffxCasContextDispatch(&_context, &dispatchParameters); errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("CAS_Dx12::CasDispatch ffxCasContextDispatch error: {0}", ResultToString(errorCode));
		return false;
	}

	return true;
}

void CAS_Dx12::SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState)
{
	if (_bufferState == InState)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = _buffer;
	barrier.Transition.StateBefore = _bufferState;
	barrier.Transition.StateAfter = InState;
	barrier.Transition.Subresource = 0;
	InCommandList->ResourceBarrier(1, &barrier);
	_bufferState = InState;
}

CAS_Dx12::CAS_Dx12(ID3D12Device* InDevice, uint32_t InWidth, uint32_t InHeight, int colorSpace = 0)
{
	spdlog::debug("CAS_Dx12::CAS_Dx12 Start!");

	const size_t scratchBufferSize = FfxCas::ffxGetScratchMemorySizeDX12Cas(FFX_CAS_CONTEXT_COUNT);
	void* casScratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = FfxCas::ffxGetInterfaceDX12Cas(&_contextDesc.backendInterface, InDevice, casScratchBuffer, scratchBufferSize, FFX_CAS_CONTEXT_COUNT);

	if (errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("CAS_Dx12::CAS_Dx12 ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
		_init = false;
		return;
	}

	_contextDesc.flags |= FfxCas::FFX_CAS_SHARPEN_ONLY;

	auto casCSC = colorSpace;

	if (casCSC < 0 || casCSC > 4)
		casCSC = 0;

	_width = InWidth;
	_height = InHeight;

	_contextDesc.colorSpaceConversion = static_cast<FfxCas::FfxCasColorSpaceConversion>(casCSC);
	_contextDesc.maxRenderSize.width = InWidth + 16;		// Sometimes when using DLAA mode
	_contextDesc.maxRenderSize.height = InHeight + 16;		// internal resolution is higher then display resolution
	_contextDesc.displaySize.width = InWidth;
	_contextDesc.displaySize.height = InHeight;

	errorCode = FfxCas::ffxCasContextCreate(&_context, &_contextDesc);

	if (errorCode != FfxCas::FFX_OK)
	{
		spdlog::error("CAS_Dx12::CAS_Dx12 ffxCasContextCreate error: {0}", ResultToString(errorCode));
		_init = false;
		return;
	}

	FfxCas::ffxAssertSetPrintingCallback(FfxCasLogCallback);

	_init = true;
}

CAS_Dx12::~CAS_Dx12()
{
	if (!_init)
		return;

	FfxCas::ffxCasContextDestroy(&_context);
	free(_contextDesc.backendInterface.scratchBuffer);

	if (_buffer != nullptr)
	{
		_buffer->Release();
		_buffer = nullptr;
	}
}
