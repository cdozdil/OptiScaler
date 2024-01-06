// This file is part of the FidelityFX SDK.
// 
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <FidelityFX/host/ffx_interface.h>
#include <FidelityFX/host/ffx_util.h>
#include <FidelityFX/host/ffx_assert.h>
#include <FidelityFX/host/backends/dx12/ffx_dx12.h>
#include <FidelityFX/host/backends/dx12/d3dx12.h>
#include "shared/ffx_shader_blobs.h"
#include <codecvt>  // convert string to wstring
#include <mutex>

// DX12 prototypes for functions in the backend interface
FfxUInt32 GetSDKVersionDX12(FfxInterface* backendInterface);
FfxErrorCode CreateBackendContextDX12(FfxInterface* backendInterface, FfxUInt32* effectContextId);
FfxErrorCode GetDeviceCapabilitiesDX12(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities);
FfxErrorCode DestroyBackendContextDX12(FfxInterface* backendInterface, FfxUInt32 effectContextId);
FfxErrorCode CreateResourceDX12(FfxInterface* backendInterface, const FfxCreateResourceDescription* desc, FfxUInt32 effectContextId, FfxResourceInternal* outTexture);
FfxErrorCode DestroyResourceDX12(FfxInterface* backendInterface, FfxResourceInternal resource, FfxUInt32 effectContextId);
FfxErrorCode RegisterResourceDX12(FfxInterface* backendInterface, const FfxResource* inResource, FfxUInt32 effectContextId, FfxResourceInternal* outResourceInternal);
FfxResource GetResourceDX12(FfxInterface* backendInterface, FfxResourceInternal resource);
FfxErrorCode UnregisterResourcesDX12(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId);
FfxResourceDescription GetResourceDescriptorDX12(FfxInterface* backendInterface, FfxResourceInternal resource);
FfxErrorCode CreatePipelineDX12(FfxInterface* backendInterface, FfxEffect effect, FfxPass passId, uint32_t permutationOptions, const FfxPipelineDescription* desc, FfxUInt32 effectContextId, FfxPipelineState* outPass);
FfxErrorCode DestroyPipelineDX12(FfxInterface* backendInterface, FfxPipelineState* pipeline, FfxUInt32 effectContextId);
FfxErrorCode ScheduleGpuJobDX12(FfxInterface* backendInterface, const FfxGpuJobDescription* job);
FfxErrorCode ExecuteGpuJobsDX12(FfxInterface* backendInterface, FfxCommandList commandList);

#define FFX_MAX_RESOURCE_IDENTIFIER_COUNT   (128)

typedef struct BackendContext_DX12 {

	// store for resources and resourceViews
	typedef struct Resource
	{
#ifdef _DEBUG
		wchar_t                 resourceName[64] = {};
#endif
		ID3D12Resource* resourcePtr;
		FfxResourceDescription  resourceDescription;
		FfxResourceStates       initialState;
		FfxResourceStates       currentState;
		uint32_t                srvDescIndex;
		uint32_t                uavDescIndex;
		uint32_t                uavDescCount;
	} Resource;

	uint32_t refCount;
	uint32_t maxEffectContexts;

	ID3D12Device* device = nullptr;

	FfxGpuJobDescription* pGpuJobs;
	uint32_t                gpuJobCount;

	uint32_t                nextRtvDescriptor;
	ID3D12DescriptorHeap* descHeapRtvCpu;

	ID3D12DescriptorHeap* descHeapSrvCpu;
	ID3D12DescriptorHeap* descHeapUavCpu;
	ID3D12DescriptorHeap* descHeapUavGpu;

	uint32_t                descRingBufferSize;
	uint32_t                descRingBufferBase;
	ID3D12DescriptorHeap* descRingBuffer;

	void* constantBufferMem;
	ID3D12Resource* constantBufferResource;
	uint32_t                constantBufferSize;
	uint32_t                constantBufferOffset;
	std::mutex              constantBufferMutex;

	D3D12_RESOURCE_BARRIER  barriers[FFX_MAX_BARRIERS];
	uint32_t                barrierCount;

	typedef struct alignas(32) EffectContext {

		// Resource allocation
		uint32_t            nextStaticResource;
		uint32_t            nextDynamicResource;

		// UAV offsets
		uint32_t            nextStaticUavDescriptor;
		uint32_t            nextDynamicUavDescriptor;

		// Usage
		bool                active;

	} EffectContext;

	// Resource holder
	Resource* pResources;
	EffectContext* pEffectContexts;

} BackendContext_DX12;

FFX_API size_t ffxGetScratchMemorySizeDX12(size_t maxContexts)
{
	uint32_t resourceArraySize = FFX_ALIGN_UP(maxContexts * FFX_MAX_RESOURCE_COUNT * sizeof(BackendContext_DX12::Resource), sizeof(uint64_t));
	uint32_t contextArraySize = FFX_ALIGN_UP(maxContexts * sizeof(BackendContext_DX12::EffectContext), sizeof(uint32_t));
	uint32_t gpuJobDescArraySize = FFX_ALIGN_UP(maxContexts * FFX_MAX_GPU_JOBS * sizeof(FfxGpuJobDescription), sizeof(uint32_t));

	return FFX_ALIGN_UP(sizeof(BackendContext_DX12) + resourceArraySize + contextArraySize + gpuJobDescArraySize, sizeof(uint64_t));
}

// Create a FfxDevice from a ID3D12Device*
FfxDevice ffxGetDeviceDX12(ID3D12Device* dx12Device)
{
	FFX_ASSERT(NULL != dx12Device);
	return reinterpret_cast<FfxDevice>(dx12Device);
}

// populate interface with DX12 pointers.
FfxErrorCode ffxGetInterfaceDX12(
	FfxInterface* backendInterface,
	FfxDevice device,
	void* scratchBuffer,
	size_t scratchBufferSize,
	uint32_t maxContexts) {

	FFX_RETURN_ON_ERROR(
		backendInterface,
		FFX_ERROR_INVALID_POINTER);
	FFX_RETURN_ON_ERROR(
		scratchBuffer,
		FFX_ERROR_INVALID_POINTER);
	FFX_RETURN_ON_ERROR(
		scratchBufferSize >= ffxGetScratchMemorySizeDX12(maxContexts),
		FFX_ERROR_INSUFFICIENT_MEMORY);

	backendInterface->fpGetSDKVersion = GetSDKVersionDX12;
	backendInterface->fpCreateBackendContext = CreateBackendContextDX12;
	backendInterface->fpGetDeviceCapabilities = GetDeviceCapabilitiesDX12;
	backendInterface->fpDestroyBackendContext = DestroyBackendContextDX12;
	backendInterface->fpCreateResource = CreateResourceDX12;
	backendInterface->fpDestroyResource = DestroyResourceDX12;
	backendInterface->fpGetResource = GetResourceDX12;
	backendInterface->fpRegisterResource = RegisterResourceDX12;
	backendInterface->fpUnregisterResources = UnregisterResourcesDX12;
	backendInterface->fpGetResourceDescription = GetResourceDescriptorDX12;
	backendInterface->fpCreatePipeline = CreatePipelineDX12;
	backendInterface->fpGetPermutationBlobByIndex = ffxGetPermutationBlobByIndex;
	backendInterface->fpDestroyPipeline = DestroyPipelineDX12;
	backendInterface->fpScheduleGpuJob = ScheduleGpuJobDX12;
	backendInterface->fpExecuteGpuJobs = ExecuteGpuJobsDX12;
	//backendInterface->fpSwapChainConfigureFrameGeneration = ffxSetFrameGenerationConfigToSwapchainDX12;

	// Memory assignments
	backendInterface->scratchBuffer = scratchBuffer;
	backendInterface->scratchBufferSize = scratchBufferSize;

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	FFX_RETURN_ON_ERROR(
		!backendContext->refCount,
		FFX_ERROR_BACKEND_API_ERROR);

	// Clear everything out
	memset(backendContext, 0, sizeof(*backendContext));

	// Set the device
	backendInterface->device = device;

	// Assign the max number of contexts we'll be using
	backendContext->maxEffectContexts = (uint32_t)maxContexts;

	return FFX_OK;
}

FfxCommandList ffxGetCommandListDX12(ID3D12CommandList* cmdList)
{
	FFX_ASSERT(NULL != cmdList);
	return reinterpret_cast<FfxCommandList>(cmdList);
}

// register a DX12 resource to the backend
FfxResource ffxGetResourceDX12(const ID3D12Resource* dx12Resource,
	FfxResourceDescription                     ffxResDescription,
	wchar_t* ffxResName,
	FfxResourceStates                          state /*=FFX_RESOURCE_STATE_COMPUTE_READ*/)
{
	FfxResource resource = {};
	resource.resource = reinterpret_cast<void*>(const_cast<ID3D12Resource*>(dx12Resource));
	resource.state = state;
	resource.description = ffxResDescription;

#ifdef _DEBUG
	if (ffxResName) {
		wcscpy_s(resource.name, ffxResName);
	}
#endif

	return resource;
}

void TIF(HRESULT result)
{
	if (FAILED(result)) {

		wchar_t errorMessage[256];
		memset(errorMessage, 0, 256);
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorMessage, 255, NULL);
		char errA[256];
		size_t returnSize;
		wcstombs_s(&returnSize, errA, 255, errorMessage, 255);
#ifdef _DEBUG
		int32_t msgboxID = MessageBoxW(NULL, errorMessage, L"Error", MB_OK);
#endif
		throw 1;
	}
}

// fix up format in case resource passed for UAV cannot be mapped
static DXGI_FORMAT convertFormatUav(DXGI_FORMAT format)
{
	switch (format) {
		// Handle Depth
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

		// Handle color: assume FLOAT for 16 and 32 bit channels, else UNORM
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R32G32_TYPELESS:
		return DXGI_FORMAT_R32G32_FLOAT;
	case DXGI_FORMAT_R16G16_TYPELESS:
		return DXGI_FORMAT_R16G16_FLOAT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R8G8_TYPELESS:
		return DXGI_FORMAT_R8G8_UNORM;
	case DXGI_FORMAT_R16_TYPELESS:
		return DXGI_FORMAT_R16_FLOAT;
	case DXGI_FORMAT_R8_TYPELESS:
		return DXGI_FORMAT_R8_UNORM;
	default:
		return format;
	}
}

// fix up format in case resource passed for SRV cannot be mapped
static DXGI_FORMAT convertFormatSrv(DXGI_FORMAT format)
{
	switch (format) {
		// Handle Depth
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

		// Handle Color
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

		// Others can map as is
	default:

		return format;
	}
}

