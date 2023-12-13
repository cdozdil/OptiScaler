#pragma once

#include "WrappedDXGIObjects.h"

class RefCountD3D12Object : public ID3D12Object
{
	ID3D12Object* m_pReal;
	unsigned int m_iRefcount;

public:
	RefCountD3D12Object(ID3D12Object* real) : m_pReal(real), m_iRefcount(1) {}
	virtual ~RefCountD3D12Object() {}
	static bool HandleWrap(const char* ifaceName, REFIID riid, void** ppvObject);
	static HRESULT WrapQueryInterface(IUnknown* real, const char* ifaceName, REFIID riid, void** ppvObject);

	//////////////////////////////
	// implement IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid, /* [annotation][iid_is][out] */ __RPC__deref_out void** ppvObject)
	{
		LOG("RefCountD3D12Object.QueryInterface");
		auto result = QueryInterface("IUnknown", riid, ppvObject);
		LOG("RefCountD3D12Object.QueryInterface result: " + int_to_hex(result));

		return result;
	}

	// optional overload that's useful for passing down the name of the current interface to put in
	// any 'unknown interface' query logs.
	HRESULT STDMETHODCALLTYPE QueryInterface(const char* ifaceName, REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(IUnknown))
		{
			AddRef();
			*ppvObject = (IUnknown*)(ID3D12Object*)this;
			return S_OK;
		}
		else if (riid == __uuidof(ID3D12Object))
		{
			AddRef();
			*ppvObject = (ID3D12Object*)this;
			return S_OK;
		}

		return WrapQueryInterface(m_pReal, ifaceName, riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		LOG("RefCountD3D12Object.AddRef");
		InterlockedIncrement(&m_iRefcount);
		LOG("RefCountD3D12Object.AddRef result: " + int_to_hex(m_iRefcount));
		return m_iRefcount;
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		LOG("RefCountD3D12Object.Release");

		unsigned int ret = InterlockedDecrement(&m_iRefcount);

		LOG("RefCountD3D12Object.Release result: " + int_to_hex(m_iRefcount));

		if (ret == 0)
		{
			LOG("RefCountD3D12Object.Release deleting object");
			delete this;
		}

		return ret;
	}

	//////////////////////////////
	// implement ID3D12Object

