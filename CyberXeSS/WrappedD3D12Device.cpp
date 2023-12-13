#include "pch.h"
#include "WrappedD3D12Device.h"

bool RefCountD3D12Object::HandleWrap(const char* ifaceName, REFIID riid, void** ppvObject)
{
	LOG("RefCountD3D12Object.HandleWrap");

	if (ppvObject == NULL || *ppvObject == NULL)
	{
		std::string str(ifaceName);
		LOG("RefCountD3D12Object.HandleWrap called with NULL ppvObject querying " + str);
		return false;
	}

	// unknown GUID that we only want to print once to avoid log spam
	  // {79D2046C-22EF-451B-9E74-2245D9C760EA}
	static const GUID Unknown_uuid = {
		0x79d2046c, 0x22ef, 0x451b, {0x9e, 0x74, 0x22, 0x45, 0xd9, 0xc7, 0x60, 0xea} };

	// unknown/undocumented internal interface
	// {7abb6563-02bc-47c4-8ef9-acc4795edbcf}
	static const GUID ID3D12DeviceInternal2_uuid = {
		0x7abb6563, 0x02bc, 0x47c4, {0x8e, 0xf9, 0xac, 0xc4, 0x79, 0x5e, 0xdb, 0xcf} };

	if (riid == __uuidof(ID3D12Device))
	{
		ID3D12Device* real = (ID3D12Device*)(*ppvObject);
		*ppvObject = (ID3D12Device*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device1))
	{
		ID3D12Device1* real = (ID3D12Device1*)(*ppvObject);
		*ppvObject = (ID3D12Device1*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device2))
	{
		ID3D12Device2* real = (ID3D12Device2*)(*ppvObject);
		*ppvObject = (ID3D12Device2*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device3))
	{
		ID3D12Device3* real = (ID3D12Device3*)(*ppvObject);
		*ppvObject = (ID3D12Device3*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device4))
	{
		ID3D12Device4* real = (ID3D12Device4*)(*ppvObject);
		*ppvObject = (ID3D12Device4*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device5))
	{
		ID3D12Device5* real = (ID3D12Device5*)(*ppvObject);
		*ppvObject = (ID3D12Device5*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device6))
	{
		ID3D12Device6* real = (ID3D12Device6*)(*ppvObject);
		*ppvObject = (ID3D12Device6*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device7))
	{
		ID3D12Device7* real = (ID3D12Device7*)(*ppvObject);
		*ppvObject = (ID3D12Device7*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device8))
	{
		ID3D12Device8* real = (ID3D12Device8*)(*ppvObject);
		*ppvObject = (ID3D12Device8*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device9))
	{
		ID3D12Device9* real = (ID3D12Device9*)(*ppvObject);
		*ppvObject = (ID3D12Device9*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == __uuidof(ID3D12Device10))
	{
		ID3D12Device10* real = (ID3D12Device10*)(*ppvObject);
		*ppvObject = (ID3D12Device10*)(new WrappedD3D12Device(real));
		return true;
	}
	else if (riid == Unknown_uuid)
	{
		LOG("RefCountDXGIObject.HandleWrap Querying Unknown_uuid, returning false");
	}
	else if (riid == ID3D12DeviceInternal2_uuid)
	{
		LOG("RefCountDXGIObject.HandleWrap Querying ID3D12DeviceInternal2_uuid, returning false");
	}
	else
	{
		std::string str(ifaceName);
		LOG("RefCountDXGIObject.HandleWrap Querying " + str + "  for unrecognized GUID: " + ToString(riid));
	}

	return false;
}

HRESULT RefCountD3D12Object::WrapQueryInterface(IUnknown* real, const char* ifaceName, REFIID riid, void** ppvObject)
{
	LOG("RefCountD3D12Object.WrapQueryInterface");

	HRESULT ret = real->QueryInterface(riid, ppvObject);

	if (ret == S_OK && HandleWrap(ifaceName, riid, ppvObject))
		return ret;

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

WrappedD3D12Device::WrappedD3D12Device(ID3D12Device* device) : RefCountD3D12Object(device), m_device(device)
{
	m_device1 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device1), (void**)&m_device1);
	m_device2 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device2), (void**)&m_device2);
	m_device3 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device3), (void**)&m_device3);
	m_device4 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device4), (void**)&m_device4);
	m_device5 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device5), (void**)&m_device5);
	m_device6 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device6), (void**)&m_device6);
	m_device7 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device7), (void**)&m_device7);
	m_device8 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device8), (void**)&m_device8);
	m_device9 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device9), (void**)&m_device9);
	m_device10 = NULL;
	device->QueryInterface(__uuidof(ID3D12Device10), (void**)&m_device10);
}