D3D12_RESOURCE_STATES ffxGetDX12StateFromResourceState(FfxResourceStates state)
{
	switch (state) {

	case (FFX_RESOURCE_STATE_GENERIC_READ):
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	case (FFX_RESOURCE_STATE_UNORDERED_ACCESS):
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case (FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ):
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case (FFX_RESOURCE_STATE_COMPUTE_READ):
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case (FFX_RESOURCE_STATE_PIXEL_READ):
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case FFX_RESOURCE_STATE_COPY_SRC:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case FFX_RESOURCE_STATE_COPY_DEST:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case FFX_RESOURCE_STATE_INDIRECT_ARGUMENT:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case FFX_RESOURCE_STATE_PRESENT:
		return D3D12_RESOURCE_STATE_PRESENT;
	case FFX_RESOURCE_STATE_COMMON:
		return D3D12_RESOURCE_STATE_COMMON;
	case FFX_RESOURCE_STATE_RENDER_TARGET:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	default:
		FFX_ASSERT_MESSAGE(false, "Resource state not yet supported");
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

DXGI_FORMAT ffxGetDX12FormatFromSurfaceFormat(FfxSurfaceFormat surfaceFormat)
{
	switch (surfaceFormat) {

	case (FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS):
		return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	case (FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT):
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case (FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT):
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case (FFX_SURFACE_FORMAT_R32G32_FLOAT):
		return DXGI_FORMAT_R32G32_FLOAT;
	case (FFX_SURFACE_FORMAT_R32_UINT):
		return DXGI_FORMAT_R32_UINT;
	case(FFX_SURFACE_FORMAT_R10G10B10A2_UNORM):
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case (FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS):
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	case (FFX_SURFACE_FORMAT_R8G8B8A8_UNORM):
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case (FFX_SURFACE_FORMAT_R8G8B8A8_SRGB):
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case (FFX_SURFACE_FORMAT_R8G8B8A8_SNORM):
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case (FFX_SURFACE_FORMAT_R11G11B10_FLOAT):
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case (FFX_SURFACE_FORMAT_R16G16_FLOAT):
		return DXGI_FORMAT_R16G16_FLOAT;
	case (FFX_SURFACE_FORMAT_R16G16_UINT):
		return DXGI_FORMAT_R16G16_UINT;
	case (FFX_SURFACE_FORMAT_R16G16_SINT):
		return DXGI_FORMAT_R16G16_SINT;
	case (FFX_SURFACE_FORMAT_R16_FLOAT):
		return DXGI_FORMAT_R16_FLOAT;
	case (FFX_SURFACE_FORMAT_R16_UINT):
		return DXGI_FORMAT_R16_UINT;
	case (FFX_SURFACE_FORMAT_R16_UNORM):
		return DXGI_FORMAT_R16_UNORM;
	case (FFX_SURFACE_FORMAT_R16_SNORM):
		return DXGI_FORMAT_R16_SNORM;
	case (FFX_SURFACE_FORMAT_R8_UNORM):
		return DXGI_FORMAT_R8_UNORM;
	case (FFX_SURFACE_FORMAT_R8_UINT):
		return DXGI_FORMAT_R8_UINT;
	case (FFX_SURFACE_FORMAT_R8G8_UINT):
		return DXGI_FORMAT_R8G8_UINT;
	case (FFX_SURFACE_FORMAT_R8G8_UNORM):
		return DXGI_FORMAT_R8G8_UNORM;
	case (FFX_SURFACE_FORMAT_R32_FLOAT):
		return DXGI_FORMAT_R32_FLOAT;
	case (FFX_SURFACE_FORMAT_UNKNOWN):
		return DXGI_FORMAT_UNKNOWN;

	default:
		FFX_ASSERT_MESSAGE(false, "Format not yet supported");
		return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_RESOURCE_FLAGS ffxGetDX12ResourceFlags(FfxResourceUsage flags)
{
	D3D12_RESOURCE_FLAGS dx12ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
	if (flags & FFX_RESOURCE_USAGE_RENDERTARGET) dx12ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	if (flags & FFX_RESOURCE_USAGE_UAV) dx12ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	return dx12ResourceFlags;
}

FfxSurfaceFormat ffxGetSurfaceFormatDX12(DXGI_FORMAT format)
{
	switch (format) {

	case(DXGI_FORMAT_R32G32B32A32_TYPELESS):
		return FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
	case(DXGI_FORMAT_R32G32B32A32_FLOAT):
		return FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return FFX_SURFACE_FORMAT_R32G32B32A32_UINT;
		//case DXGI_FORMAT_R32G32B32A32_SINT:
		//case DXGI_FORMAT_R32G32B32_TYPELESS:
		//case DXGI_FORMAT_R32G32B32_FLOAT:
		//case DXGI_FORMAT_R32G32B32_UINT:
		//case DXGI_FORMAT_R32G32B32_SINT:

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case(DXGI_FORMAT_R16G16B16A16_FLOAT):
		return FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
		//case DXGI_FORMAT_R16G16B16A16_UNORM:
		//case DXGI_FORMAT_R16G16B16A16_UINT:
		//case DXGI_FORMAT_R16G16B16A16_SNORM:
		//case DXGI_FORMAT_R16G16B16A16_SINT:

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
		return FFX_SURFACE_FORMAT_R32G32_FLOAT;
		//case DXGI_FORMAT_R32G32_FLOAT:
		//case DXGI_FORMAT_R32G32_UINT:
		//case DXGI_FORMAT_R32G32_SINT:

	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return FFX_SURFACE_FORMAT_R32_UINT;

	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return FFX_SURFACE_FORMAT_R8_UINT;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return FFX_SURFACE_FORMAT_R10G10B10A2_UNORM;
		//case DXGI_FORMAT_R10G10B10A2_UINT:

	case (DXGI_FORMAT_R11G11B10_FLOAT):
		return FFX_SURFACE_FORMAT_R11G11B10_FLOAT;

	case (DXGI_FORMAT_R8G8B8A8_TYPELESS):
		return FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
	case (DXGI_FORMAT_R8G8B8A8_UNORM):
		return FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
	case (DXGI_FORMAT_R8G8B8A8_UNORM_SRGB):
		return FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;
		//case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return FFX_SURFACE_FORMAT_R8G8B8A8_SNORM;

	case DXGI_FORMAT_R16G16_TYPELESS:
	case (DXGI_FORMAT_R16G16_FLOAT):
		return FFX_SURFACE_FORMAT_R16G16_FLOAT;
		//case DXGI_FORMAT_R16G16_UNORM:
	case (DXGI_FORMAT_R16G16_UINT):
		return FFX_SURFACE_FORMAT_R16G16_UINT;
		//case DXGI_FORMAT_R16G16_SNORM
		//case DXGI_FORMAT_R16G16_SINT 

		//case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R32_UINT:
		return FFX_SURFACE_FORMAT_R32_UINT;
	case DXGI_FORMAT_R32_TYPELESS:
	case(DXGI_FORMAT_D32_FLOAT):
	case(DXGI_FORMAT_R32_FLOAT):
		return FFX_SURFACE_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case (DXGI_FORMAT_R8G8_UINT):
		return FFX_SURFACE_FORMAT_R8G8_UINT;
		//case DXGI_FORMAT_R8G8_UNORM:
		//case DXGI_FORMAT_R8G8_SNORM:
		//case DXGI_FORMAT_R8G8_SINT:

	case DXGI_FORMAT_R16_TYPELESS:
	case (DXGI_FORMAT_R16_FLOAT):
		return FFX_SURFACE_FORMAT_R16_FLOAT;
	case (DXGI_FORMAT_R16_UINT):
		return FFX_SURFACE_FORMAT_R16_UINT;
	case DXGI_FORMAT_D16_UNORM:
	case (DXGI_FORMAT_R16_UNORM):
		return FFX_SURFACE_FORMAT_R16_UNORM;
	case (DXGI_FORMAT_R16_SNORM):
		return FFX_SURFACE_FORMAT_R16_SNORM;
		//case DXGI_FORMAT_R16_SINT:

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_A8_UNORM:
		return FFX_SURFACE_FORMAT_R8_UNORM;
	case DXGI_FORMAT_R8_UINT:
		return FFX_SURFACE_FORMAT_R8_UINT;
		//case DXGI_FORMAT_R8_SNORM:
		//case DXGI_FORMAT_R8_SINT:
		//case DXGI_FORMAT_R1_UNORM:

	case(DXGI_FORMAT_UNKNOWN):
		return FFX_SURFACE_FORMAT_UNKNOWN;
	default:
		FFX_ASSERT_MESSAGE(false, "Format not yet supported");
		return FFX_SURFACE_FORMAT_UNKNOWN;
	}
}

bool IsDepthDX12(DXGI_FORMAT format)
{
	return (format == DXGI_FORMAT_D16_UNORM) ||
		(format == DXGI_FORMAT_D32_FLOAT) ||
		(format == DXGI_FORMAT_D24_UNORM_S8_UINT) ||
		(format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
}

FfxResourceDescription GetFfxResourceDescriptionDX12(ID3D12Resource* pResource)
{
	FfxResourceDescription resourceDescription = {};

	// This is valid
	if (!pResource)
		return resourceDescription;

	if (pResource)
	{
		D3D12_RESOURCE_DESC desc = pResource->GetDesc();

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			resourceDescription.flags = FFX_RESOURCE_FLAGS_NONE;
			resourceDescription.usage = FFX_RESOURCE_USAGE_UAV;
			resourceDescription.width = (uint32_t)desc.Width;
			resourceDescription.height = (uint32_t)desc.Height;
			resourceDescription.format = ffxGetSurfaceFormatDX12(desc.Format);

			// What should we initialize this to?? No case for this yet
			resourceDescription.depth = 0;
			resourceDescription.mipCount = 0;

			// Set the type
			resourceDescription.type = FFX_RESOURCE_TYPE_BUFFER;
		}
		else
		{
			// Set flags properly for resource registration
			resourceDescription.flags = FFX_RESOURCE_FLAGS_NONE;
			resourceDescription.usage = IsDepthDX12(desc.Format) ? FFX_RESOURCE_USAGE_DEPTHTARGET : FFX_RESOURCE_USAGE_READ_ONLY;
			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
				resourceDescription.usage = (FfxResourceUsage)(resourceDescription.usage | FFX_RESOURCE_USAGE_UAV);

			resourceDescription.width = (uint32_t)desc.Width;
			resourceDescription.height = (uint32_t)desc.Height;
			resourceDescription.depth = desc.DepthOrArraySize;
			resourceDescription.mipCount = desc.MipLevels;
			resourceDescription.format = ffxGetSurfaceFormatDX12(desc.Format);

			switch (desc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE1D;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (desc.DepthOrArraySize == 1)
					resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE2D;
				else if (desc.DepthOrArraySize == 6)
					resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE_CUBE;
				else
					resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE2D;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE3D;
				break;
			default:
				FFX_ASSERT_MESSAGE(false, "FFXInterface: Cauldron: Unsupported texture dimension requested. Please implement.");
				break;
			}
		}
	}

	return resourceDescription;
}

ID3D12Resource* getDX12ResourcePtr(BackendContext_DX12* backendContext, int32_t resourceIndex)
{
	FFX_ASSERT(NULL != backendContext);
	return reinterpret_cast<ID3D12Resource*>(backendContext->pResources[resourceIndex].resourcePtr);
}

void addBarrier(BackendContext_DX12* backendContext, FfxResourceInternal* resource, FfxResourceStates newState)
{
	FFX_ASSERT(NULL != backendContext);
	FFX_ASSERT(NULL != resource);

	ID3D12Resource* dx12Resource = getDX12ResourcePtr(backendContext, resource->internalIndex);
	D3D12_RESOURCE_BARRIER* barrier = &backendContext->barriers[backendContext->barrierCount];

	FFX_ASSERT(backendContext->barrierCount < FFX_MAX_BARRIERS);

	FfxResourceStates* currentState = &backendContext->pResources[resource->internalIndex].currentState;

	if ((*currentState & newState) != newState) {

		*barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			dx12Resource,
			ffxGetDX12StateFromResourceState(*currentState),
			ffxGetDX12StateFromResourceState(newState));

		*currentState = newState;
		++backendContext->barrierCount;

	}
	else if (newState == FFX_RESOURCE_STATE_UNORDERED_ACCESS) {

		*barrier = CD3DX12_RESOURCE_BARRIER::UAV(dx12Resource);
		++backendContext->barrierCount;
	}
}

void flushBarriers(BackendContext_DX12* backendContext, ID3D12GraphicsCommandList* dx12CommandList)
{
	FFX_ASSERT(NULL != backendContext);
	FFX_ASSERT(NULL != dx12CommandList);

	if (backendContext->barrierCount > 0) {

		dx12CommandList->ResourceBarrier(backendContext->barrierCount, backendContext->barriers);
		backendContext->barrierCount = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// DX12 back end implementation

FfxUInt32 GetSDKVersionDX12(FfxInterface* backendInterface)
{
	return FFX_SDK_MAKE_VERSION(FFX_SDK_VERSION_MAJOR, FFX_SDK_VERSION_MINOR, FFX_SDK_VERSION_PATCH);
}

// initialize the DX12 backend
FfxErrorCode CreateBackendContextDX12(FfxInterface* backendInterface, FfxUInt32* effectContextId)
{
	FFX_ASSERT(NULL != backendInterface);
	FFX_ASSERT(NULL != backendInterface->device);

	HRESULT result = S_OK;
	ID3D12Device* dx12Device = reinterpret_cast<ID3D12Device*>(backendInterface->device);

	// set up some internal resources we need (space for resource views and constant buffers)
	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	// Set things up if this is the first invocation
	if (!backendContext->refCount) {

		new (&backendContext->constantBufferMutex) std::mutex();

		if (dx12Device != NULL) {

			dx12Device->AddRef();
			backendContext->device = dx12Device;
		}

		// Map all of our pointers
		uint32_t gpuJobDescArraySize = FFX_ALIGN_UP(backendContext->maxEffectContexts * FFX_MAX_GPU_JOBS * sizeof(FfxGpuJobDescription), sizeof(uint32_t));
		uint32_t resourceArraySize = FFX_ALIGN_UP(backendContext->maxEffectContexts * FFX_MAX_RESOURCE_COUNT * sizeof(BackendContext_DX12::Resource), sizeof(uint64_t));
		uint32_t contextArraySize = FFX_ALIGN_UP(backendContext->maxEffectContexts * sizeof(BackendContext_DX12::EffectContext), sizeof(uint32_t));
		uint8_t* pMem = (uint8_t*)((BackendContext_DX12*)(backendContext + 1));

		// Map gpu job array
		backendContext->pGpuJobs = (FfxGpuJobDescription*)pMem;
		memset(backendContext->pGpuJobs, 0, gpuJobDescArraySize);
		pMem += gpuJobDescArraySize;

		// Map the resources
		backendContext->pResources = (BackendContext_DX12::Resource*)(pMem);
		memset(backendContext->pResources, 0, resourceArraySize);
		pMem += resourceArraySize;

		// Map the effect contexts
		backendContext->pEffectContexts = reinterpret_cast<BackendContext_DX12::EffectContext*>(pMem);
		memset(backendContext->pEffectContexts, 0, contextArraySize);

		// CPUVisible
		D3D12_DESCRIPTOR_HEAP_DESC descHeap;
		descHeap.NumDescriptors = FFX_MAX_RESOURCE_COUNT * backendContext->maxEffectContexts;
		descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeap.NodeMask = 0;

		result = dx12Device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&backendContext->descHeapSrvCpu));
		result = dx12Device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&backendContext->descHeapUavCpu));

		// GPU
		descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		result = dx12Device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&backendContext->descHeapUavGpu));

		// descriptor ring buffer
		descHeap.NumDescriptors = FFX_RING_BUFFER_SIZE * backendContext->maxEffectContexts;
		backendContext->descRingBufferSize = descHeap.NumDescriptors;
		backendContext->descRingBufferBase = 0;
		result = dx12Device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&backendContext->descRingBuffer));

		// RTV descriptor heap to raster jobs
		descHeap.NumDescriptors = 8;
		descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeap.NodeMask = 0;
		descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dx12Device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&backendContext->descHeapRtvCpu));

		std::lock_guard<std::mutex> cbLock{ backendContext->constantBufferMutex };
		// create dynamic ring buffer for constant uploads
		backendContext->constantBufferSize = FFX_ALIGN_UP(FFX_MAX_CONST_SIZE, 256) *
			backendContext->maxEffectContexts * FFX_MAX_PASS_COUNT * FFX_MAX_QUEUED_FRAMES; // Size aligned to 256

		CD3DX12_RESOURCE_DESC constDesc = CD3DX12_RESOURCE_DESC::Buffer(backendContext->constantBufferSize);
		CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
		TIF(dx12Device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
			&constDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&backendContext->constantBufferResource)));
		backendContext->constantBufferResource->SetName(L"FFX_DX12_DynamicRingBuffer");

		// map it
		TIF(backendContext->constantBufferResource->Map(0, nullptr,
			(void**)&backendContext->constantBufferMem));
		backendContext->constantBufferOffset = 0;
	}

	// Increment the ref count
	++backendContext->refCount;

	// Get an available context id
	for (uint32_t i = 0; i < backendContext->maxEffectContexts; ++i) {
		if (!backendContext->pEffectContexts[i].active) {
			*effectContextId = i;

			// Reset everything accordingly
			BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[i];
			effectContext.active = true;
			effectContext.nextStaticResource = (i * FFX_MAX_RESOURCE_COUNT) + 1;
			effectContext.nextDynamicResource = (i * FFX_MAX_RESOURCE_COUNT) + FFX_MAX_RESOURCE_COUNT - 1;
			effectContext.nextStaticUavDescriptor = (i * FFX_MAX_RESOURCE_COUNT);
			effectContext.nextDynamicUavDescriptor = (i * FFX_MAX_RESOURCE_COUNT) + FFX_MAX_RESOURCE_COUNT - 1;
			break;
		}
	}

	return FFX_OK;
}

// query device capabilities to select the optimal shader permutation
FfxErrorCode GetDeviceCapabilitiesDX12(FfxInterface* backendInterface, FfxDeviceCapabilities* deviceCapabilities)
{
	FFX_ASSERT(NULL != backendInterface);
	FFX_ASSERT(NULL != backendInterface->device);
	FFX_ASSERT(NULL != deviceCapabilities);
	ID3D12Device* dx12Device = reinterpret_cast<ID3D12Device*>(backendInterface->device);

	// Check if we have shader model 6.6
	bool haveShaderModel66 = true;
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_6 };
	if (SUCCEEDED(dx12Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(D3D12_FEATURE_DATA_SHADER_MODEL)))) {

		switch (shaderModel.HighestShaderModel) {

		case D3D_SHADER_MODEL_5_1:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_5_1;
			break;

		case D3D_SHADER_MODEL_6_0:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_0;
			break;

		case D3D_SHADER_MODEL_6_1:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_1;
			break;

		case D3D_SHADER_MODEL_6_2:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_2;
			break;

		case D3D_SHADER_MODEL_6_3:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_3;
			break;

		case D3D_SHADER_MODEL_6_4:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_4;
			break;

		case D3D_SHADER_MODEL_6_5:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_5;
			break;

		case D3D_SHADER_MODEL_6_6:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_6;
			break;

		default:
			deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_6_6;
			break;
		}
	}
	else {

		deviceCapabilities->minimumSupportedShaderModel = FFX_SHADER_MODEL_5_1;
	}

	// check if we can force wave64 mode.
	D3D12_FEATURE_DATA_D3D12_OPTIONS1 d3d12Options1 = {};
	if (SUCCEEDED(dx12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &d3d12Options1, sizeof(d3d12Options1)))) {

		const uint32_t waveLaneCountMin = d3d12Options1.WaveLaneCountMin;
		const uint32_t waveLaneCountMax = d3d12Options1.WaveLaneCountMax;
		deviceCapabilities->waveLaneCountMin = waveLaneCountMin;
		deviceCapabilities->waveLaneCountMax = waveLaneCountMax;
	}

	// check if we have 16bit floating point.
	D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12Options = {};
	if (SUCCEEDED(dx12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12Options, sizeof(d3d12Options)))) {

		deviceCapabilities->fp16Supported = !!(d3d12Options.MinPrecisionSupport & D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT);
	}

	// check if we have raytracing support
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 d3d12Options5 = {};
	if (SUCCEEDED(dx12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options5, sizeof(d3d12Options5)))) {

		deviceCapabilities->raytracingSupported = (d3d12Options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
	}

	return FFX_OK;
}