	virtual HRESULT STDMETHODCALLTYPE SetPrivateData( /* [in] */ REFGUID Name, /* [in] */ UINT DataSize, /* [in] */ const void* pData)
	{
		LOG("RefCountDXGIObject.SetPrivateData");
		auto result = m_pReal->SetPrivateData(Name, DataSize, pData);
		LOG("RefCountDXGIObject.SetPrivateData result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( /* [in] */ REFGUID Name, /* [in] */ const IUnknown* pUnknown)
	{
		LOG("RefCountDXGIObject.SetPrivateDataInterface");
		auto result = m_pReal->SetPrivateDataInterface(Name, pUnknown);
		LOG("RefCountDXGIObject.SetPrivateDataInterface result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPrivateData( /* [in] */ REFGUID Name, /* [out][in] */ UINT* pDataSize, /* [out] */ void* pData)
	{
		LOG("RefCountDXGIObject.GetPrivateData");
		auto result = m_pReal->GetPrivateData(Name, pDataSize, pData);
		LOG("RefCountDXGIObject.GetPrivateData result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE SetName(_In_z_  LPCWSTR Name);
};

#define IMPLEMENT_ID3D12OBJECT_WITH_REFCOUNTDXGIOBJECT_CUSTOMQUERY									\
  ULONG STDMETHODCALLTYPE AddRef()																	\
  {																									\
	return RefCountD3D12Object::AddRef();															\
  }																									\
  ULONG STDMETHODCALLTYPE Release()																	\
  {																									\
	return RefCountD3D12Object::Release();															\
  }																									\
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFIID Name, UINT DataSize, const void *pData)			\
  {																									\
	return RefCountD3D12Object::SetPrivateData(Name, DataSize, pData);								\
  }																									\
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFIID Name, const IUnknown *pUnknown)			\
  {																									\
	return RefCountD3D12Object::SetPrivateDataInterface(Name, pUnknown);							\
  }																									\
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFIID Name, UINT *pDataSize, void *pData)				\
  {																									\
	return RefCountD3D12Object::GetPrivateData(Name, pDataSize, pData);								\
  }																									\
  HRESULT STDMETHODCALLTYPE SetName(_In_z_  LPCWSTR Name)					                        \
  {																									\
	return RefCountD3D12Object::SetName(Name);														\
  }

MIDL_INTERFACE("fa4994ad-dbe4-44b9-8c5c-bb5cf7188b6e")
ID3D12ProxyDevice : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetProxyAdapter(IDXGIProxyAdapter** adapter);
	virtual HRESULT STDMETHODCALLTYPE SetProxyAdapter(IDXGIProxyAdapter* adapter);
};

class WrappedD3D12Device : public ID3D12Device10, public RefCountD3D12Object, public ID3D12ProxyDevice
{
	ID3D12Device1* m_device1;
	ID3D12Device2* m_device2;
	ID3D12Device3* m_device3;
	ID3D12Device4* m_device4;
	ID3D12Device5* m_device5;
	ID3D12Device6* m_device6;
	ID3D12Device7* m_device7;
	ID3D12Device8* m_device8;
	ID3D12Device9* m_device9;
	ID3D12Device10* m_device10;
	IDXGIProxyAdapter* m_adapter;

public:
	ID3D12Device* m_device;
	WrappedD3D12Device(ID3D12Device* device);

	virtual ~WrappedD3D12Device();

	IMPLEMENT_ID3D12OBJECT_WITH_REFCOUNTDXGIOBJECT_CUSTOMQUERY;

	// Inherited via ID3D12Device10

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	UINT __stdcall GetNodeCount(void) override;
	HRESULT __stdcall CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue) override;
	HRESULT __stdcall CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator) override;
	HRESULT __stdcall CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState) override;
	HRESULT __stdcall CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, REFIID riid, void** ppPipelineState) override;
	HRESULT __stdcall CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList) override;
	HRESULT __stdcall CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) override;
	HRESULT __stdcall CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap) override;
	UINT __stdcall GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) override;
	HRESULT __stdcall CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature) override;
	void __stdcall CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, const UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, const UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType) override;
	void __stdcall CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType) override;
	D3D12_RESOURCE_ALLOCATION_INFO __stdcall GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs) override;
	D3D12_HEAP_PROPERTIES __stdcall GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType) override;
	HRESULT __stdcall CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource) override;
	HRESULT __stdcall CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap) override;
	HRESULT __stdcall CreatePlacedResource(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) override;
	HRESULT __stdcall CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) override;
	HRESULT __stdcall CreateSharedHandle(ID3D12DeviceChild* pObject, const SECURITY_ATTRIBUTES* pAttributes, DWORD Access, LPCWSTR Name, HANDLE* pHandle) override;
	HRESULT __stdcall OpenSharedHandle(HANDLE NTHandle, REFIID riid, void** ppvObj) override;
	HRESULT __stdcall OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE* pNTHandle) override;
	HRESULT __stdcall MakeResident(UINT NumObjects, ID3D12Pageable* const* ppObjects) override;
	HRESULT __stdcall Evict(UINT NumObjects, ID3D12Pageable* const* ppObjects) override;
	HRESULT __stdcall CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence) override;
	HRESULT __stdcall GetDeviceRemovedReason(void) override;
	void __stdcall GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes) override;
	HRESULT __stdcall CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap) override;
	HRESULT __stdcall SetStablePowerState(BOOL Enable) override;
	HRESULT __stdcall CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature) override;
	void __stdcall GetResourceTiling(ID3D12Resource* pTiledResource, UINT* pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* pPackedMipDesc, D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips, UINT* pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips) override;
	LUID __stdcall GetAdapterLuid(void) override;
	HRESULT __stdcall CreatePipelineLibrary(const void* pLibraryBlob, SIZE_T BlobLength, REFIID riid, void** ppPipelineLibrary) override;
	HRESULT __stdcall SetEventOnMultipleFenceCompletion(ID3D12Fence* const* ppFences, const UINT64* pFenceValues, UINT NumFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags, HANDLE hEvent) override;
	HRESULT __stdcall SetResidencyPriority(UINT NumObjects, ID3D12Pageable* const* ppObjects, const D3D12_RESIDENCY_PRIORITY* pPriorities) override;
	HRESULT __stdcall CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc, REFIID riid, void** ppPipelineState) override;
	HRESULT __stdcall OpenExistingHeapFromAddress(const void* pAddress, REFIID riid, void** ppvHeap) override;
	HRESULT __stdcall OpenExistingHeapFromFileMapping(HANDLE hFileMapping, REFIID riid, void** ppvHeap) override;
	HRESULT __stdcall EnqueueMakeResident(D3D12_RESIDENCY_FLAGS Flags, UINT NumObjects, ID3D12Pageable* const* ppObjects, ID3D12Fence* pFenceToSignal, UINT64 FenceValueToSignal) override;
	HRESULT __stdcall CreateCommandList1(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void** ppCommandList) override;
	HRESULT __stdcall CreateProtectedResourceSession(const D3D12_PROTECTED_RESOURCE_SESSION_DESC* pDesc, REFIID riid, void** ppSession) override;
	HRESULT __stdcall CreateCommittedResource1(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riidResource, void** ppvResource) override;
	HRESULT __stdcall CreateHeap1(const D3D12_HEAP_DESC* pDesc, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riid, void** ppvHeap) override;
	HRESULT __stdcall CreateReservedResource1(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riid, void** ppvResource) override;
	D3D12_RESOURCE_ALLOCATION_INFO __stdcall GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1) override;
	HRESULT __stdcall CreateLifetimeTracker(ID3D12LifetimeOwner* pOwner, REFIID riid, void** ppvTracker) override;
	void __stdcall RemoveDevice(void) override;
	HRESULT __stdcall EnumerateMetaCommands(UINT* pNumMetaCommands, D3D12_META_COMMAND_DESC* pDescs) override;
	HRESULT __stdcall EnumerateMetaCommandParameters(REFGUID CommandId, D3D12_META_COMMAND_PARAMETER_STAGE Stage, UINT* pTotalStructureSizeInBytes, UINT* pParameterCount, D3D12_META_COMMAND_PARAMETER_DESC* pParameterDescs) override;
	HRESULT __stdcall CreateMetaCommand(REFGUID CommandId, UINT NodeMask, const void* pCreationParametersData, SIZE_T CreationParametersDataSizeInBytes, REFIID riid, void** ppMetaCommand) override;
	HRESULT __stdcall CreateStateObject(const D3D12_STATE_OBJECT_DESC* pDesc, REFIID riid, void** ppStateObject) override;
	void __stdcall GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* pDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* pInfo) override;
	D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS __stdcall CheckDriverMatchingIdentifier(D3D12_SERIALIZED_DATA_TYPE SerializedDataType, const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER* pIdentifierToCheck) override;
	HRESULT __stdcall SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE Mode, D3D12_MEASUREMENTS_ACTION MeasurementsAction, HANDLE hEventToSignalUponCompletion, BOOL* pbFurtherMeasurementsDesired) override;
	HRESULT __stdcall AddToStateObject(const D3D12_STATE_OBJECT_DESC* pAddition, ID3D12StateObject* pStateObjectToGrowFrom, REFIID riid, void** ppNewStateObject) override;
	HRESULT __stdcall CreateProtectedResourceSession1(const D3D12_PROTECTED_RESOURCE_SESSION_DESC1* pDesc, REFIID riid, void** ppSession) override;
	D3D12_RESOURCE_ALLOCATION_INFO __stdcall GetResourceAllocationInfo2(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC1* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1) override;
	HRESULT __stdcall CreateCommittedResource2(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, REFIID riidResource, void** ppvResource) override;
	HRESULT __stdcall CreatePlacedResource1(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) override;
	void __stdcall CreateSamplerFeedbackUnorderedAccessView(ID3D12Resource* pTargetedResource, ID3D12Resource* pFeedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
	void __stdcall GetCopyableFootprints1(const D3D12_RESOURCE_DESC1* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, UINT64* pRowSizeInBytes, UINT64* pTotalBytes) override;
	HRESULT __stdcall CreateShaderCacheSession(const D3D12_SHADER_CACHE_SESSION_DESC* pDesc, REFIID riid, void** ppvSession) override;
	HRESULT __stdcall ShaderCacheControl(D3D12_SHADER_CACHE_KIND_FLAGS Kinds, D3D12_SHADER_CACHE_CONTROL_FLAGS Control) override;
	HRESULT __stdcall CreateCommandQueue1(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID CreatorID, REFIID riid, void** ppCommandQueue) override;
	HRESULT __stdcall CreateCommittedResource3(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riidResource, void** ppvResource) override;
	HRESULT __stdcall CreatePlacedResource2(ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riid, void** ppvResource) override;
	HRESULT __stdcall CreateReservedResource2(const D3D12_RESOURCE_DESC* pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE* pOptimizedClearValue, ID3D12ProtectedResourceSession* pProtectedSession, UINT32 NumCastableFormats, DXGI_FORMAT* pCastableFormats, REFIID riid, void** ppvResource) override;

	virtual HRESULT STDMETHODCALLTYPE GetProxyAdapter(IDXGIProxyAdapter** adapter)
	{
		*adapter = m_adapter;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetProxyAdapter(IDXGIProxyAdapter* adapter)
	{
		m_adapter = adapter;
		return S_OK;
	}
};

