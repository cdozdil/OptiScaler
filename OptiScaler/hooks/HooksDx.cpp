#include "HooksDx.h"
#include "wrapped_swapchain.h"

#include <Util.h>
#include <Logger.h>
#include <Config.h>

#include <framegen/ffx/FSRFG_Dx12.h>
#include <hudfix/Hudfix_Dx12.h>
#include <menu/menu_overlay_dx.h>

#include <nvapi/fakenvapi.h>
#include <nvapi/ReflexHooks.h>

#include <detours/detours.h>

#include <d3d11on12.h>

#pragma region FG definitions

#include <ankerl/unordered_dense.h>
#include <set>
#include <future>
#include <shared_mutex>



// Clear heap info when ResourceDiscard is called
//#define USE_RESOURCE_DISCARD

// Use CopyResource & CopyTextureRegion to capture hudless
// It's a unstable
//#define USE_COPY_RESOURCE

// Use resource barriers before and after capture operation
#define USE_RESOURCE_BARRIRER

// Uses std::thread when processing descriptor heap operations
//#define USE_THREAD_FOR_COPY_DESCS

#define CHECK_FOR_SL_PROXY_OBJECTS

typedef struct SwapChainInfo
{
    IDXGISwapChain* swapChain = nullptr;
    DXGI_FORMAT swapChainFormat = DXGI_FORMAT_UNKNOWN;
    int swapChainBufferCount = 0;
    ID3D12CommandQueue* fgCommandQueue = nullptr;
    ID3D12CommandQueue* gameCommandQueue = nullptr;
};

typedef struct HeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T cpuEnd = NULL;
    SIZE_T gpuStart = NULL;
    SIZE_T gpuEnd = NULL;
    UINT numDescriptors = 0;
    UINT increment = 0;
    UINT type = 0;
    std::shared_ptr<ResourceInfo[]> info;

    HeapInfo(SIZE_T cpuStart, SIZE_T cpuEnd, SIZE_T gpuStart, SIZE_T gpuEnd, UINT numResources, UINT increment, UINT type)
        : cpuStart(cpuStart), cpuEnd(cpuEnd), gpuStart(gpuStart), gpuEnd(gpuEnd), numDescriptors(numResources), increment(increment), info(new ResourceInfo[numResources]), type(type) {}

    ResourceInfo* GetByCpuHandle(SIZE_T cpuHandle) const
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return nullptr;

        auto index = (cpuHandle - cpuStart) / increment;

        return &info[index];
    }

    ResourceInfo* GetByGpuHandle(SIZE_T gpuHandle) const
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return nullptr;

        auto index = (gpuHandle - gpuStart) / increment;

        return &info[index];
    }

    void SetByCpuHandle(SIZE_T cpuHandle, ResourceInfo setInfo) const
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return;

        auto index = (cpuHandle - cpuStart) / increment;

        info[index] = setInfo;
    }

    void SetByGpuHandle(SIZE_T gpuHandle, ResourceInfo setInfo) const
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return;

        auto index = (gpuHandle - gpuStart) / increment;

        info[index] = setInfo;
    }
} heap_info;

/*
// Vector version for lower heap usage
typedef struct HeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T cpuEnd = NULL;
    SIZE_T gpuStart = NULL;
    SIZE_T gpuEnd = NULL;
    UINT numDescriptors = 0;
    UINT increment = 0;
    UINT type = 0;
    std::vector<ResourceInfo> info;

    HeapInfo(SIZE_T cpuStart, SIZE_T cpuEnd, SIZE_T gpuStart, SIZE_T gpuEnd, UINT numResources, UINT increment, UINT type)
        : cpuStart(cpuStart), cpuEnd(cpuEnd), gpuStart(gpuStart), gpuEnd(gpuEnd), numDescriptors(numResources), increment(increment), info(numResources), type(type) {}

    ResourceInfo* GetByCpuHandle(SIZE_T cpuHandle)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return nullptr;

        auto index = (cpuHandle - cpuStart) / increment;

        return (index < info.size()) ? &info[index] : nullptr;
    }

    ResourceInfo* GetByGpuHandle(SIZE_T gpuHandle)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return nullptr;

        auto index = (gpuHandle - gpuStart) / increment;

        return (index < info.size()) ? &info[index] : nullptr;
    }

    void SetByCpuHandle(SIZE_T cpuHandle, ResourceInfo setInfo)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return;

        auto index = (cpuHandle - cpuStart) / increment;

        if (index < info.size())
        {
            setInfo.lastUsedFrame = Util::MillisecondsNow();
            info[index] = setInfo;
        }
    }

    void SetByGpuHandle(SIZE_T gpuHandle, ResourceInfo setInfo)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return;

        auto index = (gpuHandle - gpuStart) / increment;

        if (index < info.size())
        {
            setInfo.lastUsedFrame = Util::MillisecondsNow();
            info[index] = setInfo;
        }
    }
} heap_info;
*/

typedef struct ResourceHeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T gpuStart = NULL;
} resource_heap_info;