// deinitialize the DX12 backend
FfxErrorCode DestroyBackendContextDX12(FfxInterface* backendInterface, FfxUInt32 effectContextId)
{
	FFX_ASSERT(NULL != backendInterface);
	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;
	FFX_ASSERT(backendContext->refCount > 0);

	// Delete any resources allocated by this context
	BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[effectContextId];
	for (uint32_t currentStaticResourceIndex = effectContextId * FFX_MAX_RESOURCE_COUNT; currentStaticResourceIndex < (uint32_t)effectContext.nextStaticResource; ++currentStaticResourceIndex) {
		if (backendContext->pResources[currentStaticResourceIndex].resourcePtr) {
			FFX_ASSERT_MESSAGE(false, "FFXInterface: DX12: SDK Resource was not destroyed prior to destroying the backend context. There is a resource leak.");
			FfxResourceInternal internalResource = { (int32_t)currentStaticResourceIndex };
			DestroyResourceDX12(backendInterface, internalResource, effectContextId);
		}
	}

	// Free up for use by another context
	effectContext.nextStaticResource = 0;
	effectContext.active = false;

	// Decrement ref count
	--backendContext->refCount;

	if (!backendContext->refCount) {

		// release constant buffer pool
		backendContext->constantBufferResource->Unmap(0, nullptr);
		backendContext->constantBufferResource->Release();
		backendContext->constantBufferMem = nullptr;
		backendContext->constantBufferOffset = 0;
		backendContext->constantBufferSize = 0;
		backendContext->gpuJobCount = 0;
		backendContext->barrierCount = 0;

		// release heaps
		backendContext->descHeapRtvCpu->Release();
		backendContext->descHeapSrvCpu->Release();
		backendContext->descHeapUavCpu->Release();
		backendContext->descHeapUavGpu->Release();
		backendContext->descRingBuffer->Release();

		if (backendContext->device != NULL) {
			backendContext->device->Release();
			backendContext->device = NULL;
		}
	}

	return FFX_OK;
}

// create a internal resource that will stay alive until effect gets shut down
FfxErrorCode CreateResourceDX12(
	FfxInterface* backendInterface,
	const FfxCreateResourceDescription* createResourceDescription,
	FfxUInt32 effectContextId,
	FfxResourceInternal* outTexture
)
{
	FFX_ASSERT(NULL != backendInterface);
	FFX_ASSERT(NULL != createResourceDescription);
	FFX_ASSERT(NULL != outTexture);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;
	BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[effectContextId];
	ID3D12Device* dx12Device = backendContext->device;

	FFX_ASSERT(NULL != dx12Device);

	D3D12_HEAP_PROPERTIES dx12HeapProperties = {};
	dx12HeapProperties.Type = (createResourceDescription->heapType == FFX_HEAP_TYPE_DEFAULT) ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD;

	FFX_ASSERT(effectContext.nextStaticResource + 1 < effectContext.nextDynamicResource);

	outTexture->internalIndex = effectContext.nextStaticResource++;
	BackendContext_DX12::Resource* backendResource = &backendContext->pResources[outTexture->internalIndex];
	backendResource->resourceDescription = createResourceDescription->resourceDescription;

	D3D12_RESOURCE_DESC dx12ResourceDescription = {};
	dx12ResourceDescription.Format = DXGI_FORMAT_UNKNOWN;
	dx12ResourceDescription.Width = 1;
	dx12ResourceDescription.Height = 1;
	dx12ResourceDescription.MipLevels = 1;
	dx12ResourceDescription.DepthOrArraySize = 1;
	dx12ResourceDescription.SampleDesc.Count = 1;
	dx12ResourceDescription.Flags = ffxGetDX12ResourceFlags(backendResource->resourceDescription.usage);

	switch (createResourceDescription->resourceDescription.type) {

	case FFX_RESOURCE_TYPE_BUFFER:
		dx12ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		dx12ResourceDescription.Width = createResourceDescription->resourceDescription.width;
		dx12ResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		break;

	case FFX_RESOURCE_TYPE_TEXTURE1D:
		dx12ResourceDescription.Format = ffxGetDX12FormatFromSurfaceFormat(createResourceDescription->resourceDescription.format);
		dx12ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		dx12ResourceDescription.Width = createResourceDescription->resourceDescription.width;
		dx12ResourceDescription.DepthOrArraySize = createResourceDescription->resourceDescription.depth;
		dx12ResourceDescription.MipLevels = createResourceDescription->resourceDescription.mipCount;
		break;

	case FFX_RESOURCE_TYPE_TEXTURE2D:
		dx12ResourceDescription.Format = ffxGetDX12FormatFromSurfaceFormat(createResourceDescription->resourceDescription.format);
		dx12ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dx12ResourceDescription.Width = createResourceDescription->resourceDescription.width;
		dx12ResourceDescription.Height = createResourceDescription->resourceDescription.height;
		dx12ResourceDescription.DepthOrArraySize = createResourceDescription->resourceDescription.depth;
		dx12ResourceDescription.MipLevels = createResourceDescription->resourceDescription.mipCount;
		break;

	case FFX_RESOURCE_TYPE_TEXTURE_CUBE:
	case FFX_RESOURCE_TYPE_TEXTURE3D:
		dx12ResourceDescription.Format = ffxGetDX12FormatFromSurfaceFormat(createResourceDescription->resourceDescription.format);
		dx12ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		dx12ResourceDescription.Width = createResourceDescription->resourceDescription.width;
		dx12ResourceDescription.Height = createResourceDescription->resourceDescription.height;
		dx12ResourceDescription.DepthOrArraySize = createResourceDescription->resourceDescription.depth;
		dx12ResourceDescription.MipLevels = createResourceDescription->resourceDescription.mipCount;
		break;

	default:
		break;
	}

	ID3D12Resource* dx12Resource = nullptr;
	if (createResourceDescription->heapType == FFX_HEAP_TYPE_UPLOAD) {

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT dx12Footprint = {};

		UINT rowCount;
		UINT64 rowSizeInBytes;
		UINT64 totalBytes;

		dx12Device->GetCopyableFootprints(&dx12ResourceDescription, 0, 1, 0, &dx12Footprint, &rowCount, &rowSizeInBytes, &totalBytes);

		D3D12_HEAP_PROPERTIES dx12UploadHeapProperties = {};
		dx12UploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC dx12UploadBufferDescription = {};

		dx12UploadBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		dx12UploadBufferDescription.Width = totalBytes;
		dx12UploadBufferDescription.Height = 1;
		dx12UploadBufferDescription.DepthOrArraySize = 1;
		dx12UploadBufferDescription.MipLevels = 1;
		dx12UploadBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
		dx12UploadBufferDescription.SampleDesc.Count = 1;
		dx12UploadBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		TIF(dx12Device->CreateCommittedResource(&dx12HeapProperties, D3D12_HEAP_FLAG_NONE, &dx12UploadBufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&dx12Resource)));
		backendResource->initialState = FFX_RESOURCE_STATE_GENERIC_READ;
		backendResource->currentState = FFX_RESOURCE_STATE_GENERIC_READ;

		D3D12_RANGE dx12EmptyRange = {};
		void* uploadBufferData = nullptr;
		TIF(dx12Resource->Map(0, &dx12EmptyRange, &uploadBufferData));

		const uint8_t* src = static_cast<uint8_t*>(createResourceDescription->initData);
		uint8_t* dst = static_cast<uint8_t*>(uploadBufferData);
		for (uint32_t currentRowIndex = 0; currentRowIndex < createResourceDescription->resourceDescription.height; ++currentRowIndex) {

			memcpy(dst, src, (size_t)rowSizeInBytes);
			src += rowSizeInBytes;
			dst += dx12Footprint.Footprint.RowPitch;
		}

		dx12Resource->Unmap(0, nullptr);
		dx12Resource->SetName(createResourceDescription->name);
		backendResource->resourcePtr = dx12Resource;

#ifdef _DEBUG
		wcscpy_s(backendResource->resourceName, createResourceDescription->name);
#endif
		return FFX_OK;

	}
	else {

		const FfxResourceStates resourceStates = (createResourceDescription->initData && (createResourceDescription->heapType != FFX_HEAP_TYPE_UPLOAD)) ? FFX_RESOURCE_STATE_COPY_DEST : createResourceDescription->initalState;
		// Buffers ignore any input state and create in common (but issue a warning)
		const D3D12_RESOURCE_STATES dx12ResourceStates = dx12ResourceDescription.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? D3D12_RESOURCE_STATE_COMMON : ffxGetDX12StateFromResourceState(resourceStates);

		TIF(dx12Device->CreateCommittedResource(&dx12HeapProperties, D3D12_HEAP_FLAG_NONE, &dx12ResourceDescription, dx12ResourceStates, nullptr, IID_PPV_ARGS(&dx12Resource)));
		backendResource->initialState = resourceStates;
		backendResource->currentState = resourceStates;

		dx12Resource->SetName(createResourceDescription->name);
		backendResource->resourcePtr = dx12Resource;

#ifdef _DEBUG
		wcscpy_s(backendResource->resourceName, createResourceDescription->name);
#endif

		// Create SRVs and UAVs
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC dx12UavDescription = {};
			D3D12_SHADER_RESOURCE_VIEW_DESC dx12SrvDescription = {};
			D3D12_RESOURCE_DESC dx12Desc = dx12Resource->GetDesc();
			dx12UavDescription.Format = convertFormatUav(dx12Desc.Format);
			dx12SrvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			dx12SrvDescription.Format = convertFormatSrv(dx12Desc.Format);

			bool requestArrayView = FFX_CONTAINS_FLAG(createResourceDescription->resourceDescription.usage, FFX_RESOURCE_USAGE_ARRAYVIEW);

			switch (dx12Desc.Dimension) {

			case D3D12_RESOURCE_DIMENSION_BUFFER:
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (dx12Desc.DepthOrArraySize > 1 || requestArrayView)
				{
					dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					dx12UavDescription.Texture1DArray.ArraySize = dx12Desc.DepthOrArraySize;
					dx12UavDescription.Texture1DArray.FirstArraySlice = 0;
					dx12UavDescription.Texture1DArray.MipSlice = 0;

					dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					dx12SrvDescription.Texture1DArray.ArraySize = dx12Desc.DepthOrArraySize;
					dx12SrvDescription.Texture1DArray.FirstArraySlice = 0;
					dx12SrvDescription.Texture1DArray.MipLevels = dx12Desc.MipLevels;
					dx12SrvDescription.Texture1DArray.MostDetailedMip = 0;
				}
				else
				{
					dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					dx12UavDescription.Texture1D.MipSlice = 0;

					dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					dx12SrvDescription.Texture1D.MipLevels = dx12Desc.MipLevels;
					dx12SrvDescription.Texture1D.MostDetailedMip = 0;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				if (dx12Desc.DepthOrArraySize > 1 || requestArrayView)
				{
					dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					dx12UavDescription.Texture2DArray.ArraySize = dx12Desc.DepthOrArraySize;
					dx12UavDescription.Texture2DArray.FirstArraySlice = 0;
					dx12UavDescription.Texture2DArray.MipSlice = 0;
					dx12UavDescription.Texture2DArray.PlaneSlice = 0;

					dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					dx12SrvDescription.Texture2DArray.ArraySize = dx12Desc.DepthOrArraySize;
					dx12SrvDescription.Texture2DArray.FirstArraySlice = 0;
					dx12SrvDescription.Texture2DArray.MipLevels = dx12Desc.MipLevels;
					dx12SrvDescription.Texture2DArray.MostDetailedMip = 0;
					dx12SrvDescription.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					dx12UavDescription.Texture2D.MipSlice = 0;
					dx12UavDescription.Texture2D.PlaneSlice = 0;

					dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					dx12SrvDescription.Texture2D.MipLevels = dx12Desc.MipLevels;
					dx12SrvDescription.Texture2D.MostDetailedMip = 0;
					dx12SrvDescription.Texture2D.PlaneSlice = 0;
				}
				break;
			}

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				dx12SrvDescription.Texture3D.MipLevels = dx12Resource->GetDesc().MipLevels;
				dx12SrvDescription.Texture3D.MostDetailedMip = 0;
				break;

			default:
				break;
			}

			if (dx12Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {

				// UAV
				if (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

					FFX_ASSERT(effectContext.nextStaticUavDescriptor + 1 < effectContext.nextDynamicUavDescriptor);
					backendResource->uavDescCount = 1;
					backendResource->uavDescIndex = effectContext.nextStaticUavDescriptor++;

					dx12UavDescription.Buffer.FirstElement = 0;
					dx12UavDescription.Buffer.StructureByteStride = backendResource->resourceDescription.stride;
					dx12UavDescription.Buffer.NumElements = backendResource->resourceDescription.size / backendResource->resourceDescription.stride;
					dx12UavDescription.Buffer.CounterOffsetInBytes = 0;

					D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapUavGpu->GetCPUDescriptorHandleForHeapStart();
					dx12CpuHandle.ptr += (backendResource->uavDescIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);

					dx12CpuHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
					dx12CpuHandle.ptr += (backendResource->uavDescIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);

					effectContext.nextStaticUavDescriptor++;
				}
				else
				{
					dx12SrvDescription.Buffer.FirstElement = 0;
					dx12SrvDescription.Buffer.StructureByteStride = backendResource->resourceDescription.stride;
					dx12SrvDescription.Buffer.NumElements = backendResource->resourceDescription.size / backendResource->resourceDescription.stride;
					D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
					dx12CpuHandle.ptr += outTexture->internalIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					dx12Device->CreateShaderResourceView(dx12Resource, &dx12SrvDescription, dx12CpuHandle);
				}
			}
			else {
				// CPU readable
				D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
				dx12CpuHandle.ptr += outTexture->internalIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				dx12Device->CreateShaderResourceView(dx12Resource, &dx12SrvDescription, dx12CpuHandle);

				// UAV
				if (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

					const int32_t uavDescriptorCount = (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) ? dx12Desc.MipLevels : 1;
					FFX_ASSERT(effectContext.nextStaticUavDescriptor + uavDescriptorCount < effectContext.nextDynamicUavDescriptor);

					backendResource->uavDescCount = uavDescriptorCount;
					backendResource->uavDescIndex = effectContext.nextStaticUavDescriptor;

					for (int32_t currentMipIndex = 0; currentMipIndex < uavDescriptorCount; ++currentMipIndex) {

						dx12UavDescription.Texture2D.MipSlice = currentMipIndex;

						dx12CpuHandle = backendContext->descHeapUavGpu->GetCPUDescriptorHandleForHeapStart();
						dx12CpuHandle.ptr += (backendResource->uavDescIndex + currentMipIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);

						dx12CpuHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
						dx12CpuHandle.ptr += (backendResource->uavDescIndex + currentMipIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);
					}

					effectContext.nextStaticUavDescriptor += uavDescriptorCount;
				}
			}
		}

		// create upload resource and upload job
		if (createResourceDescription->initData) {

			FfxResourceInternal copySrc;
			FfxCreateResourceDescription uploadDescription = { *createResourceDescription };
			uploadDescription.heapType = FFX_HEAP_TYPE_UPLOAD;
			uploadDescription.resourceDescription.usage = FFX_RESOURCE_USAGE_READ_ONLY;
			uploadDescription.initalState = FFX_RESOURCE_STATE_GENERIC_READ;

			backendInterface->fpCreateResource(backendInterface, &uploadDescription, effectContextId, &copySrc);

			// setup the upload job
			FfxGpuJobDescription copyJob = {

				FFX_GPU_JOB_COPY
			};
			copyJob.copyJobDescriptor.src = copySrc;
			copyJob.copyJobDescriptor.dst = *outTexture;

			backendInterface->fpScheduleGpuJob(backendInterface, &copyJob);
		}
	}

	return FFX_OK;
}

