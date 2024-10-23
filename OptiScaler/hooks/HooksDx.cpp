#include "HooksDx.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include "../imgui/imgui_overlay_dx.h"

#include "../detours/detours.h"
#include <d3d11on12.h>

#include "wrapped_swapchain.h"

#pragma region FG definitions

#include <ankerl/unordered_dense.h>
#include <shared_mutex>

// #define USE_RESOURCE_DISCARD
// #define USE_COPY_RESOURCE
#define USE_RESOURCE_BARRIRER

enum ResourceType
{
    SRV,
    RTV,
    UAV
};

typedef struct SwapChainInfo
{
    IDXGISwapChain* swapChain = nullptr;
    DXGI_FORMAT swapChainFormat = DXGI_FORMAT_UNKNOWN;
    int swapChainBufferCount = 0;
    ID3D12CommandQueue* fgCommandQueue = nullptr;
    ID3D12CommandQueue* gameCommandQueue = nullptr;
};

typedef struct ResourceInfo
{
    ID3D12Resource* buffer = nullptr;
    UINT64 width = 0;
    UINT height = 0;
    DXGI_FORMAT format;
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
    ResourceType type = SRV;
} resource_info;

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

    ResourceInfo* GetByCpuHandle(SIZE_T cpuHandle)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return nullptr;

        auto index = (cpuHandle - cpuStart) / increment;

        return &info[index];
    }

    ResourceInfo* GetByGpuHandle(SIZE_T gpuHandle)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return nullptr;

        auto index = (gpuHandle - gpuStart) / increment;

        return &info[index];
    }

    void SetByCpuHandle(SIZE_T cpuHandle, ResourceInfo setInfo)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return;

        auto index = (cpuHandle - cpuStart) / increment;

        info[index] = setInfo;
    }

    void SetByGpuHandle(SIZE_T gpuHandle, ResourceInfo setInfo)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return;

        auto index = (gpuHandle - gpuStart) / increment;

        info[index] = setInfo;
    }
} heap_info;

typedef struct ResourceHeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T gpuStart = NULL;
} resource_heap_info;