WrappedD3D12Device::~WrappedD3D12Device()
{
	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_device1);
	SAFE_RELEASE(m_device2);
	SAFE_RELEASE(m_device3);
	SAFE_RELEASE(m_device4);
	SAFE_RELEASE(m_device5);
	SAFE_RELEASE(m_device6);
	SAFE_RELEASE(m_device7);
	SAFE_RELEASE(m_device8);
	SAFE_RELEASE(m_device9);
	SAFE_RELEASE(m_device10);
}


HRESULT __stdcall WrappedD3D12Device::QueryInterface(REFIID riid, void** ppvObject)
{
	LOG("D3D12Device.QueryInterface: " + ToString(riid));

	if (ppvObject == nullptr)
		return E_POINTER;

	if (riid == __uuidof(ID3D12Device))
	{
		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	if (riid == __uuidof(ID3D12ProxyDevice))
	{
		LOG("D3D12Device.QueryInterface: Looking for ID3D12ProxyDevice, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device1))
	{
		if (m_device1 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device1 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device1, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device2))
	{
		if (m_device2 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device2 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device2, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device3))
	{
		if (m_device == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device3 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device3, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device4))
	{
		if (m_device4 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device4 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device4, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device5))
	{
		if (m_device5 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device5 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device5, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device6))
	{
		if (m_device6 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device6 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device6, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device7))
	{
		if (m_device7 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device7 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device7, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device8))
	{
		if (m_device8 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device8 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device8, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device9))
	{
		if (m_device9 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device9 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device9, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12Device10))
	{
		if (m_device10 == nullptr)
		{
			LOG("D3D12Device.QueryInterface: m_device10 is not available, returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}

		LOG("D3D12Device.QueryInterface: Looking for ID3D12Device10, returning this");
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else if (riid == __uuidof(ID3D12DeviceRemovedExtendedData) ||
		riid == __uuidof(ID3D12DeviceRemovedExtendedData1) ||
		riid == __uuidof(ID3D12DeviceRemovedExtendedData2))
	{
		LOG("D3D12Device.QueryInterface: Looking for ID3D12DeviceRemovedExtendedData");
		auto ret = m_device->QueryInterface(riid, ppvObject);
		LOG("D3D12Device.QueryInterface: Looking for ID3D12DeviceRemovedExtendedData result: " + int_to_hex(ret));
		return ret;
	}

	auto hr = m_device->QueryInterface(riid, ppvObject);

	if (hr == S_OK && *ppvObject != nullptr)
	{
		auto wrapResult = RefCountD3D12Object::HandleWrap("ID3DDevice", riid, ppvObject);

		if (!wrapResult)
		{
			LOG("D3D12Device.QueryInterface: returning E_NOINTERFACE");
			return E_NOINTERFACE;
		}
	}

	LOG("D3D12Device.QueryInterface Unknown interface result: " + int_to_hex(hr));
	return hr;
}

HRESULT STDMETHODCALLTYPE RefCountD3D12Object::SetName(_In_z_  LPCWSTR Name)
{
	return m_pReal->SetName(Name);
}

UINT __stdcall WrappedD3D12Device::GetNodeCount(void)
{
	return m_device->GetNodeCount();
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue)
{
	return m_device->CreateCommandQueue(pDesc, riid, ppCommandQueue);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator)
{
	return m_device->CreateCommandAllocator(type, riid, ppCommandAllocator);
}

HRESULT __stdcall WrappedD3D12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState)
{
	return m_device->CreateGraphicsPipelineState(pDesc, riid, ppPipelineState);
}

HRESULT __stdcall WrappedD3D12Device::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState)
{
	return m_device->CreateComputePipelineState(pDesc, riid, ppPipelineState);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList)
{
	return m_device->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);
}

HRESULT __stdcall WrappedD3D12Device::CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize)
{
	return m_device->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
}

HRESULT __stdcall WrappedD3D12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap)
{
	return m_device->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
}

UINT __stdcall WrappedD3D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType)
{
	return m_device->GetDescriptorHandleIncrementSize(DescriptorHeapType);
}