// Device hooks for FG
typedef void(*PFN_CreateRenderTargetView)(ID3D12Device* This, ID3D12Resource* pResource, D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void(*PFN_CreateShaderResourceView)(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void(*PFN_CreateUnorderedAccessView)(ID3D12Device* This, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

typedef HRESULT(*PFN_CreateDescriptorHeap)(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap);
typedef void(*PFN_CopyDescriptors)(ID3D12Device* This, UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
typedef void(*PFN_CopyDescriptorsSimple)(ID3D12Device* This, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);

typedef HRESULT(*PFN_Present)(void* This, UINT SyncInterval, UINT Flags);

// Command list hooks for FG
typedef void(*PFN_OMSetRenderTargets)(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors, BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
typedef void(*PFN_SetGraphicsRootDescriptorTable)(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
typedef void(*PFN_SetComputeRootDescriptorTable)(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
typedef void(*PFN_DrawIndexedInstanced)(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(*PFN_DrawInstanced)(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
typedef void(*PFN_Dispatch)(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

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
static PFN_Present o_FGSCPresent = nullptr;

// Original method calls for command list
static PFN_Dispatch o_Dispatch = nullptr;
static PFN_DrawInstanced o_DrawInstanced = nullptr;
static PFN_DrawIndexedInstanced o_DrawIndexedInstanced = nullptr;

static PFN_OMSetRenderTargets o_OMSetRenderTargets = nullptr;
static PFN_SetGraphicsRootDescriptorTable o_SetGraphicsRootDescriptorTable = nullptr;
static PFN_SetComputeRootDescriptorTable o_SetComputeRootDescriptorTable = nullptr;

#ifdef USE_COPY_RESOURCE
static PFN_CopyResource o_CopyResource = nullptr;
static PFN_CopyTextureRegion o_CopyTextureRegion = nullptr;
#endif

#ifdef USE_RESOURCE_DISCARD
static PFN_DiscardResource o_DiscardResource = nullptr;
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

// mutexes
static std::shared_mutex heapMutex;
static std::shared_mutex presentMutex;
static std::shared_mutex resourceMutex;
static std::shared_mutex hudlessMutex;
static std::shared_mutex captureMutex;

// Last captured upscaled hudless frame number
static bool fgPresentRunning = false;

// Used for frametime calculation
static bool fgStopAfterNextPresent = false;
static bool fgSkipSCWrapping = false;

// Swapchain frame counter
static UINT64 frameCounter = 0;

static WrappedIDXGISwapChain4* lastWrapped;

static IID streamlineRiid{};

static uint32_t fgNotAcceptedResourceFlags = D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE | D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY;

#pragma endregion                            

// dxgi stuff                                
typedef HRESULT(*PFN_CreateDXGIFactory)(REFIID riid, IDXGIFactory** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory1)(REFIID riid, IDXGIFactory1** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory2)(UINT Flags, REFIID riid, IDXGIFactory2** ppFactory);

typedef HRESULT(*PFN_EnumAdapterByGpuPreference2)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapterByLuid2)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapters12)(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
typedef HRESULT(*PFN_EnumAdapters2)(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);

typedef HRESULT(*PFN_CreateSwapChain)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**);
typedef HRESULT(*PFN_CreateSwapChainForHwnd)(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**);

static PFN_CreateDXGIFactory o_CreateDXGIFactory = nullptr;
static PFN_CreateDXGIFactory1 o_CreateDXGIFactory1 = nullptr;
static PFN_CreateDXGIFactory2 o_CreateDXGIFactory2 = nullptr;
static PFN_CreateDXGIFactory o_SL_CreateDXGIFactory = nullptr;
static PFN_CreateDXGIFactory1 o_SL_CreateDXGIFactory1 = nullptr;
static PFN_CreateDXGIFactory2 o_SL_CreateDXGIFactory2 = nullptr;

static PFN_EnumAdapters2 ptrEnumAdapters = nullptr;
static PFN_EnumAdapters12 ptrEnumAdapters1 = nullptr;
static PFN_EnumAdapterByLuid2 ptrEnumAdapterByLuid = nullptr;
static PFN_EnumAdapterByGpuPreference2 ptrEnumAdapterByGpuPreference = nullptr;

static PFN_CreateSwapChain oCreateSwapChain = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd = nullptr;

static bool skipHighPerfCheck = false;

// DirectX
typedef void(*PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef HRESULT(*PFN_CreateSamplerState)(ID3D11Device* This, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);

static PFN_D3D12_CREATE_DEVICE o_D3D12CreateDevice = nullptr;
static PFN_CreateSampler o_CreateSampler = nullptr;

static PFN_D3D11_CREATE_DEVICE o_D3D11CreateDevice = nullptr;
static PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN o_D3D11CreateDeviceAndSwapChain = nullptr;
static PFN_CreateSamplerState o_CreateSamplerState = nullptr;
static PFN_D3D11ON12_CREATE_DEVICE o_D3D11On12CreateDevice = nullptr;
static ID3D11Device* d3d11Device = nullptr;
static ID3D11Device* d3d11on12Device = nullptr;

// menu
static bool _dx11Device = false;
static bool _dx12Device = false;

// for dx11
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;

// for dx12
//static ID3D12Device* g_pd3dDeviceParam = nullptr;
//static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;

// status
static bool _isInited = false;
static bool _d3d12Captured = false;

static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static HRESULT hkCreateSamplerState(ID3D11Device* This, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);
static HRESULT hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
static HRESULT hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);

static void FfxFgLogCallback(uint32_t type, const wchar_t* message)
{
    std::wstring string(message);
    LOG_DEBUG("{}", wstring_to_string(string));
}


static bool CheckForRealObject(std::string functionName, IUnknown* pObject, IUnknown** ppRealObject)
{
#ifdef CHECK_FOR_SL_PROXY_OBJECTS
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
#endif

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

static bool IsHudFixActive()
{
    if (!Config::Instance()->FGEnabled.value_or_default() || !Config::Instance()->FGHUDFix.value_or_default())
        return false;

    if (State::Instance().currentFG == nullptr || State::Instance().currentFeature == nullptr || State::Instance().FGchanged)
        return false;

    if (!State::Instance().currentFG->IsActive())
        return false;

    if (Hudfix_Dx12::SkipHudlessChecks())
        return false;

    if (!Hudfix_Dx12::IsResourceCheckActive())
        return false;

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

    if (IsFGCommandList(This) && pRegion == nullptr)
    {
        std::unique_lock<std::shared_mutex> lock(resourceMutex);

        if (!fgHandlesByResources.contains(pResource))
            return;

        auto heapInfo = &fgHandlesByResources[pResource];
        LOG_DEBUG_ONLY(" <-- {}", heapInfo->cpuStart);

        auto heap = GetHeapByCpuHandle(heapInfo->cpuStart);
        if (heap != nullptr)
            heap->SetByCpuHandle(heapInfo->cpuStart, {});

        fgHandlesByResources.erase(pResource);
        LOG_DEBUG_ONLY("Erased");
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
        std::unique_lock<std::shared_mutex> lock(resourceMutex);
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
        std::unique_lock<std::shared_mutex> lock(resourceMutex);
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
        std::unique_lock<std::shared_mutex> lock(resourceMutex);
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


static void hkCopyDescriptors(ID3D12Device* This,
                              UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes,
                              UINT NumSrcDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes,
                              D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptors(This, NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);

    if (DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        return;

    if (!Config::Instance()->FGAlwaysTrackHeaps.value_or_default() && !IsHudFixActive())
        return;

    // make copies, just in case
    D3D12_CPU_DESCRIPTOR_HANDLE* destRangeStarts = pDestDescriptorRangeStarts;
    UINT* destRangeSizes = pDestDescriptorRangeSizes;
    D3D12_CPU_DESCRIPTOR_HANDLE* srcRangeStarts = pSrcDescriptorRangeStarts;
    UINT* srcRangeSizes = pSrcDescriptorRangeSizes;

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

                dstHeap->SetByCpuHandle(destHandle, *buffer);

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
    }

}

static void hkCopyDescriptorsSimple(ID3D12Device* This, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
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

static void hkSetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    if (BaseDescriptor.ptr == 0 || !IsHudFixActive())
    {
        o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);
        return;
    }

    LOG_DEBUG_ONLY("");

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

    capturedBuffer->state = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

    do
    {
        if (Config::Instance()->FGImmediateCapture.value_or_default() &&
            Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, capturedBuffer, capturedBuffer->state))
        {
            break;
        }

        {
            std::unique_lock<std::shared_mutex> lock(hudlessMutex);

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

static void hkOMSetRenderTargets(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
                                 BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    if (NumRenderTargetDescriptors == 0 || pRenderTargetDescriptors == nullptr || !IsHudFixActive())
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

            {
                std::unique_lock<std::shared_mutex> lock(hudlessMutex);

                auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

                // check for command list
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

static void hkSetComputeRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    if (BaseDescriptor.ptr == 0 || !IsHudFixActive())
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
            std::unique_lock<std::shared_mutex> lock(hudlessMutex);

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
static void hkDrawInstanced(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    o_DrawInstanced(This, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
        return;

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;

        {
            std::shared_lock<std::shared_mutex> lock(hudlessMutex);

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

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
                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }

            } while (false);
        }

        {
            std::unique_lock<std::shared_mutex> lock(hudlessMutex);
            val0.clear();
        }

        LOG_DEBUG_ONLY("Clear");
    }
}

static void hkDrawIndexedInstanced(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    o_DrawIndexedInstanced(This, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
        return;

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;

        {
            std::shared_lock<std::shared_mutex> lock(hudlessMutex);

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

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
                    LOG_DEBUG_ONLY("Found matching final image");

                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }

            } while (false);
        }

        {
            std::unique_lock<std::shared_mutex> lock(hudlessMutex);
            val0.clear();
        }

        LOG_DEBUG_ONLY("Clear");
    }
}

static void hkDispatch(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    o_Dispatch(This, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

    if (!IsHudFixActive())
        return;

    if (This == MenuOverlayDx::MenuCommandList() || IsFGCommandList(This))
        return;

    LOG_DEBUG_ONLY("CommandList: {:X}", (size_t)This);

    {
        ankerl::unordered_dense::map<ID3D12Resource*, ResourceInfo> val0;

        {
            std::shared_lock<std::shared_mutex> lock(hudlessMutex);

            auto fIndex = Hudfix_Dx12::ActivePresentFrame() % BUFFER_COUNT;

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
                    LOG_DEBUG_ONLY("Found matching final image");

                    if (Hudfix_Dx12::CheckForHudless(__FUNCTION__, This, &val, val.state))
                    {
                        break;
                    }
                }

            } while (false);
        }

        {
            std::unique_lock<std::shared_mutex> lock(hudlessMutex);
            val0.clear();
        }

        LOG_DEBUG_ONLY("Clear");
    }
}

#pragma endregion

#pragma region Callbacks for wrapped swapchain

static HRESULT hkFGPresent(void* This, UINT SyncInterval, UINT Flags)
{
    if (State::Instance().isShuttingDown)
    {
        auto result = o_FGSCPresent(This, SyncInterval, Flags);
        return result;
    }

    LOG_DEBUG("frameCounter: {}, flags: {:X}", frameCounter, Flags);

    IFGFeature_Dx12* fg = nullptr;
    if (State::Instance().currentFG != nullptr)
        fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

    Hudfix_Dx12::PresentStart();

    if (State::Instance().currentSwapchain == nullptr)
        return S_OK;

    if (State::Instance().FGresetCapturedResources)
    {
        LOG_DEBUG("FGResetCapturedResources");
        std::unique_lock<std::shared_mutex> lock(captureMutex);
        fgCaptureList.clear();
        State::Instance().FGcapturedResourceCount = 0;
        State::Instance().FGresetCapturedResources = false;
    }

    bool lockAccuired = false;
    if (fg != nullptr && fg->IsActive() && Config::Instance()->FGUseMutexForSwaphain.value_or_default() && fg->Mutex.getOwner() != 2)
    {
        LOG_TRACE("Waiting ffxMutex 2, current: {}", fg->Mutex.getOwner());
        fg->Mutex.lock(2);
        LOG_TRACE("Accuired ffxMutex: {}", fg->Mutex.getOwner());
        lockAccuired = true;
    }

    HRESULT result;
    result = o_FGSCPresent(This, SyncInterval, Flags);
    LOG_DEBUG("Result: {:X}", result);

    if (lockAccuired && Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", fg->Mutex.getOwner());
        fg->Mutex.unlockThis(2);
    }

    return result;
}

static HRESULT Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters, IUnknown* pDevice, HWND hWnd)
{
    // Probably not needed anymore
    //std::unique_lock<std::shared_mutex> lock(presentMutex);

    LOG_DEBUG("");

    HRESULT presentResult;

    if (State::Instance().isShuttingDown)
    {
        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        if (presentResult == S_OK)
            LOG_TRACE("1 {}", (UINT)presentResult);
        else
            LOG_ERROR("1 {:X}", (UINT)presentResult);

        return presentResult;
    }

    LOG_TRACE("{}", frameCounter);

    if (hWnd != Util::GetProcessWindow())
    {
        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        if (presentResult == S_OK)
            LOG_TRACE("2 {}", (UINT)presentResult);
        else
            LOG_ERROR("2 {:X}", (UINT)presentResult);

        return presentResult;
    }

    ID3D12CommandQueue* cq = nullptr;
    ID3D11Device* device = nullptr;
    ID3D12Device* device12 = nullptr;

    // try to obtain directx objects and find the path
    if (pDevice->QueryInterface(IID_PPV_ARGS(&device)) == S_OK)
    {
        if (!_dx11Device)
            LOG_DEBUG("D3D11Device captured");

        _dx11Device = true;
        State::Instance().swapchainApi = DX11;
    }
    else if (pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        if (!_dx12Device)
            LOG_DEBUG("D3D12CommandQueue captured");

        HooksDx::fgFSRCommandQueue = (ID3D12CommandQueue*)pDevice;
        HooksDx::fgFSRCommandQueue->SetName(L"fgFSRSwapChainQueue");
        State::Instance().swapchainApi = DX12;

        if (cq->GetDevice(IID_PPV_ARGS(&device12)) == S_OK)
        {
            if (!_dx12Device)
                LOG_DEBUG("D3D12Device captured");

            _dx12Device = true;
        }
    }

    IFGFeature_Dx12* fg = nullptr;
    if (State::Instance().currentFG != nullptr)
        fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

    if (fg != nullptr)
        ReflexHooks::update(fg->IsActive());

    // Upscaler GPU time computation
    if (HooksDx::dx12UpscaleTrig && HooksDx::readbackBuffer != nullptr && HooksDx::queryHeap != nullptr && cq != nullptr)
    {
        UINT64* timestampData;
        HooksDx::readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&timestampData));

        if (timestampData != nullptr)
        {
            // Get the GPU timestamp frequency (ticks per second)
            UINT64 gpuFrequency;
            cq->GetTimestampFrequency(&gpuFrequency);

            // Calculate elapsed time in milliseconds
            UINT64 startTime = timestampData[0];
            UINT64 endTime = timestampData[1];
            double elapsedTimeMs = (endTime - startTime) / static_cast<double>(gpuFrequency) * 1000.0;

            // filter out posibly wrong measured high values
            if (elapsedTimeMs < 100.0)
            {
                State::Instance().upscaleTimes.push_back(elapsedTimeMs);
                State::Instance().upscaleTimes.pop_front();
            }
        }
        else
        {
            LOG_WARN("timestampData is null!");
        }

        // Unmap the buffer
        HooksDx::readbackBuffer->Unmap(0, nullptr);

        HooksDx::dx12UpscaleTrig = false;
    }
    else if (HooksDx::dx11UpscaleTrig[HooksDx::currentFrameIndex] && device != nullptr && HooksDx::disjointQueries[0] != nullptr &&
             HooksDx::startQueries[0] != nullptr && HooksDx::endQueries[0] != nullptr)
    {
        if (g_pd3dDeviceContext == nullptr)
            device->GetImmediateContext(&g_pd3dDeviceContext);

        // Retrieve the results from the previous frame
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        if (g_pd3dDeviceContext->GetData(HooksDx::disjointQueries[HooksDx::previousFrameIndex], &disjointData, sizeof(disjointData), 0) == S_OK)
        {
            if (!disjointData.Disjoint && disjointData.Frequency > 0)
            {
                UINT64 startTime = 0, endTime = 0;
                if (g_pd3dDeviceContext->GetData(HooksDx::startQueries[HooksDx::previousFrameIndex], &startTime, sizeof(UINT64), 0) == S_OK &&
                    g_pd3dDeviceContext->GetData(HooksDx::endQueries[HooksDx::previousFrameIndex], &endTime, sizeof(UINT64), 0) == S_OK)
                {
                    double elapsedTimeMs = (endTime - startTime) / static_cast<double>(disjointData.Frequency) * 1000.0;

                    // filter out posibly wrong measured high values
                    if (elapsedTimeMs < 100.0)
                    {
                        State::Instance().upscaleTimes.push_back(elapsedTimeMs);
                        State::Instance().upscaleTimes.pop_front();
                    }
                }
            }
        }


        HooksDx::dx11UpscaleTrig[HooksDx::currentFrameIndex] = false;
        HooksDx::currentFrameIndex = (HooksDx::currentFrameIndex + 1) % HooksDx::QUERY_BUFFER_COUNT;
    }

    // DXVK check, it's here because of upscaler time calculations
    if (State::Instance().isRunningOnDXVK)
    {
        if (cq != nullptr)
            cq->Release();

        if (device != nullptr)
            device->Release();

        if (device12 != nullptr)
            device12->Release();

        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        if (presentResult == S_OK)
            LOG_TRACE("3 {}", (UINT)presentResult);
        else
            LOG_ERROR("3 {:X}", (UINT)presentResult);

        if (fgStopAfterNextPresent)
        {
            if (fg != nullptr)
                fg->StopAndDestroyContext(false, false, false);

            fgStopAfterNextPresent = false;
        }

        return presentResult;
    }

    MenuOverlayDx::Present(pSwapChain, SyncInterval, Flags, pPresentParameters, pDevice, hWnd);

    if (Config::Instance()->FGUseFGSwapChain.value_or_default())
    {
        fakenvapi::reportFGPresent(pSwapChain, fg != nullptr && fg->IsActive(), frameCounter % 2);
    }

    // death stranding fix???
    //if (frameCounter < 5)
    //    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    frameCounter++;

    // swapchain present
    if (pPresentParameters == nullptr)
        presentResult = pSwapChain->Present(SyncInterval, Flags);
    else
        presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

    // release used objects
    if (cq != nullptr)
        cq->Release();

    if (device != nullptr)
        device->Release();

    if (device12 != nullptr)
        device12->Release();

    if (presentResult == S_OK)
        LOG_TRACE("4 {}, Present result: {:X}", frameCounter, (UINT)presentResult);
    else
        LOG_ERROR("4 {:X}", (UINT)presentResult);

    return presentResult;
}

#pragma endregion

#pragma region DXGI hooks

static void CheckAdapter(IUnknown* unkAdapter)
{
    if (State::Instance().isRunningOnDXVK)
        return;

    //DXVK VkInterface GUID
    const GUID guid = { 0x907bf281, 0xea3c, 0x43b4, { 0xa8, 0xe4, 0x9f, 0x23, 0x11, 0x07, 0xb4, 0xff } };

    IDXGIAdapter* adapter = nullptr;
    bool adapterOk = unkAdapter->QueryInterface(IID_PPV_ARGS(&adapter)) == S_OK;

    void* dxvkAdapter = nullptr;
    if (adapterOk && adapter->QueryInterface(guid, &dxvkAdapter) == S_OK)
    {
        State::Instance().isRunningOnDXVK = dxvkAdapter != nullptr;
        ((IDXGIAdapter*)dxvkAdapter)->Release();
    }

    if (adapterOk)
        adapter->Release();
}

static void AttachToFactory(IUnknown* unkFactory)
{
    PVOID* pVTable = *(PVOID**)unkFactory;

    IDXGIFactory* factory;
    if (ptrEnumAdapters == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory)) == S_OK)
    {
        LOG_DEBUG("Hooking EnumAdapters");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters = (PFN_EnumAdapters2)pVTable[7];

        if (ptrEnumAdapters != nullptr)
            DetourAttach(&(PVOID&)ptrEnumAdapters, hkEnumAdapters);

        DetourTransactionCommit();

        factory->Release();
    }

    IDXGIFactory1* factory1;
    if (ptrEnumAdapters1 == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory1)) == S_OK)
    {
        LOG_DEBUG("Hooking EnumAdapters1");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters1 = (PFN_EnumAdapters12)pVTable[12];

        if (ptrEnumAdapters1 != nullptr)
            DetourAttach(&(PVOID&)ptrEnumAdapters1, hkEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (ptrEnumAdapterByLuid == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
    {
        LOG_DEBUG("Hooking EnumAdapterByLuid");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByLuid = (PFN_EnumAdapterByLuid2)pVTable[26];

        if (ptrEnumAdapterByLuid != nullptr)
            DetourAttach(&(PVOID&)ptrEnumAdapterByLuid, hkEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (ptrEnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK)
    {
        LOG_DEBUG("Hooking EnumAdapterByGpuPreference");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference2)pVTable[29];

        if (ptrEnumAdapterByGpuPreference != nullptr)
            DetourAttach(&(PVOID&)ptrEnumAdapterByGpuPreference, hkEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

static HRESULT hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    LOG_FUNC();

    *ppSwapChain = nullptr;

    if (State::Instance().vulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");

        if (pDesc != nullptr)
            LOG_DEBUG("Width: {}, Height: {}, Format: {:X}, Count: {}, Hwnd: {:X}, Windowed: {}, SkipWrapping: {}",
                      pDesc->BufferDesc.Width, pDesc->BufferDesc.Height, (UINT)pDesc->BufferDesc.Format, pDesc->BufferCount, (UINT)pDesc->OutputWindow, pDesc->Windowed, fgSkipSCWrapping);

        State::Instance().skipDxgiLoadChecks = true;
        auto res = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
        return res;
    }

    if (pDevice == nullptr || pDesc == nullptr)
    {
        LOG_WARN("pDevice or pDesc is nullptr!");
        State::Instance().skipDxgiLoadChecks = true;
        auto res = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
        return res;
    }

    if (pDesc->BufferDesc.Height < 100 || pDesc->BufferDesc.Width < 100)
    {
        LOG_WARN("Overlay call!");
        State::Instance().skipDxgiLoadChecks = true;
        auto res = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
        return res;
    }

    LOG_DEBUG("Width: {}, Height: {}, Format: {:X}, Count: {}, Hwnd: {:X}, Windowed: {}, SkipWrapping: {}",
              pDesc->BufferDesc.Width, pDesc->BufferDesc.Height, (UINT)pDesc->BufferDesc.Format, pDesc->BufferCount, (UINT)pDesc->OutputWindow, pDesc->Windowed, fgSkipSCWrapping);

    // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
    // https://github.com/EndlesslyFlowering/AutoHDR-ReShade
    if (Config::Instance()->ForceHDR.value_or_default() && !fgSkipSCWrapping)
    {
        LOG_INFO("Force HDR on");

        if (Config::Instance()->UseHDR10.value_or_default())
        {
            LOG_INFO("Using HDR10");
            pDesc->BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        }
        else
        {
            LOG_INFO("Not using HDR10");
            pDesc->BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        }

        if (pDesc->BufferCount < 2)
            pDesc->BufferCount = 2;

        pDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        pDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    ID3D12CommandQueue* cq = nullptr;
    if (Config::Instance()->OverlayMenu.value_or_default() && Config::Instance()->FGUseFGSwapChain.value_or_default() &&
        !fgSkipSCWrapping && FfxApiProxy::InitFfxDx12() && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        cq->SetName(L"GameQueue");
        cq->Release();

        // FG Init
        if (State::Instance().currentFG == nullptr)
            State::Instance().currentFG = new FSRFG_Dx12();

        HooksDx::ReleaseDx12SwapChain(pDesc->OutputWindow);

        auto fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

        ID3D12CommandQueue* real = nullptr;
        if (!CheckForRealObject(__FUNCTION__, pDevice, (IUnknown**)&real))
            real = (ID3D12CommandQueue*)pDevice;

        fgSkipSCWrapping = true;
        State::Instance().skipHeapCapture = true;
        State::Instance().skipDxgiLoadChecks = true;

        auto scResult = fg->CreateSwapchain(pFactory, real, pDesc, ppSwapChain);

        State::Instance().skipDxgiLoadChecks = false;
        State::Instance().skipHeapCapture = false;
        fgSkipSCWrapping = false;

        if (scResult)
        {
            if (o_FGSCPresent == nullptr && *ppSwapChain != nullptr)
            {
                void** pFactoryVTable = *reinterpret_cast<void***>(*ppSwapChain);

                o_FGSCPresent = (PFN_Present)pFactoryVTable[8];

                if (o_FGSCPresent != nullptr)
                {
                    LOG_INFO("Hooking FSR FG SwapChain present");

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    DetourAttach(&(PVOID&)o_FGSCPresent, hkFGPresent);

                    DetourTransactionCommit();
                }
            }

            State::Instance().SCbuffers.clear();
            for (size_t i = 0; i < 3; i++)
            {
                IUnknown* buffer;
                if ((*ppSwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                {
                    State::Instance().SCbuffers.push_back(buffer);
                    buffer->Release();
                }
            }

            if (Config::Instance()->ForceHDR.value_or_default())
            {
                IDXGISwapChain3* sc3 = nullptr;

                do
                {
                    if ((*ppSwapChain)->QueryInterface(IID_PPV_ARGS(&sc3)) == S_OK)
                    {
                        DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

                        if (Config::Instance()->UseHDR10.value_or_default())
                            hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

                        UINT css = 0;

                        auto result = sc3->CheckColorSpaceSupport(hdrCS, &css);

                        if (result != S_OK)
                        {
                            LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                            break;
                        }

                        if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
                        {
                            result = sc3->SetColorSpace1(hdrCS);

                            if (result != S_OK)
                            {
                                LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                                break;
                            }
                        }

                        LOG_INFO("HDR format and color space are set");
                    }

                } while (false);

                if (sc3 != nullptr)
                    sc3->Release();
            }

            State::Instance().currentSwapchain = (*ppSwapChain);

            LOG_DEBUG("Created FSR-FG swapchain");
            return S_OK;
        }

        LOG_ERROR("D3D12_CreateContext error: {}", scResult);

        return E_INVALIDARG;
    }

    State::Instance().skipDxgiLoadChecks = true;
    auto result = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
    {
        // check for SL proxy
        IDXGISwapChain* realSC = nullptr;
        if (!CheckForRealObject(__FUNCTION__, *ppSwapChain, (IUnknown**)&realSC))
            realSC = *ppSwapChain;

        IUnknown* readDevice = nullptr;
        if (!CheckForRealObject(__FUNCTION__, pDevice, (IUnknown**)&readDevice))
            readDevice = pDevice;

        if (Util::GetProcessWindow() == pDesc->OutputWindow)
        {
            State::Instance().screenWidth = pDesc->BufferDesc.Width;
            State::Instance().screenHeight = pDesc->BufferDesc.Height;
        }

        LOG_DEBUG("Created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDesc->OutputWindow);

        lastWrapped = new WrappedIDXGISwapChain4(realSC, readDevice, pDesc->OutputWindow, Present, MenuOverlayDx::CleanupRenderTarget, HooksDx::ReleaseDx12SwapChain);
        *ppSwapChain = lastWrapped;

        if (!fgSkipSCWrapping)
            State::Instance().currentSwapchain = lastWrapped;

        LOG_DEBUG("Created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);

        // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
        // https://github.com/EndlesslyFlowering/AutoHDR-ReShade
        if (Config::Instance()->ForceHDR.value_or_default())
        {
            if (!fgSkipSCWrapping)
            {
                State::Instance().SCbuffers.clear();
                for (size_t i = 0; i < pDesc->BufferCount; i++)
                {
                    IUnknown* buffer;
                    if ((*ppSwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                    {
                        State::Instance().SCbuffers.push_back(buffer);
                        buffer->Release();
                    }
                }
            }

            IDXGISwapChain3* sc3 = nullptr;
            do
            {
                if ((*ppSwapChain)->QueryInterface(IID_PPV_ARGS(&sc3)) == S_OK)
                {
                    DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

                    if (Config::Instance()->UseHDR10.value_or_default())
                        hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

                    UINT css = 0;

                    auto result = sc3->CheckColorSpaceSupport(hdrCS, &css);

                    if (result != S_OK)
                    {
                        LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                        break;
                    }

                    if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
                    {
                        result = sc3->SetColorSpace1(hdrCS);

                        if (result != S_OK)
                        {
                            LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                            break;
                        }
                    }

                    LOG_INFO("HDR format and color space are set");
                }

            } while (false);

            if (sc3 != nullptr)
                sc3->Release();
        }
    }

    return result;
}

static HRESULT hkCreateSwapChainForHwnd(IDXGIFactory* This, IUnknown* pDevice, HWND hWnd, DXGI_SWAP_CHAIN_DESC1* pDesc,
                                        DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    LOG_FUNC();

    *ppSwapChain = nullptr;

    if (State::Instance().vulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");
        State::Instance().skipDxgiLoadChecks = true;
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
    }

    if (pDevice == nullptr || pDesc == nullptr)
    {
        LOG_WARN("pDevice or pDesc is nullptr!");
        State::Instance().skipDxgiLoadChecks = true;
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
    }

    if (pDesc->Height < 100 || pDesc->Width < 100)
    {
        LOG_WARN("Overlay call!");
        State::Instance().skipDxgiLoadChecks = true;
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
        State::Instance().skipDxgiLoadChecks = false;
    }

    LOG_DEBUG("Width: {}, Height: {}, Format: {:X}, Count: {}, Hwnd: {:X}, SkipWrapping: {}",
              pDesc->Width, pDesc->Height, (UINT)pDesc->Format, pDesc->BufferCount, (UINT)hWnd, fgSkipSCWrapping);

    if (Config::Instance()->ForceHDR.value_or_default() && !fgSkipSCWrapping)
    {
        LOG_INFO("Force HDR on");

        if (Config::Instance()->UseHDR10.value_or_default())
        {
            LOG_INFO("Using HDR10");
            pDesc->Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        }
        else
        {
            LOG_INFO("Not using HDR10");
            pDesc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        }

        if (pDesc->BufferCount < 2)
            pDesc->BufferCount = 2;

        pDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        pDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    ID3D12CommandQueue* cq = nullptr;
    if (Config::Instance()->FGUseFGSwapChain.value_or_default() && !fgSkipSCWrapping && FfxApiProxy::InitFfxDx12() && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        cq->SetName(L"GameQueueHwnd");
        cq->Release();

        // FG Init
        if (State::Instance().currentFG == nullptr)
            State::Instance().currentFG = new FSRFG_Dx12();

        HooksDx::ReleaseDx12SwapChain(hWnd);

        auto fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

        ID3D12CommandQueue* real = nullptr;
        if (!CheckForRealObject(__FUNCTION__, pDevice, (IUnknown**)&real))
            real = (ID3D12CommandQueue*)pDevice;

        fgSkipSCWrapping = true;
        State::Instance().skipHeapCapture = true;
        State::Instance().skipDxgiLoadChecks = true;

        auto scResult = fg->CreateSwapchain1(This, real, hWnd, pDesc, pFullscreenDesc, ppSwapChain);

        State::Instance().skipDxgiLoadChecks = false;
        State::Instance().skipHeapCapture = false;
        fgSkipSCWrapping = false;

        if (scResult)
        {
            if (o_FGSCPresent == nullptr && *ppSwapChain != nullptr)
            {
                void** pFactoryVTable = *reinterpret_cast<void***>(*ppSwapChain);

                o_FGSCPresent = (PFN_Present)pFactoryVTable[8];

                if (o_FGSCPresent != nullptr)
                {
                    LOG_INFO("Hooking FSR FG SwapChain present");

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    DetourAttach(&(PVOID&)o_FGSCPresent, hkFGPresent);

                    DetourTransactionCommit();
                }
            }

            State::Instance().SCbuffers.clear();
            for (size_t i = 0; i < 3; i++)
            {
                IUnknown* buffer;
                if ((*ppSwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                {
                    State::Instance().SCbuffers.push_back(buffer);
                    buffer->Release();
                }
            }

            if (Config::Instance()->ForceHDR.value_or_default())
            {
                IDXGISwapChain3* sc3 = nullptr;

                do
                {
                    if ((*ppSwapChain)->QueryInterface(IID_PPV_ARGS(&sc3)) == S_OK)
                    {
                        DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

                        if (Config::Instance()->UseHDR10.value_or_default())
                            hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

                        UINT css = 0;

                        auto result = sc3->CheckColorSpaceSupport(hdrCS, &css);

                        if (result != S_OK)
                        {
                            LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                            break;
                        }

                        if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
                        {
                            result = sc3->SetColorSpace1(hdrCS);

                            if (result != S_OK)
                            {
                                LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                                break;
                            }
                        }

                        LOG_INFO("HDR format and color space are set");
                    }

                } while (false);

                if (sc3 != nullptr)
                    sc3->Release();
            }

            State::Instance().currentSwapchain = (*ppSwapChain);

            LOG_DEBUG("Created FSR-FG swapchain");
            return S_OK;
        }

        LOG_ERROR("D3D12_CreateContext error: {}", scResult);
        return E_INVALIDARG;
    }

    State::Instance().skipDxgiLoadChecks = true;
    auto result = oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
    {
        // check for SL proxy
        IDXGISwapChain1* realSC = nullptr;
        if (!CheckForRealObject(__FUNCTION__, *ppSwapChain, (IUnknown**)&realSC))
            realSC = *ppSwapChain;

        IUnknown* readDevice = nullptr;
        if (!CheckForRealObject(__FUNCTION__, pDevice, (IUnknown**)&readDevice))
            readDevice = pDevice;

        if (Util::GetProcessWindow() == hWnd)
        {
            State::Instance().screenWidth = pDesc->Width;
            State::Instance().screenHeight = pDesc->Height;
        }

        LOG_DEBUG("Created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)hWnd);
        lastWrapped = new WrappedIDXGISwapChain4(realSC, readDevice, hWnd, Present, MenuOverlayDx::CleanupRenderTarget, HooksDx::ReleaseDx12SwapChain);
        *ppSwapChain = lastWrapped;
        LOG_DEBUG("Created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);

        if (!fgSkipSCWrapping)
            State::Instance().currentSwapchain = lastWrapped;

        if (Config::Instance()->ForceHDR.value_or_default())
        {
            if (!fgSkipSCWrapping)
            {
                State::Instance().SCbuffers.clear();
                for (size_t i = 0; i < pDesc->BufferCount; i++)
                {
                    IUnknown* buffer;
                    if ((*ppSwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                    {
                        State::Instance().SCbuffers.push_back(buffer);
                        buffer->Release();
                    }
                }
            }

            IDXGISwapChain3* sc3 = nullptr;
            do
            {
                if ((*ppSwapChain)->QueryInterface(IID_PPV_ARGS(&sc3)) == S_OK)
                {
                    DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

                    if (Config::Instance()->UseHDR10.value_or_default())
                        hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

                    UINT css = 0;

                    auto result = sc3->CheckColorSpaceSupport(hdrCS, &css);

                    if (result != S_OK)
                    {
                        LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                        break;
                    }

                    if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
                    {
                        result = sc3->SetColorSpace1(hdrCS);

                        if (result != S_OK)
                        {
                            LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                            break;
                        }
                    }

                    LOG_INFO("HDR format and color space are set");
                }

            } while (false);

            if (sc3 != nullptr)
                sc3->Release();
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory(REFIID riid, IDXGIFactory** ppFactory)
{
#ifndef ENABLE_DEBUG_LAYER_DX12
    State::Instance().skipDxgiLoadChecks = true;
    auto result = o_CreateDXGIFactory(riid, ppFactory);
    State::Instance().skipDxgiLoadChecks = false;
#else
    auto result = o_CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, riid, (IDXGIFactory2**)ppFactory);
#endif

    if (result != S_OK)
        return result;

    IDXGIFactory* real = nullptr;
    if (!CheckForRealObject(__FUNCTION__, *ppFactory, (IUnknown**)&real))
        real = *ppFactory;

    AttachToFactory(real);

    if (oCreateSwapChain == nullptr)
    {
        void** pFactoryVTable = *reinterpret_cast<void***>(real);

        oCreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];

        if (oCreateSwapChain != nullptr)
        {
            LOG_INFO("Hooking native DXGIFactory");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);

            DetourTransactionCommit();
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory1(REFIID riid, IDXGIFactory1** ppFactory)
{
#ifndef ENABLE_DEBUG_LAYER_DX12
    State::Instance().skipDxgiLoadChecks = true;
    auto result = o_CreateDXGIFactory1(riid, ppFactory);
    State::Instance().skipDxgiLoadChecks = false;
#else
    auto result = o_CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, riid, (IDXGIFactory2**)ppFactory);
#endif

    if (result != S_OK)
        return result;

    IDXGIFactory1* real = nullptr;
    if (!CheckForRealObject(__FUNCTION__, *ppFactory, (IUnknown**)&real))
        real = *ppFactory;

    AttachToFactory(real);

    if (oCreateSwapChainForHwnd == nullptr)
    {
        void** pFactoryVTable = *reinterpret_cast<void***>(real);

        bool skip = false;

        if (oCreateSwapChain == nullptr)
            oCreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
        else
            skip = true;

        oCreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

        if (oCreateSwapChainForHwnd != nullptr)
        {
            LOG_INFO("Hooking native DXGIFactory1");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (!skip)
                DetourAttach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);

            DetourAttach(&(PVOID&)oCreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

            DetourTransactionCommit();
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory2(UINT Flags, REFIID riid, IDXGIFactory2** ppFactory)
{
#ifndef ENABLE_DEBUG_LAYER_DX12
    State::Instance().skipDxgiLoadChecks = true;
    auto result = o_CreateDXGIFactory2(Flags, riid, ppFactory);
    State::Instance().skipDxgiLoadChecks = false;
#else
    auto result = o_CreateDXGIFactory2(Flags | DXGI_CREATE_FACTORY_DEBUG, riid, ppFactory);
#endif

    if (result != S_OK)
        return result;

    IDXGIFactory2* real = nullptr;
    if (!CheckForRealObject(__FUNCTION__, *ppFactory, (IUnknown**)&real))
        real = *ppFactory;

    AttachToFactory(real);

    if (oCreateSwapChainForHwnd == nullptr)
    {
        void** pFactoryVTable = *reinterpret_cast<void***>(real);

        bool skip = false;

        if (oCreateSwapChain == nullptr)
            oCreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
        else
            skip = true;

        oCreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

        if (oCreateSwapChainForHwnd != nullptr)
        {
            LOG_INFO("Hooking native DXGIFactory2");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (!skip)
                DetourAttach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);

            DetourAttach(&(PVOID&)oCreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

            DetourTransactionCommit();
        }
    }

    return result;
}

static HRESULT hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter)
{
    LOG_FUNC();

    State::Instance().skipDxgiLoadChecks = true;
    auto result = ptrEnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, ppvAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter)
{
    LOG_FUNC();

    State::Instance().skipDxgiLoadChecks = true;
    auto result = ptrEnumAdapterByLuid(This, AdapterLuid, riid, ppvAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter)
{
    LOG_TRACE("HooksDx");

    State::Instance().skipDxgiLoadChecks = true;
    HRESULT result = ptrEnumAdapters1(This, Adapter, ppAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
}

static HRESULT hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter)
{
    LOG_FUNC();

    State::Instance().skipDxgiLoadChecks = true;
    HRESULT result = ptrEnumAdapters(This, Adapter, ppAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
}

#pragma endregion

#pragma region DirectX hooks

static void HookCommandList(ID3D12Device* InDevice)
{
    if (o_OMSetRenderTargets != nullptr)
        return;

    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;

    if (InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)) == S_OK)
    {
        if (InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)) == S_OK)
        {
            // Get the vtable pointer
            PVOID* pVTable = *(PVOID**)commandList;

            ID3D12GraphicsCommandList* realCL = nullptr;
            if (CheckForRealObject(__FUNCTION__, commandList, (IUnknown**)&realCL))
                pVTable = *(PVOID**)realCL;

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

static void HookToDevice(ID3D12Device* InDevice)
{
    if (o_CreateSampler != nullptr || InDevice == nullptr)
        return;

    LOG_FUNC();

    // Get the vtable pointer
    PVOID* pVTable = *(PVOID**)InDevice;

    ID3D12Device* realDevice = nullptr;
    if (CheckForRealObject(__FUNCTION__, InDevice, (IUnknown**)&realDevice))
        PVOID* pVTable = *(PVOID**)realDevice;

    // hudless
    o_CreateSampler = (PFN_CreateSampler)pVTable[22];
    o_CreateRenderTargetView = (PFN_CreateRenderTargetView)pVTable[20];
    o_CreateDescriptorHeap = (PFN_CreateDescriptorHeap)pVTable[14];
    o_CopyDescriptors = (PFN_CopyDescriptors)pVTable[23];
    o_CopyDescriptorsSimple = (PFN_CopyDescriptorsSimple)pVTable[24];
    o_CreateShaderResourceView = (PFN_CreateShaderResourceView)pVTable[18];
    o_CreateUnorderedAccessView = (PFN_CreateUnorderedAccessView)pVTable[19];

    // Apply the detour
    if (o_CreateSampler != nullptr || o_CreateRenderTargetView != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_CreateDescriptorHeap != nullptr)
            DetourAttach(&(PVOID&)o_CreateDescriptorHeap, hkCreateDescriptorHeap);

        if (o_CreateSampler != nullptr)
            DetourAttach(&(PVOID&)o_CreateSampler, hkCreateSampler);

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

    if (Config::Instance()->FGUseFGSwapChain.value_or_default() && Config::Instance()->OverlayMenu.value_or_default())
        HookCommandList(InDevice);
}

static void HookToDevice(ID3D11Device* InDevice)
{
    if (o_CreateSamplerState != nullptr || InDevice == nullptr)
        return;

    LOG_FUNC();

    // Get the vtable pointer
    PVOID* pVTable = *(PVOID**)InDevice;

    o_CreateSamplerState = (PFN_CreateSamplerState)pVTable[23];

    // Apply the detour
    if (o_CreateSamplerState != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_CreateSamplerState, hkCreateSamplerState);

        DetourTransactionCommit();
    }
}

static HRESULT hkD3D11On12CreateDevice(IUnknown* pDevice, UINT Flags, D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, IUnknown** ppCommandQueues,
                                       UINT NumQueues, UINT NodeMask, ID3D11Device** ppDevice, ID3D11DeviceContext** ppImmediateContext, D3D_FEATURE_LEVEL* pChosenFeatureLevel)
{
    LOG_FUNC();

#ifdef ENABLE_DEBUG_LAYER_DX11
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    bool rtss = false;

    // Assuming RTSS is creating a D3D11on12 device, not sure why but sometimes RTSS tries to create 
    // it's D3D11on12 device with old CommandQueue which results crash
    // I am changing it's CommandQueue with current swapchain's command queue
    if (HooksDx::gameCommandQueue != nullptr && *ppCommandQueues != HooksDx::gameCommandQueue && GetModuleHandle(L"RTSSHooks64.dll") != nullptr && pDevice == State::Instance().currentD3D12Device)
    {
        LOG_INFO("Replaced RTSS CommandQueue with correct one {0:X} -> {1:X}", (UINT64)*ppCommandQueues, (UINT64)HooksDx::gameCommandQueue);
        *ppCommandQueues = HooksDx::gameCommandQueue;
        rtss = true;
    }

    auto result = o_D3D11On12CreateDevice(pDevice, Flags, pFeatureLevels, FeatureLevels, ppCommandQueues, NumQueues, NodeMask, ppDevice, ppImmediateContext, pChosenFeatureLevel);

    if (result == S_OK && *ppDevice != nullptr && !rtss && !_d3d12Captured)
    {
        LOG_INFO("Device captured, D3D11Device: {0:X}", (UINT64)*ppDevice);
        d3d11on12Device = *ppDevice;
        HookToDevice(d3d11on12Device);
    }

    if (result == S_OK && *ppDevice != nullptr)
        State::Instance().d3d11Devices.push_back(*ppDevice);

    LOG_FUNC_RESULT(result);

    return result;
}

static HRESULT hkD3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                   UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
    LOG_FUNC();

#ifdef ENABLE_DEBUG_LAYER_DX11
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL levels[] = {
     D3D_FEATURE_LEVEL_11_1,
    };

    D3D_FEATURE_LEVEL maxLevel = D3D_FEATURE_LEVEL_1_0_CORE;

    for (UINT i = 0; i < FeatureLevels; ++i)
    {
        maxLevel = std::max(maxLevel, pFeatureLevels[i]);
    }

    if (maxLevel == D3D_FEATURE_LEVEL_11_0)
    {
        LOG_INFO("Overriding D3D_FEATURE_LEVEL, Game requested D3D_FEATURE_LEVEL_11_0, we need D3D_FEATURE_LEVEL_11_1!");
        pFeatureLevels = levels;
        FeatureLevels = ARRAYSIZE(levels);
    }

    //State::Instance().skipSpoofing = true;
    auto result = o_D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
    //auto result = o_D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
    //State::Instance().skipSpoofing = false;

    if (result == S_OK && *ppDevice != nullptr && !_d3d12Captured)
    {
        LOG_INFO("Device captured");
        d3d11Device = *ppDevice;

        HookToDevice(d3d11Device);
    }

    LOG_FUNC_RESULT(result);

    if (result == S_OK && ppDevice != nullptr && *ppDevice != nullptr)
        State::Instance().d3d11Devices.push_back(*ppDevice);

    return result;
}

static HRESULT hkD3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                               UINT FeatureLevels, UINT SDKVersion, DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
    LOG_FUNC();

#ifdef ENABLE_DEBUG_LAYER_DX11
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    };

    D3D_FEATURE_LEVEL maxLevel = D3D_FEATURE_LEVEL_1_0_CORE;

    for (UINT i = 0; i < FeatureLevels; ++i)
    {
        maxLevel = std::max(maxLevel, pFeatureLevels[i]);
    }

    if (maxLevel == D3D_FEATURE_LEVEL_11_0)
    {
        LOG_INFO("Overriding D3D_FEATURE_LEVEL, Game requested D3D_FEATURE_LEVEL_11_0, we need D3D_FEATURE_LEVEL_11_1!");
        pFeatureLevels = levels;
        FeatureLevels = ARRAYSIZE(levels);
    }

    if (pSwapChainDesc != nullptr && pSwapChainDesc->BufferDesc.Height == 2 && pSwapChainDesc->BufferDesc.Width == 2)
    {
        LOG_WARN("Overlay call!");
        //State::Instance().skipSpoofing = true;
        auto result = o_D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
        //State::Instance().skipSpoofing = false;
        return result;
    }

    // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
    // https://github.com/EndlesslyFlowering/AutoHDR-ReShade    
    if (pSwapChainDesc != nullptr && Config::Instance()->ForceHDR.value_or_default())
    {
        LOG_INFO("Force HDR on");

        if (Config::Instance()->UseHDR10.value_or_default())
        {
            LOG_INFO("Using HDR10");
            pSwapChainDesc->BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        }
        else
        {
            LOG_INFO("Not using HDR10");
            pSwapChainDesc->BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        }

        if (pSwapChainDesc->BufferCount < 2)
            pSwapChainDesc->BufferCount = 2;

        pSwapChainDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }


    //State::Instance().skipSpoofing = true;
    auto result = o_D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
    //State::Instance().skipSpoofing = false;

    if (result == S_OK && *ppDevice != nullptr && !_d3d12Captured)
    {
        LOG_INFO("Device captured");
        d3d11Device = *ppDevice;
        HookToDevice(d3d11Device);
    }

    if (result == S_OK && pSwapChainDesc != nullptr && ppSwapChain != nullptr && *ppSwapChain != nullptr && ppDevice != nullptr && *ppDevice != nullptr)
    {
        // check for SL proxy
        IDXGISwapChain* realSC = nullptr;
        if (!CheckForRealObject(__FUNCTION__, *ppSwapChain, (IUnknown**)&realSC))
            realSC = *ppSwapChain;

        IUnknown* readDevice = nullptr;
        if (!CheckForRealObject(__FUNCTION__, *ppDevice, (IUnknown**)&readDevice))
            readDevice = *ppDevice;

        if (Util::GetProcessWindow() == pSwapChainDesc->OutputWindow)
        {
            State::Instance().screenWidth = pSwapChainDesc->BufferDesc.Width;
            State::Instance().screenHeight = pSwapChainDesc->BufferDesc.Height;
        }

        LOG_DEBUG("Created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)pSwapChainDesc->OutputWindow);
        lastWrapped = new WrappedIDXGISwapChain4(realSC, readDevice, pSwapChainDesc->OutputWindow, Present, MenuOverlayDx::CleanupRenderTarget, HooksDx::ReleaseDx12SwapChain);
        *ppSwapChain = lastWrapped;

        if (!fgSkipSCWrapping)
            State::Instance().currentSwapchain = lastWrapped;

        LOG_DEBUG("Created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)*ppDevice);

        // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
        // https://github.com/EndlesslyFlowering/AutoHDR-ReShade
        if (Config::Instance()->ForceHDR.value_or_default())
        {
            if (!fgSkipSCWrapping)
            {
                State::Instance().SCbuffers.clear();
                for (size_t i = 0; i < pSwapChainDesc->BufferCount; i++)
                {
                    IUnknown* buffer;
                    if ((*ppSwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                    {
                        State::Instance().SCbuffers.push_back(buffer);
                        buffer->Release();
                    }
                }
            }

            IDXGISwapChain3* sc3 = nullptr;
            do
            {
                if ((*ppSwapChain)->QueryInterface(IID_PPV_ARGS(&sc3)) == S_OK)
                {
                    DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

                    if (Config::Instance()->UseHDR10.value_or_default())
                        hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

                    UINT css = 0;

                    auto result = sc3->CheckColorSpaceSupport(hdrCS, &css);

                    if (result != S_OK)
                    {
                        LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                        break;
                    }

                    if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
                    {
                        result = sc3->SetColorSpace1(hdrCS);

                        if (result != S_OK)
                        {
                            LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                            break;
                        }
                    }

                    LOG_INFO("HDR format and color space are set");
                }

            } while (false);

            if (sc3 != nullptr)
                sc3->Release();
        }
    }

    if (result == S_OK && ppDevice != nullptr && *ppDevice != nullptr)
        State::Instance().d3d11Devices.push_back(*ppDevice);

    LOG_FUNC_RESULT(result);
    return result;
}

#ifdef ENABLE_DEBUG_LAYER_DX12
static ID3D12Debug3* debugController = nullptr;
static ID3D12InfoQueue* infoQueue = nullptr;
static ID3D12InfoQueue1* infoQueue1 = nullptr;

static void CALLBACK D3D12DebugCallback(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext)
{
    LOG_DEBUG("Category: {}, Severity: {}, ID: {}, Message: {}", (UINT)Category, (UINT)Severity, (UINT)ID, pDescription);
}
#endif

static HRESULT hkD3D12CreateDevice(IDXGIAdapter* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    LOG_FUNC();

#ifdef ENABLE_DEBUG_LAYER_DX12
    LOG_WARN("Debug layers active!");
    if (debugController == nullptr && D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK)
    {
        debugController->EnableDebugLayer();

#ifdef ENABLE_GPU_VALIDATION
        LOG_WARN("GPU Based Validation active!");
        debugController->SetEnableGPUBasedValidation(TRUE);
#endif
    }
#endif

    //State::Instance().skipSpoofing = true;
    auto result = o_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
    //State::Instance().skipSpoofing = false;

    if (result == S_OK && ppDevice != nullptr && *ppDevice != nullptr)
    {
        LOG_DEBUG("Device captured: {0:X}", (size_t)*ppDevice);
        State::Instance().currentD3D12Device = (ID3D12Device*)*ppDevice;
        HookToDevice(State::Instance().currentD3D12Device);
        _d3d12Captured = true;

        State::Instance().d3d12Devices.push_back((ID3D12Device*)*ppDevice);

#ifdef ENABLE_DEBUG_LAYER_DX12
        if (infoQueue != nullptr)
            infoQueue->Release();

        if (infoQueue1 != nullptr)
            infoQueue1->Release();

        if (g_pd3dDeviceParam->QueryInterface(IID_PPV_ARGS(&infoQueue)) == S_OK)
        {
            LOG_DEBUG("infoQueue accuired");

            infoQueue->ClearRetrievalFilter();
            infoQueue->SetMuteDebugOutput(false);

            HRESULT res;
            res = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            //res = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            //res = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            if (infoQueue->QueryInterface(IID_PPV_ARGS(&infoQueue1)) == S_OK && infoQueue1 != nullptr)
            {
                LOG_DEBUG("infoQueue1 accuired, registering MessageCallback");
                res = infoQueue1->RegisterMessageCallback(D3D12DebugCallback, D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, NULL, NULL);
            }
        }
#endif
    }

    LOG_DEBUG("result: {:X}", (UINT)result);

    return result;
}

static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    if (pDesc == nullptr || device == nullptr)
        return;

    D3D12_SAMPLER_DESC newDesc{};

    newDesc.AddressU = pDesc->AddressU;
    newDesc.AddressV = pDesc->AddressV;
    newDesc.AddressW = pDesc->AddressW;
    newDesc.BorderColor[0] = pDesc->BorderColor[0];
    newDesc.BorderColor[1] = pDesc->BorderColor[1];
    newDesc.BorderColor[2] = pDesc->BorderColor[2];
    newDesc.BorderColor[3] = pDesc->BorderColor[3];
    newDesc.ComparisonFunc = pDesc->ComparisonFunc;

    if (Config::Instance()->AnisotropyOverride.has_value() &&
        (pDesc->Filter == D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT || pDesc->Filter == D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT ||
        pDesc->Filter == D3D12_FILTER_MIN_MAG_MIP_LINEAR || pDesc->Filter == D3D12_FILTER_ANISOTROPIC))
    {
        LOG_DEBUG("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pDesc->Filter);
        newDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        newDesc.MaxAnisotropy = Config::Instance()->AnisotropyOverride.value();
    }
    else
    {
        newDesc.Filter = pDesc->Filter;
        newDesc.MaxAnisotropy = pDesc->MaxAnisotropy;
    }

    newDesc.MaxLOD = pDesc->MaxLOD;
    newDesc.MinLOD = pDesc->MinLOD;
    newDesc.MipLODBias = pDesc->MipLODBias;

    if (newDesc.MipLODBias < 0.0f || Config::Instance()->MipmapBiasOverrideAll.value_or_default())
    {
        if (Config::Instance()->MipmapBiasOverride.has_value())
        {
            LOG_DEBUG("Overriding mipmap bias {0} -> {1}", pDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());

            if (Config::Instance()->MipmapBiasFixedOverride.value_or_default())
                newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();
            else if (Config::Instance()->MipmapBiasScaleOverride.value_or_default())
                newDesc.MipLODBias = newDesc.MipLODBias * Config::Instance()->MipmapBiasOverride.value();
            else
                newDesc.MipLODBias = newDesc.MipLODBias + Config::Instance()->MipmapBiasOverride.value();
        }

        if (State::Instance().lastMipBiasMax < newDesc.MipLODBias)
            State::Instance().lastMipBiasMax = newDesc.MipLODBias;

        if (State::Instance().lastMipBias > newDesc.MipLODBias)
            State::Instance().lastMipBias = newDesc.MipLODBias;
    }

    return o_CreateSampler(device, &newDesc, DestDescriptor);
}

static HRESULT hkCreateSamplerState(ID3D11Device* This, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
{
    if (pSamplerDesc == nullptr || This == nullptr)
        return E_INVALIDARG;

    if (_d3d12Captured)
        return o_CreateSamplerState(This, pSamplerDesc, ppSamplerState);

    LOG_FUNC();

    D3D11_SAMPLER_DESC newDesc{};

    newDesc.AddressU = pSamplerDesc->AddressU;
    newDesc.AddressV = pSamplerDesc->AddressV;
    newDesc.AddressW = pSamplerDesc->AddressW;
    newDesc.ComparisonFunc = pSamplerDesc->ComparisonFunc;
    newDesc.BorderColor[0] = pSamplerDesc->BorderColor[0];
    newDesc.BorderColor[1] = pSamplerDesc->BorderColor[1];
    newDesc.BorderColor[2] = pSamplerDesc->BorderColor[2];
    newDesc.BorderColor[3] = pSamplerDesc->BorderColor[3];
    newDesc.MinLOD = pSamplerDesc->MinLOD;
    newDesc.MaxLOD = pSamplerDesc->MaxLOD;

    if (Config::Instance()->AnisotropyOverride.has_value() &&
        (pSamplerDesc->Filter == D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT ||
        pSamplerDesc->Filter == D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT ||
        pSamplerDesc->Filter == D3D11_FILTER_MIN_MAG_MIP_LINEAR ||
        pSamplerDesc->Filter == D3D11_FILTER_ANISOTROPIC))
    {
        LOG_DEBUG("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pSamplerDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pSamplerDesc->Filter);
        newDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        newDesc.MaxAnisotropy = Config::Instance()->AnisotropyOverride.value();
    }
    else
    {
        newDesc.Filter = pSamplerDesc->Filter;
        newDesc.MaxAnisotropy = pSamplerDesc->MaxAnisotropy;
    }

    newDesc.MipLODBias = pSamplerDesc->MipLODBias;

    if (newDesc.MipLODBias < 0.0f || Config::Instance()->MipmapBiasOverrideAll.value_or_default())
    {
        if (Config::Instance()->MipmapBiasOverride.has_value())
        {
            LOG_DEBUG("Overriding mipmap bias {0} -> {1}", pSamplerDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());

            if (Config::Instance()->MipmapBiasFixedOverride.value_or_default())
                newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();
            else if (Config::Instance()->MipmapBiasScaleOverride.value_or_default())
                newDesc.MipLODBias = newDesc.MipLODBias * Config::Instance()->MipmapBiasOverride.value();
            else
                newDesc.MipLODBias = newDesc.MipLODBias + Config::Instance()->MipmapBiasOverride.value();
        }

        if (State::Instance().lastMipBiasMax < newDesc.MipLODBias)
            State::Instance().lastMipBiasMax = newDesc.MipLODBias;

        if (State::Instance().lastMipBias > newDesc.MipLODBias)
            State::Instance().lastMipBias = newDesc.MipLODBias;
    }

    return o_CreateSamplerState(This, &newDesc, ppSamplerState);
}

#pragma endregion

#pragma region Public hook methods

void HooksDx::HookDx12()
{
    if (o_D3D12CreateDevice != nullptr)
        return;

    LOG_DEBUG("");

    o_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("d3d12.dll", "D3D12CreateDevice");
    if (o_D3D12CreateDevice != nullptr)
    {
        LOG_DEBUG("Hooking D3D12CreateDevice method");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_D3D12CreateDevice, hkD3D12CreateDevice);

        DetourTransactionCommit();
    }
}

void HooksDx::HookDx11()
{
    if (o_D3D11CreateDevice != nullptr)
        return;

    LOG_DEBUG("");

    o_D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)DetourFindFunction("d3d11.dll", "D3D11CreateDevice");
    o_D3D11CreateDeviceAndSwapChain = (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)DetourFindFunction("d3d11.dll", "D3D11CreateDeviceAndSwapChain");
    o_D3D11On12CreateDevice = (PFN_D3D11ON12_CREATE_DEVICE)DetourFindFunction("d3d11.dll", "D3D11On12CreateDevice");
    if (o_D3D11CreateDevice != nullptr || o_D3D11On12CreateDevice != nullptr || o_D3D11CreateDeviceAndSwapChain != nullptr)
    {
        LOG_DEBUG("Hooking D3D11CreateDevice methods");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_D3D11CreateDevice != nullptr)
            DetourAttach(&(PVOID&)o_D3D11CreateDevice, hkD3D11CreateDevice);

        if (o_D3D11On12CreateDevice != nullptr)
            DetourAttach(&(PVOID&)o_D3D11On12CreateDevice, hkD3D11On12CreateDevice);

        if (o_D3D11CreateDeviceAndSwapChain != nullptr)
            DetourAttach(&(PVOID&)o_D3D11CreateDeviceAndSwapChain, hkD3D11CreateDeviceAndSwapChain);

        DetourTransactionCommit();
    }
}

void HooksDx::HookDxgi()
{
    if (o_CreateDXGIFactory != nullptr)
        return;

    LOG_DEBUG("");

    o_CreateDXGIFactory = (PFN_CreateDXGIFactory)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
    o_CreateDXGIFactory1 = (PFN_CreateDXGIFactory1)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
    o_CreateDXGIFactory2 = (PFN_CreateDXGIFactory2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

    if (o_CreateDXGIFactory != nullptr)
    {
        LOG_DEBUG("Hooking CreateDXGIFactory methods");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_CreateDXGIFactory != nullptr)
            DetourAttach(&(PVOID&)o_CreateDXGIFactory, hkCreateDXGIFactory);

        if (o_CreateDXGIFactory1 != nullptr)
            DetourAttach(&(PVOID&)o_CreateDXGIFactory1, hkCreateDXGIFactory1);

        if (o_CreateDXGIFactory2 != nullptr)
            DetourAttach(&(PVOID&)o_CreateDXGIFactory2, hkCreateDXGIFactory2);

        DetourTransactionCommit();
    }
}

void HooksDx::ReleaseDx12SwapChain(HWND hwnd)
{
    IFGFeature_Dx12* fg = nullptr;

    if (State::Instance().currentFG != nullptr)
        fg = reinterpret_cast<IFGFeature_Dx12*>(State::Instance().currentFG);

    if (fg != nullptr)
        fg->ReleaseSwapchain(hwnd);
}

DXGI_FORMAT HooksDx::CurrentSwapchainFormat()
{
    if (State::Instance().currentSwapchain == nullptr)
        return DXGI_FORMAT_UNKNOWN;

    DXGI_SWAP_CHAIN_DESC scDesc{};
    if (State::Instance().currentSwapchain->GetDesc(&scDesc) != S_OK)
        return DXGI_FORMAT_UNKNOWN;

    return scDesc.BufferDesc.Format;
}

void HooksDx::UnHookDx()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_D3D11CreateDevice != nullptr)
    {
        DetourDetach(&(PVOID&)o_D3D11CreateDevice, hkD3D11CreateDevice);
        o_D3D11CreateDevice = nullptr;
    }

    if (o_D3D11On12CreateDevice != nullptr)
    {
        DetourDetach(&(PVOID&)o_D3D11On12CreateDevice, hkD3D11On12CreateDevice);
        o_D3D11On12CreateDevice = nullptr;
    }

    if (o_D3D12CreateDevice != nullptr)
    {
        DetourDetach(&(PVOID&)o_D3D12CreateDevice, hkD3D12CreateDevice);
        o_D3D12CreateDevice = nullptr;
    }

    if (o_CreateDXGIFactory1 != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateDXGIFactory1, hkCreateDXGIFactory1);
        o_CreateDXGIFactory1 = nullptr;
    }

    if (o_CreateDXGIFactory2 != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateDXGIFactory2, hkCreateDXGIFactory2);
        o_CreateDXGIFactory2 = nullptr;
    }

    if (oCreateSwapChain != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);
        oCreateSwapChain = nullptr;
    }

    if (oCreateSwapChainForHwnd != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForHwnd, hkCreateSwapChainForHwnd);
        oCreateSwapChainForHwnd = nullptr;
    }

    if (o_CreateSampler != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateSampler, hkCreateSampler);
        o_CreateSampler = nullptr;
    }

    DetourTransactionCommit();
    _isInited = false;
}

#pragma endregion