// Device hooks for FG
typedef void(*PFN_CreateRenderTargetView)(ID3D12Device* This, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
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
#ifdef USE_COPY_RESOURCE
typedef void(*PFN_CopyResource)(ID3D12GraphicsCommandList* This, ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource);
#endif
typedef void(*PFN_CopyTextureRegion)(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox);
typedef void(*PFN_Dispatch)(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
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
static PFN_OMSetRenderTargets o_OMSetRenderTargets = nullptr;
static PFN_DrawInstanced o_DrawInstanced = nullptr;
static PFN_DrawIndexedInstanced o_DrawIndexedInstanced = nullptr;
#ifdef USE_COPY_RESOURCE
static PFN_CopyResource o_CopyResource = nullptr;
#endif
static PFN_CopyTextureRegion o_CopyTextureRegion = nullptr;
static PFN_SetGraphicsRootDescriptorTable o_SetGraphicsRootDescriptorTable = nullptr;
static PFN_SetComputeRootDescriptorTable o_SetComputeRootDescriptorTable = nullptr;
static PFN_Dispatch o_Dispatch = nullptr;
#ifdef USE_RESOURCE_DISCARD
static PFN_DiscardResource o_DiscardResource = nullptr;
#endif

// swapchains variables
static ankerl::unordered_dense::map <HWND, SwapChainInfo> fgSwapChains;
static bool fgSkipSCWrapping = false;
static DXGI_SWAP_CHAIN_DESC fgScDesc{};

// queues
static std::vector<ID3D12CommandQueue*> fgQueues;

// heaps
static std::vector<HeapInfo> fgHeaps;

#ifdef USE_RESOURCE_DISCARD
// created resources
static ankerl::unordered_dense::map <ID3D12Resource*, ResourceHeapInfo> fgHandlesByResources;
#endif

// possibleHudless lisy by cmdlist
static ankerl::unordered_dense::map <ID3D12GraphicsCommandList*, ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo>> fgPossibleHudless[FrameGen_Dx12::FG_BUFFER_SIZE];

// mutexes
static std::shared_mutex heapMutex;
static std::shared_mutex resourceMutex;
static std::shared_mutex hudlessMutex[FrameGen_Dx12::FG_BUFFER_SIZE];
static std::shared_mutex counterMutex[FrameGen_Dx12::FG_BUFFER_SIZE];

// found hudless info
static ID3D12Resource* fgCopySource[FrameGen_Dx12::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
static ID3D12Resource* fgHudless[FrameGen_Dx12::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };
static ID3D12Resource* fgHudlessBuffer[FrameGen_Dx12::FG_BUFFER_SIZE] = { nullptr, nullptr, nullptr, nullptr };

static UINT fgFrameIndex = 0;

// Last captured upscaled hudless frame number
static UINT64 fgHudlessFrame = 0;
static UINT64 fgPresentedFrame = 0;
static bool fgPresentRunning = false;

// Used for frametime calculation
static double fgLastFrameTime = 0.0;
static double fgLastDeltaTime = 0.0;
static bool fgDispatchCalled = false;
static bool fgStopAfterNextPresent = false;

// Swapchain frame counter
static UINT64 frameCounter = 0;

// Swapchain frame target while capturing RTVs etc
static UINT64 fgLastFGFrame = 0;
static bool fgUpscaledFound = false;

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

inline static PFN_EnumAdapters2 ptrEnumAdapters = nullptr;
inline static PFN_EnumAdapters12 ptrEnumAdapters1 = nullptr;
inline static PFN_EnumAdapterByLuid2 ptrEnumAdapterByLuid = nullptr;
inline static PFN_EnumAdapterByGpuPreference2 ptrEnumAdapterByGpuPreference = nullptr;

static PFN_CreateSwapChain oCreateSwapChain = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd = nullptr;

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
static ID3D12Device* g_pd3dDeviceParam = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;

// status
static bool _isInited = false;
static bool _d3d12Captured = false;

static void DeatachAllHooks();
static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static HRESULT hkCreateSamplerState(ID3D11Device* This, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);
static HRESULT hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
static HRESULT hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);

static void FfxFgLogCallback(uint32_t type, const wchar_t* message)
{
    std::wstring string(message);
    LOG_DEBUG("    FG Log: {0}", wstring_to_string(string));
}

static bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource)
{
    if (InDevice == nullptr || InSource == nullptr)
        return false;

    D3D12_RESOURCE_DESC texDesc = InSource->GetDesc();

    if (*OutResource != nullptr)
    {
        auto bufDesc = (*OutResource)->GetDesc();

        if (bufDesc.Width != (UINT64)(texDesc.Width) || bufDesc.Height != (UINT)(texDesc.Height) || bufDesc.Format != texDesc.Format)
        {
            (*OutResource)->Release();
            (*OutResource) = nullptr;
        }
        else
            return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = InSource->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        LOG_ERROR("GetHeapProperties result: {0:X}", (UINT64)hr);
        return false;
    }

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

static bool InUpscaledList(ID3D12Resource* resource)
{
    auto fIndex = fgFrameIndex;
    if (FrameGen_Dx12::fgUpscaledImage[fIndex] == resource)
    {
        LOG_DEBUG_ONLY("Found upscaled image!");
        fgUpscaledFound = true;
        return true;
    }

    return false;
}

static void FillResourceInfo(ID3D12Resource* resource, ResourceInfo* info)
{
    auto desc = resource->GetDesc();
    info->buffer = resource;
    info->width = desc.Width;
    info->height = desc.Height;
    info->format = desc.Format;
}

static void GetHudless(ID3D12GraphicsCommandList* This)
{
    auto fIndex = fgFrameIndex;
    if (This != g_pd3dCommandList && fgCopySource[fIndex] != nullptr && Config::Instance()->CurrentFeature != nullptr &&
        fgHudlessFrame != Config::Instance()->CurrentFeature->FrameCount() && FrameGen_Dx12::fgTarget <= Config::Instance()->CurrentFeature->FrameCount())
    {
        LOG_DEBUG("FrameCount: {0}, fgHudlessFrame: {1}, CommandList: {2:X}", Config::Instance()->CurrentFeature->FrameCount(), fgHudlessFrame, (UINT64)This);

        FrameGen_Dx12::fgSkipHudlessChecks = true;

        // hudless captured for this frame
        fgHudlessFrame = Config::Instance()->CurrentFeature->FrameCount();
        auto frame = fgHudlessFrame;

        LOG_DEBUG("running, frame: {0}", frame);

        // switch dlss targets for next depth and mv 
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};

        m_FrameGenerationConfig.HUDLessColor = ffxApiGetResourceDX12(fgHudless[fIndex], FFX_API_RESOURCE_STATE_COPY_DEST, 0);

        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = true;
        m_FrameGenerationConfig.flags = 0;

        if (Config::Instance()->FGDebugView.value_or(false))
            m_FrameGenerationConfig.flags |= FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW;

        m_FrameGenerationConfig.allowAsyncWorkloads = Config::Instance()->FGAsync.value_or(false);

        // assume symmetric letterbox
        m_FrameGenerationConfig.generationRect.left = 0;
        m_FrameGenerationConfig.generationRect.top = 0;
        m_FrameGenerationConfig.generationRect.width = Config::Instance()->CurrentFeature->DisplayWidth();
        m_FrameGenerationConfig.generationRect.height = Config::Instance()->CurrentFeature->DisplayHeight();

        m_FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
            {
                HRESULT result;
                ffxReturnCode_t dispatchResult;
                auto fIndex = fgFrameIndex;

                // check for status
                if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || Config::Instance()->FGChanged ||
                    FrameGen_Dx12::fgContext == nullptr || FrameGen_Dx12::fgCopyCommandList == nullptr || FrameGen_Dx12::fgCopyCommandQueue == nullptr)
                {
                    LOG_WARN("Cancel async dispatch");
                    fgDispatchCalled = false;
                    FrameGen_Dx12::fgSkipHudlessChecks = false;
                    return FFX_API_RETURN_OK;
                }

                // If fg is active but upscaling paused
                if (!fgDispatchCalled || Config::Instance()->CurrentFeature == nullptr ||
                    fgLastFGFrame == Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive)
                {
                    LOG_WARN("Callback without hudless! frameID: {}", params->frameID);

                    auto allocator = FrameGen_Dx12::fgCopyCommandAllocators[fIndex];
                    result = allocator->Reset();
                    result = FrameGen_Dx12::fgCopyCommandList->Reset(allocator, nullptr);

                    params->frameID = fgLastFGFrame;
                    params->numGeneratedFrames = 0;
                }

                if (Config::Instance()->CurrentFeature != nullptr)
                    fgLastFGFrame = Config::Instance()->CurrentFeature->FrameCount();

                dispatchResult = FfxApiProxy::D3D12_Dispatch()(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
                ID3D12CommandList* cl[1] = { nullptr };
                result = FrameGen_Dx12::fgCopyCommandList->Close();
                cl[0] = FrameGen_Dx12::fgCopyCommandList;
                FrameGen_Dx12::gameCommandQueue->ExecuteCommandLists(1, cl);

                LOG_DEBUG("_dispatch result: {0}", (UINT)result);

                fgDispatchCalled = false;
                FrameGen_Dx12::fgSkipHudlessChecks = false;

                return dispatchResult;
            };

        m_FrameGenerationConfig.frameGenerationCallbackUserContext = &FrameGen_Dx12::fgContext;

        m_FrameGenerationConfig.onlyPresentGenerated = Config::Instance()->FGOnlyGenerated;
        m_FrameGenerationConfig.frameID = Config::Instance()->CurrentFeature->FrameCount();
        m_FrameGenerationConfig.swapChain = HooksDx::currentSwapchain;

        ffxConfigureDescGlobalDebug1 debugDesc;
        debugDesc.header.type = FFX_API_CONFIGURE_DESC_TYPE_GLOBALDEBUG1;
        debugDesc.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_VERBOSE;
        debugDesc.fpMessage = FfxFgLogCallback;
        m_FrameGenerationConfig.header.pNext = &debugDesc.header;

        Config::Instance()->dxgiSkipSpoofing = true;
        ffxReturnCode_t retCode = FfxApiProxy::D3D12_Configure()(&FrameGen_Dx12::fgContext, &m_FrameGenerationConfig.header);
        Config::Instance()->dxgiSkipSpoofing = false;
        LOG_DEBUG("_configure result: {0:X}, frame: {1}", retCode, frame);

        if (retCode == FFX_API_RETURN_OK)
        {
            ffxCreateBackendDX12Desc backendDesc{};
            backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
            backendDesc.device = g_pd3dDeviceParam;

            ffxDispatchDescFrameGenerationPrepare dfgPrepare{};
            dfgPrepare.header.type = FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATION_PREPARE;
            dfgPrepare.header.pNext = &backendDesc.header;

            dfgPrepare.commandList = FrameGen_Dx12::fgCopyCommandList; // This;

            dfgPrepare.frameID = frame;
            dfgPrepare.flags = m_FrameGenerationConfig.flags;

            dfgPrepare.renderSize = { Config::Instance()->CurrentFeature->RenderWidth(), Config::Instance()->CurrentFeature->RenderHeight() };

            dfgPrepare.jitterOffset.x = FrameGen_Dx12::jitterX;
            dfgPrepare.jitterOffset.y = FrameGen_Dx12::jitterY;

            // They will be always copies
            dfgPrepare.motionVectors = ffxApiGetResourceDX12(FrameGen_Dx12::paramVelocity[fIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
            dfgPrepare.depth = ffxApiGetResourceDX12(FrameGen_Dx12::paramDepth[fIndex], FFX_API_RESOURCE_STATE_COPY_DEST);

            dfgPrepare.motionVectorScale.x = FrameGen_Dx12::mvScaleX;
            dfgPrepare.motionVectorScale.y = FrameGen_Dx12::mvScaleY;

            if (Config::Instance()->CurrentFeature->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
            {
                dfgPrepare.cameraFar = Config::Instance()->FsrCameraNear.value_or(0.01f);
                dfgPrepare.cameraNear = Config::Instance()->FsrCameraFar.value_or(0.99f);
            }
            else
            {
                dfgPrepare.cameraFar = Config::Instance()->FsrCameraFar.value_or(0.99f);
                dfgPrepare.cameraNear = Config::Instance()->FsrCameraNear.value_or(0.01f);
            }

            dfgPrepare.cameraFovAngleVertical = 1.0471975511966f;
            dfgPrepare.viewSpaceToMetersFactor = 1.0;
            dfgPrepare.frameTimeDelta = FrameGen_Dx12::fgFrameTime;

            // If somehow context is destroyed before this point
            if (Config::Instance()->CurrentFeature == nullptr || FrameGen_Dx12::fgContext == nullptr || !FrameGen_Dx12::fgIsActive)
            {
                LOG_WARN("!! Config::Instance()->CurrentFeature == nullptr || HooksDx::fgContext == nullptr");
                return;
            }

            Config::Instance()->dxgiSkipSpoofing = true;
            retCode = FfxApiProxy::D3D12_Dispatch()(&FrameGen_Dx12::fgContext, &dfgPrepare.header);
            fgDispatchCalled = true;
            Config::Instance()->dxgiSkipSpoofing = false;
            LOG_DEBUG("_dispatch result: {0}, frame: {1}", retCode, frame);
        }
    }
}

static bool CheckCapture()
{
    auto fIndex = fgFrameIndex;

    {
        std::unique_lock<std::shared_mutex> lock(counterMutex[fIndex]);
        FrameGen_Dx12::fgHUDlessCaptureCounter[fIndex]++;

        LOG_DEBUG("frameCounter: {}, fgHUDlessCaptureCounter: {}, Limit: {}", frameCounter, FrameGen_Dx12::fgHUDlessCaptureCounter[fIndex], Config::Instance()->FGHUDLimit.value_or(1));

        if (FrameGen_Dx12::fgHUDlessCaptureCounter[fIndex] != Config::Instance()->FGHUDLimit.value_or(1))
            return false;
    }

    return true;
}

static void CaptureHudless(ID3D12GraphicsCommandList* cmdList, ResourceInfo* resource, D3D12_RESOURCE_STATES state)
{
    auto fIndex = fgFrameIndex;
    FrameGen_Dx12::upscaleRan = false;
    fgUpscaledFound = false;
    fgCopySource[fIndex] = resource->buffer;

    LOG_TRACE("Capture resource: {0:X}", (size_t)resource->buffer);

    if (resource->format != HooksDx::swapchainFormat && Config::Instance()->FGHUDFixExtended.value_or(false) && FrameGen_Dx12::fgFormatTransfer != nullptr &&
        (resource->format == DXGI_FORMAT_R16G16B16A16_FLOAT || resource->format == DXGI_FORMAT_R11G11B10_FLOAT || resource->format == DXGI_FORMAT_R32G32B32A32_FLOAT || resource->format == DXGI_FORMAT_R32G32B32_FLOAT) &&
        (HooksDx::swapchainFormat == DXGI_FORMAT_R8G8B8A8_UNORM || HooksDx::swapchainFormat == DXGI_FORMAT_B8G8R8A8_UNORM || HooksDx::swapchainFormat == DXGI_FORMAT_R10G10B10A2_UNORM))
    {
        if (FrameGen_Dx12::fgFormatTransfer->CreateBufferResource(g_pd3dDeviceParam, resource->buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
            CreateBufferResource(g_pd3dDeviceParam, resource->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, &fgHudlessBuffer[fIndex]))
        {
#ifdef USE_RESOURCE_BARRIRER
            ResourceBarrier(cmdList, resource->buffer, state, D3D12_RESOURCE_STATE_COPY_SOURCE);
            ResourceBarrier(cmdList, fgHudlessBuffer[fIndex], D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
#endif

            cmdList->CopyResource(fgHudlessBuffer[fIndex], resource->buffer);
#ifdef USE_RESOURCE_BARRIRER
            ResourceBarrier(cmdList, fgHudlessBuffer[fIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            ResourceBarrier(cmdList, resource->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, state);
#endif

            FrameGen_Dx12::fgFormatTransfer->SetBufferState(FrameGen_Dx12::fgCopyCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            FrameGen_Dx12::fgFormatTransfer->Dispatch(g_pd3dDeviceParam, FrameGen_Dx12::fgCopyCommandList, fgHudlessBuffer[fIndex], FrameGen_Dx12::fgFormatTransfer->Buffer());
            FrameGen_Dx12::fgFormatTransfer->SetBufferState(FrameGen_Dx12::fgCopyCommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

            LOG_TRACE("Using fgFormatTransfer->Buffer()");
            fgHudless[fIndex] = FrameGen_Dx12::fgFormatTransfer->Buffer();
        }
        else
        {
            LOG_WARN("Can't create fgHudlessBuffer or fgFormatTransfer buffer!");
            return;
        }
    }
    else
    {
#ifdef USE_RESOURCE_BARRIRER
        ResourceBarrier(cmdList, resource->buffer, state, D3D12_RESOURCE_STATE_COPY_SOURCE);
#endif

        if (CreateBufferResource(g_pd3dDeviceParam, resource->buffer, D3D12_RESOURCE_STATE_COPY_DEST, &fgHudless[fIndex]))
            cmdList->CopyResource(fgHudless[fIndex], resource->buffer);

#ifdef USE_RESOURCE_BARRIRER
        ResourceBarrier(cmdList, resource->buffer, D3D12_RESOURCE_STATE_COPY_SOURCE, state);
#endif
    }

    GetHudless(cmdList);
}

static bool CheckForHudless(ResourceInfo* resource, bool checkFormat = true)
{
    if (HooksDx::currentSwapchain == nullptr)
        return false;

    DXGI_SWAP_CHAIN_DESC scDesc{};
    if (HooksDx::currentSwapchain->GetDesc(&scDesc) != S_OK)
    {
        LOG_WARN("Can't get swapchain desc!");
        return false;
    }

    if (scDesc.BufferDesc.Height != fgScDesc.BufferDesc.Height || scDesc.BufferDesc.Width != fgScDesc.BufferDesc.Width || scDesc.BufferDesc.Format != fgScDesc.BufferDesc.Format)
    {
        LOG_DEBUG("Format change, recreate the FormatTransfer");
        delete FrameGen_Dx12::fgFormatTransfer;
        FrameGen_Dx12::fgFormatTransfer = nullptr;
        FrameGen_Dx12::fgFormatTransfer = new FT_Dx12("FormatTransfer", g_pd3dDeviceParam, scDesc.BufferDesc.Format);

        HooksDx::swapchainFormat = scDesc.BufferDesc.Format;
        fgScDesc = scDesc;
    }

    if (resource->height == fgScDesc.BufferDesc.Height && resource->width == fgScDesc.BufferDesc.Width && (!checkFormat || resource->format == fgScDesc.BufferDesc.Format ||
        (Config::Instance()->FGHUDFixExtended.value_or(false) && FrameGen_Dx12::fgFormatTransfer != nullptr &&
        (resource->format == DXGI_FORMAT_R16G16B16A16_FLOAT || resource->format == DXGI_FORMAT_R11G11B10_FLOAT || resource->format == DXGI_FORMAT_R32G32B32A32_FLOAT || resource->format == DXGI_FORMAT_R32G32B32_FLOAT) &&
        (fgScDesc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || fgScDesc.BufferDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || fgScDesc.BufferDesc.Format == DXGI_FORMAT_R10G10B10A2_UNORM))))
    {
        LOG_TRACE("Width: {}/{}, Height: {}/{}, Format: {}/{}, checkFormat: {} -> TRUE",
                  resource->width, fgScDesc.BufferDesc.Width, resource->height, fgScDesc.BufferDesc.Height, (UINT)resource->format, (UINT)fgScDesc.BufferDesc.Format, checkFormat);

        return true;
    }

    LOG_TRACE("Width: {}/{}, Height: {}/{}, Format: {}/{}, checkFormat: {} -> FALSE",
              resource->width, fgScDesc.BufferDesc.Width, resource->height, fgScDesc.BufferDesc.Height, (UINT)resource->format, (UINT)fgScDesc.BufferDesc.Format, checkFormat);

    return false;
}

#ifdef USE_RESOURCE_DISCARD
static void hkDiscardResource(ID3D12GraphicsCommandList* This, ID3D12Resource* pResource, D3D12_DISCARD_REGION* pRegion)
{
    o_DiscardResource(This, pResource, pRegion);

    if (This != g_pd3dCommandList && pRegion == nullptr)
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

#pragma region "Resource inputs"

static void hkCreateRenderTargetView(ID3D12Device* This, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    o_CreateRenderTargetView(This, pResource, pDesc, DestDescriptor);


    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_SRV_DIMENSION_TEXTURE2D)
        return;

    auto gpuHandle = GetGPUHandle(This, DestDescriptor.ptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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

    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

static void hkCreateShaderResourceView(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
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

    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

static void hkCreateUnorderedAccessView(ID3D12Device* This, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    o_CreateUnorderedAccessView(This, pResource, pCounterResource, pDesc, DestDescriptor);


    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_SRV_DIMENSION_TEXTURE2D)
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

    auto heap = GetHeapByCpuHandle(DestDescriptor.ptr);
    if (heap != nullptr)
        heap->SetByCpuHandle(DestDescriptor.ptr, resInfo);
}

#pragma endregion

#pragma region "Resource copy"

#ifdef USE_COPY_RESOURCE
static void hkCopyResource(ID3D12GraphicsCommandList* This, ID3D12Resource* Dest, ID3D12Resource* Source)
{
    o_CopyResource(This, Dest, Source);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || HooksDx::fgCopyCommandList == This /* || fgPresentRunning */ || HooksDx::fgSkipHudlessChecks ||
        Config::Instance()->CurrentFeature == nullptr || !HooksDx::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || HooksDx::fgContext == nullptr ||
        HooksDx::fgTarget > Config::Instance()->CurrentFeature->FrameCount() ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <--");

    ResourceInfo resInfo{};
    FillResourceInfo(Dest, &resInfo);

    // Copy source is not in sources and dest is not matching for swapchain format
    if (!InUpscaledList(Source) && !CheckForHudless(&resInfo, false))
        return;

    // not matching to swapchain format or limit is not ok
    if (CheckForHudless(&resInfo) && CheckCapture())
    {
        LOG_DEBUG("Capture");
        CaptureHudless(This, Dest);
    }
}
#endif

static void hkCopyTextureRegion(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox)
{
    o_CopyTextureRegion(This, pDst, DstX, DstY, DstZ, pSrc, pSrcBox);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This || FrameGen_Dx12::fgSkipHudlessChecks ||
        Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <--");

    ResourceInfo resInfo{};
    FillResourceInfo(pDst->pResource, &resInfo);

    // Copy source is not in sources and dest is not matching for swapchain format
    if (!InUpscaledList(pSrc->pResource) && !CheckForHudless(&resInfo, false))
        return;

    // not matching to swapchain format or limit is not ok
    if (CheckForHudless(&resInfo) && CheckCapture())
    {
        LOG_DEBUG("Capture");
        CaptureHudless(This, &resInfo, D3D12_RESOURCE_STATE_COPY_DEST);
    }
}

#pragma endregion

#pragma region "Heap methods"

static HRESULT hkCreateDescriptorHeap(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap)
{
    auto result = o_CreateDescriptorHeap(This, pDescriptorHeapDesc, riid, ppvHeap);

    // try to calculate handle ranges for heap
    if (result == S_OK && (pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV))  //&& pDescriptorHeapDesc->Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
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

        LOG_DEBUG_ONLY("Heap type: {}, Cpu: {}-{}, Gpu: {}-{}, Desc count: {}", info.type, info.cpuStart, info.cpuEnd, info.gpuStart, info.gpuEnd, info.numDescriptors);
        {
            std::unique_lock<std::shared_mutex> lock(heapMutex);
            fgHeaps.push_back(info);
        }
    }
    else
    {
        auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
        LOG_DEBUG_ONLY("Skipping, Heap type: {}, Cpu: {}, Gpu: {}", (UINT)pDescriptorHeapDesc->Type, heap->GetCPUDescriptorHandleForHeapStart().ptr, heap->GetGPUDescriptorHandleForHeapStart().ptr);
    }

    return result;
}


static void hkCopyDescriptors(ID3D12Device* This,
                              UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes,
                              UINT NumSrcDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes,
                              D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptors(This, NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);

    auto fIndex = fgFrameIndex;

    if (Config::Instance()->CurrentFeature == nullptr)
        return;

    if (DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        return;

    LOG_DEBUG_ONLY("SrcRanges: {}, DestRanges: {}, Type: {}", NumSrcDescriptorRanges, NumDestDescriptorRanges, (UINT)DescriptorHeapsType);

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

    UINT destRangeIndex = 0;
    UINT destIndex = 0;

    for (size_t i = 0; i < NumSrcDescriptorRanges; i++)
    {
        UINT copyCount = 1;

        if (pSrcDescriptorRangeSizes != nullptr)
            copyCount = pSrcDescriptorRangeSizes[i];

        for (size_t j = 0; j < copyCount; j++)
        {
            auto handle = pSrcDescriptorRangeStarts[i].ptr + j * size;

            auto heap = GetHeapByCpuHandle(handle);
            if (heap == nullptr)
                continue;

            auto buffer = heap->GetByCpuHandle(handle);
            auto destHandle = pDestDescriptorRangeStarts[destRangeIndex].ptr + destIndex * size;
            heap->SetByCpuHandle(destHandle, *buffer);
        }

        if (pDestDescriptorRangeSizes == nullptr)
        {
            destIndex = 0;
            destRangeIndex++;
        }
        else
        {
            if (pDestDescriptorRangeSizes[destRangeIndex] == destIndex)
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

static void hkCopyDescriptorsSimple(ID3D12Device* This, UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
                                    D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptorsSimple(This, NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);

    auto fIndex = fgFrameIndex;

    if (Config::Instance()->CurrentFeature == nullptr)
        return;

    if (DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && DescriptorHeapsType != D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    auto size = This->GetDescriptorHandleIncrementSize(DescriptorHeapsType);

    for (size_t i = 0; i < NumDescriptors; i++)
    {
        auto handle = SrcDescriptorRangeStart.ptr + i * size;

        auto heap = GetHeapByCpuHandle(handle);
        if (heap == nullptr)
            continue;

        auto buffer = heap->GetByCpuHandle(handle);
        auto destHandle = DestDescriptorRangeStart.ptr + i * size;
        heap->SetByCpuHandle(destHandle, *buffer);
    }
}

#pragma endregion

#pragma region "Shader inputs"

static void hkSetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);

    auto fIndex = fgFrameIndex;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    auto heap = GetHeapByGpuHandle(BaseDescriptor.ptr);
    if (heap == nullptr)
        return;

    auto capturedBuffer = heap->GetByGpuHandle(BaseDescriptor.ptr);
    if (capturedBuffer == nullptr || capturedBuffer->buffer == nullptr)
    {
        LOG_DEBUG_ONLY("Miss RootParameterIndex: {1}, CommandList: {0:X}, gpuHandle: {2}", (SIZE_T)This, RootParameterIndex, BaseDescriptor.ptr);
        return;
    }

    if (!InUpscaledList(capturedBuffer->buffer) && !CheckForHudless(capturedBuffer, false))
        return;

    capturedBuffer->state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[fIndex]);
        if (fgPossibleHudless[fIndex].contains(This))
        {
            fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
            return;
        }

        ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
        fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
        fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
    }
}

#pragma endregion

#pragma region "Shader outputs"

static void hkOMSetRenderTargets(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
                                 BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    o_OMSetRenderTargets(This, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This /* || fgPresentRunning */ || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[fIndex]);

        for (size_t i = 0; i < NumRenderTargetDescriptors; i++)
        {
            auto handle = pRenderTargetDescriptors[i];
            auto heap = GetHeapByCpuHandle(handle.ptr);
            if (heap == nullptr)
                continue;

            auto resource = heap->GetByCpuHandle(handle.ptr);
            if (resource == nullptr || resource->buffer == nullptr)
            {
                LOG_DEBUG_ONLY("Miss index: {0}, cpu: {1}", i, handle.ptr);
                continue;
            }

            if (!CheckForHudless(resource, false))
                continue;

            resource->state = D3D12_RESOURCE_STATE_RENDER_TARGET;

            if (fgPossibleHudless[fIndex].contains(This))
            {
                fgPossibleHudless[fIndex][This].insert_or_assign(resource->buffer, *resource);
                return;
            }

            ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
            fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
            fgPossibleHudless[fIndex][This].insert_or_assign(resource->buffer, *resource);
        }
    }
}

#pragma endregion

#pragma region "Compute paramters"

static void hkSetComputeRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    o_SetComputeRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);

    auto fIndex = fgFrameIndex;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    auto heap = GetHeapByGpuHandle(BaseDescriptor.ptr);
    if (heap == nullptr)
        return;

    auto capturedBuffer = heap->GetByGpuHandle(BaseDescriptor.ptr);
    if (capturedBuffer != nullptr && capturedBuffer->buffer != nullptr && (InUpscaledList(capturedBuffer->buffer) || CheckForHudless(capturedBuffer, false)))
    {
        if (capturedBuffer->type == UAV)
            capturedBuffer->state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        else
            capturedBuffer->state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        std::unique_lock<std::shared_mutex> lock4(hudlessMutex[fIndex]);
        if (fgPossibleHudless[fIndex].contains(This))
        {
            fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
            return;
        }

        ankerl::unordered_dense::map <ID3D12Resource*, ResourceInfo> newMap;
        fgPossibleHudless[fIndex].insert_or_assign(This, newMap);
        fgPossibleHudless[fIndex][This].insert_or_assign(capturedBuffer->buffer, *capturedBuffer);
    }
}

#pragma endregion

#pragma region "Shader finalizers"

// Capture if render target matches, wait for DrawIndexed
static void hkDrawInstanced(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    o_DrawInstanced(This, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[fIndex]);
        // if can't find output skip
        if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
        {
            fgPossibleHudless[fIndex][This].clear();
            return;
        }

        do
        {
            auto& val0 = fgPossibleHudless[fIndex][This];

            // if this command list does not have entries skip
            if (val0.size() == 0)
                break;

            for (auto& [key, val] : val0)
            {
                if (CheckForHudless(&val))
                {
                    if (CheckCapture())
                    {
                        LOG_DEBUG("Capture");
                        CaptureHudless(This, &val, val.state);
                        break;
                    }
                }
            }

        } while (false);

        fgPossibleHudless[fIndex][This].clear();
        LOG_DEBUG_ONLY("Clear");
    }
}

static void hkDrawIndexedInstanced(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    o_DrawIndexedInstanced(This, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This /* || fgPresentRunning */ || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[fIndex]);
        // if can't find output skip
        if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
        {
            fgPossibleHudless[fIndex][This].clear();
            LOG_DEBUG_ONLY("Early exit");
            return;
        }

        do
        {
            auto& val0 = fgPossibleHudless[fIndex][This];

            // if this command list does not have entries skip
            if (val0.size() == 0)
                break;

            for (auto& [key, val] : val0)
            {
                if (CheckForHudless(&val))
                {
                    LOG_DEBUG_ONLY("Found matching final image");

                    if (CheckCapture())
                    {
                        LOG_DEBUG("Capture");
                        CaptureHudless(This, &val, val.state);
                        break;
                    }
                }
            }

        } while (false);

        fgPossibleHudless[fIndex][This].clear();
        LOG_DEBUG_ONLY("Clear");
    }
}

static void hkDispatch(ID3D12GraphicsCommandList* This, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    o_Dispatch(This, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

    auto fIndex = fgFrameIndex;

    if (This == g_pd3dCommandList || FrameGen_Dx12::fgCopyCommandList == This /* || fgPresentRunning */ || FrameGen_Dx12::fgSkipHudlessChecks || Config::Instance()->CurrentFeature == nullptr || !FrameGen_Dx12::upscaleRan)
        return;

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || FrameGen_Dx12::fgContext == nullptr ||
        FrameGen_Dx12::fgTarget > Config::Instance()->CurrentFeature->FrameCount() || !FrameGen_Dx12::fgIsActive ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() && fgCopySource[fIndex] != nullptr)
        return;

    LOG_DEBUG_ONLY(" <-- {0:X}", (size_t)This);

    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[fIndex]);
        // if can't find output skip
        if (fgPossibleHudless[fIndex].size() == 0 || !fgPossibleHudless[fIndex].contains(This))
        {
            fgPossibleHudless[fIndex][This].clear();
            LOG_DEBUG_ONLY("Early exit");
            return;
        }

        do
        {
            auto& val0 = fgPossibleHudless[fIndex][This];

            // if this command list does not have entries skip
            if (val0.size() == 0)
                break;

            for (auto& [key, val] : val0)
            {
                if (CheckForHudless(&val))
                {
                    LOG_DEBUG_ONLY("Found matching final image");

                    if (CheckCapture())
                    {
                        LOG_DEBUG("Capture");
                        CaptureHudless(This, &val, val.state);
                        break;
                    }
                }
            }

        } while (false);

        fgPossibleHudless[fIndex][This].clear();
        LOG_DEBUG_ONLY("Clear");
    }
}

#pragma endregion

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
    switch (eCurrentFormat)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

#pragma region Callbacks for wrapped swapchain

static HRESULT Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters, IUnknown* pDevice, HWND hWnd)
{
    LOG_DEBUG("");

    HRESULT presentResult;
    auto fIndex = fgFrameIndex;

    if (hWnd != Util::GetProcessWindow())
    {
        //fgPresentRunning = true;

        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        //fgPresentRunning = false;

        HooksDx::currentSwapchain = nullptr;
        HooksDx::swapchainFormat = DXGI_FORMAT_UNKNOWN;
        FrameGen_Dx12::fgSkipHudlessChecks = false;

        LOG_FUNC_RESULT(presentResult);

        //LOG_DEBUG("Return");

        return presentResult;
    }

    if (fgSwapChains.contains(hWnd))
    {
        auto swInfo = &fgSwapChains[hWnd];
        HooksDx::currentSwapchain = swInfo->swapChain;

        if (HooksDx::swapchainFormat == DXGI_FORMAT_UNKNOWN)
            HooksDx::swapchainFormat = swInfo->swapChainFormat;

        swInfo->fgCommandQueue = (ID3D12CommandQueue*)pDevice;
        FrameGen_Dx12::gameCommandQueue = swInfo->gameCommandQueue;
    }

    //if (Config::Instance()->CurrentFeature != nullptr)
    //    LOG_DEBUG("FrameCount: {}, fgHudlessFrame: {}", Config::Instance()->CurrentFeature->FrameCount(), fgHudlessFrame);

    ID3D12CommandQueue* cq = nullptr;
    ID3D11Device* device = nullptr;
    ID3D12Device* device12 = nullptr;

    // try to obtain directx objects and find the path
    if (pDevice->QueryInterface(IID_PPV_ARGS(&device)) == S_OK)
    {
        if (!_dx11Device)
            LOG_DEBUG("D3D11Device captured");

        _dx11Device = true;
    }
    else if (pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        if (!_dx12Device)
            LOG_DEBUG("D3D12CommandQueue captured");

        FrameGen_Dx12::fgCommandQueue = (ID3D12CommandQueue*)pDevice;
        FrameGen_Dx12::fgCommandQueue->SetName(L"fgSwapChainQueue");
        HooksDx::GameCommandQueue = (ID3D12CommandQueue*)pDevice;

        if (cq->GetDevice(IID_PPV_ARGS(&device12)) == S_OK)
        {
            if (!_dx12Device)
                LOG_DEBUG("D3D12Device captured");

            _dx12Device = true;
        }
    }

    // DXVK check, it's here because of upscaler time calculations
    if (Config::Instance()->IsRunningOnDXVK)
    {
        if (cq != nullptr)
            cq->Release();

        if (device != nullptr)
            device->Release();

        if (device12 != nullptr)
            device12->Release();

        //fgPresentRunning = true;

        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        FrameGen_Dx12::fgSkipHudlessChecks = false;

        if (Config::Instance()->CurrentFeature != nullptr)
            fgPresentedFrame = Config::Instance()->CurrentFeature->FrameCount();

        if (fgStopAfterNextPresent)
        {
            FrameGen_Dx12::StopAndDestroyFGContext(false, false);
            fgStopAfterNextPresent = false;
        }

        auto now = Util::MillisecondsNow();

        if (fgLastFrameTime != 0)
        {
            fgLastDeltaTime = now - fgLastFrameTime;
            LOG_DEBUG("fgLastDeltaTime: {0:.2f}, frameCounter: {1}", fgLastDeltaTime, frameCounter);
        }

        fgLastFrameTime = now;

        //LOG_DEBUG("Return");

        return presentResult;
    }

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

            Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
            Config::Instance()->upscaleTimes.pop_front();
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
                    Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
                    Config::Instance()->upscaleTimes.pop_front();
                }
            }
        }


        HooksDx::dx11UpscaleTrig[HooksDx::currentFrameIndex] = false;
        HooksDx::currentFrameIndex = (HooksDx::currentFrameIndex + 1) % HooksDx::QUERY_BUFFER_COUNT;
    }


    ImGuiOverlayDx::Present(pSwapChain, SyncInterval, Flags, pPresentParameters, pDevice, hWnd);

    frameCounter++;

    // swapchain present
    if (pPresentParameters == nullptr)
        presentResult = pSwapChain->Present(SyncInterval, Flags);
    else
        presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

    FrameGen_Dx12::fgSkipHudlessChecks = false;

    if (Config::Instance()->CurrentFeature != nullptr)
        fgPresentedFrame = Config::Instance()->CurrentFeature->FrameCount();

    // release used objects
    if (cq != nullptr)
        cq->Release();

    if (device != nullptr)
        device->Release();

    if (device12 != nullptr)
        device12->Release();

    auto now = Util::MillisecondsNow();

    if (fgLastFrameTime != 0)
    {
        fgLastDeltaTime = now - fgLastFrameTime;
        LOG_DEBUG("fgLastDeltaTime: {0:.2f}, frameCounter: {1}", fgLastDeltaTime, frameCounter);
    }

    fgLastFrameTime = now;

    return presentResult;
}