FfxErrorCode DestroyResourceDX12(
	FfxInterface* backendInterface,
	FfxResourceInternal resource,
	FfxUInt32 effectContextId)
{
	FFX_ASSERT(NULL != backendInterface);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;
	BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[effectContextId];
	if ((resource.internalIndex >= int32_t(effectContextId * FFX_MAX_RESOURCE_COUNT)) && (resource.internalIndex < int32_t(effectContext.nextStaticResource))) {
		ID3D12Resource* dx12Resource = getDX12ResourcePtr(backendContext, resource.internalIndex);

		if (dx12Resource) {
			dx12Resource->Release();
			backendContext->pResources[resource.internalIndex].resourcePtr = nullptr;
		}

		return FFX_OK;
	}

	return FFX_ERROR_OUT_OF_RANGE;
}

DXGI_FORMAT patchDxgiFormatWithFfxUsage(DXGI_FORMAT dxResFmt, FfxSurfaceFormat ffxFmt)
{
	DXGI_FORMAT fromFfx = ffxGetDX12FormatFromSurfaceFormat(ffxFmt);
	DXGI_FORMAT fmt = dxResFmt;

	switch (fmt)
	{
		// fixup typeless formats with what is passed in the ffxSurfaceFormat
	case DXGI_FORMAT_UNKNOWN:
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return fromFfx;

		// fixup RGBA8 with SRGB flag passed in the ffxSurfaceFormat
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return fromFfx;

		// fixup depth formats as ffxGetDX12FormatFromSurfaceFormat will result in wrong format
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

	default:
		break;
	}
	return fmt;
}

FfxErrorCode RegisterResourceDX12(
	FfxInterface* backendInterface,
	const FfxResource* inFfxResource,
	FfxUInt32 effectContextId,
	FfxResourceInternal* outFfxResourceInternal
)
{
	FFX_ASSERT(NULL != backendInterface);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)(backendInterface->scratchBuffer);
	ID3D12Device* dx12Device = reinterpret_cast<ID3D12Device*>(backendContext->device);
	ID3D12Resource* dx12Resource = reinterpret_cast<ID3D12Resource*>(inFfxResource->resource);
	BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[effectContextId];

	FfxResourceStates state = inFfxResource->state;

	if (dx12Resource == nullptr) {

		outFfxResourceInternal->internalIndex = 0; // Always maps to FFX_<feature>_RESOURCE_IDENTIFIER_NULL;
		return FFX_OK;
	}

	FFX_ASSERT(effectContext.nextDynamicResource > effectContext.nextStaticResource);
	outFfxResourceInternal->internalIndex = effectContext.nextDynamicResource--;

	BackendContext_DX12::Resource* backendResource = &backendContext->pResources[outFfxResourceInternal->internalIndex];
	backendResource->resourcePtr = dx12Resource;
	backendResource->initialState = state;
	backendResource->currentState = state;

#ifdef _DEBUG
	const wchar_t* name = inFfxResource->name;
	if (name) {
		wcscpy_s(backendResource->resourceName, name);
	}
#endif

	// create resource views
	if (dx12Resource) {

		D3D12_UNORDERED_ACCESS_VIEW_DESC dx12UavDescription = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC dx12SrvDescription = {};
		D3D12_RESOURCE_DESC dx12Desc = dx12Resource->GetDesc();

		// we still want to respect the format provided in the description for SRGB or TYPELESS resources
		DXGI_FORMAT descFormat = patchDxgiFormatWithFfxUsage(dx12Desc.Format, inFfxResource->description.format);

		dx12UavDescription.Format = convertFormatUav(descFormat);
		// Will support something other than this only where there is an actual need for it
		dx12SrvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		dx12SrvDescription.Format = convertFormatSrv(descFormat);

		bool requestArrayView = FFX_CONTAINS_FLAG(inFfxResource->description.usage, FFX_RESOURCE_USAGE_ARRAYVIEW);

		switch (dx12Desc.Dimension) {

		case D3D12_RESOURCE_DIMENSION_BUFFER:
			dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			backendResource->resourceDescription.type = FFX_RESOURCE_TYPE_BUFFER;
			backendResource->resourceDescription.size = inFfxResource->description.size;
			backendResource->resourceDescription.stride = inFfxResource->description.stride;
			backendResource->resourceDescription.alignment = 0;
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			if (dx12Desc.DepthOrArraySize > 1 || requestArrayView)
			{
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
				dx12UavDescription.Texture1DArray.ArraySize = dx12Desc.DepthOrArraySize;
				dx12UavDescription.Texture1DArray.FirstArraySlice = 0;
				dx12UavDescription.Texture1DArray.MipSlice = 0;

				dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				dx12SrvDescription.Texture1DArray.ArraySize = dx12Desc.DepthOrArraySize;
				dx12SrvDescription.Texture1DArray.FirstArraySlice = 0;
				dx12SrvDescription.Texture1DArray.MipLevels = dx12Desc.MipLevels;
				dx12SrvDescription.Texture1DArray.MostDetailedMip = 0;
			}
			else
			{
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				dx12UavDescription.Texture1D.MipSlice = 0;

				dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				dx12SrvDescription.Texture1D.MipLevels = dx12Desc.MipLevels;
				dx12SrvDescription.Texture1D.MostDetailedMip = 0;
			}

			backendResource->resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE1D;
			backendResource->resourceDescription.format = inFfxResource->description.format;
			backendResource->resourceDescription.width = inFfxResource->description.width;
			backendResource->resourceDescription.mipCount = inFfxResource->description.mipCount;
			backendResource->resourceDescription.depth = inFfxResource->description.depth;
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		{
			if (dx12Desc.DepthOrArraySize > 1 || requestArrayView)
			{
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				dx12UavDescription.Texture2DArray.ArraySize = dx12Desc.DepthOrArraySize;
				dx12UavDescription.Texture2DArray.FirstArraySlice = 0;
				dx12UavDescription.Texture2DArray.MipSlice = 0;
				dx12UavDescription.Texture2DArray.PlaneSlice = 0;

				dx12SrvDescription.ViewDimension = inFfxResource->description.type == FFX_RESOURCE_TYPE_TEXTURE_CUBE ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				dx12SrvDescription.Texture2DArray.ArraySize = dx12Desc.DepthOrArraySize;
				dx12SrvDescription.Texture2DArray.FirstArraySlice = 0;
				dx12SrvDescription.Texture2DArray.MipLevels = dx12Desc.MipLevels;
				dx12SrvDescription.Texture2DArray.MostDetailedMip = 0;
				dx12SrvDescription.Texture2DArray.PlaneSlice = 0;
			}
			else
			{
				dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				dx12UavDescription.Texture2D.MipSlice = 0;
				dx12UavDescription.Texture2D.PlaneSlice = 0;

				dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				dx12SrvDescription.Texture2D.MipLevels = dx12Desc.MipLevels;
				dx12SrvDescription.Texture2D.MostDetailedMip = 0;
				dx12SrvDescription.Texture2D.PlaneSlice = 0;
			}

			backendResource->resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE2D;
			backendResource->resourceDescription.format = inFfxResource->description.format;
			backendResource->resourceDescription.width = inFfxResource->description.width;
			backendResource->resourceDescription.height = inFfxResource->description.height;
			backendResource->resourceDescription.mipCount = inFfxResource->description.mipCount;
			backendResource->resourceDescription.depth = inFfxResource->description.depth;
			break;
		}

		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			dx12UavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			dx12SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			dx12SrvDescription.Texture3D.MipLevels = dx12Desc.MipLevels;
			dx12SrvDescription.Texture3D.MostDetailedMip = 0;

			backendResource->resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE3D;
			backendResource->resourceDescription.format = inFfxResource->description.format;
			backendResource->resourceDescription.width = inFfxResource->description.width;
			backendResource->resourceDescription.height = inFfxResource->description.height;
			backendResource->resourceDescription.mipCount = inFfxResource->description.mipCount;
			backendResource->resourceDescription.depth = inFfxResource->description.depth;
			break;

		default:
			break;
		}

		if (dx12Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {

			// UAV
			if (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

				FFX_ASSERT(effectContext.nextDynamicUavDescriptor > effectContext.nextStaticUavDescriptor);
				backendResource->uavDescCount = 1;
				backendResource->uavDescIndex = effectContext.nextDynamicUavDescriptor--;

				dx12UavDescription.Format = DXGI_FORMAT_UNKNOWN;
				dx12UavDescription.Buffer.FirstElement = 0;
				dx12UavDescription.Buffer.StructureByteStride = backendResource->resourceDescription.stride;
				dx12UavDescription.Buffer.NumElements = backendResource->resourceDescription.size / backendResource->resourceDescription.stride;
				dx12UavDescription.Buffer.CounterOffsetInBytes = 0;

				D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapUavGpu->GetCPUDescriptorHandleForHeapStart();
				dx12CpuHandle.ptr += (backendResource->uavDescIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);

				dx12CpuHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
				dx12CpuHandle.ptr += (backendResource->uavDescIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, dx12CpuHandle);
			}
			else
			{
				dx12SrvDescription.Buffer.FirstElement = 0;
				dx12SrvDescription.Buffer.StructureByteStride = backendResource->resourceDescription.stride;
				dx12SrvDescription.Buffer.NumElements = backendResource->resourceDescription.size / backendResource->resourceDescription.stride;
				D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
				dx12CpuHandle.ptr += outFfxResourceInternal->internalIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				dx12Device->CreateShaderResourceView(dx12Resource, &dx12SrvDescription, dx12CpuHandle);
				backendResource->srvDescIndex = outFfxResourceInternal->internalIndex;
			}
		}
		else {

			// CPU readable
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
			cpuHandle.ptr += outFfxResourceInternal->internalIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			dx12Device->CreateShaderResourceView(dx12Resource, &dx12SrvDescription, cpuHandle);
			backendResource->srvDescIndex = outFfxResourceInternal->internalIndex;

			// UAV
			if (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

				const int32_t uavDescriptorsCount = (dx12Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) ? dx12Desc.MipLevels : 1;
				FFX_ASSERT(effectContext.nextDynamicUavDescriptor - uavDescriptorsCount + 1 > effectContext.nextStaticUavDescriptor);

				backendResource->uavDescCount = uavDescriptorsCount;
				backendResource->uavDescIndex = effectContext.nextDynamicUavDescriptor - uavDescriptorsCount + 1;

				for (int32_t currentMipIndex = 0; currentMipIndex < uavDescriptorsCount; ++currentMipIndex) {

					switch (dx12Desc.Dimension)
					{
					case D3D12_RESOURCE_DIMENSION_BUFFER:
						break;

					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
						// TextureXD<Y>.MipSlice values map to same mem
						dx12UavDescription.Texture2D.MipSlice = currentMipIndex;
						break;

					default:
						FFX_ASSERT_MESSAGE(false, "Invalid View Dimension");
						break;
					}

					cpuHandle = backendContext->descHeapUavGpu->GetCPUDescriptorHandleForHeapStart();
					cpuHandle.ptr += (backendResource->uavDescIndex + currentMipIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, cpuHandle);

					cpuHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
					cpuHandle.ptr += (backendResource->uavDescIndex + currentMipIndex) * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					dx12Device->CreateUnorderedAccessView(dx12Resource, 0, &dx12UavDescription, cpuHandle);
				}

				effectContext.nextDynamicUavDescriptor -= uavDescriptorsCount;
			}
		}
	}

	return FFX_OK;
}

FfxResource GetResourceDX12(FfxInterface* backendInterface, FfxResourceInternal inResource)
{
	FFX_ASSERT(nullptr != backendInterface);
	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	FfxResourceDescription ffxResDescription = backendInterface->fpGetResourceDescription(backendInterface, inResource);

	FfxResource resource = {};
	resource.resource = resource.resource = reinterpret_cast<void*>(backendContext->pResources[inResource.internalIndex].resourcePtr);
	resource.state = backendContext->pResources[inResource.internalIndex].currentState;
	resource.description = ffxResDescription;

#ifdef _DEBUG
	if (backendContext->pResources[inResource.internalIndex].resourceName)
	{
		wcscpy_s(resource.name, backendContext->pResources[inResource.internalIndex].resourceName);
	}
#endif

	return resource;
}


// dispose dynamic resources: This should be called at the end of the frame
FfxErrorCode UnregisterResourcesDX12(FfxInterface* backendInterface, FfxCommandList commandList, FfxUInt32 effectContextId)
{
	FFX_ASSERT(NULL != backendInterface);
	BackendContext_DX12* backendContext = (BackendContext_DX12*)(backendInterface->scratchBuffer);
	BackendContext_DX12::EffectContext& effectContext = backendContext->pEffectContexts[effectContextId];

	// Walk back all the resources that don't belong to us and reset them to their initial state
	for (uint32_t resourceIndex = ++effectContext.nextDynamicResource; resourceIndex < (effectContextId * FFX_MAX_RESOURCE_COUNT) + FFX_MAX_RESOURCE_COUNT; ++resourceIndex)
	{
		FfxResourceInternal internalResource;
		internalResource.internalIndex = resourceIndex;

		BackendContext_DX12::Resource* backendResource = &backendContext->pResources[resourceIndex];
		addBarrier(backendContext, &internalResource, backendResource->initialState);
	}

	FFX_ASSERT(nullptr != commandList);
	ID3D12GraphicsCommandList* pCmdList = reinterpret_cast<ID3D12GraphicsCommandList*>(commandList);

	flushBarriers(backendContext, pCmdList);

	effectContext.nextDynamicResource = (effectContextId * FFX_MAX_RESOURCE_COUNT) + FFX_MAX_RESOURCE_COUNT - 1;
	effectContext.nextDynamicUavDescriptor = (effectContextId * FFX_MAX_RESOURCE_COUNT) + FFX_MAX_RESOURCE_COUNT - 1;

	return FFX_OK;
}

