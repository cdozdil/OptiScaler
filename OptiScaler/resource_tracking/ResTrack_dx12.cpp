#include "ResTrack_dx12.h"

#include <Config.h>
#include <State.h>
#include <Util.h>

#include <menu/menu_overlay_dx.h>

#include <future>

#include <detours/detours.h>
#include <ankerl/unordered_dense.h>

#define LOG_HEAP_MOVES

// Device hooks for FG
typedef void(*PFN_CreateRenderTargetView)(ID3D12Device* This, ID3D12Resource* pResource, D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void(*PFN_CreateShaderResourceView)(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void(*PFN_CreateUnorderedAccessView)(ID3D12Device* This, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

typedef HRESULT(*PFN_CreateDescriptorHeap)(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap);
typedef void(*PFN_CopyDescriptors)(ID3D12Device* This, UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
typedef void(*PFN_CopyDescriptorsSimple)(ID3D12Device* This, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);

// Command list hooks for FG
typedef void(*PFN_OMSetRenderTargets)(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
typedef void(*PFN_SetGraphicsRootDescriptorTable)(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
typedef void(*PFN_SetComputeRootDescriptorTable)(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
typedef void(*PFN_DrawIndexedInstanced)(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(*PFN_DrawInstanced)(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
typedef void(*PFN_Dispatch)(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

typedef void(*PFN_ExecuteCommandLists)(ID3D12CommandQueue* This, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

#ifdef USE_COPY_RESOURCE
typedef void(*PFN_CopyResource)(ID3D12GraphicsCommandList* This, ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource);
typedef void(*PFN_CopyTextureRegion)(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox);
#endif

#ifdef USE_RESOURCE_DISCARD
typedef void(*PFN_DiscardResource)(ID3D12GraphicsCommandList* This, ID3D12Resource* pResource, D3D12_DISCARD_REGION* pRegion);
#endif

// Original method calls for device
static PFN_CreateRenderTargetView o_CreateRenderTargetView = nullptr;
static PFN_CreateShaderResourceView o_CreateShaderResourceView = nullptr;
static PFN_CreateUnorderedAccessView o_CreateUnorderedAccessView = nullptr;
static PFN_CreateDescriptorHeap o_CreateDescriptorHeap = nullptr;
static PFN_CopyDescriptors o_CopyDescriptors = nullptr;
static PFN_CopyDescriptorsSimple o_CopyDescriptorsSimple = nullptr;

// Original method calls for command list
static PFN_Dispatch o_Dispatch = nullptr;
static PFN_DrawInstanced o_DrawInstanced = nullptr;
static PFN_DrawIndexedInstanced o_DrawIndexedInstanced = nullptr;

static PFN_ExecuteCommandLists o_ExecuteCommandLists = nullptr;

static PFN_OMSetRenderTargets o_OMSetRenderTargets = nullptr;
static PFN_SetGraphicsRootDescriptorTable o_SetGraphicsRootDescriptorTable = nullptr;
static PFN_SetComputeRootDescriptorTable o_SetComputeRootDescriptorTable = nullptr;

#ifdef USE_COPY_RESOURCE
static PFN_CopyResource o_CopyResource = nullptr;
static PFN_CopyTextureRegion o_CopyTextureRegion = nullptr;
#endif

#ifdef USE_RESOURCE_DISCARD
static PFN_DiscardResource o_DiscardResource = nullptr;
static std::mutex _resourceMutex;
#endif

// heaps
static std::vector<HeapInfo> fgHeaps;

#ifdef USE_RESOURCE_DISCARD
// created resources
static ankerl::unordered_dense::map <ID3D12Resource*, ResourceHeapInfo> fgHandlesByResources;
#endif

static std::set<ID3D12Resource*> fgCaptureList;

// possibleHudless lisy by cmdlist
static ankerl::unordered_dense::map <ID3D12GraphicsCommandList*, ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo>> fgPossibleHudless[BUFFER_COUNT];

static std::shared_mutex heapMutex;
static std::mutex hudlessMutex;

inline static IID streamlineRiid{};
static bool CheckForRealObject(std::string functionName, IUnknown* pObject, IUnknown** ppRealObject)
{
    if (streamlineRiid.Data1 == 0)
    {
        auto iidResult = IIDFromString(L"{ADEC44E2-61F0-45C3-AD9F-1B37379284FF}", &streamlineRiid);

        if (iidResult != S_OK)
            return false;
    }

    auto qResult = pObject->QueryInterface(streamlineRiid, (void**)ppRealObject);

    if (qResult == S_OK && *ppRealObject != nullptr)
    {
        LOG_INFO("{} Streamline proxy found!", functionName);
        (*ppRealObject)->Release();
        return true;
    }

    return false;
}


#pragma region Resource methods

static bool CreateBufferResource(ID3D12Device* InDevice, ResourceInfo* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource)
{
    if (InDevice == nullptr || InSource == nullptr)
        return false;

    if (*OutResource != nullptr)
    {
        auto bufDesc = (*OutResource)->GetDesc();

        if (bufDesc.Width != (UINT64)(InSource->width) || bufDesc.Height != (UINT)(InSource->height) || bufDesc.Format != InSource->format)
        {
            (*OutResource)->Release();
            (*OutResource) = nullptr;
        }
        else
            return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = InSource->buffer->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        LOG_ERROR("GetHeapProperties result: {0:X}", (UINT64)hr);
        return false;
    }

    D3D12_RESOURCE_DESC texDesc = InSource->buffer->GetDesc();
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(OutResource));

    if (hr != S_OK)
    {
        LOG_ERROR("CreateCommittedResource result: {0:X}", (UINT64)hr);
        return false;
    }

    (*OutResource)->SetName(L"fgHudlessSCBufferCopy");
    return true;
}

static void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = InResource;
    barrier.Transition.StateBefore = InBeforeState;
    barrier.Transition.StateAfter = InAfterState;
    barrier.Transition.Subresource = 0;
    InCommandList->ResourceBarrier(1, &barrier);
}

#pragma endregion

#pragma region Heap helpers

static SIZE_T GetGPUHandle(ID3D12Device* This, SIZE_T cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    std::shared_lock<std::shared_mutex> lock(heapMutex);
    for (auto& val : fgHeaps)
    {
        if (val.cpuStart <= cpuHandle && val.cpuEnd >= cpuHandle && val.gpuStart != 0)
        {
            auto incSize = This->GetDescriptorHandleIncrementSize(type);
            auto addr = cpuHandle - val.cpuStart;
            auto index = addr / incSize;
            auto gpuAddr = val.gpuStart + (index * incSize);

            return gpuAddr;
        }
    }

    return NULL;
}

static SIZE_T GetCPUHandle(ID3D12Device* This, SIZE_T gpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    std::shared_lock<std::shared_mutex> lock(heapMutex);
    for (auto& val : fgHeaps)
    {
        if (val.gpuStart <= gpuHandle && val.gpuEnd >= gpuHandle && val.cpuStart != 0)
        {
            auto incSize = This->GetDescriptorHandleIncrementSize(type);
            auto addr = gpuHandle - val.gpuStart;
            auto index = addr / incSize;
            auto cpuAddr = val.cpuStart + (index * incSize);

            return cpuAddr;
        }
    }

    return NULL;
}

static HeapInfo* GetHeapByCpuHandle(SIZE_T cpuHandle)
{
    std::shared_lock<std::shared_mutex> lock(heapMutex);
    for (size_t i = 0; i < fgHeaps.size(); i++)
    {
        if (fgHeaps[i].cpuStart <= cpuHandle && fgHeaps[i].cpuEnd >= cpuHandle)
            return &fgHeaps[i];
    }

    return nullptr;
}

static HeapInfo* GetHeapByGpuHandle(SIZE_T gpuHandle)
{
    std::shared_lock<std::shared_mutex> lock(heapMutex);
    for (size_t i = 0; i < fgHeaps.size(); i++)
    {
        if (fgHeaps[i].gpuStart <= gpuHandle && fgHeaps[i].gpuEnd >= gpuHandle)
            return &fgHeaps[i];
    }

    return nullptr;
}

#pragma endregion

#pragma region Hudless methods

static void FillResourceInfo(ID3D12Resource* resource, ResourceInfo* info)
{
    auto desc = resource->GetDesc();
    info->buffer = resource;
    info->width = desc.Width;
    info->height = desc.Height;
    info->format = desc.Format;
    info->flags = desc.Flags;
}

static bool _written = false;
static bool _fgEnabled = false;
static bool _currentFG = false;
static bool _currentFGActive = false;
static bool _skipHudless = false;
static bool _rcActive = false;
static bool _cmdList = false;

bool ResTrack_Dx12::IsHudFixActive()
{
    if (!Config::Instance()->FGEnabled.value_or_default() || !Config::Instance()->FGHUDFix.value_or_default())
    {
        if (!_fgEnabled)
        {
            //LOG_WARN("!_fgEnabled");
            _fgEnabled = true;
        }

        return false;
    }

    if (State::Instance().currentFG == nullptr || State::Instance().currentFeature == nullptr || State::Instance().FGchanged)
    {
        if (!_currentFG)
        {
            //LOG_WARN("!_currentFG");
            _currentFG = true;
        }

        return false;
    }

    if (!State::Instance().currentFG->IsActive())
    {
        if (!_currentFGActive)
        {
            //LOG_WARN("!_currentFGActive");
            _currentFGActive = true;
        }

        return false;
    }

    if (!_presentDone)
    {
        if (!_written)
        {
            //LOG_WARN("!_presentDone");
            _presentDone = true;
        }

        return false;
    }

    if (Hudfix_Dx12::SkipHudlessChecks())
    {
        if (!_skipHudless)
        {
            //LOG_WARN("!_skipHudless");
            _skipHudless = true;
        }

        return false;
    }

    if (!Hudfix_Dx12::IsResourceCheckActive())
    {
        if (!_rcActive)
        {
            //LOG_WARN("!_rcActive");
            _rcActive = true;
        }

        return false;
    }

    return true;
}

static bool IsFGCommandList(IUnknown* cmdList)
{
    // prevent hudfix check
    if (State::Instance().currentFG == nullptr)
        return true;

    IFGFeature_Dx12* fg = nullptr;

    fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

    // prevent hudfix check
    if (fg == nullptr)
        return true;

    return fg->IsFGCommandList(cmdList);
}


#pragma endregion

#pragma region Resource discard hooks

#ifdef USE_RESOURCE_DISCARD
static void hkDiscardResource(ID3D12GraphicsCommandList* This, ID3D12Resource* pResource, D3D12_DISCARD_REGION* pRegion)
{
    o_DiscardResource(This, pResource, pRegion);

    if (!IsFGCommandList(This) && pRegion == nullptr)
    {
        std::lock_guard<std::mutex> lock(_resourceMutex);

        if (!fgHandlesByResources.contains(pResource))
            return;

        auto heapInfo = &fgHandlesByResources[pResource];
        LOG_DEBUG_ONLY(" <-- {}", heapInfo->cpuStart);

        auto heap = GetHeapByCpuHandle(heapInfo->cpuStart);
        if (heap != nullptr)
            heap->SetByCpuHandle(heapInfo->cpuStart, {});

        LOG_TRACE("Erased: {:X}", (size_t)pResource);
        fgHandlesByResources.erase(pResource);
    }
}
#endif

#pragma endregion

#pragma region Resource input hooks

static void hkCreateRenderTargetView(ID3D12Device* This, ID3D12Resource* pResource, D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    // force hdr for swapchain buffer
    if (pResource != nullptr && pDesc != nullptr && Config::Instance()->ForceHDR.value_or_default())
    {
        for (size_t i = 0; i < State::Instance().SCbuffers.size(); i++)
        {
            if (State::Instance().SCbuffers[i] == pResource)
            {
                if (Config::Instance()->UseHDR10.value_or_default())
                    pDesc->Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                else
                    pDesc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

                break;
            }
        }
    }

    o_CreateRenderTargetView(This, pResource, pDesc, DestDescriptor);

    if (pResource == nullptr && (pDesc == nullptr || pDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D))
    {
        LOG_TRACE("Unbind: {:X}", DestDescriptor.ptr);

        auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
        if (heap != nullptr)
            heap->SetByCpuHandle(DestDescriptor.ptr, {});

        return;
    }

    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_RTV_DIMENSION_TEXTURE2D)
        return;

    auto gpuHandle = GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    ResourceHeapInfo info{};
    info.cpuStart = DestDescriptor.ptr;
    info.gpuStart = gpuHandle;

    ResourceInfo resInfo{};
    FillResourceInfo(pResource, &resInfo);
    resInfo.type = RTV;

#ifdef USE_RESOURCE_DISCARD
    {
        std::lock_guard<std::mutex> lock(_resourceMutex);
        fgHandlesByResources.insert_or_assign(pResource, info);
    }
#endif

#ifdef DETAILED_DEBUG_LOGS

    if (CheckForHudless("", &resInfo))
        LOG_TRACE("<------ pResource: {0:X}, CpuHandle: {1}, GpuHandle: {2}", (UINT64)pResource, DestDescriptor.ptr, GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

#endif //  _DEBUG

    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

static void hkCreateShaderResourceView(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    // force hdr for swapchain buffer
    if (pResource != nullptr && pDesc != nullptr && Config::Instance()->ForceHDR.value_or_default())
    {
        for (size_t i = 0; i < State::Instance().SCbuffers.size(); i++)
        {
            if (State::Instance().SCbuffers[i] == pResource)
            {
                if (Config::Instance()->UseHDR10.value_or_default())
                    pDesc->Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                else
                    pDesc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

                break;
            }
        }
    }

    o_CreateShaderResourceView(This, pResource, pDesc, DestDescriptor);

    if (pResource == nullptr && (pDesc == nullptr || pDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D))
    {
        LOG_TRACE("Unbind: {:X}", DestDescriptor.ptr);

        auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
        if (heap != nullptr)
            heap->SetByCpuHandle(DestDescriptor.ptr, {});

        return;
    }

    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_SRV_DIMENSION_TEXTURE2D)
        return;

    auto gpuHandle = GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ResourceHeapInfo info{};
    info.cpuStart = DestDescriptor.ptr;
    info.gpuStart = gpuHandle;

    ResourceInfo resInfo{};
    FillResourceInfo(pResource, &resInfo);
    resInfo.type = SRV;

#ifdef USE_RESOURCE_DISCARD
    {
        std::lock_guard<std::mutex> lock(_resourceMutex);
        fgHandlesByResources.insert_or_assign(pResource, info);
    }
#endif

#ifdef DETAILED_DEBUG_LOGS

    if (CheckForHudless("", &resInfo))
        LOG_TRACE("<------ pResource: {0:X}, CpuHandle: {1}, GpuHandle: {2}", (UINT64)pResource, DestDescriptor.ptr, GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

#endif //  _DEBUG

    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

static void hkCreateUnorderedAccessView(ID3D12Device* This, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    if (pResource != nullptr && pDesc != nullptr && Config::Instance()->ForceHDR.value_or_default())
    {
        for (size_t i = 0; i < State::Instance().SCbuffers.size(); i++)
        {
            if (State::Instance().SCbuffers[i] == pResource)
            {
                if (Config::Instance()->UseHDR10.value_or_default())
                    pDesc->Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                else
                    pDesc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

                break;
            }
        }
    }

    o_CreateUnorderedAccessView(This, pResource, pCounterResource, pDesc, DestDescriptor);

    if (pResource == nullptr && (pDesc == nullptr || pDesc->ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D))
    {
        LOG_TRACE("Unbind: {:X}", DestDescriptor.ptr);

        auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
        if (heap != nullptr)
            heap->SetByCpuHandle(DestDescriptor.ptr, {});

        return;
    }

    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_UAV_DIMENSION_TEXTURE2D)
        return;

    auto gpuHandle = GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ResourceHeapInfo info{};
    info.cpuStart = DestDescriptor.ptr;
    info.gpuStart = gpuHandle;

    ResourceInfo resInfo{};
    FillResourceInfo(pResource, &resInfo);
    resInfo.type = UAV;

#ifdef USE_RESOURCE_DISCARD
    {
        std::lock_guard<std::mutex> lock(_resourceMutex);
        fgHandlesByResources.insert_or_assign(pResource, info);
    }
#endif

#ifdef DETAILED_DEBUG_LOGS

    if (CheckForHudless("", &resInfo))
        LOG_TRACE("<------ pResource: {0:X}, CpuHandle: {1}, GpuHandle: {2}", (UINT64)pResource, DestDescriptor.ptr, GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

#endif //  _DEBUG


    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

#pragma endregion

#pragma region Resource copy hooks

#ifdef USE_COPY_RESOURCE
static void hkCopyResource(ID3D12GraphicsCommandList* This, ID3D12Resource* Dest, ID3D12Resource* Source)
{
    o_CopyResource(This, Dest, Source);

    auto fIndex = GetFrameIndex();

    if (This == MenuOverlayDx::MenuCommandList() || FrameGen_Dx12::fgCommandList[fIndex] == This)
        return;

    if (!IsHudFixActive())
        return;

    ResourceInfo srcInfo{};
    FillResourceInfo(Source, &srcInfo);

    if (CheckForHudless(__FUNCTION__, &srcInfo) && CheckCapture(__FUNCTION__))
    {
        CaptureHudless(This, &srcInfo, D3D12_RESOURCE_STATE_COPY_SOURCE);
        return;
    }

    ResourceInfo dstInfo{};
    FillResourceInfo(Dest, &dstInfo);

    if (CheckForHudless(__FUNCTION__, &dstInfo) && CheckCapture(__FUNCTION__))
        CaptureHudless(This, &dstInfo, D3D12_RESOURCE_STATE_COPY_DEST);
}

static void hkCopyTextureRegion(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox)
{
    o_CopyTextureRegion(This, pDst, DstX, DstY, DstZ, pSrc, pSrcBox);

    auto fIndex = GetFrameIndex();

    if (This == MenuOverlayDx::MenuCommandList() || FrameGen_Dx12::fgCommandList[fIndex] == This)
        return;

    if (!IsHudFixActive())
        return;

    ResourceInfo srcInfo{};
    FillResourceInfo(pSrc->pResource, &srcInfo);

    if (CheckForHudless(__FUNCTION__, &srcInfo) && CheckCapture(__FUNCTION__))
    {
        CaptureHudless(This, &srcInfo, D3D12_RESOURCE_STATE_COPY_SOURCE);
        return;
    }

    ResourceInfo dstInfo{};
    FillResourceInfo(pDst->pResource, &dstInfo);

    if (CheckForHudless(__FUNCTION__, &dstInfo) && CheckCapture(__FUNCTION__))
        CaptureHudless(This, &dstInfo, D3D12_RESOURCE_STATE_COPY_DEST);
}

#endif

#pragma endregion

static void hkExecuteCommandLists(ID3D12CommandQueue* This, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    //for (size_t i = 0; i < NumCommandLists; i++)
    //{
    //    Hudfix_Dx12::CaptureHudless(ppCommandLists[i]);
    //}

    o_ExecuteCommandLists(This, NumCommandLists, ppCommandLists);
}

#pragma region Heap hooks

static HRESULT hkCreateDescriptorHeap(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap)
{
    auto result = o_CreateDescriptorHeap(This, pDescriptorHeapDesc, riid, ppvHeap);

    if (State::Instance().skipHeapCapture)
        return result;

    // try to calculate handle ranges for heap
    if (result == S_OK && (pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
    {
        auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
        auto increment = This->GetDescriptorHandleIncrementSize(pDescriptorHeapDesc->Type);
        auto numDescriptors = pDescriptorHeapDesc->NumDescriptors;
        auto cpuStart = (SIZE_T)(heap->GetCPUDescriptorHandleForHeapStart().ptr);
        auto cpuEnd = cpuStart + (increment * numDescriptors);
        auto gpuStart = (SIZE_T)(heap->GetGPUDescriptorHandleForHeapStart().ptr);
        auto gpuEnd = gpuStart + (increment * numDescriptors);
        auto type = (UINT)pDescriptorHeapDesc->Type;
        HeapInfo info(cpuStart, cpuEnd, gpuStart, gpuEnd, numDescriptors, increment, type);

        LOG_TRACE("Heap type: {}, Cpu: {}-{}, Gpu: {}-{}, Desc count: {}", info.type, info.cpuStart, info.cpuEnd, info.gpuStart, info.gpuEnd, info.numDescriptors);
        {
            std::unique_lock<std::shared_mutex> lock(heapMutex);
            fgHeaps.push_back(info);
        }
    }
    else
    {
        if (*ppvHeap != nullptr)
        {
            auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
            LOG_TRACE("Skipping, Heap type: {}, Cpu: {}, Gpu: {}", (UINT)pDescriptorHeapDesc->Type, heap->GetCPUDescriptorHandleForHeapStart().ptr, heap->GetGPUDescriptorHandleForHeapStart().ptr);
        }
    }

    return result;
}


void ResTrack_Dx12::hkCopyDescriptors(ID3D12Device* This,
                                      UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes,
                                      UINT NumSrcDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes,
                                      D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptors(This, NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);

    if (DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        return;

    if (pDestDescriptorRangeStarts == nullptr || pSrcDescriptorRangeStarts == nullptr)
        return;

    if (pDestDescriptorRangeSizes == nullptr && pSrcDescriptorRangeSizes == nullptr)
        return;

    if (!Config::Instance()->FGAlwaysTrackHeaps.value_or_default() && !IsHudFixActive())
        return;

    // make copies, just in case
    D3D12_CPU_DESCRIPTOR_HANDLE* destRangeStarts = pDestDescriptorRangeStarts;
    UINT* destRangeSizes = pDestDescriptorRangeSizes;
    D3D12_CPU_DESCRIPTOR_HANDLE* srcRangeStarts = pSrcDescriptorRangeStarts;
    UINT* srcRangeSizes = pSrcDescriptorRangeSizes;

    //LOG_DEBUG("NumDestDescriptorRanges: {}, pDestDescriptorRangeStarts: {}, pDestDescriptorRangeSizes: {}",
    //          NumDestDescriptorRanges, pDestDescriptorRangeStarts == nullptr ? "null" : "not null", pDestDescriptorRangeSizes == nullptr ? "null" : "not null");
    //LOG_DEBUG("NumSrcDescriptorRanges: {}, pSrcDescriptorRangeStarts: {}, pSrcDescriptorRangeSizes: {}",
    //          NumSrcDescriptorRanges, pSrcDescriptorRangeStarts == nullptr ? "null" : "not null", pSrcDescriptorRangeSizes == nullptr ? "null" : "not null");

    if (State::Instance().useThreadingForHeaps)
    {
        auto asyncTask = std::async(std::launch::async, [=]()
                                    {
                                        auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

                                        size_t destRangeIndex = 0;
                                        size_t destIndex = 0;

                                        for (size_t i = 0; i < NumSrcDescriptorRanges; i++)
                                        {
                                            UINT copyCount = 1;

                                            if (srcRangeSizes != nullptr)
                                                copyCount = srcRangeSizes[i];

                                            for (size_t j = 0; j < copyCount; j++)
                                            {
                                                // source
                                                auto srcHandle = srcRangeStarts[i].ptr + j * size;
                                                auto srcHeap = GetHeapByCpuHandle(srcHandle);
                                                if (srcHeap == nullptr)
                                                    continue;

                                                auto buffer = srcHeap->GetByCpuHandle(srcHandle);

                                                // destination
                                                auto destHandle = destRangeStarts[destRangeIndex].ptr + destIndex * size;
                                                auto dstHeap = GetHeapByCpuHandle(destHandle);
                                                if (dstHeap == nullptr)
                                                    continue;

                                                LOG_DEBUG_ONLY("Cpu Src: {}, Cpu Dest: {}, Gpu Src: {} Gpu Dest: {}, Type: {}",
                                                               handle, destHandle, GetGPUHandle(This, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), GetGPUHandle(This, destHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), (UINT)DescriptorHeapsType);
                                            }

                                            if (destRangeSizes == nullptr)
                                            {
                                                destIndex = 0;
                                                destRangeIndex++;
                                            }
                                            else
                                            {
                                                if (destRangeSizes[destRangeIndex] == destIndex)
                                                {
                                                    destIndex = 0;
                                                    destRangeIndex++;
                                                }
                                                else
                                                {
                                                    destIndex++;
                                                }
                                            }
                                        }

                                    });
    }
    else
    {
        auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

        if (srcRangeSizes != nullptr)
        {
            size_t destRangeIndex = 0;
            size_t destIndex = 0;

            for (size_t i = 0; i < NumSrcDescriptorRanges; i++)
            {
                UINT copyCount = srcRangeSizes[i];

                for (size_t j = 0; j < copyCount; j++)
                {
                    // source
                    auto srcHandle = srcRangeStarts[i].ptr + j * size;
                    auto srcHeap = GetHeapByCpuHandle(srcHandle);
                    if (srcHeap == nullptr)
                        continue;

                    auto buffer = srcHeap->GetByCpuHandle(srcHandle);

                    // destination
                    auto destHandle = destRangeStarts[destRangeIndex].ptr + destIndex * size;
                    auto dstHeap = GetHeapByCpuHandle(destHandle);
                    if (dstHeap == nullptr)
                        continue;

                    dstHeap->SetByCpuHandle(destHandle, *buffer);
                }

                destIndex++;

                if (destRangeSizes != nullptr && destRangeSizes[destRangeIndex] == destIndex)
                {
                    destIndex = 0;
                    destRangeIndex++;
                }
            }

        }
        else
        {
            size_t srcRangeIndex = 0;
            size_t srcIndex = 0;

            for (size_t i = 0; i < NumDestDescriptorRanges; i++)
            {
                UINT copyCount = 1;

                if (destRangeSizes != nullptr)
                    copyCount = destRangeSizes[i];

                for (size_t j = 0; j < copyCount; j++)
                {
                    // source
                    auto srcHandle = srcRangeStarts[srcRangeIndex].ptr + srcIndex * size;
                    auto srcHeap = GetHeapByCpuHandle(srcHandle);
                    if (srcHeap == nullptr)
                        continue;

                    auto buffer = srcHeap->GetByCpuHandle(srcHandle);

                    // destination
                    auto destHandle = destRangeStarts[i].ptr + j * size;
                    auto dstHeap = GetHeapByCpuHandle(destHandle);
                    if (dstHeap == nullptr)
                        continue;

                    dstHeap->SetByCpuHandle(destHandle, *buffer);
                }

                srcIndex++;

                if (srcRangeSizes != nullptr && srcRangeSizes[srcRangeIndex] == srcIndex)
                {
                    srcIndex = 0;
                    srcRangeIndex++;
                }
            }

        }
    }

}

void ResTrack_Dx12::hkCopyDescriptorsSimple(ID3D12Device* This, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
                                            D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptorsSimple(This, NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);

    if (DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        return;

    if (!Config::Instance()->FGAlwaysTrackHeaps.value_or_default() && !IsHudFixActive())
        return;

    if (State::Instance().useThreadingForHeaps)
    {
        auto asyncTask = std::async(std::launch::async, [=]()
                                    {
                                        auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

                                        for (size_t i = 0; i < NumDescriptors; i++)
                                        {
                                            // source
                                            auto srcHandle = SrcDescriptorRangeStart.ptr + i * size;
                                            auto srcHeap = GetHeapByCpuHandle(srcHandle);
                                            if (srcHeap == nullptr)
                                                continue;

                                            auto buffer = srcHeap->GetByCpuHandle(srcHandle);

                                            // destination
                                            auto destHandle = DestDescriptorRangeStart.ptr + i * size;
                                            auto dstHeap = GetHeapByCpuHandle(destHandle);
                                            if (dstHeap == nullptr)
                                                continue;

                                            dstHeap->SetByCpuHandle(destHandle, *buffer);

                                            LOG_DEBUG_ONLY("Cpu Src: {}, Cpu Dest: {}, Gpu Src: {} Gpu Dest: {}, Type: {}",
                                                           handle, destHandle, GetGPUHandle(This, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), GetGPUHandle(This, destHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), (UINT)DescriptorHeapsType);
                                        }

                                    });
    }
    else
    {
        auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

        for (size_t i = 0; i < NumDescriptors; i++)
        {
            // source
            auto srcHandle = SrcDescriptorRangeStart.ptr + i * size;
            auto srcHeap = GetHeapByCpuHandle(srcHandle);
            if (srcHeap == nullptr)
                continue;

            auto buffer = srcHeap->GetByCpuHandle(srcHandle);

            // destination
            auto destHandle = DestDescriptorRangeStart.ptr + i * size;
            auto dstHeap = GetHeapByCpuHandle(destHandle);
            if (dstHeap == nullptr)
                continue;

            dstHeap->SetByCpuHandle(destHandle, *buffer);

            LOG_DEBUG_ONLY("Cpu Src: {}, Cpu Dest: {}, Gpu Src: {} Gpu Dest: {}, Type: {}",
                           handle, destHandle, GetGPUHandle(This, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), GetGPUHandle(This, destHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), (UINT)DescriptorHeapsType);
        }
    }
}

#pragma endregion

#pragma region Shader input hooks

void ResTrack_Dx12::hkSetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    if (BaseDescriptor.ptr == 0 || !IsHudFixActive() || Hudfix_Dx12::SkipHudlessChecks())
    {
        o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
    {
        LOG_DEBUG_ONLY("Menu cmdlist: {} || fgCommandList: {}", This == MenuOverlayDx::MenuCommandList(), IsFGCommandList(This));
        o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    auto heap = GetHeapByGpuHandle(BaseDescriptor.ptr);
    if (heap == nullptr)
    {
        LOG_DEBUG_ONLY("No heap!");
        o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    auto capturedBuffer = heap->GetByGpuHandle(BaseDescriptor.ptr);
    if (capturedBuffer == nullptr || capturedBuffer->buffer == nullptr)
    {
        LOG_DEBUG_ONLY("Miss RootParameterIndex: {1}, CommandList: {0:X}, gpuHandle: {2}", (SIZE_T)This, RootParameterIndex, BaseDescriptor.ptr);
        o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    capturedBuffer->state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    do
    {
        if (Config::Instance()->FGImmediateCapture.value_or_default() &&
            Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, capturedBuffer, capturedBuffer->state))
        {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(hudlessMutex);

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

            if (!fgPossibleHudless[fIndex].contains(This))
            {
                ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
                fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
            }

            fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
        }
    } while (false);

    o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
}

#pragma endregion

#pragma region Shader output hooks

void ResTrack_Dx12::hkOMSetRenderTargets(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
                                         BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    if (NumRenderTargetDescriptors == 0 || pRenderTargetDescriptors == nullptr || !IsHudFixActive() || Hudfix_Dx12::SkipHudlessChecks())
    {
        o_OMSetRenderTargets(This, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
        return;
    }

    LOG_DEBUG_ONLY("NumRenderTargetDescriptors: {}", NumRenderTargetDescriptors);

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
    {
        LOG_DEBUG_ONLY("Menu cmdlist: {} || fgCommandList: {}", This == MenuOverlayDx::MenuCommandList(), IsFGCommandList(This));
        o_OMSetRenderTargets(This, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
        return;
    }

    {
        for (size_t i = 0; i < NumRenderTargetDescriptors; i++)
        {
            HeapInfo* heap = nullptr;
            D3D12_CPU_DESCRIPTOR_HANDLE handle{};

            if (RTsSingleHandleToDescriptorRange)
            {
                heap = GetHeapByCpuHandle(pRenderTargetDescriptors[0].ptr);
                if (heap == nullptr)
                {
                    LOG_DEBUG_ONLY("No heap!");
                    continue;
                }

                handle.ptr = pRenderTargetDescriptors[0].ptr + (i * heap->increment);
            }
            else
            {
                handle = pRenderTargetDescriptors[i];

                heap = GetHeapByCpuHandle(handle.ptr);
                if (heap == nullptr)
                {
                    LOG_DEBUG_ONLY("No heap!");
                    continue;
                }
            }

            auto resource = heap->GetByCpuHandle(handle.ptr);
            if (resource == nullptr || resource->buffer == nullptr)
            {
                LOG_DEBUG_ONLY("Miss index: {0}, cpu: {1}", i, handle.ptr);
                continue;
            }

            LOG_DEBUG_ONLY("CommandList: {:X}", (size_t)This);
            resource->state = D3D12_RESOURCE_STATE_RENDER_TARGET;

            if (Config::Instance()->FGImmediateCapture.value_or_default() &&
                Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, resource, resource->state))
            {
                break;
            }

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

            {
                // check for command list
                std::lock_guard<std::mutex> lock(hudlessMutex);

                if (!fgPossibleHudless[fIndex].contains(This))
                {
                    ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
                    fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
                }

                // add found resource
                fgPossibleHudless[fIndex][This].insert_or_assign(resource->buffer, *resource);
            }
        }
    }

    o_OMSetRenderTargets(This, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
}

#pragma endregion

#pragma region Compute paramter hooks

void ResTrack_Dx12::hkSetComputeRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    if (BaseDescriptor.ptr == 0 || !IsHudFixActive() || Hudfix_Dx12::SkipHudlessChecks())
    {
        o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    LOG_DEBUG_ONLY("");

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
    {
        LOG_DEBUG_ONLY("Menu cmdlist: {} || fgCommandList: {}", This == MenuOverlayDx::MenuCommandList(), IsFGCommandList(This));
        o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    auto heap = GetHeapByGpuHandle(BaseDescriptor.ptr);
    if (heap == nullptr)
    {
        LOG_DEBUG_ONLY("No heap!");
        o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    auto capturedBuffer = heap->GetByGpuHandle(BaseDescriptor.ptr);
    if (capturedBuffer == nullptr || capturedBuffer->buffer == nullptr)
    {
        LOG_DEBUG_ONLY("Miss RootParameterIndex: {1}, CommandList: {0:X}, gpuHandle: {2}", (SIZE_T)This, RootParameterIndex, BaseDescriptor.ptr);
        o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    LOG_DEBUG_ONLY("CommandList: {:X}", (size_t)This);

    if (capturedBuffer->type == UAV)
        capturedBuffer->state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    else
        capturedBuffer->state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    do
    {
        if (Config::Instance()->FGImmediateCapture.value_or_default() &&
            Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, capturedBuffer, capturedBuffer->state))
        {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(hudlessMutex);

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

            if (!fgPossibleHudless[fIndex].contains(This))
            {
                ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
                fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
            }

            fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
        }
    } while (false);

    o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
}

#pragma endregion

#pragma region Shader finalizer hooks

// Capture if render target matches, wait for DrawIndexed
void ResTrack_Dx12::hkDrawInstanced(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    o_DrawInstanced(This, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);


    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
    {
        if (!_cmdList)
        {
            LOG_WARN("_cmdList");
            _cmdList = true;
        }


        return;
    }

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;
        auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

        {
            std::lock_guard<std::mutex> lock(hudlessMutex);

            // if can't find output skip
            if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
            {
                LOG_DEBUG_ONLY("Early exit");
                return;
            }

            val0 = fgPossibleHudless[fIndex][This];

            do
            {
                // if this command list does not have entries skip
                if (val0.size() == 0)
                    break;

                for (auto& [key, val] : val0)
                {
                    //LOG_DEBUG("Waiting _drawMutex {:X}", (size_t)val.buffer);
                    std::lock_guard<std::mutex> lock(_drawMutex);

                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }
            } while (false);

            fgPossibleHudless[fIndex][This].clear();
        }

        LOG_DEBUG_ONLY("Clear");
    }
}

void ResTrack_Dx12::hkDrawIndexedInstanced(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    o_DrawIndexedInstanced(This, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);


    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
    {
        if (!_cmdList)
        {
            LOG_WARN("_cmdList");
            _cmdList = true;
        }

        return;
    }

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;
        auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

        {
            std::lock_guard<std::mutex> lock(hudlessMutex);

            // if can't find output skip
            if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
            {
                LOG_DEBUG_ONLY("Early exit");
                return;
            }

            val0 = fgPossibleHudless[fIndex][This];

            do
            {
                // if this command list does not have entries skip
                if (val0.size() == 0)
                    break;

                for (auto& [key, val] : val0)
                {
                    //LOG_DEBUG("Waiting _drawMutex {:X}", (size_t)val.buffer);
                    std::lock_guard<std::mutex> lock(_drawMutex);

                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }
            } while (false);

            fgPossibleHudless[fIndex][This].clear();
        }

        LOG_DEBUG_ONLY("Clear");
    }
}

void ResTrack_Dx12::hkDispatch(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    o_Dispatch(This, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
        return;

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;
        auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

        {
            std::lock_guard<std::mutex> lock(hudlessMutex);

            // if can't find output skip
            if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
            {
                LOG_DEBUG_ONLY("Early exit");
                return;
            }

            val0 = fgPossibleHudless[fIndex][This];

            do
            {
                // if this command list does not have entries skip
                if (val0.size() == 0)
                    break;

                for (auto& [key, val] : val0)
                {
                    //LOG_DEBUG("Waiting _drawMutex {:X}", (size_t)val.buffer);
                    std::lock_guard<std::mutex> lock(_drawMutex);

                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }
            } while (false);

            fgPossibleHudless[fIndex][This].clear();
        }


        LOG_DEBUG_ONLY("Clear");
    }
}

#pragma endregion

void ResTrack_Dx12::HookCommandList(ID3D12Device* InDevice)
{
    if (o_OMSetRenderTargets != nullptr)
        return;

    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;

    if (InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)) == S_OK)
    {
        if (InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)) == S_OK)
        {
            ID3D12GraphicsCommandList* realCL = nullptr;
            if (!CheckForRealObject(__FUNCTION__, commandList, (IUnknown**)&realCL))
                realCL = commandList;

            // Get the vtable pointer
            PVOID* pVTable = *(PVOID**)realCL;

            // hudless shader
            o_OMSetRenderTargets = (PFN_OMSetRenderTargets)pVTable[46];
            o_SetGraphicsRootDescriptorTable = (PFN_SetGraphicsRootDescriptorTable)pVTable[32];

            o_DrawInstanced = (PFN_DrawInstanced)pVTable[12];
            o_DrawIndexedInstanced = (PFN_DrawIndexedInstanced)pVTable[13];
            o_Dispatch = (PFN_Dispatch)pVTable[14];

            // hudless compute
            o_SetComputeRootDescriptorTable = (PFN_SetComputeRootDescriptorTable)pVTable[31];

            // hudless copy
#ifdef USE_COPY_RESOURCE
            o_CopyResource = (PFN_CopyResource)pVTable[17];
            o_CopyTextureRegion = (PFN_CopyTextureRegion)pVTable[16];
#endif

#ifdef USE_RESOURCE_DISCARD
            // release resource
            o_DiscardResource = (PFN_DiscardResource)pVTable[51];
#endif

            if (o_OMSetRenderTargets != nullptr)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (o_OMSetRenderTargets != nullptr)
                    DetourAttach(&(PVOID&)o_OMSetRenderTargets, hkOMSetRenderTargets);

#ifdef USE_COPY_RESOURCE
                if (o_CopyTextureRegion != nullptr)
                    DetourAttach(&(PVOID&)o_CopyTextureRegion, hkCopyTextureRegion);

                if (o_CopyResource != nullptr)
                    DetourAttach(&(PVOID&)o_CopyResource, hkCopyResource);
#endif


                if (o_SetGraphicsRootDescriptorTable != nullptr)
                    DetourAttach(&(PVOID&)o_SetGraphicsRootDescriptorTable, hkSetGraphicsRootDescriptorTable);

                if (o_SetComputeRootDescriptorTable != nullptr)
                    DetourAttach(&(PVOID&)o_SetComputeRootDescriptorTable, hkSetComputeRootDescriptorTable);

                if (o_DrawIndexedInstanced != nullptr)
                    DetourAttach(&(PVOID&)o_DrawIndexedInstanced, hkDrawIndexedInstanced);

                if (o_DrawInstanced != nullptr)
                    DetourAttach(&(PVOID&)o_DrawInstanced, hkDrawInstanced);

                if (o_Dispatch != nullptr)
                    DetourAttach(&(PVOID&)o_Dispatch, hkDispatch);

#ifdef USE_RESOURCE_DISCARD
                if (o_DiscardResource != nullptr)
                    DetourAttach(&(PVOID&)o_DiscardResource, hkDiscardResource);
#endif
                DetourTransactionCommit();
            }

            commandList->Close();
            commandList->Release();
        }

        commandAllocator->Reset();
        commandAllocator->Release();
    }
}

static void HookToQueue(ID3D12Device* InDevice)
{
    if (o_ExecuteCommandLists != nullptr)
        return;

    ID3D12CommandQueue* queue = nullptr;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

    auto hr = InDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));

    if (hr == S_OK)
    {
        // Get the vtable pointer
        PVOID* pVTable = *(PVOID**)queue;

        o_ExecuteCommandLists = (PFN_ExecuteCommandLists)pVTable[10];

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_ExecuteCommandLists != nullptr)
            DetourAttach(&(PVOID&)o_ExecuteCommandLists, hkExecuteCommandLists);

        DetourTransactionCommit();

        queue->Release();
    }
}


void ResTrack_Dx12::HookDevice(ID3D12Device* device)
{
    if (o_CreateDescriptorHeap != nullptr || device == nullptr)
        return;

    LOG_FUNC();

    ID3D12Device* realDevice = nullptr;
    if (!CheckForRealObject(__FUNCTION__, device, (IUnknown**)&realDevice))
        realDevice = device;

    // Get the vtable pointer
    PVOID* pVTable = *(PVOID**)realDevice;

    // hudless
    o_CreateRenderTargetView = (PFN_CreateRenderTargetView)pVTable[20];
    o_CreateDescriptorHeap = (PFN_CreateDescriptorHeap)pVTable[14];
    o_CopyDescriptors = (PFN_CopyDescriptors)pVTable[23];
    o_CopyDescriptorsSimple = (PFN_CopyDescriptorsSimple)pVTable[24];
    o_CreateShaderResourceView = (PFN_CreateShaderResourceView)pVTable[18];
    o_CreateUnorderedAccessView = (PFN_CreateUnorderedAccessView)pVTable[19];

    // Apply the detour
    if (o_CreateDescriptorHeap != nullptr || o_CreateRenderTargetView != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_CreateDescriptorHeap != nullptr)
            DetourAttach(&(PVOID&)o_CreateDescriptorHeap, hkCreateDescriptorHeap);

        if (Config::Instance()->FGUseFGSwapChain.value_or_default() && Config::Instance()->OverlayMenu.value_or_default())
        {
            if (o_CreateRenderTargetView != nullptr)
                DetourAttach(&(PVOID&)o_CreateRenderTargetView, hkCreateRenderTargetView);

            if (o_CreateShaderResourceView != nullptr)
                DetourAttach(&(PVOID&)o_CreateShaderResourceView, hkCreateShaderResourceView);

            if (o_CreateUnorderedAccessView != nullptr)
                DetourAttach(&(PVOID&)o_CreateUnorderedAccessView, hkCreateUnorderedAccessView);

            if (o_CopyDescriptors != nullptr)
                DetourAttach(&(PVOID&)o_CopyDescriptors, hkCopyDescriptors);

            if (o_CopyDescriptorsSimple != nullptr)
                DetourAttach(&(PVOID&)o_CopyDescriptorsSimple, hkCopyDescriptorsSimple);
        }

        DetourTransactionCommit();
    }

    HookCommandList(device);
}

void ResTrack_Dx12::ResetCaptureList()
{
    std::lock_guard<std::mutex> lock(hudlessMutex);
    fgCaptureList.clear();
}

void ResTrack_Dx12::ClearPossibleHudless()
{
    LOG_DEBUG("");

    std::lock_guard<std::mutex> lock(hudlessMutex);
    fgPossibleHudless->clear();
    _presentDone = false;

    _written = false;
    _fgEnabled = false;
    _currentFG = false;
    _currentFGActive = false;
    _skipHudless = false;
    _rcActive = false;
    _cmdList = false;

}

void ResTrack_Dx12::PresentDone()
{
    _presentDone = true;
}