#pragma endregion

#pragma region DXGI hooks

static void CheckAdapter(IUnknown* unkAdapter)
{
    if (Config::Instance()->IsRunningOnDXVK)
        return;

    //DXVK VkInterface GUID
    const GUID guid = { 0x907bf281,0xea3c,0x43b4,{0xa8,0xe4,0x9f,0x23,0x11,0x07,0xb4,0xff} };

    IDXGIAdapter* adapter = nullptr;
    bool adapterOk = unkAdapter->QueryInterface(IID_PPV_ARGS(&adapter)) == S_OK;

    void* dxvkAdapter = nullptr;
    if (adapterOk && adapter->QueryInterface(guid, &dxvkAdapter) == S_OK)
    {

        Config::Instance()->IsRunningOnDXVK = dxvkAdapter != nullptr;
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
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters = (PFN_EnumAdapters2)pVTable[7];

        DetourAttach(&(PVOID&)ptrEnumAdapters, hkEnumAdapters);

        DetourTransactionCommit();

        factory->Release();
    }

    IDXGIFactory1* factory1;
    if (ptrEnumAdapters1 == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory1)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters1 = (PFN_EnumAdapters12)pVTable[12];

        DetourAttach(&(PVOID&)ptrEnumAdapters1, hkEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (ptrEnumAdapterByLuid == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByLuid = (PFN_EnumAdapterByLuid2)pVTable[26];

        DetourAttach(&(PVOID&)ptrEnumAdapterByLuid, hkEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (ptrEnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference2)pVTable[29];

        DetourAttach(&(PVOID&)ptrEnumAdapterByGpuPreference, hkEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

static HRESULT hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    LOG_FUNC();

    *ppSwapChain = nullptr;

    if (Config::Instance()->VulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");

        if (pDesc != nullptr)
            LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Windowed: {4}", pDesc->BufferDesc.Width, pDesc->BufferDesc.Height, (UINT)pDesc->BufferDesc.Format, pDesc->BufferCount, pDesc->Windowed);

        return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    if (pDesc->BufferDesc.Height == 2 && pDesc->BufferDesc.Width == 2)
    {
        LOG_WARN("RTSS call!");
        return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    ID3D12CommandQueue* cq = nullptr;
    if (Config::Instance()->FGUseFGSwapChain.value_or(true) && !fgSkipSCWrapping && FfxApiProxy::InitFfxDx12() && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        cq->SetName(L"GameQueue");
        SwapChainInfo scInfo{};
        scInfo.gameCommandQueue = cq;
        cq->Release();

        ffxCreateContextDescFrameGenerationSwapChainNewDX12 createSwapChainDesc{};
        createSwapChainDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_NEW_DX12;

        createSwapChainDesc.dxgiFactory = pFactory;
        createSwapChainDesc.gameQueue = (ID3D12CommandQueue*)pDevice;
        createSwapChainDesc.desc = pDesc;
        createSwapChainDesc.swapchain = (IDXGISwapChain4**)ppSwapChain;


        fgSkipSCWrapping = true;
        Config::Instance()->dxgiSkipSpoofing = true;
        Config::Instance()->SkipHeapCapture = true;

        auto result = FfxApiProxy::D3D12_CreateContext()(&FrameGen_Dx12::fgSwapChainContext, &createSwapChainDesc.header, nullptr);

        Config::Instance()->SkipHeapCapture = false;
        Config::Instance()->dxgiSkipSpoofing = false;
        fgSkipSCWrapping = false;

        if (result == FFX_API_RETURN_OK)
        {
            scInfo.swapChainFormat = pDesc->BufferDesc.Format;
            scInfo.swapChainBufferCount = pDesc->BufferCount;
            scInfo.swapChain = (IDXGISwapChain4*)*ppSwapChain;
            fgSwapChains.insert_or_assign(pDesc->OutputWindow, scInfo);

            LOG_DEBUG("Created FSR-FG swapchain");
            return S_OK;
        }

        LOG_ERROR("D3D12_CreateContext error: {}", result);

        return E_INVALIDARG;
    }

    auto result = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    if (result == S_OK)
    {
        // check for SL proxy
        IID riid;
        IDXGISwapChain* real = nullptr;
        auto iidResult = IIDFromString(L"{ADEC44E2-61F0-45C3-AD9F-1B37379284FF}", &riid);

        if (iidResult == S_OK)
        {
            auto qResult = (*ppSwapChain)->QueryInterface(riid, (void**)&real);

            if (qResult == S_OK && real != nullptr)
            {
                LOG_INFO("Streamline proxy found");
                real->Release();
            }
            else
            {
                LOG_DEBUG("Streamline proxy not found");
            }
        }

        LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Windowed: {4}", pDesc->BufferDesc.Width, pDesc->BufferDesc.Height, (UINT)pDesc->BufferDesc.Format, pDesc->BufferCount, pDesc->Windowed);

        if (Util::GetProcessWindow() == pDesc->OutputWindow)
        {
            Config::Instance()->ScreenWidth = pDesc->BufferDesc.Width;
            Config::Instance()->ScreenHeight = pDesc->BufferDesc.Height;
        }

        LOG_DEBUG("created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDesc->OutputWindow);
        // TODO: Bind overlay menu event here
        *ppSwapChain = new WrappedIDXGISwapChain4(real == nullptr ? *ppSwapChain : real, pDevice, pDesc->OutputWindow, Present, ImGuiOverlayDx::CleanupRenderTarget);
        LOG_DEBUG("created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);
    }

    return result;
}

static HRESULT hkCreateSwapChainForHwnd(IDXGIFactory* This, IUnknown* pDevice, HWND hWnd, DXGI_SWAP_CHAIN_DESC1* pDesc,
                                        DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    LOG_FUNC();

    *ppSwapChain = nullptr;

    if (Config::Instance()->VulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    if (pDesc->Height == 2 && pDesc->Width == 2)
    {
        LOG_WARN("RTSS call!");
        return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    ID3D12CommandQueue* cq = nullptr;
    if (Config::Instance()->FGUseFGSwapChain.value_or(true) && !fgSkipSCWrapping && FfxApiProxy::InitFfxDx12() && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        SwapChainInfo scInfo{};
        scInfo.gameCommandQueue = cq;

        cq->SetName(L"GameQueueHwnd");
        cq->Release();

        fgQueues.push_back((ID3D12CommandQueue*)pDevice);

        ffxCreateContextDescFrameGenerationSwapChainForHwndDX12 createSwapChainDesc{};
        createSwapChainDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_FOR_HWND_DX12;

        createSwapChainDesc.fullscreenDesc = pFullscreenDesc;
        createSwapChainDesc.hwnd = hWnd;
        createSwapChainDesc.dxgiFactory = This;
        createSwapChainDesc.gameQueue = (ID3D12CommandQueue*)pDevice;
        createSwapChainDesc.desc = pDesc;
        createSwapChainDesc.swapchain = (IDXGISwapChain4**)ppSwapChain;

        Config::Instance()->dxgiSkipSpoofing = true;
        fgSkipSCWrapping = true;
        Config::Instance()->SkipHeapCapture = true;

        auto result = FfxApiProxy::D3D12_CreateContext()(&FrameGen_Dx12::fgSwapChainContext, &createSwapChainDesc.header, nullptr);

        Config::Instance()->SkipHeapCapture = false;
        fgSkipSCWrapping = false;
        Config::Instance()->dxgiSkipSpoofing = false;

        if (result == FFX_API_RETURN_OK)
        {
            scInfo.swapChainFormat = pDesc->Format;
            scInfo.swapChainBufferCount = pDesc->BufferCount;
            scInfo.swapChain = (IDXGISwapChain4*)*ppSwapChain;
            fgSwapChains.insert_or_assign(hWnd, scInfo);

            LOG_DEBUG("Created FSR-FG swapchain");
            return S_OK;
        }

        LOG_ERROR("_createContext error: {}", result);

        return result;
    }

    auto result = oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    if (result == S_OK)
    {
        // check for SL proxy
        IID riid;
        IDXGISwapChain1* real = nullptr;
        auto iidResult = IIDFromString(L"{ADEC44E2-61F0-45C3-AD9F-1B37379284FF}", &riid);

        if (iidResult == S_OK)
        {
            IUnknown* real = nullptr;
            auto qResult = (*ppSwapChain)->QueryInterface(riid, (void**)&real);

            if (qResult == S_OK && real != nullptr)
            {
                LOG_INFO("Streamline proxy found");
                real->Release();
            }
            else
            {
                LOG_DEBUG("Streamline proxy not found");
            }
        }

        LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Flags: {4:X}", pDesc->Width, pDesc->Height, (UINT)pDesc->Format, pDesc->BufferCount, pDesc->Flags);

        if (Util::GetProcessWindow() == hWnd)
        {
            Config::Instance()->ScreenWidth = pDesc->Width;
            Config::Instance()->ScreenHeight = pDesc->Height;
        }

        LOG_DEBUG("created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)hWnd);
        // TODO: Bind overlay menu event here
        *ppSwapChain = new WrappedIDXGISwapChain4(real == nullptr ? *ppSwapChain : real, pDevice, hWnd, Present, ImGuiOverlayDx::CleanupRenderTarget);
        LOG_DEBUG("created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);
    }

    return result;
}

static HRESULT hkCreateDXGIFactory(REFIID riid, IDXGIFactory** ppFactory)
{
    auto result = o_CreateDXGIFactory(riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

    if (result == S_OK && oCreateSwapChain == nullptr)
    {
        void** pFactoryVTable = *reinterpret_cast<void***>(*ppFactory);

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
    auto result = o_CreateDXGIFactory1(riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

    if (result == S_OK && oCreateSwapChainForHwnd == nullptr)
    {
        IDXGIFactory2* factory2 = nullptr;

        if ((*ppFactory)->QueryInterface(IID_PPV_ARGS(&factory2)) == S_OK && factory2 != nullptr)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory2);

            bool skip = false;

            if (oCreateSwapChain == nullptr)
                oCreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            oCreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

            if (oCreateSwapChainForHwnd != nullptr)
            {
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

                DetourTransactionCommit();
            }

            factory2->Release();
            factory2 = nullptr;
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory2(UINT Flags, REFIID riid, IDXGIFactory2** ppFactory)
{
    auto result = o_CreateDXGIFactory2(Flags, riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

    if (result == S_OK && oCreateSwapChainForHwnd == nullptr)
    {
        IDXGIFactory2* factory2 = nullptr;

        if ((*ppFactory)->QueryInterface(IID_PPV_ARGS(&factory2)) == S_OK && factory2 != nullptr)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory2);

            bool skip = false;

            if (oCreateSwapChain == nullptr)
                oCreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            oCreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

            if (oCreateSwapChainForHwnd != nullptr)
            {
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain, hkCreateSwapChain);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

                DetourTransactionCommit();
            }

            factory2->Release();
            factory2 = nullptr;
        }
    }

    return result;
}

static HRESULT hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter)
{
    auto result = ptrEnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, ppvAdapter);

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter)
{
    auto result = ptrEnumAdapterByLuid(This, AdapterLuid, riid, ppvAdapter);

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter)
{
    auto result = ptrEnumAdapters1(This, Adapter, ppAdapter);

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
}

static HRESULT hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter)
{
    auto result = ptrEnumAdapters(This, Adapter, ppAdapter);

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
}

#pragma endregion

#pragma region DirectX hooks



static void HookCommandList(ID3D12Device* InDevice)
{
    if (o_OMSetRenderTargets != nullptr || o_DrawIndexedInstanced != nullptr)
        return;

    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;

    if (InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)) == S_OK)
    {
        if (InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)) == S_OK)
        {
            // Get the vtable pointer
            PVOID* pVTable = *(PVOID**)commandList;

            // hudless shader
            o_OMSetRenderTargets = (PFN_OMSetRenderTargets)pVTable[46];
            o_SetGraphicsRootDescriptorTable = (PFN_SetGraphicsRootDescriptorTable)pVTable[32];
            o_DrawInstanced = (PFN_DrawInstanced)pVTable[12];
            o_DrawIndexedInstanced = (PFN_DrawIndexedInstanced)pVTable[13];

            // hudless compute
            o_SetComputeRootDescriptorTable = (PFN_SetComputeRootDescriptorTable)pVTable[31];
            o_Dispatch = (PFN_Dispatch)pVTable[14];

            // hudless copy
#ifdef USE_COPY_RESOURCE
            o_CopyResource = (PFN_CopyResource)pVTable[17];
#endif
            o_CopyTextureRegion = (PFN_CopyTextureRegion)pVTable[16];

#ifdef USE_RESOURCE_DISCARD
            // release resource
            o_DiscardResource = (PFN_DiscardResource)pVTable[51];
#endif

            if (o_OMSetRenderTargets != nullptr || o_DrawIndexedInstanced != nullptr || o_DrawInstanced != nullptr)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (o_OMSetRenderTargets != nullptr)
                    DetourAttach(&(PVOID&)o_OMSetRenderTargets, hkOMSetRenderTargets);

                if (o_DrawIndexedInstanced != nullptr)
                    DetourAttach(&(PVOID&)o_DrawIndexedInstanced, hkDrawIndexedInstanced);

                if (o_DrawInstanced != nullptr)
                    DetourAttach(&(PVOID&)o_DrawInstanced, hkDrawInstanced);

                if (o_CopyTextureRegion != nullptr)
                    DetourAttach(&(PVOID&)o_CopyTextureRegion, hkCopyTextureRegion);

#ifdef USE_COPY_RESOURCE
                if (o_CopyResource != nullptr)
                    DetourAttach(&(PVOID&)o_CopyResource, hkCopyResource);
#endif
                if (o_SetGraphicsRootDescriptorTable != nullptr)
                    DetourAttach(&(PVOID&)o_SetGraphicsRootDescriptorTable, hkSetGraphicsRootDescriptorTable);

                if (o_SetComputeRootDescriptorTable != nullptr)
                    DetourAttach(&(PVOID&)o_SetComputeRootDescriptorTable, hkSetComputeRootDescriptorTable);

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

        if (Config::Instance()->FGUseFGSwapChain.value_or(true) && Config::Instance()->OverlayMenu.value_or(true))
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

    if (Config::Instance()->FGUseFGSwapChain.value_or(true) && Config::Instance()->OverlayMenu.value_or(true))
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

    bool rtss = false;

    // Assuming RTSS is creating a D3D11on12 device, not sure why but sometimes RTSS tries to create 
    // it's D3D11on12 device with old CommandQueue which results crash
    // I am changing it's CommandQueue with current swapchain's command queue
    if (HooksDx::GameCommandQueue != nullptr && *ppCommandQueues != HooksDx::GameCommandQueue && GetModuleHandle(L"RTSSHooks64.dll") != nullptr && pDevice == g_pd3dDeviceParam)
    {
        LOG_INFO("Replaced RTSS CommandQueue with correct one {0:X} -> {1:X}", (UINT64)*ppCommandQueues, (UINT64)HooksDx::GameCommandQueue);
        *ppCommandQueues = HooksDx::GameCommandQueue;
        rtss = true;
    }

    auto result = o_D3D11On12CreateDevice(pDevice, Flags, pFeatureLevels, FeatureLevels, ppCommandQueues, NumQueues, NodeMask, ppDevice, ppImmediateContext, pChosenFeatureLevel);

    if (result == S_OK && *ppDevice != nullptr && !rtss && !_d3d12Captured)
    {
        LOG_INFO("Device captured, D3D11Device: {0:X}", (UINT64)*ppDevice);
        d3d11on12Device = *ppDevice;
        HookToDevice(d3d11on12Device);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static HRESULT hkD3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                   UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
    LOG_FUNC();

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

    auto result = o_D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

    if (result == S_OK && *ppDevice != nullptr && !_d3d12Captured)
    {
        LOG_INFO("Device captured");
        d3d11Device = *ppDevice;

        HookToDevice(d3d11Device);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static HRESULT hkD3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels,
                                               UINT FeatureLevels, UINT SDKVersion, DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
{
    LOG_FUNC();

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
        LOG_WARN("RTSS call!");
        return o_D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
    }

    IDXGISwapChain* buffer = nullptr;
    auto result = o_D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, &buffer, ppDevice, pFeatureLevel, ppImmediateContext);

    if (result == S_OK && *ppDevice != nullptr && !_d3d12Captured)
    {
        LOG_INFO("Device captured");
        d3d11Device = *ppDevice;

        HookToDevice(d3d11Device);

        if (pSwapChainDesc != nullptr)
        {
            LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Windowed: {4}", pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height, (UINT)pSwapChainDesc->BufferDesc.Format, pSwapChainDesc->BufferCount, pSwapChainDesc->Windowed);

            if (Util::GetProcessWindow() == pSwapChainDesc->OutputWindow)
            {
                Config::Instance()->ScreenWidth = pSwapChainDesc->BufferDesc.Width;
                Config::Instance()->ScreenHeight = pSwapChainDesc->BufferDesc.Height;
            }

            IDXGIFactory* factory = nullptr;
            result = CreateDXGIFactory(IID_PPV_ARGS(&factory));
            if (result == S_OK)
            {
                LOG_DEBUG("creating new swapchain");
                result = factory->CreateSwapChain(*ppDevice, pSwapChainDesc, ppSwapChain);

                if (result == S_OK)
                    LOG_DEBUG("created new WrappedIDXGISwapChain4: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)d3d11Device);
                else
                    LOG_DEBUG("factory->CreateSwapChain error: {0:X}", (UINT64)result);

                factory->Release();
            }
        }
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static HRESULT hkD3D12CreateDevice(IDXGIAdapter* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    LOG_FUNC();

    auto result = o_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (result == S_OK && *ppDevice != nullptr)
    {
        LOG_DEBUG("Device captured: {0:X}", (size_t)*ppDevice);
        g_pd3dDeviceParam = (ID3D12Device*)*ppDevice;
        HookToDevice(g_pd3dDeviceParam);
        _d3d12Captured = true;
    }

    LOG_FUNC_RESULT(result);

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
        LOG_INFO("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pDesc->Filter);
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

    if (newDesc.MipLODBias < 0.0f)
    {
        if (Config::Instance()->MipmapBiasOverride.has_value())
        {
            LOG_INFO("Overriding mipmap bias {0} -> {1}", pDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());
            newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();
        }

        Config::Instance()->lastMipBias = newDesc.MipLODBias;
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
        LOG_INFO("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pSamplerDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pSamplerDesc->Filter);
        newDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        newDesc.MaxAnisotropy = Config::Instance()->AnisotropyOverride.value();
    }
    else
    {
        newDesc.Filter = pSamplerDesc->Filter;
        newDesc.MaxAnisotropy = pSamplerDesc->MaxAnisotropy;
    }

    newDesc.MipLODBias = pSamplerDesc->MipLODBias;

    if (newDesc.MipLODBias < 0.0f)
    {
        if (Config::Instance()->MipmapBiasOverride.has_value())
        {
            LOG_INFO("Overriding mipmap bias {0} -> {1}", pSamplerDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());
            newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();
        }

        Config::Instance()->lastMipBias = newDesc.MipLODBias;
    }

    return o_CreateSamplerState(This, &newDesc, ppSamplerState);
}

#pragma endregion

void DeatachAllHooks()
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
}

void HooksDx::HookDx()
{
    if (_isInited)
        return;

    o_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("d3d12.dll", "D3D12CreateDevice");
    if (o_D3D12CreateDevice != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_D3D12CreateDevice, hkD3D12CreateDevice);

        DetourTransactionCommit();
    }

    o_D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)DetourFindFunction("d3d11.dll", "D3D11CreateDevice");
    o_D3D11CreateDeviceAndSwapChain = (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)DetourFindFunction("d3d11.dll", "D3D11CreateDeviceAndSwapChain");
    o_D3D11On12CreateDevice = (PFN_D3D11ON12_CREATE_DEVICE)DetourFindFunction("d3d11.dll", "D3D11On12CreateDevice");
    if (o_D3D11CreateDevice != nullptr || o_D3D11On12CreateDevice != nullptr || o_D3D11CreateDeviceAndSwapChain != nullptr)
    {
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

    o_CreateDXGIFactory = (PFN_CreateDXGIFactory)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
    o_CreateDXGIFactory1 = (PFN_CreateDXGIFactory1)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
    o_CreateDXGIFactory2 = (PFN_CreateDXGIFactory2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

    if (o_CreateDXGIFactory1 != nullptr)
    {
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

void HooksDx::UnHookDx()
{
    DeatachAllHooks();
    _isInited = false;
}

UINT FrameGen_Dx12::ClearFrameResources()
{
    LOG_DEBUG_ONLY(" <-- {}", fgFrameIndex);

    fgFrameIndex = (fgFrameIndex + 1) % FG_BUFFER_SIZE;

    LOG_DEBUG_ONLY(" <-- {}", fgFrameIndex);
    fgUpscaledFound = false;

    return fgFrameIndex;
}

UINT FrameGen_Dx12::GetFrame()
{
    return fgFrameIndex;
}

void FrameGen_Dx12::NewFrame()
{
    auto fIndex = fgFrameIndex;
    auto newIndex = (fIndex + 2) % FG_BUFFER_SIZE;

    {
        fgCopySource[newIndex] = nullptr;
    }

    if (fgPossibleHudless[newIndex].size() != 0)
    {
        std::unique_lock<std::shared_mutex> lock(hudlessMutex[newIndex]);
        fgPossibleHudless[newIndex].clear();
    }

    if (FrameGen_Dx12::fgHUDlessCaptureCounter[newIndex] != 0)
    {
        std::unique_lock<std::shared_mutex> lock(counterMutex[newIndex]);
        FrameGen_Dx12::fgHUDlessCaptureCounter[newIndex] = 0;
    }
}


void FrameGen_Dx12::ReleaseFGObjects()
{
    for (size_t i = 0; i < 4; i++)
    {
        if (FrameGen_Dx12::fgCopyCommandAllocators[i] != nullptr)
        {
            FrameGen_Dx12::fgCopyCommandAllocators[i]->Release();
            FrameGen_Dx12::fgCopyCommandAllocators[i] = nullptr;
        }
    }

    if (FrameGen_Dx12::fgCopyCommandList != nullptr)
    {
        FrameGen_Dx12::fgCopyCommandList->Release();
        FrameGen_Dx12::fgCopyCommandList = nullptr;
    }

    if (FrameGen_Dx12::fgCopyCommandQueue != nullptr)
    {
        FrameGen_Dx12::fgCopyCommandQueue->Release();
        FrameGen_Dx12::fgCopyCommandQueue = nullptr;
    }

    if (FrameGen_Dx12::fgFormatTransfer != nullptr)
    {
        delete FrameGen_Dx12::fgFormatTransfer;
        FrameGen_Dx12::fgFormatTransfer = nullptr;
    }
}

void FrameGen_Dx12::CreateFGObjects(ID3D12Device* InDevice)
{
    if (FrameGen_Dx12::fgCopyCommandQueue != nullptr)
        return;

    do
    {
        HRESULT result;

        for (size_t i = 0; i < 4; i++)
        {
            result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameGen_Dx12::fgCopyCommandAllocators[i]));
            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocators[{0}]: {1:X}", i, (unsigned long)result);
                break;
            }
            FrameGen_Dx12::fgCopyCommandAllocators[i]->SetName(L"fgCopyCommandAllocator");
        }

        result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FrameGen_Dx12::fgCopyCommandAllocators[0], NULL, IID_PPV_ARGS(&FrameGen_Dx12::fgCopyCommandList));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandList: {0:X}", (unsigned long)result);
            break;
        }
        FrameGen_Dx12::fgCopyCommandList->SetName(L"fgCopyCommandList");

        result = FrameGen_Dx12::fgCopyCommandList->Close();
        if (result != S_OK)
        {
            LOG_ERROR("HooksDx::fgCopyCommandList->Close: {0:X}", (unsigned long)result);
            break;
        }

        // Create a command queue for frame generation
        D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
        copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        copyQueueDesc.NodeMask = 0;

        if (Config::Instance()->FGHighPriority.value_or(false))
            copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        else
            copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

        HRESULT hr = InDevice->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&FrameGen_Dx12::fgCopyCommandQueue));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandQueue: {0:X}", (unsigned long)result);
            break;
        }
        FrameGen_Dx12::fgCopyCommandQueue->SetName(L"fgCopyCommandQueue");

        FrameGen_Dx12::fgFormatTransfer = new FT_Dx12("FormatTransfer", InDevice, HooksDx::swapchainFormat);

    } while (false);
}

void FrameGen_Dx12::CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext)
{
    if (FrameGen_Dx12::fgContext != nullptr)
    {
        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = true;
        m_FrameGenerationConfig.swapChain = HooksDx::currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        auto result = FfxApiProxy::D3D12_Configure()(&FrameGen_Dx12::fgContext, &m_FrameGenerationConfig.header);

        FrameGen_Dx12::fgIsActive = (result == FFX_API_RETURN_OK);

        LOG_DEBUG("Reactivate");

        return;
    }

    ffxCreateBackendDX12Desc backendDesc{};
    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
    backendDesc.device = InDevice;

    ffxCreateContextDescFrameGeneration createFg{};
    createFg.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATION;
    createFg.displaySize = { deviceContext->DisplayWidth(), deviceContext->DisplayHeight() };
    createFg.maxRenderSize = { deviceContext->DisplayWidth(), deviceContext->DisplayHeight() };
    createFg.flags = 0;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_HIGH_DYNAMIC_RANGE;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DEPTH_INVERTED;

    if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVJittered)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;

    if ((deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0)
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;

    if (Config::Instance()->FGAsync.value_or(false))
        createFg.flags |= FFX_FRAMEGENERATION_ENABLE_ASYNC_WORKLOAD_SUPPORT;

    createFg.backBufferFormat = ffxApiGetSurfaceFormatDX12(HooksDx::swapchainFormat);
    createFg.header.pNext = &backendDesc.header;

    Config::Instance()->dxgiSkipSpoofing = true;
    Config::Instance()->SkipHeapCapture = true;
    ffxReturnCode_t retCode = FfxApiProxy::D3D12_CreateContext()(&FrameGen_Dx12::fgContext, &createFg.header, nullptr);
    Config::Instance()->SkipHeapCapture = false;
    Config::Instance()->dxgiSkipSpoofing = false;
    LOG_INFO("_createContext result: {0:X}", retCode);

    FrameGen_Dx12::fgIsActive = (retCode == FFX_API_RETURN_OK);

    LOG_DEBUG("Create");
}

void FrameGen_Dx12::StopAndDestroyFGContext(bool destroy, bool shutDown)
{
    FrameGen_Dx12::fgSkipHudlessChecks = false;
    Config::Instance()->dxgiSkipSpoofing = true;

    if (FrameGen_Dx12::fgContext != nullptr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = false;
        m_FrameGenerationConfig.swapChain = HooksDx::currentSwapchain;
        m_FrameGenerationConfig.presentCallback = nullptr;
        m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});

        auto result = FfxApiProxy::D3D12_Configure()(&FrameGen_Dx12::fgContext, &m_FrameGenerationConfig.header);

        FrameGen_Dx12::fgIsActive = false;

        if (!shutDown)
            LOG_INFO("    FG _configure result: {0:X}", result);
    }

    if (destroy && FrameGen_Dx12::fgContext != nullptr)
    {
        auto result = FfxApiProxy::D3D12_DestroyContext()(&FrameGen_Dx12::fgContext, nullptr);

        if (!shutDown)
            LOG_INFO("    FG _destroyContext result: {0:X}", result);

        FrameGen_Dx12::fgContext = nullptr;
    }

    Config::Instance()->dxgiSkipSpoofing = false;

    if (shutDown)
        ReleaseFGObjects();
}