FfxResourceDescription GetResourceDescriptorDX12(
	FfxInterface* backendInterface,
	FfxResourceInternal resource)
{
	FFX_ASSERT(NULL != backendInterface);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	FfxResourceDescription resourceDescription = backendContext->pResources[resource.internalIndex].resourceDescription;
	return resourceDescription;
}

D3D12_TEXTURE_ADDRESS_MODE FfxGetAddressModeDX12(const FfxAddressMode& addressMode)
{
	switch (addressMode)
	{
	case FFX_ADDRESS_MODE_WRAP:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case FFX_ADDRESS_MODE_MIRROR:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case FFX_ADDRESS_MODE_CLAMP:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case FFX_ADDRESS_MODE_BORDER:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case FFX_ADDRESS_MODE_MIRROR_ONCE:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	default:
		FFX_ASSERT_MESSAGE(false, "Unsupported addressing mode requested. Please implement");
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	}
}

FfxErrorCode CreatePipelineDX12(
	FfxInterface* backendInterface,
	FfxEffect effect,
	FfxPass pass,
	uint32_t permutationOptions,
	const FfxPipelineDescription* pipelineDescription,
	FfxUInt32                     effectContextId,
	FfxPipelineState* outPipeline)
{
	FFX_ASSERT(NULL != backendInterface);
	FFX_ASSERT(NULL != pipelineDescription);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;
	ID3D12Device* dx12Device = backendContext->device;

	if (pipelineDescription->stage == FfxBindStage(FFX_BIND_VERTEX_SHADER_STAGE | FFX_BIND_PIXEL_SHADER_STAGE))
	{
		// create rasterPipeline
		FfxShaderBlob shaderBlobVS = {};
		backendInterface->fpGetPermutationBlobByIndex(effect, pass, FFX_BIND_VERTEX_SHADER_STAGE, permutationOptions, &shaderBlobVS);
		FFX_ASSERT(shaderBlobVS.data && shaderBlobVS.size);

		FfxShaderBlob shaderBlobPS = {};
		backendInterface->fpGetPermutationBlobByIndex(effect, pass, FFX_BIND_PIXEL_SHADER_STAGE, permutationOptions, &shaderBlobPS);
		FFX_ASSERT(shaderBlobPS.data && shaderBlobPS.size);

		{
			// set up root signature
			// easiest implementation: simply create one root signature per pipeline
			// should add some management later on to avoid unnecessarily re-binding the root signature
			{
				FFX_ASSERT(pipelineDescription->samplerCount <= FFX_MAX_SAMPLERS);
				const size_t              samplerCount = pipelineDescription->samplerCount;
				D3D12_STATIC_SAMPLER_DESC dx12SamplerDescriptions[FFX_MAX_SAMPLERS];
				for (uint32_t currentSamplerIndex = 0; currentSamplerIndex < samplerCount; ++currentSamplerIndex)
				{
					D3D12_STATIC_SAMPLER_DESC dx12SamplerDesc = {};

					dx12SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
					dx12SamplerDesc.MinLOD = 0.f;
					dx12SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
					dx12SamplerDesc.MipLODBias = 0.f;
					dx12SamplerDesc.MaxAnisotropy = 16;
					dx12SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
					dx12SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					dx12SamplerDesc.AddressU = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeU);
					dx12SamplerDesc.AddressV = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeV);
					dx12SamplerDesc.AddressW = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeW);

					switch (pipelineDescription->samplers[currentSamplerIndex].filter)
					{
					case FFX_FILTER_TYPE_MINMAGMIP_POINT:
						dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
						break;
					case FFX_FILTER_TYPE_MINMAGMIP_LINEAR:
						dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						break;
					case FFX_FILTER_TYPE_MINMAGLINEARMIP_POINT:
						dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
						break;

					default:
						FFX_ASSERT_MESSAGE(false, "Unsupported filter type requested. Please implement");
						break;
					}

					dx12SamplerDescriptions[currentSamplerIndex] = dx12SamplerDesc;
					dx12SamplerDescriptions[currentSamplerIndex].ShaderRegister = (UINT)currentSamplerIndex;
				}

				// storage for maximum number of descriptor ranges.
				const int32_t          maximumDescriptorRangeSize = 2;
				D3D12_DESCRIPTOR_RANGE dx12Ranges[maximumDescriptorRangeSize] = {};
				int32_t                currentDescriptorRangeIndex = 0;

				// storage for maximum number of root parameters.
				const int32_t        maximumRootParameters = 10;
				D3D12_ROOT_PARAMETER dx12RootParameters[maximumRootParameters] = {};
				int32_t              currentRootParameterIndex = 0;

				// Allocate a max of binding slots
				int32_t uavCount = shaderBlobVS.uavBufferCount || shaderBlobVS.uavTextureCount || shaderBlobVS.uavBufferCount || shaderBlobVS.uavTextureCount
					? FFX_MAX_RESOURCE_IDENTIFIER_COUNT
					: 0;
				int32_t srvCount = shaderBlobVS.srvBufferCount || shaderBlobVS.srvTextureCount || shaderBlobPS.srvBufferCount || shaderBlobPS.srvTextureCount
					? FFX_MAX_RESOURCE_IDENTIFIER_COUNT
					: 0;
				if (uavCount > 0)
				{
					FFX_ASSERT(currentDescriptorRangeIndex < maximumDescriptorRangeSize);
					D3D12_DESCRIPTOR_RANGE* dx12DescriptorRange = &dx12Ranges[currentDescriptorRangeIndex];
					memset(dx12DescriptorRange, 0, sizeof(D3D12_DESCRIPTOR_RANGE));
					dx12DescriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					dx12DescriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					dx12DescriptorRange->BaseShaderRegister = 0;
					dx12DescriptorRange->NumDescriptors = uavCount;
					currentDescriptorRangeIndex++;

					FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
					D3D12_ROOT_PARAMETER* dx12RootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
					memset(dx12RootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
					dx12RootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					dx12RootParameterSlot->DescriptorTable.NumDescriptorRanges = 1;
					currentRootParameterIndex++;
				}

				if (srvCount > 0)
				{
					FFX_ASSERT(currentDescriptorRangeIndex < maximumDescriptorRangeSize);
					D3D12_DESCRIPTOR_RANGE* dx12DescriptorRange = &dx12Ranges[currentDescriptorRangeIndex];
					memset(dx12DescriptorRange, 0, sizeof(D3D12_DESCRIPTOR_RANGE));
					dx12DescriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					dx12DescriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					dx12DescriptorRange->BaseShaderRegister = 0;
					dx12DescriptorRange->NumDescriptors = srvCount;
					currentDescriptorRangeIndex++;

					FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
					D3D12_ROOT_PARAMETER* dx12RootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
					memset(dx12RootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
					dx12RootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					dx12RootParameterSlot->DescriptorTable.NumDescriptorRanges = 1;
					currentRootParameterIndex++;
				}

				// Setup the descriptor table bindings for the above
				for (int32_t currentRangeIndex = 0; currentRangeIndex < currentDescriptorRangeIndex; currentRangeIndex++)
				{
					dx12RootParameters[currentRangeIndex].DescriptorTable.pDescriptorRanges = &dx12Ranges[currentRangeIndex];
				}

				// Setup root constants as push constants for dx12
				FFX_ASSERT(pipelineDescription->rootConstantBufferCount <= FFX_MAX_NUM_CONST_BUFFERS);
				size_t   rootConstantsSize = pipelineDescription->rootConstantBufferCount;
				uint32_t rootConstants[FFX_MAX_NUM_CONST_BUFFERS];

				for (uint32_t currentRootConstantIndex = 0; currentRootConstantIndex < pipelineDescription->rootConstantBufferCount; ++currentRootConstantIndex)
				{
					rootConstants[currentRootConstantIndex] = pipelineDescription->rootConstants[currentRootConstantIndex].size;
				}

				for (int32_t currentRootConstantIndex = 0; currentRootConstantIndex < (int32_t)pipelineDescription->rootConstantBufferCount;
					currentRootConstantIndex++)
				{
					FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
					D3D12_ROOT_PARAMETER* rootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
					memset(rootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
					rootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					rootParameterSlot->Constants.ShaderRegister = currentRootConstantIndex;
					currentRootParameterIndex++;
				}

				D3D12_ROOT_SIGNATURE_DESC dx12RootSignatureDescription = {};
				dx12RootSignatureDescription.NumParameters = currentRootParameterIndex;
				dx12RootSignatureDescription.pParameters = dx12RootParameters;
				dx12RootSignatureDescription.NumStaticSamplers = (UINT)samplerCount;
				dx12RootSignatureDescription.pStaticSamplers = dx12SamplerDescriptions;

				ID3DBlob* outBlob = nullptr;
				ID3DBlob* errorBlob = nullptr;

				//Query D3D12SerializeRootSignature from d3d12.dll handle
				typedef HRESULT(__stdcall* D3D12SerializeRootSignatureType)(
					const D3D12_ROOT_SIGNATURE_DESC*, D3D_ROOT_SIGNATURE_VERSION, ID3DBlob**, ID3DBlob**);

				//Do not pass hD3D12 handle to the FreeLibrary function, as GetModuleHandle will not increment refcount
				HMODULE d3d12ModuleHandle = GetModuleHandleW(L"D3D12.dll");

				if (NULL != d3d12ModuleHandle)
				{
					D3D12SerializeRootSignatureType dx12SerializeRootSignatureType =
						(D3D12SerializeRootSignatureType)GetProcAddress(d3d12ModuleHandle, "D3D12SerializeRootSignature");

					if (nullptr != dx12SerializeRootSignatureType)
					{
						HRESULT result = dx12SerializeRootSignatureType(&dx12RootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &outBlob, &errorBlob);
						if (FAILED(result))
						{
							return FFX_ERROR_BACKEND_API_ERROR;
						}

						size_t   blobSize = outBlob->GetBufferSize();
						int64_t* blobData = (int64_t*)outBlob->GetBufferPointer();

						result = dx12Device->CreateRootSignature(0,
							outBlob->GetBufferPointer(),
							outBlob->GetBufferSize(),
							IID_PPV_ARGS(reinterpret_cast<ID3D12RootSignature**>(&outPipeline->rootSignature)));
						if (FAILED(result))
						{
							return FFX_ERROR_BACKEND_API_ERROR;
						}
					}
					else
					{
						return FFX_ERROR_BACKEND_API_ERROR;
					}
				}
				else
				{
					return FFX_ERROR_BACKEND_API_ERROR;
				}
			}

			ID3D12RootSignature* dx12RootSignature = reinterpret_cast<ID3D12RootSignature*>(outPipeline->rootSignature);

			// Only set the command signature if this is setup as an indirect workload
			if (pipelineDescription->indirectWorkload)
			{
				D3D12_INDIRECT_ARGUMENT_DESC argumentDescs = { D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH };
				D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
				commandSignatureDesc.pArgumentDescs = &argumentDescs;
				commandSignatureDesc.NumArgumentDescs = 1;
				commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);

				HRESULT result = dx12Device->CreateCommandSignature(
					&commandSignatureDesc, nullptr, IID_PPV_ARGS(reinterpret_cast<ID3D12CommandSignature**>(&outPipeline->cmdSignature)));
				if (FAILED(result))
				{
					return FFX_ERROR_BACKEND_API_ERROR;
				}
			}
			else
			{
				outPipeline->cmdSignature = nullptr;
			}

			FFX_ASSERT(shaderBlobVS.srvTextureCount + shaderBlobVS.srvBufferCount + shaderBlobPS.srvTextureCount + shaderBlobPS.srvBufferCount <=
				FFX_MAX_NUM_SRVS);
			FFX_ASSERT(shaderBlobVS.uavTextureCount + shaderBlobVS.uavBufferCount + shaderBlobPS.uavTextureCount + shaderBlobPS.uavBufferCount <=
				FFX_MAX_NUM_UAVS);
			FFX_ASSERT(shaderBlobVS.cbvCount + shaderBlobPS.cbvCount <= FFX_MAX_NUM_CONST_BUFFERS);

			// populate the pass.
			outPipeline->srvTextureCount = shaderBlobVS.srvTextureCount + shaderBlobPS.srvTextureCount;
			outPipeline->uavTextureCount = shaderBlobVS.uavTextureCount + shaderBlobPS.uavTextureCount;
			outPipeline->srvBufferCount = shaderBlobVS.srvBufferCount + shaderBlobPS.srvBufferCount;
			outPipeline->uavBufferCount = shaderBlobVS.uavBufferCount + shaderBlobPS.uavBufferCount;
			outPipeline->constCount = shaderBlobVS.cbvCount + shaderBlobPS.cbvCount;

			// Todo when needed
			//outPipeline->samplerCount      = shaderBlob.samplerCount;
			//outPipeline->rtAccelStructCount= shaderBlob.rtAccelStructCount;

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			uint32_t                                               srvIndex = 0;
			for (uint32_t vsSrvIndex = 0; vsSrvIndex < shaderBlobVS.srvTextureCount; ++vsSrvIndex)
			{
				outPipeline->srvTextureBindings[srvIndex].slotIndex = shaderBlobVS.boundSRVTextures[vsSrvIndex];
				outPipeline->srvTextureBindings[srvIndex].bindCount = shaderBlobVS.boundSRVTextureCounts[vsSrvIndex];
				wcscpy_s(outPipeline->srvTextureBindings[srvIndex].name, converter.from_bytes(shaderBlobVS.boundSRVTextureNames[vsSrvIndex]).c_str());
				++srvIndex;
			}
			for (uint32_t psSrvIndex = 0; psSrvIndex < shaderBlobPS.srvTextureCount; ++psSrvIndex)
			{
				outPipeline->srvTextureBindings[srvIndex].slotIndex = shaderBlobPS.boundSRVTextures[psSrvIndex];
				outPipeline->srvTextureBindings[srvIndex].bindCount = shaderBlobPS.boundSRVTextureCounts[psSrvIndex];
				wcscpy_s(outPipeline->srvTextureBindings[srvIndex].name, converter.from_bytes(shaderBlobPS.boundSRVTextureNames[psSrvIndex]).c_str());
				++srvIndex;
			}

			uint32_t uavIndex = 0;
			for (uint32_t vsUavIndex = 0; vsUavIndex < shaderBlobVS.uavTextureCount; ++vsUavIndex)
			{
				outPipeline->uavTextureBindings[uavIndex].slotIndex = shaderBlobVS.boundUAVTextures[vsUavIndex];
				outPipeline->uavTextureBindings[uavIndex].bindCount = shaderBlobVS.boundUAVTextureCounts[vsUavIndex];
				wcscpy_s(outPipeline->uavTextureBindings[uavIndex].name, converter.from_bytes(shaderBlobVS.boundUAVTextureNames[vsUavIndex]).c_str());
				++uavIndex;
			}
			for (uint32_t psUavIndex = 0; psUavIndex < shaderBlobPS.uavTextureCount; ++psUavIndex)
			{
				outPipeline->uavTextureBindings[uavIndex].slotIndex = shaderBlobPS.boundUAVTextures[psUavIndex];
				outPipeline->uavTextureBindings[uavIndex].bindCount = shaderBlobPS.boundUAVTextureCounts[psUavIndex];
				wcscpy_s(outPipeline->uavTextureBindings[uavIndex].name, converter.from_bytes(shaderBlobPS.boundUAVTextureNames[psUavIndex]).c_str());
				++uavIndex;
			}

			uint32_t srvBufferIndex = 0;
			for (uint32_t vsSrvIndex = 0; vsSrvIndex < shaderBlobVS.srvBufferCount; ++vsSrvIndex)
			{
				outPipeline->srvBufferBindings[srvBufferIndex].slotIndex = shaderBlobVS.boundSRVBuffers[vsSrvIndex];
				outPipeline->srvBufferBindings[srvBufferIndex].bindCount = shaderBlobVS.boundSRVBufferCounts[vsSrvIndex];
				wcscpy_s(outPipeline->srvBufferBindings[srvBufferIndex].name, converter.from_bytes(shaderBlobVS.boundSRVBufferNames[vsSrvIndex]).c_str());
				++srvBufferIndex;
			}
			for (uint32_t psSrvIndex = 0; psSrvIndex < shaderBlobPS.srvBufferCount; ++psSrvIndex)
			{
				outPipeline->srvBufferBindings[srvBufferIndex].slotIndex = shaderBlobPS.boundSRVBuffers[psSrvIndex];
				outPipeline->srvBufferBindings[srvBufferIndex].bindCount = shaderBlobPS.boundSRVBufferCounts[psSrvIndex];
				wcscpy_s(outPipeline->srvBufferBindings[srvBufferIndex].name, converter.from_bytes(shaderBlobPS.boundSRVBufferNames[psSrvIndex]).c_str());
				++srvBufferIndex;
			}

			uint32_t uavBufferIndex = 0;
			for (uint32_t vsUavIndex = 0; vsUavIndex < shaderBlobVS.uavBufferCount; ++vsUavIndex)
			{
				outPipeline->uavBufferBindings[uavBufferIndex].slotIndex = shaderBlobVS.boundUAVBuffers[vsUavIndex];
				outPipeline->uavBufferBindings[uavBufferIndex].bindCount = shaderBlobVS.boundUAVBufferCounts[vsUavIndex];
				wcscpy_s(outPipeline->uavBufferBindings[uavBufferIndex].name, converter.from_bytes(shaderBlobVS.boundUAVBufferNames[vsUavIndex]).c_str());
				++uavBufferIndex;
			}
			for (uint32_t psUavIndex = 0; psUavIndex < shaderBlobPS.uavBufferCount; ++psUavIndex)
			{
				outPipeline->uavBufferBindings[uavBufferIndex].slotIndex = shaderBlobPS.boundUAVBuffers[psUavIndex];
				outPipeline->uavBufferBindings[uavBufferIndex].bindCount = shaderBlobPS.boundUAVBufferCounts[psUavIndex];
				wcscpy_s(outPipeline->uavBufferBindings[uavBufferIndex].name, converter.from_bytes(shaderBlobPS.boundUAVBufferNames[psUavIndex]).c_str());
				++uavBufferIndex;
			}

			uint32_t cbIndex = 0;
			for (uint32_t vsCbIndex = 0; vsCbIndex < shaderBlobVS.cbvCount; ++vsCbIndex)
			{
				outPipeline->constantBufferBindings[cbIndex].slotIndex = shaderBlobVS.boundConstantBuffers[vsCbIndex];
				outPipeline->constantBufferBindings[cbIndex].bindCount = shaderBlobVS.boundConstantBufferCounts[vsCbIndex];
				wcscpy_s(outPipeline->constantBufferBindings[cbIndex].name, converter.from_bytes(shaderBlobVS.boundConstantBufferNames[vsCbIndex]).c_str());
				++cbIndex;
			}
			for (uint32_t psCbIndex = 0; psCbIndex < shaderBlobPS.cbvCount; ++psCbIndex)
			{
				outPipeline->constantBufferBindings[cbIndex].slotIndex = shaderBlobPS.boundConstantBuffers[psCbIndex];
				outPipeline->constantBufferBindings[cbIndex].bindCount = shaderBlobPS.boundConstantBufferCounts[psCbIndex];
				wcscpy_s(outPipeline->constantBufferBindings[cbIndex].name, converter.from_bytes(shaderBlobPS.boundConstantBufferNames[psCbIndex]).c_str());
				++cbIndex;
			}
		}

		ID3D12RootSignature* dx12RootSignature = reinterpret_cast<ID3D12RootSignature*>(outPipeline->rootSignature);

		FfxSurfaceFormat ffxBackbufferFmt = pipelineDescription->backbufferFormat;

		// create the PSO
		D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12PipelineStateDescription = {};
		dx12PipelineStateDescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		dx12PipelineStateDescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		dx12PipelineStateDescription.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		dx12PipelineStateDescription.DepthStencilState.DepthEnable = FALSE;
		dx12PipelineStateDescription.SampleMask = UINT_MAX;
		dx12PipelineStateDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		dx12PipelineStateDescription.SampleDesc = { 1, 0 };
		dx12PipelineStateDescription.NumRenderTargets = 1;
		dx12PipelineStateDescription.RTVFormats[0] = ffxGetDX12FormatFromSurfaceFormat(ffxBackbufferFmt);

		dx12PipelineStateDescription.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		dx12PipelineStateDescription.pRootSignature = dx12RootSignature;
		dx12PipelineStateDescription.VS.pShaderBytecode = shaderBlobVS.data;
		dx12PipelineStateDescription.VS.BytecodeLength = shaderBlobVS.size;
		dx12PipelineStateDescription.PS.pShaderBytecode = shaderBlobPS.data;
		dx12PipelineStateDescription.PS.BytecodeLength = shaderBlobPS.size;

		if (FAILED(dx12Device->CreateGraphicsPipelineState(&dx12PipelineStateDescription,
			IID_PPV_ARGS(reinterpret_cast<ID3D12PipelineState**>(&outPipeline->pipeline)))))
			return FFX_ERROR_BACKEND_API_ERROR;

	}
	else
	{
		FfxShaderBlob shaderBlob = {};
		backendInterface->fpGetPermutationBlobByIndex(effect, pass, FFX_BIND_COMPUTE_SHADER_STAGE, permutationOptions, &shaderBlob);
		FFX_ASSERT(shaderBlob.data && shaderBlob.size);

		// set up root signature
		// easiest implementation: simply create one root signature per pipeline
		// should add some management later on to avoid unnecessarily re-binding the root signature
		{
			FFX_ASSERT(pipelineDescription->samplerCount <= FFX_MAX_SAMPLERS);
			const size_t              samplerCount = pipelineDescription->samplerCount;
			D3D12_STATIC_SAMPLER_DESC dx12SamplerDescriptions[FFX_MAX_SAMPLERS];
			for (uint32_t currentSamplerIndex = 0; currentSamplerIndex < samplerCount; ++currentSamplerIndex)
			{
				D3D12_STATIC_SAMPLER_DESC dx12SamplerDesc = {};

				dx12SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				dx12SamplerDesc.MinLOD = 0.f;
				dx12SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
				dx12SamplerDesc.MipLODBias = 0.f;
				dx12SamplerDesc.MaxAnisotropy = 16;
				dx12SamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				dx12SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				dx12SamplerDesc.AddressU = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeU);
				dx12SamplerDesc.AddressV = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeV);
				dx12SamplerDesc.AddressW = FfxGetAddressModeDX12(pipelineDescription->samplers[currentSamplerIndex].addressModeW);

				switch (pipelineDescription->samplers[currentSamplerIndex].filter)
				{
				case FFX_FILTER_TYPE_MINMAGMIP_POINT:
					dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
					break;
				case FFX_FILTER_TYPE_MINMAGMIP_LINEAR:
					dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
					break;
				case FFX_FILTER_TYPE_MINMAGLINEARMIP_POINT:
					dx12SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
					break;

				default:
					FFX_ASSERT_MESSAGE(false, "Unsupported filter type requested. Please implement");
					break;
				}

				dx12SamplerDescriptions[currentSamplerIndex] = dx12SamplerDesc;
				dx12SamplerDescriptions[currentSamplerIndex].ShaderRegister = (UINT)currentSamplerIndex;
			}

			// storage for maximum number of descriptor ranges.
			const int32_t          maximumDescriptorRangeSize = 2;
			D3D12_DESCRIPTOR_RANGE dx12Ranges[maximumDescriptorRangeSize] = {};
			int32_t                currentDescriptorRangeIndex = 0;

			// storage for maximum number of root parameters.
			const int32_t        maximumRootParameters = 10;
			D3D12_ROOT_PARAMETER dx12RootParameters[maximumRootParameters] = {};
			int32_t              currentRootParameterIndex = 0;

			// Allocate a max of binding slots
			int32_t uavCount = shaderBlob.uavBufferCount || shaderBlob.uavTextureCount ? FFX_MAX_RESOURCE_IDENTIFIER_COUNT : 0;
			int32_t srvCount = shaderBlob.srvBufferCount || shaderBlob.srvTextureCount ? FFX_MAX_RESOURCE_IDENTIFIER_COUNT : 0;
			if (uavCount > 0)
			{
				FFX_ASSERT(currentDescriptorRangeIndex < maximumDescriptorRangeSize);
				D3D12_DESCRIPTOR_RANGE* dx12DescriptorRange = &dx12Ranges[currentDescriptorRangeIndex];
				memset(dx12DescriptorRange, 0, sizeof(D3D12_DESCRIPTOR_RANGE));
				dx12DescriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				dx12DescriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				dx12DescriptorRange->BaseShaderRegister = 0;
				dx12DescriptorRange->NumDescriptors = uavCount;
				currentDescriptorRangeIndex++;

				FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
				D3D12_ROOT_PARAMETER* dx12RootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
				memset(dx12RootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
				dx12RootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				dx12RootParameterSlot->DescriptorTable.NumDescriptorRanges = 1;
				currentRootParameterIndex++;
			}

			if (srvCount > 0)
			{
				FFX_ASSERT(currentDescriptorRangeIndex < maximumDescriptorRangeSize);
				D3D12_DESCRIPTOR_RANGE* dx12DescriptorRange = &dx12Ranges[currentDescriptorRangeIndex];
				memset(dx12DescriptorRange, 0, sizeof(D3D12_DESCRIPTOR_RANGE));
				dx12DescriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				dx12DescriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				dx12DescriptorRange->BaseShaderRegister = 0;
				dx12DescriptorRange->NumDescriptors = srvCount;
				currentDescriptorRangeIndex++;

				FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
				D3D12_ROOT_PARAMETER* dx12RootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
				memset(dx12RootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
				dx12RootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				dx12RootParameterSlot->DescriptorTable.NumDescriptorRanges = 1;
				currentRootParameterIndex++;
			}

			// Setup the descriptor table bindings for the above
			for (int32_t currentRangeIndex = 0; currentRangeIndex < currentDescriptorRangeIndex; currentRangeIndex++)
			{
				dx12RootParameters[currentRangeIndex].DescriptorTable.pDescriptorRanges = &dx12Ranges[currentRangeIndex];
			}

			// Setup root constants as push constants for dx12
			FFX_ASSERT(pipelineDescription->rootConstantBufferCount <= FFX_MAX_NUM_CONST_BUFFERS);
			size_t   rootConstantsSize = pipelineDescription->rootConstantBufferCount;
			uint32_t rootConstants[FFX_MAX_NUM_CONST_BUFFERS];

			for (uint32_t currentRootConstantIndex = 0; currentRootConstantIndex < pipelineDescription->rootConstantBufferCount; ++currentRootConstantIndex)
			{
				rootConstants[currentRootConstantIndex] = pipelineDescription->rootConstants[currentRootConstantIndex].size;
			}

			for (int32_t currentRootConstantIndex = 0; currentRootConstantIndex < (int32_t)pipelineDescription->rootConstantBufferCount;
				currentRootConstantIndex++)
			{
				FFX_ASSERT(currentRootParameterIndex < maximumRootParameters);
				D3D12_ROOT_PARAMETER* rootParameterSlot = &dx12RootParameters[currentRootParameterIndex];
				memset(rootParameterSlot, 0, sizeof(D3D12_ROOT_PARAMETER));
				rootParameterSlot->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				rootParameterSlot->Constants.ShaderRegister = currentRootConstantIndex;
				currentRootParameterIndex++;
			}

			D3D12_ROOT_SIGNATURE_DESC dx12RootSignatureDescription = {};
			dx12RootSignatureDescription.NumParameters = currentRootParameterIndex;
			dx12RootSignatureDescription.pParameters = dx12RootParameters;
			dx12RootSignatureDescription.NumStaticSamplers = (UINT)samplerCount;
			dx12RootSignatureDescription.pStaticSamplers = dx12SamplerDescriptions;

			ID3DBlob* outBlob = nullptr;
			ID3DBlob* errorBlob = nullptr;

			//Query D3D12SerializeRootSignature from d3d12.dll handle
			typedef HRESULT(__stdcall* D3D12SerializeRootSignatureType)(const D3D12_ROOT_SIGNATURE_DESC*, D3D_ROOT_SIGNATURE_VERSION, ID3DBlob**, ID3DBlob**);

			//Do not pass hD3D12 handle to the FreeLibrary function, as GetModuleHandle will not increment refcount
			HMODULE d3d12ModuleHandle = GetModuleHandleW(L"D3D12.dll");

			if (NULL != d3d12ModuleHandle)
			{
				D3D12SerializeRootSignatureType dx12SerializeRootSignatureType =
					(D3D12SerializeRootSignatureType)GetProcAddress(d3d12ModuleHandle, "D3D12SerializeRootSignature");

				if (nullptr != dx12SerializeRootSignatureType)
				{
					HRESULT result = dx12SerializeRootSignatureType(&dx12RootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &outBlob, &errorBlob);
					if (FAILED(result))
					{
						return FFX_ERROR_BACKEND_API_ERROR;
					}

					size_t   blobSize = outBlob->GetBufferSize();
					int64_t* blobData = (int64_t*)outBlob->GetBufferPointer();

					result = dx12Device->CreateRootSignature(0,
						outBlob->GetBufferPointer(),
						outBlob->GetBufferSize(),
						IID_PPV_ARGS(reinterpret_cast<ID3D12RootSignature**>(&outPipeline->rootSignature)));
					if (FAILED(result))
					{
						return FFX_ERROR_BACKEND_API_ERROR;
					}
				}
				else
				{
					return FFX_ERROR_BACKEND_API_ERROR;
				}
			}
			else
			{
				return FFX_ERROR_BACKEND_API_ERROR;
			}
		}

		ID3D12RootSignature* dx12RootSignature = reinterpret_cast<ID3D12RootSignature*>(outPipeline->rootSignature);

		// Only set the command signature if this is setup as an indirect workload
		if (pipelineDescription->indirectWorkload)
		{
			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs = { D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH };
			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs = &argumentDescs;
			commandSignatureDesc.NumArgumentDescs = 1;
			commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);

			HRESULT result = dx12Device->CreateCommandSignature(
				&commandSignatureDesc, nullptr, IID_PPV_ARGS(reinterpret_cast<ID3D12CommandSignature**>(&outPipeline->cmdSignature)));
			if (FAILED(result))
			{
				return FFX_ERROR_BACKEND_API_ERROR;
			}
		}
		else
		{
			outPipeline->cmdSignature = nullptr;
		}

		// populate the pass.
		outPipeline->srvTextureCount = shaderBlob.srvTextureCount;
		outPipeline->uavTextureCount = shaderBlob.uavTextureCount;
		outPipeline->srvBufferCount = shaderBlob.srvBufferCount;
		outPipeline->uavBufferCount = shaderBlob.uavBufferCount;

		// Todo when needed
		//outPipeline->samplerCount      = shaderBlob.samplerCount;
		//outPipeline->rtAccelStructCount= shaderBlob.rtAccelStructCount;

		outPipeline->constCount = shaderBlob.cbvCount;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		for (uint32_t srvIndex = 0; srvIndex < outPipeline->srvTextureCount; ++srvIndex)
		{
			outPipeline->srvTextureBindings[srvIndex].slotIndex = shaderBlob.boundSRVTextures[srvIndex];
			outPipeline->srvTextureBindings[srvIndex].bindCount = shaderBlob.boundSRVTextureCounts[srvIndex];
			wcscpy_s(outPipeline->srvTextureBindings[srvIndex].name, converter.from_bytes(shaderBlob.boundSRVTextureNames[srvIndex]).c_str());
		}
		for (uint32_t uavIndex = 0; uavIndex < outPipeline->uavTextureCount; ++uavIndex)
		{
			outPipeline->uavTextureBindings[uavIndex].slotIndex = shaderBlob.boundUAVTextures[uavIndex];
			outPipeline->uavTextureBindings[uavIndex].bindCount = shaderBlob.boundUAVTextureCounts[uavIndex];
			wcscpy_s(outPipeline->uavTextureBindings[uavIndex].name, converter.from_bytes(shaderBlob.boundUAVTextureNames[uavIndex]).c_str());
		}
		for (uint32_t srvIndex = 0; srvIndex < outPipeline->srvBufferCount; ++srvIndex)
		{
			outPipeline->srvBufferBindings[srvIndex].slotIndex = shaderBlob.boundSRVBuffers[srvIndex];
			outPipeline->srvBufferBindings[srvIndex].bindCount = shaderBlob.boundSRVBufferCounts[srvIndex];
			wcscpy_s(outPipeline->srvBufferBindings[srvIndex].name, converter.from_bytes(shaderBlob.boundSRVBufferNames[srvIndex]).c_str());
		}
		for (uint32_t uavIndex = 0; uavIndex < outPipeline->uavBufferCount; ++uavIndex)
		{
			outPipeline->uavBufferBindings[uavIndex].slotIndex = shaderBlob.boundUAVBuffers[uavIndex];
			outPipeline->uavBufferBindings[uavIndex].bindCount = shaderBlob.boundUAVBufferCounts[uavIndex];
			wcscpy_s(outPipeline->uavBufferBindings[uavIndex].name, converter.from_bytes(shaderBlob.boundUAVBufferNames[uavIndex]).c_str());
		}
		for (uint32_t cbIndex = 0; cbIndex < outPipeline->constCount; ++cbIndex)
		{
			outPipeline->constantBufferBindings[cbIndex].slotIndex = shaderBlob.boundConstantBuffers[cbIndex];
			outPipeline->constantBufferBindings[cbIndex].bindCount = shaderBlob.boundConstantBufferCounts[cbIndex];
			wcscpy_s(outPipeline->constantBufferBindings[cbIndex].name, converter.from_bytes(shaderBlob.boundConstantBufferNames[cbIndex]).c_str());
		}

		// create the PSO
		D3D12_COMPUTE_PIPELINE_STATE_DESC dx12PipelineStateDescription = {};
		dx12PipelineStateDescription.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		dx12PipelineStateDescription.pRootSignature = dx12RootSignature;
		dx12PipelineStateDescription.CS.pShaderBytecode = shaderBlob.data;
		dx12PipelineStateDescription.CS.BytecodeLength = shaderBlob.size;

		if (FAILED(dx12Device->CreateComputePipelineState(&dx12PipelineStateDescription,
			IID_PPV_ARGS(reinterpret_cast<ID3D12PipelineState**>(&outPipeline->pipeline)))))
			return FFX_ERROR_BACKEND_API_ERROR;

		// Set the pipeline name
		reinterpret_cast<ID3D12PipelineState*>(outPipeline->pipeline)->SetName(pipelineDescription->name);
	}
	return FFX_OK;
}

