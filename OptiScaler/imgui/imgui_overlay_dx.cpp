#include "imgui_overlay_base.h"
#include "imgui_overlay_dx.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"
#include <d3d11on12.h>

#include "wrapped_swapchain.h"

#include <ankerl/unordered_dense.h>
#include "../depth_upscale/DU_Dx12.h"
#include <shared_mutex>

// FG stuff

typedef struct BackbufferInfo
{
    // RTV handle for backbuffer
    SIZE_T cpuHandle = NULL;

    // Resource copied or used to draw on backbuffer
    ID3D12Resource* prev = nullptr;

    // Handle of previous resource
    SIZE_T prevCpuHandle = NULL;

    // Captured hudless resource
    ID3D12Resource* hudless = nullptr;
} backbuffer_info;

typedef struct HeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T cpuEnd = NULL;
    SIZE_T gpuStart = NULL;
    SIZE_T gpuEnd = NULL;
    UINT numDescriptors = 0;
} heap_info;

// Hooks for FG
typedef void (*PFN_CreateRenderTargetView)(ID3D12Device* This, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void (*PFN_OMSetRenderTargets)(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
                                       BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
typedef void (*PFN_DrawIndexedInstanced)(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void (*PFN_DrawInstanced)(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
typedef void (*PFN_CopyResource)(ID3D12GraphicsCommandList* This, ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource);
typedef void (*PFN_CopyTextureRegion)(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox);
typedef HRESULT(*PFN_CreateDescriptorHeap)(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap);
typedef void(*PFN_CreateShaderResourceView)(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
typedef void(*PFN_SetGraphicsRootDescriptorTable)(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
typedef void(*PFN_CopyDescriptors)(UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges,    
                                   D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
typedef void(*PFN_CopyDescriptorsSimple)(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);


static PFN_CreateRenderTargetView o_CreateRenderTargetView = nullptr;
static PFN_OMSetRenderTargets o_OMSetRenderTargets = nullptr;
static PFN_DrawIndexedInstanced o_DrawIndexedInstanced = nullptr;
static PFN_DrawInstanced o_DrawInstanced = nullptr;
static PFN_CopyResource o_CopyResource = nullptr;
static PFN_CopyTextureRegion o_CopyTextureRegion = nullptr;
static PFN_CreateDescriptorHeap o_CreateDescriptorHeap = nullptr;
static PFN_CreateShaderResourceView o_CreateShaderResourceView = nullptr;
static PFN_SetGraphicsRootDescriptorTable o_SetGraphicsRootDescriptorTable = nullptr;
static PFN_CopyDescriptors o_CopyDescriptors = nullptr;
static PFN_CopyDescriptorsSimple o_CopyDescriptorsSimple = nullptr;

// FSR 3.x methods
static PfnFfxCreateContext _createContext = nullptr;
static PfnFfxDestroyContext _destroyContext = nullptr;
static PfnFfxConfigure _configure = nullptr;
static PfnFfxQuery _query = nullptr;
static PfnFfxDispatch _dispatch = nullptr;

/// <summary>
/// Captured swapchains
/// </summary>
static ankerl::unordered_dense::map <HWND, IDXGISwapChain*> fgSwapChains;

/// <summary>
/// Captured swapchain buffers
/// </summary>
static ankerl::unordered_dense::map <ID3D12Resource*, BackbufferInfo> fgBuffers;

/// <summary>
/// Captured heap infos
/// </summary>
static std::list<HeapInfo> fgHeaps;

/// <summary>
/// Captured resources by gpu addresses
/// </summary>
static ankerl::unordered_dense::map <ID3D12Resource*, HeapInfo> fgHandleAddressResources;
static ankerl::unordered_dense::map <SIZE_T, ID3D12Resource*> fgHandleAddressResourcesByGpuHandle;

static std::shared_mutex heapMutex;
static std::shared_mutex resourceMutex;

/// <summary>
/// Captured cpu handles
/// </summary>
//static ankerl::unordered_dense::map <ID3D12Resource*, SIZE_T> fgAllCpuHandles;

/// <summary>
/// Counter for GetHUDless calls, used to skip calls
/// </summary>
static int fgHUDlessCaptureCounter = 0;

/// <summary>
/// To prevent double wrapping FG swapchain
/// </summary>
static bool fgSkipSCWrapping = false;

/// <summary>
/// Backbuffer used for current frame
/// </summary>
static ID3D12Resource* fgCurrentSCBuffer = nullptr;
static ID3D12Resource* fgPreviousBuffer = nullptr;

/// <summary>
/// Found backbuffer for current frame
/// </summary>
static bool fgSCBufferCaptured = false;

/// <summary>
/// Last captured upscaled hudless frame number
/// </summary>
static UINT64 fgHudlessFrame = 0;

/// <summary>
/// Used for frametime calculation
/// </summary>
static UINT64 fgLastFrameTime = 0;

/// <summary>
/// Swapchain frame counter
/// </summary>
static UINT64 frameCounter = 0;

/// <summary>
/// Swapchain frame target while capturing RTVs etc
/// </summary>
static UINT64 fgTargetFrame = 0;

/// <summary>
/// Is RTV capturing for swapchain buffers active
/// </summary>
static bool fgCaptureRTVs = false;

/// <summary>
/// Captured swapchain buffer count
/// </summary>
static int fgCapturedBufferCount = 0;

// dxgi stuff
typedef HRESULT(*PFN_CreateDXGIFactory)(REFIID riid, IDXGIFactory** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory1)(REFIID riid, IDXGIFactory1** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory2)(UINT Flags, REFIID riid, _COM_Outptr_ IDXGIFactory2** ppFactory);

typedef HRESULT(WINAPI* PFN_EnumAdapterByGpuPreference2)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapterByLuid2)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters12)(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters2)(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);

static PFN_CreateDXGIFactory o_CreateDXGIFactory = nullptr;
static PFN_CreateDXGIFactory1 o_CreateDXGIFactory1 = nullptr;
static PFN_CreateDXGIFactory2 o_CreateDXGIFactory2 = nullptr;

inline static PFN_EnumAdapters2 ptrEnumAdapters = nullptr;
inline static PFN_EnumAdapters12 ptrEnumAdapters1 = nullptr;
inline static PFN_EnumAdapterByLuid2 ptrEnumAdapterByLuid = nullptr;
inline static PFN_EnumAdapterByGpuPreference2 ptrEnumAdapterByGpuPreference = nullptr;
inline static PFN_Present o_Present = nullptr;
inline static PFN_Present1 o_Present1 = nullptr;

static PFN_CreateSwapChain oCreateSwapChain = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd = nullptr;

// MipMap hooks

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

// current command queue for dx12 swapchain
static IUnknown* currentSCCommandQueue = nullptr;

// menu
static int const NUM_BACK_BUFFERS = 8;
static bool _dx11Device = false;
static bool _dx12Device = false;

// for dx11
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static ID3D11RenderTargetView* g_pd3dRenderTarget = nullptr;

// for dx12
static ID3D12Device* g_pd3dDeviceParam = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };

// status
static bool _isInited = false;

// for showing 
static bool _showRenderImGuiDebugOnce = true;

// mutexes
static std::mutex _dx11CleanMutex;
static std::mutex _dx12CleanMutex;


static void RenderImGui_DX11(IDXGISwapChain* pSwapChain);
static void RenderImGui_DX12(IDXGISwapChain* pSwapChain);
static void DeatachAllHooks();
static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static HRESULT hkCreateSamplerState(ID3D11Device* This, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);
static HRESULT hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
static HRESULT hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
static HRESULT hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);

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

static void LoadFSR31Funcs()
{

    ID3D12Resource* textureResource;
    ID3D12DescriptorHeap* srvHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;

    LOG_DEBUG("Loading amd_fidelityfx_dx12.dll methods");

    auto file = Util::DllPath().parent_path() / "amd_fidelityfx_dx12.dll";
    LOG_INFO("Trying to load {}", file.string());

    auto _dll = LoadLibrary(file.wstring().c_str());
    if (_dll != nullptr)
    {
        _configure = (PfnFfxConfigure)GetProcAddress(_dll, "ffxConfigure");
        _createContext = (PfnFfxCreateContext)GetProcAddress(_dll, "ffxCreateContext");
        _destroyContext = (PfnFfxDestroyContext)GetProcAddress(_dll, "ffxDestroyContext");
        _dispatch = (PfnFfxDispatch)GetProcAddress(_dll, "ffxDispatch");
        _query = (PfnFfxQuery)GetProcAddress(_dll, "ffxQuery");
    }

    if (_configure == nullptr)
    {
        LOG_INFO("Trying to load amd_fidelityfx_dx12.dll with detours");

        _configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxConfigure");
        _createContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxCreateContext");
        _destroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDestroyContext");
        _dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDispatch");
        _query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxQuery");
    }

    if (_configure != nullptr)
        LOG_INFO("amd_fidelityfx_dx12.dll methods loaded!");
    else
        LOG_ERROR("can't load amd_fidelityfx_dx12.dll methods!");
}

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
        LOG_ERROR("    FG GetHeapProperties result: {0:X}", (UINT64)hr);
        return false;
    }

    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(OutResource));

    if (hr != S_OK)
    {
        LOG_ERROR("    FG CreateCommittedResource result: {0:X}", (UINT64)hr);
        return false;
    }

    (*OutResource)->SetName(L"HUDLess_Buffer");
    return true;
}