HRESULT __stdcall WrappedD3D12Device::CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature)
{
	return m_device->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
}

void __stdcall WrappedD3D12Device::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateConstantBufferView(pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateDepthStencilView(pResource, pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device->CreateSampler(pDesc, DestDescriptor);
}

void __stdcall WrappedD3D12Device::CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, const UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, const UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	return m_device->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
}

void __stdcall WrappedD3D12Device::CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	return m_device->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
}

D3D12_RESOURCE_ALLOCATION_INFO __stdcall WrappedD3D12Device::GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs)
{
	return m_device->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
}

D3D12_HEAP_PROPERTIES __stdcall WrappedD3D12Device::GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType)
{
	return m_device->GetCustomHeapProperties(nodeMask, heapType);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource)
{
	return m_device->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap)
{
	return m_device->CreateHeap(pDesc, riid, ppvHeap);
}

HRESULT __stdcall WrappedD3D12Device::CreatePlacedResource(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource)
{
	return m_device->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource)
{
	return m_device->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreateSharedHandle(ID3D12DeviceChild* pObject, const SECURITY_ATTRIBUTES* pAttributes, DWORD Access, LPCWSTR Name, HANDLE* pHandle)
{
	return m_device->CreateSharedHandle(pObject, pAttributes, Access, Name, pHandle);
}

HRESULT __stdcall WrappedD3D12Device::OpenSharedHandle(HANDLE NTHandle, REFIID riid, void** ppvObj)
{
	return m_device->OpenSharedHandle(NTHandle, riid, ppvObj);
}

HRESULT __stdcall WrappedD3D12Device::OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE* pNTHandle)
{
	return m_device->OpenSharedHandleByName(Name, Access, pNTHandle);
}

HRESULT __stdcall WrappedD3D12Device::MakeResident(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
	return m_device->MakeResident(NumObjects, ppObjects);
}

HRESULT __stdcall WrappedD3D12Device::Evict(UINT NumObjects, ID3D12Pageable* const* ppObjects)
{
	return m_device->Evict(NumObjects, ppObjects);
}

HRESULT __stdcall WrappedD3D12Device::CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence)
{
	return m_device->CreateFence(InitialValue, Flags, riid, ppFence);
}

HRESULT __stdcall WrappedD3D12Device::GetDeviceRemovedReason(void)
{
	return m_device->GetDeviceRemovedReason();
}

void __stdcall WrappedD3D12Device::GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes)
{
	return m_device->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
}

HRESULT __stdcall WrappedD3D12Device::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap)
{
	return m_device->CreateQueryHeap(pDesc, riid, ppvHeap);
}

HRESULT __stdcall WrappedD3D12Device::SetStablePowerState(BOOL Enable)
{
	return m_device->SetStablePowerState(Enable);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature)
{
	return m_device->CreateCommandSignature(pDesc, pRootSignature, riid, ppvCommandSignature);
}

void __stdcall WrappedD3D12Device::GetResourceTiling(ID3D12Resource* pTiledResource, UINT* pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* pPackedMipDesc, D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips, UINT* pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips)
{
	return m_device->GetResourceTiling(pTiledResource, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
}

LUID __stdcall WrappedD3D12Device::GetAdapterLuid(void)
{
	return LUID{ 0, 56090 };
}

HRESULT __stdcall WrappedD3D12Device::CreatePipelineLibrary(const void* pLibraryBlob, SIZE_T BlobLength, REFIID riid, void** ppPipelineLibrary)
{
	return m_device1->CreatePipelineLibrary(pLibraryBlob, BlobLength, riid, ppPipelineLibrary);
}

HRESULT __stdcall WrappedD3D12Device::SetEventOnMultipleFenceCompletion(ID3D12Fence* const* ppFences, const UINT64* pFenceValues, UINT NumFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags, HANDLE hEvent)
{
	return m_device1->SetEventOnMultipleFenceCompletion(ppFences, pFenceValues, NumFences, Flags, hEvent);
}

HRESULT __stdcall WrappedD3D12Device::SetResidencyPriority(UINT NumObjects, ID3D12Pageable* const* ppObjects, const D3D12_RESIDENCY_PRIORITY* pPriorities)
{
	return m_device1->SetResidencyPriority(NumObjects, ppObjects, pPriorities);
}

HRESULT __stdcall WrappedD3D12Device::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc, REFIID riid, void** ppPipelineState)
{
	return m_device2->CreatePipelineState(pDesc, riid, ppPipelineState);
}