FfxErrorCode DestroyPipelineDX12(
	FfxInterface* backendInterface,
	FfxPipelineState* pipeline,
	FfxUInt32 effectContextId)
{
	FFX_ASSERT(backendInterface != nullptr);
	if (!pipeline) {
		return FFX_OK;
	}

	// destroy Rootsignature
	ID3D12RootSignature* dx12RootSignature = reinterpret_cast<ID3D12RootSignature*>(pipeline->rootSignature);
	if (dx12RootSignature) {
		dx12RootSignature->Release();
	}
	pipeline->rootSignature = nullptr;

	// destroy CmdSignature
	ID3D12CommandSignature* dx12CmdSignature = reinterpret_cast<ID3D12CommandSignature*>(pipeline->cmdSignature);
	if (dx12CmdSignature) {
		dx12CmdSignature->Release();
	}
	pipeline->cmdSignature = nullptr;

	// destroy pipeline
	ID3D12PipelineState* dx12Pipeline = reinterpret_cast<ID3D12PipelineState*>(pipeline->pipeline);
	if (dx12Pipeline) {
		dx12Pipeline->Release();
	}
	pipeline->pipeline = nullptr;

	return FFX_OK;
}

FfxErrorCode ScheduleGpuJobDX12(
	FfxInterface* backendInterface,
	const FfxGpuJobDescription* job
)
{
	FFX_ASSERT(NULL != backendInterface);
	FFX_ASSERT(NULL != job);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	FFX_ASSERT(backendContext->gpuJobCount < FFX_MAX_GPU_JOBS);

	backendContext->pGpuJobs[backendContext->gpuJobCount] = *job;

	if (job->jobType == FFX_GPU_JOB_COMPUTE) {

		// needs to copy SRVs and UAVs in case they are on the stack only
		FfxComputeJobDescription* computeJob = &backendContext->pGpuJobs[backendContext->gpuJobCount].computeJobDescriptor;
		const uint32_t numConstBuffers = job->computeJobDescriptor.pipeline.constCount;
		for (uint32_t currentRootConstantIndex = 0; currentRootConstantIndex < numConstBuffers; ++currentRootConstantIndex)
		{
			computeJob->cbs[currentRootConstantIndex].num32BitEntries = job->computeJobDescriptor.cbs[currentRootConstantIndex].num32BitEntries;
			memcpy(computeJob->cbs[currentRootConstantIndex].data, job->computeJobDescriptor.cbs[currentRootConstantIndex].data, computeJob->cbs[currentRootConstantIndex].num32BitEntries * sizeof(uint32_t));
		}
	}

	backendContext->gpuJobCount++;

	return FFX_OK;
}