static void GetHUDless(ID3D12GraphicsCommandList* This, D3D12_RESOURCE_STATES State)
{
    LOG_DEBUG_ONLY("FrameCount: {}, fgHudlessFrame: {}", Config::Instance()->CurrentFeature->FrameCount(), fgHudlessFrame);

    if (fgHudlessFrame == 0)
    {
        fgHudlessFrame = Config::Instance()->CurrentFeature->FrameCount();
        fgSCBufferCaptured = false;
        ImGuiOverlayDx::upscaleRan = false;
        return;
    }

    if (fgSCBufferCaptured && fgCurrentSCBuffer != nullptr && Config::Instance()->CurrentFeature != nullptr && fgHudlessFrame != Config::Instance()->CurrentFeature->FrameCount() && fgBuffers.contains(fgCurrentSCBuffer))
    {
        if (fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount())
            return;

        // hudless captured for this frame
        fgHudlessFrame = Config::Instance()->CurrentFeature->FrameCount();
        auto frame = fgHudlessFrame;
        fgSCBufferCaptured = false;

        LOG_DEBUG("    HUDFix running, frame: {0}", frame);

        auto bufferInfo = &fgBuffers[fgCurrentSCBuffer];

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = fgCurrentSCBuffer;
        barrier.Transition.StateBefore = State;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.Subresource = 0;
        This->ResourceBarrier(1, &barrier);

        // take hudless copy
        This->CopyResource(bufferInfo->hudless, fgCurrentSCBuffer);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.StateAfter = State;
        This->ResourceBarrier(1, &barrier);

        // current depth and mv
        auto resIndex = ImGuiOverlayDx::resourceIndex;
        // switch dlss targets for next depth and mv 
        ImGuiOverlayDx::resourceIndex = (ImGuiOverlayDx::resourceIndex + 1) % 2;

        ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};

        m_FrameGenerationConfig.HUDLessColor = ffxApiGetResourceDX12(bufferInfo->hudless, FFX_API_RESOURCE_STATE_COPY_DEST, 0);

        m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
        m_FrameGenerationConfig.frameGenerationEnabled = true; // check here
        m_FrameGenerationConfig.flags = 0;

        if (Config::Instance()->FGDebugView.value_or(false))
            m_FrameGenerationConfig.flags |= FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW; // check here

        m_FrameGenerationConfig.allowAsyncWorkloads = Config::Instance()->FGAsync.value_or(true);

        // assume symmetric letterbox
        m_FrameGenerationConfig.generationRect.left = 0;
        m_FrameGenerationConfig.generationRect.top = 0;
        m_FrameGenerationConfig.generationRect.width = Config::Instance()->CurrentFeature->DisplayWidth();
        m_FrameGenerationConfig.generationRect.height = Config::Instance()->CurrentFeature->DisplayHeight();

        m_FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
            {
                return _dispatch(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
            };

        m_FrameGenerationConfig.frameGenerationCallbackUserContext = &ImGuiOverlayDx::fgContext;
        m_FrameGenerationConfig.onlyPresentGenerated = Config::Instance()->FGOnlyGenerated; // check here
        m_FrameGenerationConfig.frameID = Config::Instance()->CurrentFeature->FrameCount();
        m_FrameGenerationConfig.swapChain = ImGuiOverlayDx::currentSwapchain;

        ffxConfigureDescGlobalDebug1 debugDesc;
        debugDesc.header.type = FFX_API_CONFIGURE_DESC_TYPE_GLOBALDEBUG1;
        debugDesc.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_VERBOSE;
        debugDesc.fpMessage = FfxFgLogCallback;
        m_FrameGenerationConfig.header.pNext = &debugDesc.header;

        Config::Instance()->dxgiSkipSpoofing = true;
        ffxReturnCode_t retCode = _configure(&ImGuiOverlayDx::fgContext, &m_FrameGenerationConfig.header);
        Config::Instance()->dxgiSkipSpoofing = false;
        LOG_DEBUG("    HUDFix _configure result: {0:X}, frame: {1}", retCode, frame);

        if (retCode == FFX_API_RETURN_OK)
        {
            ffxCreateBackendDX12Desc backendDesc{};
            backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
            backendDesc.device = g_pd3dDeviceParam;

            ffxDispatchDescFrameGenerationPrepare dfgPrepare{};
            dfgPrepare.header.type = FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATION_PREPARE;
            dfgPrepare.header.pNext = &backendDesc.header;

            dfgPrepare.commandList = This;
            dfgPrepare.frameID = Config::Instance()->CurrentFeature->FrameCount();
            dfgPrepare.flags = m_FrameGenerationConfig.flags;

            dfgPrepare.renderSize = { Config::Instance()->CurrentFeature->RenderWidth(), Config::Instance()->CurrentFeature->RenderHeight() };

            dfgPrepare.jitterOffset.x = ImGuiOverlayDx::jitterX;
            dfgPrepare.jitterOffset.y = ImGuiOverlayDx::jitterY;

            dfgPrepare.motionVectors = ffxApiGetResourceDX12(ImGuiOverlayDx::paramVelocity[resIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
            dfgPrepare.depth = ffxApiGetResourceDX12(ImGuiOverlayDx::paramDepth[resIndex], FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);

            dfgPrepare.motionVectorScale.x = ImGuiOverlayDx::mvScaleX;
            dfgPrepare.motionVectorScale.y = ImGuiOverlayDx::mvScaleY;

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

            float msDelta = 0.0;
            auto now = Util::MillisecondsNow();

            if (fgLastFrameTime != 0)
            {
                msDelta = now - fgLastFrameTime;
                LOG_DEBUG("    HUDFix msDelta: {0}, frame: {1}", msDelta, Config::Instance()->CurrentFeature->FrameCount());
            }

            fgLastFrameTime = now;

            dfgPrepare.frameTimeDelta = msDelta;

            //#ifdef _DEBUG
            //            if (msDelta < 110.0 || msDelta > 1000.0)
            //            {
            //                Config::Instance()->dxgiSkipSpoofing = true;
            //                retCode = _dispatch(&ImGuiOverlayDx::fgContext, &dfgPrepare.header);
            //                Config::Instance()->dxgiSkipSpoofing = false;
            //                LOG_DEBUG("    HUDFix _dispatch result: {0}, frame: {1}", retCode, frame);
            //            }
            //            else
            //            {
            //                LOG_WARN("    HUDFix delta too big disabling FG: {0}", msDelta);
            //
            //                Config::Instance()->FGEnabled = false;
            //
            //                ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};
            //                m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
            //                m_FrameGenerationConfig.frameGenerationEnabled = false;
            //                m_FrameGenerationConfig.swapChain = ImGuiOverlayDx::currentSwapchain;
            //                m_FrameGenerationConfig.presentCallback = nullptr;
            //                m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});
            //                Config::Instance()->dxgiSkipSpoofing = true;
            //                _configure(&ImGuiOverlayDx::fgContext, &m_FrameGenerationConfig.header);
            //                Config::Instance()->dxgiSkipSpoofing = false;
            //
            //                _destroyContext(&ImGuiOverlayDx::fgContext, nullptr);
            //                ImGuiOverlayDx::fgContext = nullptr;
            //            }
            //#else

            Config::Instance()->dxgiSkipSpoofing = true;
            retCode = _dispatch(&ImGuiOverlayDx::fgContext, &dfgPrepare.header);
            Config::Instance()->dxgiSkipSpoofing = false;
            LOG_DEBUG("    HUDFix _dispatch result: {0}, frame: {1}", retCode, frame);
            //#endif // _DEBUG

            ImGuiOverlayDx::upscaleRan = false;
        }
    }
}

static void hkCreateRenderTargetView(ID3D12Device* This, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    o_CreateRenderTargetView(This, pResource, pDesc, DestDescriptor);

    // Swapchain changed
    if (Config::Instance()->SCChanged)
    {
        LOG_DEBUG_ONLY("     FG SCChanged: true");
        fgCaptureRTVs = true;
        fgTargetFrame = frameCounter + 60;

        for (auto const& [key, val] : fgBuffers)
        {
            if (val.hudless != nullptr)
                val.hudless->Release();
        }

        fgBuffers.clear();

        fgCapturedBufferCount = 0;
        Config::Instance()->SCChanged = false;
    }

    if (pResource == nullptr)
        return;

    {
        std::shared_lock<std::shared_mutex> lock(heapMutex);

        // Try to match resource with cpu & gpu handle info
        for (auto& val : fgHeaps)
        {
            if (val.cpuStart <= DestDescriptor.ptr && val.cpuEnd >= DestDescriptor.ptr)
            {
                auto increment = This->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                auto addr = DestDescriptor.ptr - val.cpuStart;
                auto index = addr / increment;
                auto gpuAddr = val.gpuStart + (index * increment);

                HeapInfo info{};
                info.cpuStart = DestDescriptor.ptr;
                info.gpuStart = gpuAddr;

                std::unique_lock<std::shared_mutex> lock2(resourceMutex);
                fgHandleAddressResources.insert_or_assign(pResource, info);

                if (val.gpuStart != 0)
                    fgHandleAddressResourcesByGpuHandle.insert_or_assign(gpuAddr, pResource);
                else
                    LOG_DEBUG_ONLY("Gpu start is 0, CPU ptr : {}", (UINT64)DestDescriptor.ptr);

                break;
            }
        }
    }

    // Try to capture swapchain buffer RTV's once (cause issues otherwise)
    if (!fgCaptureRTVs || frameCounter > fgTargetFrame)
    {
        if (fgCaptureRTVs)
        {
            LOG_DEBUG_ONLY("    FG Stop capturing RTV buffers");
            fgCaptureRTVs = false;
        }

        return;
    }

    // Match swapchain buffers & cpu handles
    ID3D12Resource* buffer;
    DXGI_SWAP_CHAIN_DESC desc{};
    for (auto const& [key, val] : fgSwapChains)
    {
        if (val->GetDesc(&desc) == S_OK)
        {
            LOG_DEBUG_ONLY("SwapChain buffer count: {0}", desc.BufferCount);

            for (size_t i = 0; i < desc.BufferCount; i++)
            {
                if (val->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
                {
                    LOG_DEBUG_ONLY("Check buffer: {0}", i);

                    if (buffer == pResource)
                    {
                        LOG_DEBUG_ONLY("    FG SwapChain buffer RTV found for buffer: {0} ({1:X})", i, (UINT64)pResource);

                        if (!fgBuffers.contains(pResource))
                        {
                            BackbufferInfo newInfo{};
                            newInfo.cpuHandle = DestDescriptor.ptr;
                            fgBuffers.insert_or_assign(pResource, newInfo);

                            fgCapturedBufferCount++;
                            if (fgCapturedBufferCount == desc.BufferCount)
                            {
                                LOG_DEBUG_ONLY("    FG all SC buffers captured, stop capturing RTV buffers");
                                fgCaptureRTVs = false;
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

// Capture if render target matches, wait for DrawIndexed
static void hkOMSetRenderTargets(ID3D12GraphicsCommandList* This, UINT NumRenderTargetDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
                                 BOOL RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    o_OMSetRenderTargets(This, NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    LOG_DEBUG_ONLY("");

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || fgSCBufferCaptured || Config::Instance()->CurrentFeature == nullptr ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() || !ImGuiOverlayDx::upscaleRan || ImGuiOverlayDx::fgContext == nullptr)
    {
        fgHUDlessCaptureCounter = 0;
        return;
    }

    // Previous resource buffer
    if (Config::Instance()->FGHUDPrevious.value_or(false))
    {
        for (size_t i = 0; i < NumRenderTargetDescriptors; i++)
        {
            for (auto& [key, val] : fgBuffers)
            {
                if (pRenderTargetDescriptors[i].ptr == val.prevCpuHandle)
                {
                    fgHUDlessCaptureCounter++;
                    if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
                    {
                        ID3D12Resource* hudless = nullptr;
                        LOG_DEBUG_ONLY("  FG Pre SwapChain buffer found for: {0:X}", (UINT64)key);

                        auto bufferInfo = &fgBuffers[key];

                        if (val.prev == key)
                        {
                            if (CreateBufferResource(g_pd3dDeviceParam, key, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                            {
                                hudless = bufferInfo->hudless;

                                std::shared_lock<std::shared_mutex> lock2(resourceMutex);

                                if (bufferInfo->prevCpuHandle == NULL && fgHandleAddressResources.contains(bufferInfo->prev))
                                    bufferInfo->prevCpuHandle = fgHandleAddressResources[bufferInfo->prev].cpuStart;
                            }
                        }
                        else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, key, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
                        {
                            bufferInfo->hudless = hudless;

                            std::shared_lock<std::shared_mutex> lock2(resourceMutex);

                            if (fgHandleAddressResources.contains(bufferInfo->prev))
                                bufferInfo->prevCpuHandle = fgHandleAddressResources[bufferInfo->prev].cpuStart;
                        }

                        if (hudless != nullptr)
                        {
                            fgCurrentSCBuffer = key;
                            fgSCBufferCaptured = true;
                            LOG_DEBUG_ONLY("  FG Pre Before SwapChain buffer found 1");
                        }
                    }

                    // if buffer matches exit
                    return;
                }
            }
        }
    }

    // check for swapchain buffers
    for (size_t i = 0; i < NumRenderTargetDescriptors; i++)
    {
        for (auto& [key, val] : fgBuffers)
        {
            if (pRenderTargetDescriptors[i].ptr == val.cpuHandle)
            {
                fgHUDlessCaptureCounter++;
                if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
                {
                    ID3D12Resource* hudless = nullptr;
                    LOG_DEBUG_ONLY("  FG SwapChain buffer found for: {0:X}", (UINT64)key);
                    auto bufferInfo = &fgBuffers[key];

                    if (bufferInfo->hudless != nullptr)
                    {
                        if (CreateBufferResource(g_pd3dDeviceParam, key, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                            hudless = bufferInfo->hudless;
                    }
                    else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, key, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
                    {
                        bufferInfo->hudless = hudless;
                    }

                    if (hudless != nullptr)
                    {
                        fgCurrentSCBuffer = key;
                        fgSCBufferCaptured = true;
                        LOG_DEBUG_ONLY("  FG SwapChain buffer found 1");
                        return;
                    }
                }
            }
        }
    }
}

static void hkCopyResource(ID3D12GraphicsCommandList* This, ID3D12Resource* Dest, ID3D12Resource* Source)
{
    o_CopyResource(This, Dest, Source);

    //LOG_DEBUG_ONLY("");

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || fgSCBufferCaptured || Config::Instance()->CurrentFeature == nullptr ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() || !ImGuiOverlayDx::upscaleRan || ImGuiOverlayDx::fgContext == nullptr)
    {
        fgHUDlessCaptureCounter = 0;
        return;
    }

    // If copy to previous resource
    if (Config::Instance()->FGHUDPrevious.value_or(false) && fgBuffers.size() > 0)
    {
        for (auto& [key, val] : fgBuffers)
        {
            if (val.prev == Dest)
            {
                fgHUDlessCaptureCounter++;
                if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
                {
                    ID3D12Resource* hudless = nullptr;
                    auto bufferInfo = &fgBuffers[key];

                    // capture hudless
                    if (bufferInfo->hudless != nullptr)
                    {
                        if (CreateBufferResource(g_pd3dDeviceParam, Source, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                            hudless = bufferInfo->hudless;
                    }
                    else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, Source, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
                    {
                        bufferInfo->hudless = hudless;
                    }

                    if (hudless != nullptr)
                    {
                        LOG_DEBUG_ONLY("  FG Resource before swapChain buffer found for: {0:X}", (UINT64)Dest);
                        fgCurrentSCBuffer = key;
                        fgSCBufferCaptured = true;
                        GetHUDless(This, D3D12_RESOURCE_STATE_COPY_DEST);
                    }
                }

                // on case of error too
                return;
            }
        }
    }

    // If this is a swapchain buffer
    if (fgBuffers.size() != 0 && fgBuffers.contains(Dest))
    {
        fgHUDlessCaptureCounter++;
        if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
        {
            ID3D12Resource* hudless = nullptr;
            LOG_DEBUG_ONLY("  FG SwapChain buffer found for: {0:X}", (UINT64)Dest);

            auto bufferInfo = &fgBuffers[Dest];
            bufferInfo->prev = Source;

            // capture hudless
            if (bufferInfo->hudless != nullptr)
            {
                if (CreateBufferResource(g_pd3dDeviceParam, Dest, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                    hudless = bufferInfo->hudless;
            }
            else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, Dest, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
            {
                bufferInfo->hudless = hudless;
            }

            // call ffx with hudless
            if (hudless != nullptr)
            {
                fgCurrentSCBuffer = Dest;
                fgSCBufferCaptured = true;
                GetHUDless(This, D3D12_RESOURCE_STATE_COPY_DEST);
            }
        }
    }
}

static void hkCopyTextureRegion(ID3D12GraphicsCommandList* This, D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, D3D12_TEXTURE_COPY_LOCATION* pSrc, D3D12_BOX* pSrcBox)
{
    o_CopyTextureRegion(This, pDst, DstX, DstY, DstZ, pSrc, pSrcBox);

    LOG_DEBUG_ONLY("");

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || fgSCBufferCaptured || Config::Instance()->CurrentFeature == nullptr ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() || !ImGuiOverlayDx::upscaleRan || ImGuiOverlayDx::fgContext == nullptr)
    {
        fgHUDlessCaptureCounter = 0;
        return;
    }

    // Previous resource
    if (Config::Instance()->FGHUDPrevious.value_or(false) && fgBuffers.size() > 0)
    {
        for (auto& [key, val] : fgBuffers)
        {
            if (val.prev == pDst->pResource)
            {
                fgHUDlessCaptureCounter++;
                if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
                {
                    ID3D12Resource* hudless = nullptr;
                    auto bufferInfo = &fgBuffers[key];

                    // capture hudless
                    if (bufferInfo->hudless != nullptr)
                    {
                        if (CreateBufferResource(g_pd3dDeviceParam, pSrc->pResource, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                            hudless = bufferInfo->hudless;
                    }
                    else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, pSrc->pResource, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
                    {
                        bufferInfo->hudless = hudless;
                    }

                    if (hudless != nullptr)
                    {
                        LOG_DEBUG_ONLY("  FG Resource before swapChain buffer found for: {0:X}", (UINT64)pDst->pResource);
                        fgCurrentSCBuffer = key;
                        fgSCBufferCaptured = true;
                        GetHUDless(This, D3D12_RESOURCE_STATE_COPY_DEST);
                    }
                }

                // on case of error too
                return;
            }
        }
    }

    // If this is a swapchain buffer
    if (fgBuffers.size() != 0 && fgBuffers.contains(pDst->pResource))
    {
        fgHUDlessCaptureCounter++;
        if (fgHUDlessCaptureCounter == Config::Instance()->FGHUDLimit.value_or(1))
        {
            ID3D12Resource* hudless = nullptr;
            LOG_DEBUG_ONLY("  FG SwapChain buffer found for: {0:X}", (UINT64)pDst->pResource);

            auto bufferInfo = &fgBuffers[pDst->pResource];
            bufferInfo->prev = pSrc->pResource;

            {
                std::shared_lock<std::shared_mutex> lock2(resourceMutex);

                if (fgHandleAddressResources.contains(bufferInfo->prev))
                    bufferInfo->prevCpuHandle = fgHandleAddressResources[bufferInfo->prev].cpuStart;
            }

            // capture hudless
            if (bufferInfo->hudless != nullptr)
            {
                if (CreateBufferResource(g_pd3dDeviceParam, pDst->pResource, D3D12_RESOURCE_STATE_COPY_DEST, &bufferInfo->hudless))
                    hudless = bufferInfo->hudless;
            }
            else if (g_pd3dDeviceParam != nullptr && CreateBufferResource(g_pd3dDeviceParam, pDst->pResource, D3D12_RESOURCE_STATE_COPY_DEST, &hudless))
            {
                bufferInfo->hudless = hudless;
            }

            // call ffx with hudless
            if (hudless != nullptr)
            {
                fgCurrentSCBuffer = pDst->pResource;
                fgSCBufferCaptured = true;
                GetHUDless(This, D3D12_RESOURCE_STATE_COPY_DEST);
            }
        }
    }
}

static void hkDrawInstanced(ID3D12GraphicsCommandList* This, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
    o_DrawInstanced(This, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

    if (!fgSCBufferCaptured || fgCurrentSCBuffer == nullptr || fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount())
    {
        fgHUDlessCaptureCounter = 0;
        fgPreviousBuffer = nullptr;
        fgCurrentSCBuffer = nullptr;
        return;
    }

    if (!Config::Instance()->FGEnabled.value_or(false) || Config::Instance()->FGSkipIndexedInstanced.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) ||
        Config::Instance()->CurrentFeature == nullptr || _dispatch == nullptr || ImGuiOverlayDx::fgContext == nullptr || !ImGuiOverlayDx::upscaleRan)
    {
        fgSCBufferCaptured = false;
        fgHUDlessCaptureCounter = 0;
        fgCurrentSCBuffer = nullptr;
        fgPreviousBuffer = nullptr;
        return;
    }

    // Prev buffer is captured save the info
    if (Config::Instance()->FGHUDPrevious.value_or(false) && fgPreviousBuffer != nullptr)
    {
        auto bufferInfo = &fgBuffers[fgCurrentSCBuffer];
        bufferInfo->prev = fgPreviousBuffer;

        std::shared_lock<std::shared_mutex> lock2(resourceMutex);
        bufferInfo->prevCpuHandle = fgHandleAddressResources[fgPreviousBuffer].cpuStart;
    }

    fgPreviousBuffer = nullptr;

    GetHUDless(This, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

static void hkDrawIndexedInstanced(ID3D12GraphicsCommandList* This, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
    o_DrawIndexedInstanced(This, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

    //LOG_DEBUG_ONLY("");

    if (!fgSCBufferCaptured || fgCurrentSCBuffer == nullptr || fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount())
    {
        fgHUDlessCaptureCounter = 0;
        fgPreviousBuffer = nullptr;
        fgCurrentSCBuffer = nullptr;
        return;
    }

    if (!Config::Instance()->FGEnabled.value_or(false) || Config::Instance()->FGSkipIndexedInstanced.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) ||
        Config::Instance()->CurrentFeature == nullptr || _dispatch == nullptr || ImGuiOverlayDx::fgContext == nullptr || !ImGuiOverlayDx::upscaleRan)
    {
        fgSCBufferCaptured = false;
        fgHUDlessCaptureCounter = 0;
        fgCurrentSCBuffer = nullptr;
        return;
    }

    // Prev buffer is captured save the info
    if (fgPreviousBuffer != nullptr)
    {
        auto bufferInfo = &fgBuffers[fgCurrentSCBuffer];
        bufferInfo->prev = fgPreviousBuffer;

        std::shared_lock<std::shared_mutex> lock2(resourceMutex);
        bufferInfo->prevCpuHandle = fgHandleAddressResources[fgPreviousBuffer].cpuStart;
    }

    fgPreviousBuffer = nullptr;

    GetHUDless(This, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

static void hkSetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    o_SetGraphicsRootDescriptorTable(This, RootParameterIndex, BaseDescriptor);

    if (!Config::Instance()->FGEnabled.value_or(false) || !Config::Instance()->FGHUDFix.value_or(false) || fgSCBufferCaptured || Config::Instance()->CurrentFeature == nullptr ||
        fgHudlessFrame == Config::Instance()->CurrentFeature->FrameCount() || !ImGuiOverlayDx::upscaleRan || ImGuiOverlayDx::fgContext == nullptr)
    {
        return;
    }

    if (!Config::Instance()->FGHUDPrevious.value_or(false))
        return;

    {
        std::shared_lock<std::shared_mutex> lock(resourceMutex);

        // assume first one is source
        if (RootParameterIndex == 0 && fgHandleAddressResourcesByGpuHandle.contains(BaseDescriptor.ptr))
        {
            fgPreviousBuffer = fgHandleAddressResourcesByGpuHandle[BaseDescriptor.ptr];
        }
        else
        {
            LOG_DEBUG_ONLY("Miss: {0:X}", (UINT64)BaseDescriptor.ptr);
        }
    }
}

static HRESULT hkCreateDescriptorHeap(ID3D12Device* This, D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap)
{
    auto result = o_CreateDescriptorHeap(This, pDescriptorHeapDesc, riid, ppvHeap);

    // try to calculate handle ranges for heap
    if (result == S_OK && !Config::Instance()->SkipHeapCapture && (pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || pDescriptorHeapDesc->Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV))  //&& pDescriptorHeapDesc->Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        HeapInfo info{};
        auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
        auto increment = This->GetDescriptorHandleIncrementSize(pDescriptorHeapDesc->Type);
        info.numDescriptors = pDescriptorHeapDesc->NumDescriptors;
        info.gpuStart = (SIZE_T)(heap->GetGPUDescriptorHandleForHeapStart().ptr);
        info.cpuStart = (SIZE_T)(heap->GetCPUDescriptorHandleForHeapStart().ptr);
        info.cpuEnd = info.cpuStart + (increment * info.numDescriptors);
        info.gpuEnd = info.gpuStart + (increment * info.numDescriptors);

        //if (info.gpuStart != 0)
        //{
        LOG_DEBUG_ONLY("Found heap, type: {}, cpu: {}-{}, gpu: {}-{}, desc count: {}", (UINT)pDescriptorHeapDesc->Type, info.cpuStart, info.cpuEnd, info.gpuStart, info.gpuEnd, info.numDescriptors);

        std::unique_lock<std::shared_mutex> lock(heapMutex);
        fgHeaps.push_back(info);
        //}
        //else
        //{
        //    auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
        //    LOG_DEBUG_ONLY("Skipping, no GPU handle, type: {}, cpu: {}-{}", (UINT)pDescriptorHeapDesc->Type, info.cpuStart, info.cpuEnd);
        //}
    }
    else
    {
        auto heap = (ID3D12DescriptorHeap*)(*ppvHeap);
        LOG_DEBUG_ONLY("Skipping, type: {}, cpuStart: {}, gpu start: {}", (UINT)pDescriptorHeapDesc->Type, (SIZE_T)(heap->GetCPUDescriptorHandleForHeapStart().ptr), (SIZE_T)(heap->GetGPUDescriptorHandleForHeapStart().ptr));
    }

    return result;
}

static void hkCreateShaderResourceView(ID3D12Device* This, ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    o_CreateShaderResourceView(This, pResource, pDesc, DestDescriptor);

    if (pResource == nullptr || pDesc == nullptr || pDesc->ViewDimension != D3D12_SRV_DIMENSION_TEXTURE2D) // || pDesc->Shader4ComponentMapping != D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING)
        return;

    {
        std::shared_lock<std::shared_mutex> lock(heapMutex);

        // try to find heap of resource
        for (auto& val : fgHeaps)
        {
            if (val.cpuStart <= DestDescriptor.ptr && val.cpuEnd >= DestDescriptor.ptr)
            {
                auto incSize = This->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                auto addr = DestDescriptor.ptr - val.cpuStart;
                auto index = addr / incSize;
                auto gpuAddr = val.gpuStart + (index * incSize);

                HeapInfo info{};
                info.cpuStart = DestDescriptor.ptr;
                info.gpuStart = gpuAddr;

                std::unique_lock<std::shared_mutex> lock(resourceMutex);
                fgHandleAddressResources.insert_or_assign(pResource, info);

                if (val.gpuStart != 0)
                    fgHandleAddressResourcesByGpuHandle.insert_or_assign(gpuAddr, pResource);
                else
                    LOG_DEBUG_ONLY("Gpu start is 0, CPU ptr : {}", (UINT64)DestDescriptor.ptr);

                return;
            }
        }
    }

    LOG_DEBUG_ONLY("Not found, CPU ptr: {}", (UINT64)DestDescriptor.ptr);
}

static void __stdcall hkCopyDescriptors(UINT NumDestDescriptorRanges, D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts, UINT* pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges,
                              D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts, UINT* pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);

    if (DescriptorHeapsType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || DescriptorHeapsType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    {
        LOG_DEBUG_ONLY("Found");
    }
}

static void __stdcall hkCopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    o_CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);

    if (DescriptorHeapsType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || DescriptorHeapsType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    {
        LOG_DEBUG_ONLY("Found");
    }
}


static int GetCorrectDXGIFormat(int eCurrentFormat)
{
    switch (eCurrentFormat)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

static void CreateRenderTargetDx12(ID3D12Device* device, IDXGISwapChain* pSwapChain)
{
    LOG_FUNC();

    DXGI_SWAP_CHAIN_DESC desc;
    HRESULT hr = pSwapChain->GetDesc(&desc);

    if (hr != S_OK)
    {
        LOG_ERROR("pSwapChain->GetDesc: {0:X}", (unsigned long)hr);
        return;
    }

    for (UINT i = 0; i < desc.BufferCount; ++i)
    {
        ID3D12Resource* pBackBuffer = NULL;

        auto result = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));

        if (result != S_OK)
        {
            LOG_ERROR("pSwapChain->GetBuffer: {0:X}", (unsigned long)result);
            return;
        }

        if (pBackBuffer)
        {
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);

            D3D12_RENDER_TARGET_VIEW_DESC desc = { };
            desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            device->CreateRenderTargetView(pBackBuffer, &desc, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }

    LOG_INFO("done!");
}

static void CleanupRenderTargetDx12(bool clearQueue)
{
    if (!_isInited || !_dx12Device)
        return;

    LOG_FUNC();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
    {
        if (g_mainRenderTargetResource[i])
        {
            g_mainRenderTargetResource[i]->Release();
            g_mainRenderTargetResource[i] = NULL;
        }
    }

    if (clearQueue)
    {
        if (ImGuiOverlayBase::IsInited() && g_pd3dDeviceParam != nullptr && g_pd3dSrvDescHeap != nullptr && ImGui::GetIO().BackendRendererUserData)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            ImGui_ImplDX12_Shutdown();
        }

        if (g_pd3dRtvDescHeap != nullptr)
        {
            g_pd3dRtvDescHeap->Release();
            g_pd3dRtvDescHeap = nullptr;

        }

        if (g_pd3dSrvDescHeap != nullptr)
        {
            g_pd3dSrvDescHeap->Release();
            g_pd3dSrvDescHeap = nullptr;

        }

        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            if (g_commandAllocators[i] != nullptr)
            {
                g_commandAllocators[i]->Release();
                g_commandAllocators[i] = nullptr;
            }
        }

        if (g_pd3dCommandList != nullptr)
        {
            g_pd3dCommandList->Release();
            g_pd3dCommandList = nullptr;
        }

        if (g_pd3dCommandQueue != nullptr)
        {
            g_pd3dCommandQueue->Release();
            g_pd3dCommandQueue = nullptr;
        }

        if (g_pd3dDeviceParam != nullptr)
        {
            g_pd3dDeviceParam->Release();
            g_pd3dDeviceParam = nullptr;
        }

        _dx12Device = false;
        _isInited = false;
    }
}

static void CreateRenderTargetDx11(IDXGISwapChain* pSwapChain)
{
    ID3D11Texture2D* pBackBuffer = NULL;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    if (pBackBuffer)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);

        D3D11_RENDER_TARGET_VIEW_DESC desc = { };
        desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &desc, &g_pd3dRenderTarget);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTargetDx11(bool shutDown)
{
    if (!_isInited || !_dx11Device)
        return;

    if (!shutDown)
        LOG_FUNC();

    if (g_pd3dRenderTarget != nullptr)
    {
        g_pd3dRenderTarget->Release();
        g_pd3dRenderTarget = nullptr;
    }

    if (g_pd3dDevice != nullptr)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }

    _dx11Device = false;
    _isInited = false;
}

#pragma region Callbacks for wrapped swapchain

static void CleanupRenderTarget(bool clearQueue, HWND hWnd)
{
    LOG_FUNC();

    if (clearQueue)
        currentSCCommandQueue = nullptr;

    if (_dx11Device)
        CleanupRenderTargetDx11(false);
    else
        CleanupRenderTargetDx12(clearQueue);

    // Releasing RTSS D3D11on12 device
    if (clearQueue && d3d11on12Device != nullptr && GetModuleHandle(L"RTSSHooks64.dll") != nullptr)
    {
        LOG_DEBUG("Releasing D3d11on12 device");
        d3d11on12Device = nullptr;
    }
}

static HRESULT Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters, IUnknown* pDevice, HWND hWnd)
{
    LOG_FUNC();

    HRESULT presentResult;

    if (hWnd != Util::GetProcessWindow())
    {
        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        ImGuiOverlayDx::currentSwapchain = nullptr;
        ImGuiOverlayDx::swapchainFormat = DXGI_FORMAT_UNKNOWN;

        LOG_FUNC_RESULT(presentResult);
        return presentResult;
    }

    if (fgSwapChains.contains(hWnd))
    {
        ImGuiOverlayDx::currentSwapchain = fgSwapChains[hWnd];
        if (ImGuiOverlayDx::swapchainFormat == DXGI_FORMAT_UNKNOWN)
        {
            DXGI_SWAP_CHAIN_DESC desc;
            if (pSwapChain->GetDesc(&desc) == S_OK)
                ImGuiOverlayDx::swapchainFormat = desc.BufferDesc.Format;
        }
    }

    if (Config::Instance()->CurrentFeature != nullptr)
        LOG_DEBUG_ONLY("FrameCount: {}, fgHudlessFrame: {}", Config::Instance()->CurrentFeature->FrameCount(), fgHudlessFrame);

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

        currentSCCommandQueue = pDevice;
        ImGuiOverlayDx::GameCommandQueue = (ID3D12CommandQueue*)pDevice;

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

        if (pPresentParameters == nullptr)
            presentResult = pSwapChain->Present(SyncInterval, Flags);
        else
            presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

        LOG_FUNC_RESULT(presentResult);
        return presentResult;
    }

    // Upscaler GPU time computation
    if (ImGuiOverlayDx::dx12UpscaleTrig && ImGuiOverlayDx::readbackBuffer != nullptr && ImGuiOverlayDx::queryHeap != nullptr && cq != nullptr)
    {
        if (ImGuiOverlayBase::IsInited() && ImGuiOverlayBase::IsVisible())
        {
            UINT64* timestampData;
            ImGuiOverlayDx::readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&timestampData));

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
            ImGuiOverlayDx::readbackBuffer->Unmap(0, nullptr);
        }

        ImGuiOverlayDx::dx12UpscaleTrig = false;
    }
    else if (ImGuiOverlayDx::dx11UpscaleTrig[ImGuiOverlayDx::currentFrameIndex] && device != nullptr && ImGuiOverlayDx::disjointQueries[0] != nullptr &&
             ImGuiOverlayDx::startQueries[0] != nullptr && ImGuiOverlayDx::endQueries[0] != nullptr)
    {
        if (g_pd3dDeviceContext == nullptr)
            device->GetImmediateContext(&g_pd3dDeviceContext);

        if (ImGuiOverlayBase::IsInited() && ImGuiOverlayBase::IsVisible())
        {
            // Retrieve the results from the previous frame
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
            if (g_pd3dDeviceContext->GetData(ImGuiOverlayDx::disjointQueries[ImGuiOverlayDx::previousFrameIndex], &disjointData, sizeof(disjointData), 0) == S_OK)
            {
                if (!disjointData.Disjoint && disjointData.Frequency > 0)
                {
                    UINT64 startTime = 0, endTime = 0;
                    if (g_pd3dDeviceContext->GetData(ImGuiOverlayDx::startQueries[ImGuiOverlayDx::previousFrameIndex], &startTime, sizeof(UINT64), 0) == S_OK &&
                        g_pd3dDeviceContext->GetData(ImGuiOverlayDx::endQueries[ImGuiOverlayDx::previousFrameIndex], &endTime, sizeof(UINT64), 0) == S_OK)
                    {
                        double elapsedTimeMs = (endTime - startTime) / static_cast<double>(disjointData.Frequency) * 1000.0;
                        Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
                        Config::Instance()->upscaleTimes.pop_front();
                    }
                }
            }
        }


        ImGuiOverlayDx::dx11UpscaleTrig[ImGuiOverlayDx::currentFrameIndex] = false;
        ImGuiOverlayDx::currentFrameIndex = (ImGuiOverlayDx::currentFrameIndex + 1) % ImGuiOverlayDx::QUERY_BUFFER_COUNT;
    }

    // Process window handle changed, update base
    if (ImGuiOverlayBase::Handle() != hWnd)
    {
        LOG_DEBUG("Handle changed");

        if (ImGuiOverlayBase::IsInited())
            ImGuiOverlayBase::Shutdown();

        ImGuiOverlayBase::Init(hWnd);

        _isInited = false;
    }

    // Init
    if (!_isInited)
    {
        if (_dx11Device)
        {
            CleanupRenderTargetDx11(false);

            g_pd3dDevice = device;
            g_pd3dDevice->AddRef();

            CreateRenderTargetDx11(pSwapChain);
            ImGuiOverlayBase::Dx11Ready();
            _isInited = true;
        }
        else if (_dx12Device && (g_pd3dDeviceParam != nullptr || device12 != nullptr))
        {
            if (g_pd3dDeviceParam != nullptr && device12 == nullptr)
                device12 = g_pd3dDeviceParam;

            CleanupRenderTargetDx12(true);

            g_pd3dCommandQueue = cq;
            g_pd3dDeviceParam = device12;

            g_pd3dCommandQueue->AddRef();
            g_pd3dDeviceParam->AddRef();

            ImGuiOverlayBase::Dx12Ready();
            _isInited = true;
        }
    }

    // dx11 multi thread safety
    ID3D11Multithread* dx11MultiThread = nullptr;
    ID3D11DeviceContext* dx11Context = nullptr;
    bool mtState = false;

    if (_dx11Device)
    {
        ID3D11Device* dx11Device = g_pd3dDevice;
        if (dx11Device == nullptr)
            dx11Device = d3d11Device;

        if (dx11Device == nullptr)
            dx11Device = d3d11on12Device;

        if (dx11Device != nullptr)
        {
            dx11Device->GetImmediateContext(&dx11Context);

            if (dx11Context != nullptr && dx11Context->QueryInterface(IID_PPV_ARGS(&dx11MultiThread)) == S_OK && dx11MultiThread != nullptr)
            {
                mtState = dx11MultiThread->GetMultithreadProtected();
                dx11MultiThread->SetMultithreadProtected(TRUE);
                dx11MultiThread->Enter();
            }
        }
    }

    // Render menu
    if (_dx11Device)
        RenderImGui_DX11(pSwapChain);
    else if (_dx12Device)
        RenderImGui_DX12(pSwapChain);

    // swapchain present
    if (pPresentParameters == nullptr)
        presentResult = pSwapChain->Present(SyncInterval, Flags);
    else
        presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

    // dx11 multi thread safety
    if (_dx11Device && dx11MultiThread != nullptr)
    {
        dx11MultiThread->Leave();
        dx11MultiThread->SetMultithreadProtected(mtState);

        dx11MultiThread->Release();
        dx11Context->Release();
    }

    // release used objects
    if (cq != nullptr)
        cq->Release();

    if (device != nullptr)
        device->Release();

    if (device12 != nullptr)
        device12->Release();

    frameCounter++;

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

        if(pDesc != nullptr)
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
        //return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    ID3D12CommandQueue* cq = nullptr;
    if (!fgSkipSCWrapping && _createContext != nullptr && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        cq->Release();

        ffxCreateContextDescFrameGenerationSwapChainNewDX12 createSwapChainDesc{};
        createSwapChainDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_NEW_DX12;

        createSwapChainDesc.dxgiFactory = pFactory;
        createSwapChainDesc.gameQueue = (ID3D12CommandQueue*)pDevice;
        createSwapChainDesc.desc = pDesc;
        createSwapChainDesc.swapchain = (IDXGISwapChain4**)ppSwapChain;

        Config::Instance()->dxgiSkipSpoofing = true;
        fgSkipSCWrapping = true;
        Config::Instance()->SkipHeapCapture = true;
        auto result = _createContext(&ImGuiOverlayDx::fgSwapChainContext, &createSwapChainDesc.header, nullptr);
        Config::Instance()->SkipHeapCapture = false;
        fgSkipSCWrapping = false;
        Config::Instance()->dxgiSkipSpoofing = false;

        if (result == FFX_API_RETURN_OK)
        {
            fgSwapChains.insert_or_assign(pDesc->OutputWindow, *ppSwapChain);
            fgCaptureRTVs = true;
            fgTargetFrame = frameCounter + 25;
            return S_OK;
        }

        LOG_ERROR("_createContext error: {}", result);

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
        *ppSwapChain = new WrappedIDXGISwapChain4(real == nullptr ? *ppSwapChain : real, pDevice, pDesc->OutputWindow, Present, CleanupRenderTarget);
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
        //return oCreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    ID3D12CommandQueue* cq = nullptr;
    if (!fgSkipSCWrapping && _createContext != nullptr && pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        cq->Release();

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
        auto result = _createContext(&ImGuiOverlayDx::fgSwapChainContext, &createSwapChainDesc.header, nullptr);
        Config::Instance()->SkipHeapCapture = false;
        fgSkipSCWrapping = false;
        Config::Instance()->dxgiSkipSpoofing = false;

        if (result == FFX_API_RETURN_OK)
        {
            fgSwapChains.insert_or_assign(hWnd, *ppSwapChain);
            fgCaptureRTVs = true;
            fgTargetFrame = frameCounter + 25;
            return S_OK;
        }

        LOG_ERROR("_createContext error: {}", result);

        return E_INVALIDARG;
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
        *ppSwapChain = new WrappedIDXGISwapChain4(real == nullptr ? *ppSwapChain : real, pDevice, hWnd, Present, CleanupRenderTarget);
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

            o_OMSetRenderTargets = (PFN_OMSetRenderTargets)pVTable[46];
            o_DrawInstanced = (PFN_DrawInstanced)pVTable[12];
            o_DrawIndexedInstanced = (PFN_DrawIndexedInstanced)pVTable[13];
            o_CopyTextureRegion = (PFN_CopyTextureRegion)pVTable[16];
            o_CopyResource = (PFN_CopyResource)pVTable[17];
            o_SetGraphicsRootDescriptorTable = (PFN_SetGraphicsRootDescriptorTable)pVTable[31];

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

                if (o_CopyResource != nullptr)
                    DetourAttach(&(PVOID&)o_CopyResource, hkCopyResource);

                //if (o_SetGraphicsRootDescriptorTable != nullptr)
                //    DetourAttach(&(PVOID&)o_SetGraphicsRootDescriptorTable, hkSetGraphicsRootDescriptorTable);

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

    o_CreateDescriptorHeap = (PFN_CreateDescriptorHeap)pVTable[14];
    o_CreateSampler = (PFN_CreateSampler)pVTable[22];
    o_CopyDescriptors = (PFN_CopyDescriptors)pVTable[23];
    o_CopyDescriptorsSimple = (PFN_CopyDescriptorsSimple)pVTable[24];
    o_CreateShaderResourceView = (PFN_CreateShaderResourceView)pVTable[18];
    o_CreateRenderTargetView = (PFN_CreateRenderTargetView)pVTable[20];

    // Apply the detour
    if (o_CreateSampler != nullptr || o_CreateRenderTargetView != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        //if (o_CreateDescriptorHeap != nullptr)
        //    DetourAttach(&(PVOID&)o_CreateDescriptorHeap, hkCreateDescriptorHeap);

        if (o_CreateSampler != nullptr)
            DetourAttach(&(PVOID&)o_CreateSampler, hkCreateSampler);

        if (o_CreateRenderTargetView != nullptr)
            DetourAttach(&(PVOID&)o_CreateRenderTargetView, hkCreateRenderTargetView);

        //if (o_CreateShaderResourceView != nullptr)
        //    DetourAttach(&(PVOID&)o_CreateShaderResourceView, hkCreateShaderResourceView);

        //if (o_CopyDescriptors != nullptr)
        //    DetourAttach(&(PVOID&)o_CopyDescriptors, hkCopyDescriptors);

        //if (o_CopyDescriptorsSimple != nullptr)
        //    DetourAttach(&(PVOID&)o_CopyDescriptorsSimple, hkCopyDescriptorsSimple);

        DetourTransactionCommit();
    }

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

    // Assuming RTSS is creating a D3D11on12 device, not sure why but sometimes RTSS tries to create 
    // it's D3D11on12 device with old CommandQueue which results crash
    // I am changing it's CommandQueue with current swapchain's command queue
    if (currentSCCommandQueue != nullptr && *ppCommandQueues != currentSCCommandQueue && GetModuleHandle(L"RTSSHooks64.dll") != nullptr && pDevice == g_pd3dDeviceParam)
    {
        LOG_INFO("Replaced RTSS CommandQueue with correct one {0:X} -> {1:X}", (UINT64)*ppCommandQueues, (UINT64)currentSCCommandQueue);
        *ppCommandQueues = currentSCCommandQueue;
    }

    auto result = o_D3D11On12CreateDevice(pDevice, Flags, pFeatureLevels, FeatureLevels, ppCommandQueues, NumQueues, NodeMask, ppDevice, ppImmediateContext, pChosenFeatureLevel);

    if (result == S_OK && *ppDevice != nullptr)
    {
        LOG_INFO("Device captured, CommandQueue: {0:X}", (UINT64)*ppCommandQueues);
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

    if (result == S_OK && *ppDevice != nullptr)
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

    IDXGISwapChain* buffer = nullptr;
    auto result = o_D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, &buffer, ppDevice, pFeatureLevel, ppImmediateContext);

    if (result == S_OK && *ppDevice != nullptr)
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

static HRESULT hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    LOG_FUNC();

    auto result = o_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (result == S_OK)
    {
        LOG_INFO("Device captured");
        g_pd3dDeviceParam = (ID3D12Device*)*ppDevice;
        HookToDevice(g_pd3dDeviceParam);
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
        (pDesc->Filter == D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT ||
        pDesc->Filter == D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT ||
        pDesc->Filter == D3D12_FILTER_MIN_MAG_MIP_LINEAR ||
        pDesc->Filter == D3D12_FILTER_ANISOTROPIC))
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

static void RenderImGui_DX11(IDXGISwapChain* pSwapChain)
{
    bool drawMenu = false;

    do
    {
        if (!ImGuiOverlayBase::IsInited())
            break;

        // Draw only when menu activated
        if (!ImGuiOverlayBase::IsVisible())
            break;

        if (!_dx11Device || g_pd3dDevice == nullptr)
            break;

        drawMenu = true;

    } while (false);

    if (!drawMenu)
    {
        ImGuiOverlayBase::HideMenu();
        return;
    }

    LOG_FUNC();

    if (ImGui::GetIO().BackendRendererUserData == nullptr)
    {
        if (pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice)) == S_OK)
        {
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
        }
    }

    if (_isInited)
    {
        if (!g_pd3dRenderTarget)
            CreateRenderTargetDx11(pSwapChain);

        if (ImGui::GetCurrentContext() && g_pd3dRenderTarget)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGuiOverlayBase::RenderMenu();

            ImGui::Render();

            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pd3dRenderTarget, NULL);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
    }
}

static void RenderImGui_DX12(IDXGISwapChain* pSwapChainPlain)
{
    bool drawMenu = false;
    IDXGISwapChain3* pSwapChain = nullptr;

    do
    {
        if (pSwapChainPlain->QueryInterface(IID_PPV_ARGS(&pSwapChain)) != S_OK || pSwapChain == nullptr)
            return;

        if (!ImGuiOverlayBase::IsInited())
            break;

        // Draw only when menu activated
        if (!ImGuiOverlayBase::IsVisible())
            break;

        if (!_dx12Device || g_pd3dCommandQueue == nullptr || g_pd3dDeviceParam == nullptr)
            break;

        drawMenu = true;

    } while (false);

    if (!drawMenu)
    {
        ImGuiOverlayBase::HideMenu();
        auto releaseResult = pSwapChain->Release();
        return;
    }

    LOG_FUNC();

    // Get device from swapchain
    ID3D12Device* device = g_pd3dDeviceParam;

    // Generate ImGui resources
    if (!ImGui::GetIO().BackendRendererUserData && g_pd3dCommandQueue != nullptr)
    {
        LOG_DEBUG("ImGui::GetIO().BackendRendererUserData == nullptr");

        HRESULT result;

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = { };
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;

            Config::Instance()->SkipHeapCapture = true;

            result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
            if (result != S_OK)
            {
                LOG_ERROR("CreateDescriptorHeap(g_pd3dRtvDescHeap): {0:X}", (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTargetDx12(true);
                pSwapChain->Release();
                return;
            }

            SIZE_T rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

            for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
            {
                g_mainRenderTargetDescriptor[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = { };
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));

            Config::Instance()->SkipHeapCapture = false;
            if (result != S_OK)
            {
                LOG_ERROR("CreateDescriptorHeap(g_pd3dSrvDescHeap): {0:X}", (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTargetDx12(true);
                pSwapChain->Release();
                return;
            }
        }

        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i]));

            if (result != S_OK)
            {
                LOG_ERROR("CreateCommandAllocator[{0}]: {1:X}", i, (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTargetDx12(true);
                pSwapChain->Release();
                return;
            }
        }

        result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList));
        if (result != S_OK)
        {
            LOG_ERROR("CreateCommandList: {0:X}", (unsigned long)result);
            ImGuiOverlayBase::HideMenu();
            CleanupRenderTargetDx12(true);
            pSwapChain->Release();
            return;
        }

        result = g_pd3dCommandList->Close();
        if (result != S_OK)
        {
            LOG_ERROR("g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
            ImGuiOverlayBase::HideMenu();
            CleanupRenderTargetDx12(false);
            pSwapChain->Release();
            return;
        }

        ImGui_ImplDX12_Init(device, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

        pSwapChain->Release();
        return;
    }

    if (_isInited)
    {
        // Generate render targets
        if (!g_mainRenderTargetResource[0])
        {
            CreateRenderTargetDx12(device, pSwapChain);
            pSwapChain->Release();
            return;
        }

        // If everything is ready render the frame
        if (ImGui::GetCurrentContext() && g_mainRenderTargetResource[0])
        {
            _showRenderImGuiDebugOnce = true;

            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGuiOverlayBase::RenderMenu();

            ImGui::Render();


            UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
            ID3D12CommandAllocator* commandAllocator = g_commandAllocators[backBufferIdx];

            auto result = commandAllocator->Reset();
            if (result != S_OK)
            {
                LOG_ERROR("commandAllocator->Reset: {0:X}", (unsigned long)result);
                CleanupRenderTargetDx12(false);
                pSwapChain->Release();
                return;
            }

            D3D12_RESOURCE_BARRIER barrier = { };
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

            result = g_pd3dCommandList->Reset(commandAllocator, nullptr);
            if (result != S_OK)
            {
                LOG_ERROR("g_pd3dCommandList->Reset: {0:X}", (unsigned long)result);
                pSwapChain->Release();
                return;
            }

            g_pd3dCommandList->ResourceBarrier(1, &barrier);
            g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
            g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
            g_pd3dCommandList->ResourceBarrier(1, &barrier);

            result = g_pd3dCommandList->Close();
            if (result != S_OK)
            {
                LOG_ERROR("g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
                CleanupRenderTargetDx12(true);
                pSwapChain->Release();
                return;
            }

            ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };
            g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        }
        else
        {
            if (_showRenderImGuiDebugOnce)
                LOG_INFO("!(ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])");

            ImGuiOverlayBase::HideMenu();
            _showRenderImGuiDebugOnce = false;
        }
    }

    pSwapChain->Release();
}

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

void ImGuiOverlayDx::HookDx()
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

    LoadFSR31Funcs();
}

void ImGuiOverlayDx::UnHookDx()
{
    if (!Config::Instance()->IsRunningOnDXVK)
    {
        if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
        {
            if (_dx11Device)
                ImGui_ImplDX11_Shutdown();
            else
                ImGui_ImplDX12_Shutdown();
        }

        ImGuiOverlayBase::Shutdown();

        if (_isInited)
        {
            if (_dx11Device)
                CleanupRenderTargetDx11(true);
            else
                CleanupRenderTargetDx12(true);
        }
    }

    DeatachAllHooks();

    _isInited = false;
}