HRESULT __stdcall WrappedD3D12Device::OpenExistingHeapFromAddress(const void* pAddress, REFIID riid, void** ppvHeap)
{
	return m_device3->OpenExistingHeapFromAddress(pAddress, riid, ppvHeap);
}

HRESULT __stdcall WrappedD3D12Device::OpenExistingHeapFromFileMapping(HANDLE hFileMapping, REFIID riid, void** ppvHeap)
{
	return m_device3->OpenExistingHeapFromFileMapping(hFileMapping, riid, ppvHeap);
}

HRESULT __stdcall WrappedD3D12Device::EnqueueMakeResident(D3D12_RESIDENCY_FLAGS Flags, UINT NumObjects, ID3D12Pageable* const* ppObjects, ID3D12Fence* pFenceToSignal, UINT64 FenceValueToSignal)
{
	return m_device3->EnqueueMakeResident(Flags, NumObjects, ppObjects, pFenceToSignal, FenceValueToSignal);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandList1(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void** ppCommandList)
{
	return m_device4->CreateCommandList1(nodeMask, type, flags, riid, ppCommandList);
}

HRESULT __stdcall WrappedD3D12Device::CreateProtectedResourceSession(const D3D12_PROTECTED_RESOURCE_SESSION_DESC* pDesc, REFIID riid, void** ppSession)
{
	return m_device4->CreateProtectedResourceSession(pDesc, riid, ppSession);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommittedResource1(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riidResource, void** ppvResource)
{
	return m_device4->CreateCommittedResource1(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riidResource, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreateHeap1(const D3D12_HEAP_DESC* pDesc, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riid, void** ppvHeap)
{
	return m_device4->CreateHeap1(pDesc, pProtectedSession, riid, ppvHeap);
}

HRESULT __stdcall WrappedD3D12Device::CreateReservedResource1(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riid, void** ppvResource)
{
	return m_device4->CreateReservedResource1(pDesc, InitialState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);
}

D3D12_RESOURCE_ALLOCATION_INFO __stdcall WrappedD3D12Device::GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1)
{
	return m_device4->GetResourceAllocationInfo1(visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);
}

HRESULT __stdcall WrappedD3D12Device::CreateLifetimeTracker(ID3D12LifetimeOwner* pOwner, REFIID riid, void** ppvTracker)
{
	return m_device5->CreateLifetimeTracker(pOwner, riid, ppvTracker);
}

void __stdcall WrappedD3D12Device::RemoveDevice(void)
{
	return m_device5->RemoveDevice();
}

HRESULT __stdcall WrappedD3D12Device::EnumerateMetaCommands(UINT* pNumMetaCommands, D3D12_META_COMMAND_DESC* pDescs)
{
	return m_device5->EnumerateMetaCommands(pNumMetaCommands, pDescs);
}

HRESULT __stdcall WrappedD3D12Device::EnumerateMetaCommandParameters(REFGUID CommandId, D3D12_META_COMMAND_PARAMETER_STAGE Stage, UINT* pTotalStructureSizeInBytes, UINT* pParameterCount, D3D12_META_COMMAND_PARAMETER_DESC* pParameterDescs)
{
	return m_device5->EnumerateMetaCommandParameters(CommandId, Stage, pTotalStructureSizeInBytes, pParameterCount, pParameterDescs);
}

HRESULT __stdcall WrappedD3D12Device::CreateMetaCommand(REFGUID CommandId, UINT NodeMask, const void* pCreationParametersData, SIZE_T CreationParametersDataSizeInBytes, REFIID riid, void** ppMetaCommand)
{
	return m_device5->CreateMetaCommand(CommandId, NodeMask, pCreationParametersData, CreationParametersDataSizeInBytes, riid, ppMetaCommand);
}

HRESULT __stdcall WrappedD3D12Device::CreateStateObject(const D3D12_STATE_OBJECT_DESC* pDesc, REFIID riid, void** ppStateObject)
{
	return m_device5->CreateStateObject(pDesc, riid, ppStateObject);
}

void __stdcall WrappedD3D12Device::GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* pDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* pInfo)
{
	return m_device5->GetRaytracingAccelerationStructurePrebuildInfo(pDesc, pInfo);
}

D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS __stdcall WrappedD3D12Device::CheckDriverMatchingIdentifier(D3D12_SERIALIZED_DATA_TYPE SerializedDataType, const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER* pIdentifierToCheck)
{
	return m_device5->CheckDriverMatchingIdentifier(SerializedDataType, pIdentifierToCheck);
}

HRESULT __stdcall WrappedD3D12Device::SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE Mode, D3D12_MEASUREMENTS_ACTION MeasurementsAction, HANDLE hEventToSignalUponCompletion, BOOL* pbFurtherMeasurementsDesired)
{
	return m_device6->SetBackgroundProcessingMode(Mode, MeasurementsAction, hEventToSignalUponCompletion, pbFurtherMeasurementsDesired);
}

HRESULT __stdcall WrappedD3D12Device::AddToStateObject(const D3D12_STATE_OBJECT_DESC* pAddition, ID3D12StateObject* pStateObjectToGrowFrom, REFIID riid, void** ppNewStateObject)
{
	return m_device7->AddToStateObject(pAddition, pStateObjectToGrowFrom, riid, ppNewStateObject);
}

HRESULT __stdcall WrappedD3D12Device::CreateProtectedResourceSession1(const D3D12_PROTECTED_RESOURCE_SESSION_DESC1* pDesc, REFIID riid, void** ppSession)
{
	return m_device7->CreateProtectedResourceSession1(pDesc, riid, ppSession);
}

D3D12_RESOURCE_ALLOCATION_INFO __stdcall WrappedD3D12Device::GetResourceAllocationInfo2(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC1* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1)
{
	return m_device8->GetResourceAllocationInfo2(visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommittedResource2(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riidResource, void** ppvResource)
{
	return m_device8->CreateCommittedResource2(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riidResource, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreatePlacedResource1(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource)
{
	return m_device8->CreatePlacedResource1(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}

void __stdcall WrappedD3D12Device::CreateSamplerFeedbackUnorderedAccessView(ID3D12Resource* pTargetedResource, ID3D12Resource* pFeedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	return m_device8->CreateSamplerFeedbackUnorderedAccessView(pTargetedResource, pFeedbackResource, DestDescriptor);
}

void __stdcall WrappedD3D12Device::GetCopyableFootprints1(const D3D12_RESOURCE_DESC1* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes)
{
	return m_device8->GetCopyableFootprints1(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
}

HRESULT __stdcall WrappedD3D12Device::CreateShaderCacheSession(const D3D12_SHADER_CACHE_SESSION_DESC* pDesc, REFIID riid, void** ppvSession)
{
	return m_device9->CreateShaderCacheSession(pDesc, riid, ppvSession);
}

HRESULT __stdcall WrappedD3D12Device::ShaderCacheControl(D3D12_SHADER_CACHE_KIND_FLAGS Kinds, D3D12_SHADER_CACHE_CONTROL_FLAGS Control)
{
	return m_device9->ShaderCacheControl(Kinds, Control);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommandQueue1(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID CreatorID, REFIID riid, void** ppCommandQueue)
{
	return m_device9->CreateCommandQueue1(pDesc, CreatorID, riid, ppCommandQueue);
}

HRESULT __stdcall WrappedD3D12Device::CreateCommittedResource3(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riidResource, void** ppvResource)
{
	return m_device10->CreateCommittedResource3(pHeapProperties, HeapFlags, pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riidResource, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreatePlacedResource2(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riid, void** ppvResource)
{
	return m_device10->CreatePlacedResource2(pHeap, HeapOffset, pDesc, InitialLayout, pOptimizedClearValue, NumCastableFormats, pCastableFormats, riid, ppvResource);
}

HRESULT __stdcall WrappedD3D12Device::CreateReservedResource2(const D3D12_RESOURCE_DESC* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riid, void** ppvResource)
{
	return m_device10->CreateReservedResource2(pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riid, ppvResource);
}