static FfxErrorCode executeGpuJobCompute(BackendContext_DX12* backendContext, FfxGpuJobDescription* job, ID3D12Device* dx12Device, ID3D12GraphicsCommandList* dx12CommandList)
{
	ID3D12DescriptorHeap* dx12DescriptorHeap = reinterpret_cast<ID3D12DescriptorHeap*>(backendContext->descRingBuffer);

	// set root signature
	ID3D12RootSignature* dx12RootSignature = reinterpret_cast<ID3D12RootSignature*>(job->computeJobDescriptor.pipeline.rootSignature);
	dx12CommandList->SetComputeRootSignature(dx12RootSignature);

	// set descriptor heap
	dx12CommandList->SetDescriptorHeaps(1, &dx12DescriptorHeap);

	uint32_t descriptorTableIndex = 0;

	// bind texture & buffer UAVs (note the binding order here MUST match the root signature mapping order from CreatePipeline!)
	{
		// Set a baseline minimal value
		uint32_t maximumUavIndex = job->computeJobDescriptor.pipeline.uavTextureCount + job->computeJobDescriptor.pipeline.uavBufferCount;

		// Textures
		for (uint32_t currentPipelineUavIndex = 0; currentPipelineUavIndex < job->computeJobDescriptor.pipeline.uavTextureCount; ++currentPipelineUavIndex)
		{
			uint32_t uavResourceOffset = job->computeJobDescriptor.pipeline.uavTextureBindings[currentPipelineUavIndex].slotIndex +
				job->computeJobDescriptor.pipeline.uavTextureBindings[currentPipelineUavIndex].bindCount - 1;
			maximumUavIndex = uavResourceOffset > maximumUavIndex ? uavResourceOffset : maximumUavIndex;
		}

		// Buffers
		for (uint32_t currentPipelineUavIndex = 0; currentPipelineUavIndex < job->computeJobDescriptor.pipeline.uavBufferCount; ++currentPipelineUavIndex)
		{
			uint32_t uavResourceOffset = job->computeJobDescriptor.pipeline.uavBufferBindings[currentPipelineUavIndex].slotIndex;
			maximumUavIndex = uavResourceOffset > maximumUavIndex ? uavResourceOffset : maximumUavIndex;
		}

		if (maximumUavIndex)
		{
			// check if this fits into the ringbuffer, loop if not fitting
			if (backendContext->descRingBufferBase + maximumUavIndex + 1 > FFX_RING_BUFFER_SIZE * backendContext->maxEffectContexts)
				backendContext->descRingBufferBase = 0;

			D3D12_GPU_DESCRIPTOR_HANDLE gpuView = dx12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			gpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Set Texture UAVs
			for (uint32_t currentPipelineUavIndex = 0, currentUAVResource = 0; currentPipelineUavIndex < job->computeJobDescriptor.pipeline.uavTextureCount; ++currentPipelineUavIndex) {

				addBarrier(backendContext, &job->computeJobDescriptor.uavTextures[currentPipelineUavIndex], FFX_RESOURCE_STATE_UNORDERED_ACCESS);

				for (uint32_t uavEntry = 0; uavEntry < job->computeJobDescriptor.pipeline.uavTextureBindings[currentPipelineUavIndex].bindCount; ++uavEntry, ++currentUAVResource)
				{
					// source: UAV of resource to bind
					const uint32_t resourceIndex = job->computeJobDescriptor.uavTextures[currentUAVResource].internalIndex;
					const uint32_t uavIndex = backendContext->pResources[resourceIndex].uavDescIndex + job->computeJobDescriptor.uavTextureMips[currentUAVResource];

					// where to bind it
					const uint32_t currentUavResourceIndex = job->computeJobDescriptor.pipeline.uavTextureBindings[currentPipelineUavIndex].slotIndex + uavEntry;

					D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
					srcHandle.ptr += uavIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					D3D12_CPU_DESCRIPTOR_HANDLE cpuView = dx12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					cpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					cpuView.ptr += currentUavResourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					// Copy descriptor
					dx12Device->CopyDescriptorsSimple(1, cpuView, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}

			// Set Buffer UAVs
			for (uint32_t currentPipelineUavIndex = 0; currentPipelineUavIndex < job->computeJobDescriptor.pipeline.uavBufferCount; ++currentPipelineUavIndex) {

				addBarrier(backendContext, &job->computeJobDescriptor.uavBuffers[currentPipelineUavIndex], FFX_RESOURCE_STATE_UNORDERED_ACCESS);

				// source: UAV of buffer to bind
				const uint32_t resourceIndex = job->computeJobDescriptor.uavBuffers[currentPipelineUavIndex].internalIndex;
				const uint32_t uavIndex = backendContext->pResources[resourceIndex].uavDescIndex;

				// where to bind it
				const uint32_t currentUavResourceIndex = job->computeJobDescriptor.pipeline.uavBufferBindings[currentPipelineUavIndex].slotIndex;

				D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
				srcHandle.ptr += uavIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				D3D12_CPU_DESCRIPTOR_HANDLE cpuView = dx12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				cpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				cpuView.ptr += currentUavResourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				// Copy descriptor
				dx12Device->CopyDescriptorsSimple(1, cpuView, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			backendContext->descRingBufferBase += maximumUavIndex + 1;
			dx12CommandList->SetComputeRootDescriptorTable(descriptorTableIndex++, gpuView);
		}
	}

	// bind texture & buffer SRVs
	{
		// Set a baseline minimal value
		// Textures
		uint32_t maximumSrvIndex = job->computeJobDescriptor.pipeline.srvTextureCount;
		for (uint32_t currentPipelineSrvIndex = 0; currentPipelineSrvIndex < job->computeJobDescriptor.pipeline.srvTextureCount; ++currentPipelineSrvIndex) {

			const uint32_t currentSrvResourceIndex = job->computeJobDescriptor.pipeline.srvTextureBindings[currentPipelineSrvIndex].slotIndex +
				job->computeJobDescriptor.pipeline.srvTextureBindings[currentPipelineSrvIndex].bindCount - 1;
			maximumSrvIndex = currentSrvResourceIndex > maximumSrvIndex ? currentSrvResourceIndex : maximumSrvIndex;
		}
		// Buffers
		for (uint32_t currentPipelineSrvIndex = 0; currentPipelineSrvIndex < job->computeJobDescriptor.pipeline.srvBufferCount; ++currentPipelineSrvIndex)
		{
			uint32_t srvResourceOffset = job->computeJobDescriptor.pipeline.srvBufferBindings[currentPipelineSrvIndex].slotIndex;
			maximumSrvIndex = srvResourceOffset > maximumSrvIndex ? srvResourceOffset : maximumSrvIndex;
		}

		if (maximumSrvIndex)
		{
			// check if this fits into the ringbuffer, loop if not fitting
			if (backendContext->descRingBufferBase + maximumSrvIndex + 1 > FFX_RING_BUFFER_SIZE * backendContext->maxEffectContexts) {

				backendContext->descRingBufferBase = 0;
			}

			D3D12_GPU_DESCRIPTOR_HANDLE gpuView = dx12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			gpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (uint32_t currentPipelineSrvIndex = 0; currentPipelineSrvIndex < job->computeJobDescriptor.pipeline.srvTextureCount; ++currentPipelineSrvIndex)
			{
				for (uint32_t bindNum = 0; bindNum < job->computeJobDescriptor.pipeline.srvTextureBindings[currentPipelineSrvIndex].bindCount; ++bindNum)
				{
					uint32_t currPipelineSrvIndex = currentPipelineSrvIndex + bindNum;
					if (job->computeJobDescriptor.srvTextures[currPipelineSrvIndex].internalIndex == 0)
						break;

					addBarrier(backendContext, &job->computeJobDescriptor.srvTextures[currPipelineSrvIndex], FFX_RESOURCE_STATE_COMPUTE_READ);

					// source: SRV of resource to bind
					const uint32_t resourceIndex = job->computeJobDescriptor.srvTextures[currPipelineSrvIndex].internalIndex;
					D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
					srcHandle.ptr += resourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					// Where to bind it
					uint32_t currentSrvResourceIndex;
					if (bindNum >= 1)
					{
						currentSrvResourceIndex = job->computeJobDescriptor.pipeline.srvTextureBindings[currentPipelineSrvIndex].slotIndex + bindNum;
					}
					else
					{
						currentSrvResourceIndex = job->computeJobDescriptor.pipeline.srvTextureBindings[currPipelineSrvIndex].slotIndex;
					}

					D3D12_CPU_DESCRIPTOR_HANDLE cpuView = dx12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					cpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					cpuView.ptr += currentSrvResourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					dx12Device->CopyDescriptorsSimple(1, cpuView, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
			}

			// Set Buffer SRVs
			for (uint32_t currentPipelineSrvIndex = 0; currentPipelineSrvIndex < job->computeJobDescriptor.pipeline.srvBufferCount; ++currentPipelineSrvIndex)
			{
				addBarrier(backendContext, &job->computeJobDescriptor.srvBuffers[currentPipelineSrvIndex], FFX_RESOURCE_STATE_COMPUTE_READ);

				// source: SRV of buffer to bind
				const uint32_t resourceIndex = job->computeJobDescriptor.srvBuffers[currentPipelineSrvIndex].internalIndex;

				// where to bind it
				const uint32_t currentSrvResourceIndex = job->computeJobDescriptor.pipeline.srvBufferBindings[currentPipelineSrvIndex].slotIndex;

				D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = backendContext->descHeapSrvCpu->GetCPUDescriptorHandleForHeapStart();
				srcHandle.ptr += resourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				D3D12_CPU_DESCRIPTOR_HANDLE cpuView = dx12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				cpuView.ptr += backendContext->descRingBufferBase * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				cpuView.ptr += currentSrvResourceIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				// Copy descriptor
				dx12Device->CopyDescriptorsSimple(1, cpuView, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			backendContext->descRingBufferBase += maximumSrvIndex + 1;
			dx12CommandList->SetComputeRootDescriptorTable(descriptorTableIndex++, gpuView);
		}
	}

	// If we are dispatching indirectly, transition the argument resource to indirect argument
	if (job->computeJobDescriptor.pipeline.cmdSignature)
	{
		addBarrier(backendContext, &job->computeJobDescriptor.cmdArgument, FFX_RESOURCE_STATE_INDIRECT_ARGUMENT);
	}

	flushBarriers(backendContext, dx12CommandList);

	// bind pipeline
	ID3D12PipelineState* dx12PipelineStateObject = reinterpret_cast<ID3D12PipelineState*>(job->computeJobDescriptor.pipeline.pipeline);
	dx12CommandList->SetPipelineState(dx12PipelineStateObject);

	// copy data to constant buffer and bind
	{
		std::lock_guard<std::mutex> cbLock{ backendContext->constantBufferMutex };
		for (uint32_t currentRootConstantIndex = 0; currentRootConstantIndex < job->computeJobDescriptor.pipeline.constCount; ++currentRootConstantIndex) {

			uint32_t size = FFX_ALIGN_UP(job->computeJobDescriptor.cbs[currentRootConstantIndex].num32BitEntries * sizeof(uint32_t), 256);

			if (backendContext->constantBufferOffset + size >= backendContext->constantBufferSize)
				backendContext->constantBufferOffset = 0;

			void* pBuffer = (void*)((uint8_t*)(backendContext->constantBufferMem) + backendContext->constantBufferOffset);
			memcpy(pBuffer, job->computeJobDescriptor.cbs[currentRootConstantIndex].data, job->computeJobDescriptor.cbs[currentRootConstantIndex].num32BitEntries * sizeof(uint32_t));

			uint32_t slotIndex = job->computeJobDescriptor.pipeline.constantBufferBindings[currentRootConstantIndex].slotIndex;
			D3D12_GPU_VIRTUAL_ADDRESS bufferViewDesc = backendContext->constantBufferResource->GetGPUVirtualAddress() + backendContext->constantBufferOffset;
			dx12CommandList->SetComputeRootConstantBufferView(descriptorTableIndex + slotIndex, bufferViewDesc);

			// update the offset
			backendContext->constantBufferOffset += size;
		}
	}

	// Dispatch (or dispatch indirect)
	if (job->computeJobDescriptor.pipeline.cmdSignature)
	{
		const uint32_t resourceIndex = job->computeJobDescriptor.cmdArgument.internalIndex;
		ID3D12Resource* pResource = backendContext->pResources[resourceIndex].resourcePtr;

		dx12CommandList->ExecuteIndirect(reinterpret_cast<ID3D12CommandSignature*>(job->computeJobDescriptor.pipeline.cmdSignature), 1, pResource, job->computeJobDescriptor.cmdArgumentOffset, nullptr, 0);
	}
	else
	{
		dx12CommandList->Dispatch(job->computeJobDescriptor.dimensions[0], job->computeJobDescriptor.dimensions[1], job->computeJobDescriptor.dimensions[2]);
	}
	return FFX_OK;
}

static FfxErrorCode executeGpuJobCopy(BackendContext_DX12* backendContext, FfxGpuJobDescription* job, ID3D12Device* dx12Device, ID3D12GraphicsCommandList* dx12CommandList)
{
	ID3D12Resource* dx12ResourceSrc = getDX12ResourcePtr(backendContext, job->copyJobDescriptor.src.internalIndex);
	ID3D12Resource* dx12ResourceDst = getDX12ResourcePtr(backendContext, job->copyJobDescriptor.dst.internalIndex);
	D3D12_RESOURCE_DESC dx12ResourceDescriptionDst = dx12ResourceDst->GetDesc();
	D3D12_RESOURCE_DESC dx12ResourceDescriptionSrc = dx12ResourceSrc->GetDesc();

	addBarrier(backendContext, &job->copyJobDescriptor.src, FFX_RESOURCE_STATE_COPY_SRC);
	addBarrier(backendContext, &job->copyJobDescriptor.dst, FFX_RESOURCE_STATE_COPY_DEST);
	flushBarriers(backendContext, dx12CommandList);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT dx12Footprint = {};
	UINT rowCount;
	UINT64 rowSizeInBytes;
	UINT64 totalBytes;
	dx12Device->GetCopyableFootprints(&dx12ResourceDescriptionDst, 0, 1, 0, &dx12Footprint, &rowCount, &rowSizeInBytes, &totalBytes);

	if (dx12ResourceDescriptionDst.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		dx12CommandList->CopyBufferRegion(dx12ResourceDst, 0, dx12ResourceSrc, 0, totalBytes);
	}
	else if (dx12ResourceDescriptionSrc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		D3D12_TEXTURE_COPY_LOCATION dx12SourceLocation = {};
		dx12SourceLocation.pResource = dx12ResourceSrc;
		dx12SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dx12SourceLocation.PlacedFootprint = dx12Footprint;

		D3D12_TEXTURE_COPY_LOCATION dx12DestinationLocation = {};
		dx12DestinationLocation.pResource = dx12ResourceDst;
		dx12DestinationLocation.SubresourceIndex = 0;
		dx12DestinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		dx12CommandList->CopyTextureRegion(&dx12DestinationLocation, 0, 0, 0, &dx12SourceLocation, nullptr);
	}
	else
	{
		D3D12_TEXTURE_COPY_LOCATION dx12SourceLocation = {};
		dx12SourceLocation.pResource = dx12ResourceSrc;
		dx12SourceLocation.SubresourceIndex = 0;
		dx12SourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION dx12DestinationLocation = {};
		dx12DestinationLocation.pResource = dx12ResourceDst;
		dx12DestinationLocation.SubresourceIndex = 0;
		dx12DestinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		dx12CommandList->CopyTextureRegion(&dx12DestinationLocation, 0, 0, 0, &dx12SourceLocation, nullptr);
	}

	return FFX_OK;
}

static FfxErrorCode executeGpuJobBarrier(BackendContext_DX12* backendContext, FfxGpuJobDescription* job, ID3D12Device* dx12Device, ID3D12GraphicsCommandList* dx12CommandList)
{
	ID3D12Resource* dx12ResourceSrc = getDX12ResourcePtr(backendContext, job->barrierDescriptor.resource.internalIndex);
	D3D12_RESOURCE_DESC dx12ResourceDescriptionSrc = dx12ResourceSrc->GetDesc();

	addBarrier(backendContext, &job->barrierDescriptor.resource, job->barrierDescriptor.newState);
	flushBarriers(backendContext, dx12CommandList);

	return FFX_OK;
}

static FfxErrorCode executeGpuJobTimestamp(BackendContext_DX12* backendContext, FfxGpuJobDescription* job, ID3D12Device* dx12Device, ID3D12GraphicsCommandList* dx12CommandList)
{
	return FFX_OK;
}

static FfxErrorCode executeGpuJobClearFloat(BackendContext_DX12* backendContext, FfxGpuJobDescription* job, ID3D12Device* dx12Device, ID3D12GraphicsCommandList* dx12CommandList)
{
	uint32_t idx = job->clearJobDescriptor.target.internalIndex;
	BackendContext_DX12::Resource ffxResource = backendContext->pResources[idx];
	ID3D12Resource* dx12Resource = reinterpret_cast<ID3D12Resource*>(ffxResource.resourcePtr);
	uint32_t srvIndex = ffxResource.srvDescIndex;
	uint32_t uavIndex = ffxResource.uavDescIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE dx12CpuHandle = backendContext->descHeapUavCpu->GetCPUDescriptorHandleForHeapStart();
	dx12CpuHandle.ptr += uavIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE dx12GpuHandle = backendContext->descHeapUavGpu->GetGPUDescriptorHandleForHeapStart();
	dx12GpuHandle.ptr += uavIndex * dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	dx12CommandList->SetDescriptorHeaps(1, &backendContext->descHeapUavGpu);

	addBarrier(backendContext, &job->clearJobDescriptor.target, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	flushBarriers(backendContext, dx12CommandList);
	uint32_t clearColorAsUint[4];
	clearColorAsUint[0] = reinterpret_cast<uint32_t&> (job->clearJobDescriptor.color[0]);
	clearColorAsUint[1] = reinterpret_cast<uint32_t&> (job->clearJobDescriptor.color[1]);
	clearColorAsUint[2] = reinterpret_cast<uint32_t&> (job->clearJobDescriptor.color[2]);
	clearColorAsUint[3] = reinterpret_cast<uint32_t&> (job->clearJobDescriptor.color[3]);
	dx12CommandList->ClearUnorderedAccessViewUint(dx12GpuHandle, dx12CpuHandle, dx12Resource, clearColorAsUint, 0, nullptr);

	return FFX_OK;
}

FfxErrorCode ExecuteGpuJobsDX12(
	FfxInterface* backendInterface,
	FfxCommandList commandList)
{
	FFX_ASSERT(NULL != backendInterface);

	BackendContext_DX12* backendContext = (BackendContext_DX12*)backendInterface->scratchBuffer;

	FfxErrorCode errorCode = FFX_OK;

	// execute all GpuJobs
	for (uint32_t currentGpuJobIndex = 0; currentGpuJobIndex < backendContext->gpuJobCount; ++currentGpuJobIndex) {

		FfxGpuJobDescription* GpuJob = &backendContext->pGpuJobs[currentGpuJobIndex];
		ID3D12GraphicsCommandList* dx12CommandList = reinterpret_cast<ID3D12GraphicsCommandList*>(commandList);
		ID3D12Device* dx12Device = reinterpret_cast<ID3D12Device*>(backendContext->device);

		switch (GpuJob->jobType) {

		case FFX_GPU_JOB_CLEAR_FLOAT:
			errorCode = executeGpuJobClearFloat(backendContext, GpuJob, dx12Device, dx12CommandList);
			break;

		case FFX_GPU_JOB_COPY:
			errorCode = executeGpuJobCopy(backendContext, GpuJob, dx12Device, dx12CommandList);
			break;

		case FFX_GPU_JOB_COMPUTE:
			errorCode = executeGpuJobCompute(backendContext, GpuJob, dx12Device, dx12CommandList);
			break;

		case FFX_GPU_JOB_BARRIER:
			errorCode = executeGpuJobBarrier(backendContext, GpuJob, dx12Device, dx12CommandList);
			break;

		default:
			break;
		}
	}

	// check the execute function returned cleanly.
	FFX_RETURN_ON_ERROR(
		errorCode == FFX_OK,
		FFX_ERROR_BACKEND_API_ERROR);

	backendContext->gpuJobCount = 0;

	return FFX_OK;
}

FfxCommandQueue ffxGetCommandQueueDX12(ID3D12CommandQueue* pCommandQueue)
{
	FFX_ASSERT(nullptr != pCommandQueue);
	return reinterpret_cast<FfxCommandQueue>(pCommandQueue);
}

FfxSwapchain ffxGetSwapchainDX12(IDXGISwapChain4* pSwapchain)
{
	FFX_ASSERT(nullptr != pSwapchain);
	return reinterpret_cast<FfxSwapchain>(pSwapchain);
}

IDXGISwapChain4* ffxGetDX12SwapchainPtr(FfxSwapchain ffxSwapchain)
{
	return reinterpret_cast<IDXGISwapChain4*>(ffxSwapchain);
}

//#include <FidelityFX/host/ffx_fsr2.h>
//#include <FidelityFX/host/ffx_fsr3.h>
//#include "FrameInterpolationSwapchain/FrameInterpolationSwapchainDX12.h"


// fix up format in case resource passed to FSR2 was created as typeless
static DXGI_FORMAT convertFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// Handle Depth
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

		// Handle color: assume FLOAT for 16 and 32 bit channels, else UNORM
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R32G32_TYPELESS:
		return DXGI_FORMAT_R32G32_FLOAT;
	case DXGI_FORMAT_R16G16_TYPELESS:
		return DXGI_FORMAT_R16G16_FLOAT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R8G8_TYPELESS:
		return DXGI_FORMAT_R8G8_UNORM;
	case DXGI_FORMAT_R16_TYPELESS:
		return DXGI_FORMAT_R16_FLOAT;
	case DXGI_FORMAT_R8_TYPELESS:
		return DXGI_FORMAT_R8_UNORM;
	default:
		return format;
	}
}

static DXGI_FORMAT sRgbFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	default:
		return format;
	}
}
