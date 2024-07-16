#include "imgui_overlay_base.h"
#include "imgui_overlay_dx12.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include "ffx_api.h"
#include "ffx_framegeneration.h"
#include "dx12/ffx_api_dx12.h"


#include <d3d12.h>
#include <dxgi1_6.h>

#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"

#include "wrapped_swapchain.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

// MipMap hooks
typedef void(__fastcall* PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static inline PFN_CreateSampler orgCreateSampler = nullptr;

typedef struct FfxFrameGenerationConfig
{
    IDXGISwapChain4* swapChain;
    void* presentCallback;
    void* frameGenerationCallback;
    bool  frameGenerationEnabled;
    bool  allowAsyncWorkloads;
    void* HUDLessColor;
    uint32_t flags;
    bool onlyPresentInterpolated;
} FfxFrameGenerationConfig;

static ID3D12Device* g_pd3dDeviceParam = nullptr;

static int const NUM_BACK_BUFFERS = 4;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };

typedef void(WINAPI* PFN_ExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);
typedef ULONG(WINAPI* PFN_Release)(IUnknown*);

typedef void* (WINAPI* PFN_ffxGetDX12SwapchainPtr)(void* ffxSwapChain);
typedef int32_t(WINAPI* PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12)(HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* desc1,
                                                                             const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc, ID3D12CommandQueue* queue, IDXGIFactory* dxgiFactory, IDXGISwapChain4** outGameSwapChain);

typedef int32_t(WINAPI* PFN_ffxFsr3ConfigureFrameGeneration)(void* context, const FfxFrameGenerationConfig* config);

typedef HRESULT(WINAPI* PFN_CreateCommandQueue)(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

// Dx12 early binding
static PFN_D3D12_CREATE_DEVICE oD3D12CreateDevice = nullptr;
static PFN_CreateCommandQueue oCreateCommandQueue = nullptr;
static PFN_CreateDXGIFactory oCreateDXGIFactory = nullptr;
static PFN_CreateDXGIFactory1 oCreateDXGIFactory1 = nullptr;
static PFN_CreateDXGIFactory2 oCreateDXGIFactory2 = nullptr;
static PFN_Release oCommandQueueRelease = nullptr;

// Dx12
static PFN_Present oPresent_Dx12 = nullptr;
static PFN_Present1 oPresent1_Dx12 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12 = nullptr;

// Streamline
static PFN_Present oPresent_SL = nullptr;
static PFN_Present1 oPresent1_SL = nullptr;
static PFN_ResizeBuffers oResizeBuffers_SL = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_SL = nullptr;

// Native FSR3
static PFN_Present oPresent_FSR3 = nullptr;
static PFN_Present1 oPresent1_FSR3 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_FSR3 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_FSR3 = nullptr;

// Mod FSR3
static PFN_Present oPresent_Mod = nullptr;
static PFN_Present1 oPresent1_Mod = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Mod = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Mod = nullptr;

// Dx12
static PFN_ExecuteCommandLists oExecuteCommandLists_Dx12 = nullptr;

// Streamline 
static PFN_ExecuteCommandLists oExecuteCommandLists_SL = nullptr;

// Dx12 Late Binding
static PFN_CreateSwapChain oCreateSwapChain_Dx12 = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_Dx12 = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_Dx12 = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_Dx12 = nullptr;

// Dx12 early binding
static PFN_CreateSwapChain oCreateSwapChain_EB = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_EB = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_EB = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_EB = nullptr;

// Streamline
static PFN_CreateSwapChain oCreateSwapChain_SL = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_SL = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_SL = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_SL = nullptr;

// Mod FSR3
static PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12 offxCreateFrameinterpolationSwapchainForHwndDX12_Mod = nullptr;
static PFN_ffxFsr3ConfigureFrameGeneration offxFsr3ConfigureFrameGeneration_Mod = nullptr;

// Native FSR3
static PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12 offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 = nullptr;
static PFN_ffxFsr3ConfigureFrameGeneration offxFsr3ConfigureFrameGeneration_FSR3 = nullptr;
// FSR3.1
static PfnFfxCreateContext oFfxCreateContext_FSR3 = nullptr;
static PfnFfxConfigure oFfxConfigure_FSR3 = nullptr;

static bool _isInited = false;
static bool _reInit = false;
static bool _dx12BindingActive = false; // To prevent wrapping of swapchains created for hook by mod
static bool _skipCleanup = false;

// capture right command queue
static ID3D12CommandList* _dlssCommandList = nullptr;

static bool _isEarlyBind = false;	// if dx12 calls loaded early (non native nvngx mode)

static bool _usingNative = false;	// Native Dx12 present received a call
static bool _usingDLSSG = false;	// Streamline presenct received a call
static bool _usingFSR3_Native = false;	// FSR3 native present received a call
static bool _usingFSR3_Mod = false;	// FSR3 mod received present call

/*
* From what I've observed priotry order is
* 1. FSR3 Native
* 2. FSR3 Mod (only if Uniscaler is active)
* 3. Streamline (For Nukem & native SL)
* 4. Native (For non-fg situations)
*/

static bool _bindedNative = false;	// if native dx12 calls detoured
static bool _bindedDLSSG = false;	// if streamline calls detoured
static bool _bindedFSR3_Native = false;		// if fsr3 native calls detoured
static bool _bindedFSR3_Uniscaler = false;	// if Uniscaler calls detoured
static bool _bindedFSR3_Nukem = false;	// if Nukem's calls detoured
static bool _bindedFSR3_Mod = false;	// if fsr3 mod calls detoured (_bindedFSR3_Uniscaler || _bindedFSR3_Nukem)

// Optiscaler will try to capture right commandqueue for each possible swapchain
static ID3D12CommandQueue* _cqFsr3Mod = nullptr;	// Not used
static ID3D12CommandQueue* _cqFsr3Native = nullptr;	// Not used
static ID3D12CommandQueue* _cqSL = nullptr;	// Not used
static ID3D12CommandQueue* _cqDx12 = nullptr;

// for showing 
static bool _showRenderImGuiDebugOnce = true;

// FFXModHookCounter for disabling to prevent 
static long _changeFFXModHookCounter_Mod = 0;
static long _changeFFXModHookCounter_Native = 0;

// present counters
static long _nativeCounter;
static long _nativeCounterEB;
static long _slCounter;
static long _fsr3ModCounter;
static long _fsr3NativeCounter;

inline static std::mutex _dx12CleanMutex;

enum SwapchainSource
{
    Unknown,
    Dx12,
    SL,
    FSR3_Mod,
    FSR3_Native
};

const std::string swapchainSourceNames[] = { "Unknown", "Dx12", "SL", "FSR3 Mod", "FSR3 Native" };

// Last swapchain source
static SwapchainSource _lastActiveSource = Unknown;

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain);
static bool BindAll(HWND InHWnd, ID3D12Device* InDevice);
static void DeatachAllHooks();
static void CleanupRenderTarget(bool clearQueue);

static void ClearActivePathInfo()
{
    spdlog::debug("ImGuiOverlayDx12::ClearActivePathInfo!");

    _lastActiveSource = Unknown;
    _nativeCounter = 0;
    _nativeCounterEB = 0;
    _slCounter = 0;
    _fsr3ModCounter = 0;
    _fsr3NativeCounter = 0;

    _changeFFXModHookCounter_Mod = 0;
    _changeFFXModHookCounter_Native = 0;

    _usingNative = false;
    _usingDLSSG = false;
    _usingFSR3_Native = false;
    _usingFSR3_Mod = false;

}

static bool IsActivePath(SwapchainSource source, bool justQuery = false)
{
    bool result = false;

    switch (source)
    {
        case Dx12:
            // native need 2x more frames to be sure because of fg
            result = (_nativeCounter > 170 || _nativeCounterEB > 170) && !IsActivePath(SL, true) && !IsActivePath(FSR3_Mod, true) && !IsActivePath(FSR3_Native, true) && _usingNative && _cqDx12 != nullptr;

            if (result)
                g_pd3dCommandQueue = _cqDx12;

            break;

        case SL:
            result = _slCounter > 75 && !IsActivePath(FSR3_Native, true) && _usingDLSSG && _cqDx12 != nullptr;

            if (result)
                g_pd3dCommandQueue = _cqDx12;

            break;

        case FSR3_Mod:
            result = _fsr3ModCounter > 65 && !IsActivePath(FSR3_Native, true) && _usingFSR3_Mod && _cqDx12 != nullptr;

            if (result)
                g_pd3dCommandQueue = _cqDx12;

            break;

        case FSR3_Native:
            result = _fsr3NativeCounter > 30 && _usingFSR3_Native && _cqDx12 != nullptr;

            if (result)
                g_pd3dCommandQueue = _cqDx12;

            break;

    }

    if (!justQuery && result && _lastActiveSource != source)
    {
        spdlog::info("ImGuiOverlayDx12::IsActivePath changed from {0} to {1}!", swapchainSourceNames[(int)_lastActiveSource], swapchainSourceNames[(int)source]);
        ClearActivePathInfo();
        _lastActiveSource = source;
    }

    return result;
}

static bool IsBeingUsed(IDXGISwapChain1* InSwapchain)
{
    HWND hwnd = nullptr;
    InSwapchain->GetHwnd(&hwnd);
    return hwnd == ImGuiOverlayBase::Handle();
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

static void CleanupRenderTarget(bool clearQueue)
{
    if (!_isInited || _skipCleanup)
        return;

    spdlog::debug("ImGuiOverlayDx12::CleanupRenderTarget({0})!", clearQueue);

    _dx12CleanMutex.lock();

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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
            ImGui_ImplDX12_Shutdown();

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
    }

    ClearActivePathInfo();

    _dx12CleanMutex.unlock();
}

#pragma region Mipmap Hook

static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    if (pDesc == nullptr || device == nullptr)
        return;

    if (pDesc->MipLODBias < 0.0f && Config::Instance()->MipmapBiasOverride.has_value())
    {
        spdlog::info("ImGuiOverlayDx12::hkCreateSampler Overriding mipmap bias {0} -> {1}", pDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());

        D3D12_SAMPLER_DESC newDesc{};

        newDesc.AddressU = pDesc->AddressU;
        newDesc.AddressV = pDesc->AddressV;
        newDesc.AddressW = pDesc->AddressW;
        newDesc.BorderColor[0] = pDesc->BorderColor[0];
        newDesc.BorderColor[1] = pDesc->BorderColor[1];
        newDesc.BorderColor[2] = pDesc->BorderColor[2];
        newDesc.BorderColor[3] = pDesc->BorderColor[3];
        newDesc.ComparisonFunc = pDesc->ComparisonFunc;
        newDesc.Filter = pDesc->Filter;
        newDesc.MaxAnisotropy = pDesc->MaxAnisotropy;
        newDesc.MaxLOD = pDesc->MaxLOD;
        newDesc.MinLOD = pDesc->MinLOD;
        newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();

        Config::Instance()->lastMipBias = newDesc.MipLODBias;

        return orgCreateSampler(device, &newDesc, DestDescriptor);
    }
    else if (pDesc->MipLODBias < 0.0f)
    {
        Config::Instance()->lastMipBias = pDesc->MipLODBias;
    }

    return orgCreateSampler(device, pDesc, DestDescriptor);
}


static void HookToDevice(ID3D12Device* InDevice)
{
    if (!ImGuiOverlayDx12::IsEarlyBind() && orgCreateSampler != nullptr || InDevice == nullptr)
        return;

    // Get the vtable pointer
    PVOID* pVTable = *(PVOID**)InDevice;

    // Get the address of the SetComputeRootSignature function from the vtable
    orgCreateSampler = (PFN_CreateSampler)pVTable[22];

    // Apply the detour
    if (orgCreateSampler != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)orgCreateSampler, hkCreateSampler);

        DetourTransactionCommit();
    }
}

#pragma endregion

#pragma region Hooks for native EB Prensent Methods

static HRESULT WINAPI hkPresent_EB(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
        return S_OK;

    if (IsActivePath(Dx12))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _nativeCounterEB++;
        _usingNative = true;
    }
    else
    {
        _nativeCounterEB = 0;
        _usingNative = false;
    }

    return S_OK;
}

static HRESULT WINAPI hkPresent1_EB(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
        return S_OK;

    if (IsActivePath(Dx12))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _nativeCounterEB++;
        _usingNative = true;
    }
    else
    {
        _nativeCounterEB = 0;
        _usingNative = false;
    }


    return S_OK;
}

#pragma endregion

#pragma region Hooks for native Dx12 Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
        return oPresent_Dx12(pSwapChain, SyncInterval, Flags);

    if ((!_isEarlyBind || _nativeCounterEB == 0) && IsActivePath(Dx12))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _nativeCounter++;
        _usingNative = true;
    }
    else
    {
        _nativeCounter = 0;
        _usingNative = false;
    }

    return oPresent_Dx12(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
        return oPresent1_Dx12(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

    if ((!_isEarlyBind || _nativeCounterEB == 0) && IsActivePath(Dx12))
        RenderImGui_DX12(pSwapChain);

    _usingNative = IsBeingUsed(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _nativeCounter++;
        _usingNative = true;
    }
    else
    {
        _nativeCounter = 0;
        _usingNative = false;
    }

    return oPresent1_Dx12(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (!_isEarlyBind && IsActivePath(Dx12))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    return result;
}

static HRESULT WINAPI hkResizeBuffers1_Dx12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                                            UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    if (!_isEarlyBind && IsActivePath(Dx12))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers1_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

    return result;
}

#pragma endregion

#pragma region Hooks for native StreamLine Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
        return oPresent_SL(pSwapChain, SyncInterval, Flags);

    if (IsActivePath(SL))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _slCounter++;
        _usingDLSSG = true;
    }
    else
    {
        _slCounter = 0;
        _usingDLSSG = false;
    }

    return oPresent_SL(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
        return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

    if (IsActivePath(SL))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _slCounter++;
        _usingDLSSG = true;
    }
    else
    {
        _slCounter = 0;
        _usingDLSSG = false;
    }

    return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_SL(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (!_isEarlyBind && IsActivePath(SL))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    return result;
}

static HRESULT WINAPI hkResizeBuffers1_SL(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                                          UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    if (!_isEarlyBind && IsActivePath(SL))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers1_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

    return result;
}

#pragma endregion

#pragma region Hooks for Native FSR3 Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
        return oPresent_FSR3(pSwapChain, SyncInterval, Flags);

    if (IsActivePath(FSR3_Native))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _fsr3NativeCounter++;
        _usingFSR3_Native = true;
    }
    else
    {
        _fsr3NativeCounter = 0;
        _usingFSR3_Native = false;
    }

    return oPresent_FSR3(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
        return oPresent1_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

    if (IsActivePath(FSR3_Native))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _fsr3NativeCounter++;
        _usingFSR3_Native = true;
    }
    else
    {
        _fsr3NativeCounter = 0;
        _usingFSR3_Native = false;
    }

    return oPresent1_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_FSR3(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (!_isEarlyBind && IsActivePath(FSR3_Native))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    return result;
}

static HRESULT WINAPI hkResizeBuffers1_FSR3(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                                            UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    if (!_isEarlyBind && IsActivePath(FSR3_Native))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers1_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

    return result;
}

#pragma endregion

#pragma region Hooks for FSR3 Mods Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
        return oPresent_Mod(pSwapChain, SyncInterval, Flags);

    if (IsActivePath(FSR3_Mod))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _fsr3ModCounter++;
        _usingFSR3_Mod = true;
    }
    else
    {
        _fsr3ModCounter = 0;
        _usingFSR3_Mod = false;
    }

    return oPresent_Mod(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
        return oPresent1_Mod(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

    if (IsActivePath(FSR3_Mod))
        RenderImGui_DX12(pSwapChain);

    if (IsBeingUsed(pSwapChain))
    {
        _fsr3ModCounter++;
        _usingFSR3_Mod = true;
    }
    else
    {
        _fsr3ModCounter = 0;
        _usingFSR3_Mod = false;
    }


    return oPresent1_Mod(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Mod(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (!_isEarlyBind && IsActivePath(FSR3_Mod))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    return result;
}

static HRESULT WINAPI hkResizeBuffers1_Mod(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                                           UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    if (!_isEarlyBind && IsActivePath(FSR3_Mod))
        CleanupRenderTarget(false);

    auto result = oResizeBuffers1_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

    return result;
}

#pragma endregion

#pragma region Hook for ffxCreateFrameinterpolationSwapchainForHwndDX12

static int32_t WINAPI hkffxCreateFrameinterpolationSwapchainForHwndDX12_Mod(HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* desc1,
                                                                            const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc, ID3D12CommandQueue* queue, IDXGIFactory* dxgiFactory, IDXGISwapChain4** outGameSwapChain)
{
    CleanupRenderTarget(true);

    auto result = offxCreateFrameinterpolationSwapchainForHwndDX12_Mod(hWnd, desc1, fullscreenDesc, queue, dxgiFactory, outGameSwapChain);

    if (oPresent_Mod == nullptr && *outGameSwapChain != nullptr)
    {
        void** pVTable = *reinterpret_cast<void***>(*outGameSwapChain);

        oPresent_Mod = (PFN_Present)pVTable[8];
        oPresent1_Mod = (PFN_Present1)pVTable[22];
        oResizeBuffers_Mod = (PFN_ResizeBuffers)pVTable[13];
        oResizeBuffers1_Mod = (PFN_ResizeBuffers1)pVTable[39];

        // Apply the detour
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (oPresent_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent_Mod, hkPresent_Mod);

        if (oPresent1_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent1_Mod, hkPresent1_Mod);

        if (oResizeBuffers_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers_Mod, hkResizeBuffers_Mod);

        if (oResizeBuffers1_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers1_Mod, hkResizeBuffers1_Mod);

        DetourTransactionCommit();

        spdlog::info("ImGuiOverlayDx12::hkffxCreateFrameinterpolationSwapchainForHwndDX12_Mod added swapchain hooks!");
    }

    return result;
}

static int32_t WINAPI hkffxFsr3ConfigureFrameGeneration_Mod(void* context, FfxFrameGenerationConfig* config)
{
    auto result = offxFsr3ConfigureFrameGeneration_Mod(context, config);

    if (oPresent_Mod == nullptr && config->swapChain != nullptr)
    {
        void** pVTable = *reinterpret_cast<void***>(config->swapChain);

        oPresent_Mod = (PFN_Present)pVTable[8];
        oPresent1_Mod = (PFN_Present1)pVTable[22];
        oResizeBuffers_Mod = (PFN_ResizeBuffers)pVTable[13];
        oResizeBuffers1_Mod = (PFN_ResizeBuffers1)pVTable[39];

        // Apply the detour
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (oPresent_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent_Mod, hkPresent_Mod);

        if (oPresent1_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent1_Mod, hkPresent1_Mod);

        if (oResizeBuffers_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers_Mod, hkResizeBuffers_Mod);

        if (oResizeBuffers1_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers1_Mod, hkResizeBuffers1_Mod);

        DetourTransactionCommit();

        spdlog::info("ImGuiOverlayDx12::hkffxFsr3ConfigureFrameGeneration_Mod added swapchain hooks!");
    }

    return result;
}

static int32_t WINAPI hkffxCreateFrameinterpolationSwapchainForHwndDX12_FSR3(HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* desc1,
                                                                             const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc, ID3D12CommandQueue* queue, IDXGIFactory* dxgiFactory, IDXGISwapChain4** outGameSwapChain)
{
    CleanupRenderTarget(true);

    auto result = offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3(hWnd, desc1, fullscreenDesc, queue, dxgiFactory, outGameSwapChain);

    if (oPresent_FSR3 == nullptr && *outGameSwapChain != nullptr)
    {
        void** pVTable = *reinterpret_cast<void***>(*outGameSwapChain);

        oPresent_FSR3 = (PFN_Present)pVTable[8];
        oPresent1_FSR3 = (PFN_Present1)pVTable[22];
        oResizeBuffers_FSR3 = (PFN_ResizeBuffers)pVTable[13];
        oResizeBuffers1_FSR3 = (PFN_ResizeBuffers1)pVTable[39];

        // Apply the detour
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (oPresent_FSR3 != nullptr)
            DetourAttach(&(PVOID&)oPresent_FSR3, hkPresent_FSR3);

        if (oPresent1_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent1_FSR3, hkPresent1_FSR3);

        if (oResizeBuffers_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers_FSR3, hkResizeBuffers_FSR3);

        if (oResizeBuffers1_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers1_FSR3, hkResizeBuffers1_FSR3);

        DetourTransactionCommit();

        spdlog::info("ImGuiOverlayDx12::hkffxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 added swapchain hooks!");
    }

    return result;
}

static ffxReturnCode_t hkFfxCreateContext_FSR3(ffxContext* context, ffxCreateContextDescHeader* desc, const ffxAllocationCallbacks* memCb)
{
    auto result = oFfxCreateContext_FSR3(context, desc, memCb);

    if (oPresent_FSR3 == nullptr)
    {
        auto header = (ffxCreateContextDescHeader*)desc;

        while (header != nullptr)
        {
            if (header->type == FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_WRAP_DX12 ||
                header->type == FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_NEW_DX12 ||
                header->type == FFX_API_CREATE_CONTEXT_DESC_TYPE_FRAMEGENERATIONSWAPCHAIN_FOR_HWND_DX12)
            {
                auto cscHeader = (ffxCreateContextDescFrameGenerationSwapChainWrapDX12*)header;

                if (*cscHeader->swapchain != nullptr)
                {
                    void** pVTable = *reinterpret_cast<void***>(*cscHeader->swapchain);

                    oPresent_FSR3 = (PFN_Present)pVTable[8];
                    oPresent1_FSR3 = (PFN_Present1)pVTable[22];
                    oResizeBuffers_FSR3 = (PFN_ResizeBuffers)pVTable[13];
                    oResizeBuffers1_FSR3 = (PFN_ResizeBuffers1)pVTable[39];

                    // Apply the detour
                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    if (oPresent_FSR3 != nullptr)
                        DetourAttach(&(PVOID&)oPresent_FSR3, hkPresent_FSR3);

                    if (oPresent1_Mod != nullptr)
                        DetourAttach(&(PVOID&)oPresent1_FSR3, hkPresent1_FSR3);

                    if (oResizeBuffers_Mod != nullptr)
                        DetourAttach(&(PVOID&)oResizeBuffers_FSR3, hkResizeBuffers_FSR3);

                    if (oResizeBuffers1_Mod != nullptr)
                        DetourAttach(&(PVOID&)oResizeBuffers1_FSR3, hkResizeBuffers1_FSR3);

                    DetourTransactionCommit();

                    spdlog::info("ImGuiOverlayDx12::hkFfxCreateContext_FSR3 added swapchain hooks!");

                    break;
                }
            }

            header = (ffxCreateContextDescHeader*)header->pNext;
        }
    }

    return result;
}

static int32_t WINAPI hkffxFsr3ConfigureFrameGeneration_FSR3(void* context, FfxFrameGenerationConfig* config)
{
    auto result = offxFsr3ConfigureFrameGeneration_FSR3(context, config);

    if (oPresent_FSR3 == nullptr && config->swapChain != nullptr)
    {
        void** pVTable = *reinterpret_cast<void***>(config->swapChain);

        oPresent_FSR3 = (PFN_Present)pVTable[8];
        oPresent1_FSR3 = (PFN_Present1)pVTable[22];
        oResizeBuffers_FSR3 = (PFN_ResizeBuffers)pVTable[13];
        oResizeBuffers1_FSR3 = (PFN_ResizeBuffers1)pVTable[39];

        // Apply the detour
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (oPresent_FSR3 != nullptr)
            DetourAttach(&(PVOID&)oPresent_FSR3, hkPresent_FSR3);

        if (oPresent1_Mod != nullptr)
            DetourAttach(&(PVOID&)oPresent1_FSR3, hkPresent1_FSR3);

        if (oResizeBuffers_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers_FSR3, hkResizeBuffers_FSR3);

        if (oResizeBuffers1_Mod != nullptr)
            DetourAttach(&(PVOID&)oResizeBuffers1_FSR3, hkResizeBuffers1_FSR3);

        DetourTransactionCommit();

        spdlog::info("ImGuiOverlayDx12::hkffxFsr3ConfigureFrameGeneration_FSR3 added swapchain hooks!");
    }

    return result;
}

static ffxReturnCode_t hkFfxConfigure_FSR3(ffxContext* context, const ffxConfigureDescHeader* desc)
{
    auto result = oFfxConfigure_FSR3(context, desc);

    if (oPresent_FSR3 == nullptr)
    {
        auto header = (ffxConfigureDescHeader*)desc;

        while (header != nullptr)
        {
            if (header->type == FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION)
            {
                auto cfgHeader = (ffxConfigureDescFrameGeneration*)header;

                if (cfgHeader->swapChain != nullptr)
                {
                    void** pVTable = *reinterpret_cast<void***>(cfgHeader->swapChain);

                    oPresent_FSR3 = (PFN_Present)pVTable[8];
                    oPresent1_FSR3 = (PFN_Present1)pVTable[22];
                    oResizeBuffers_FSR3 = (PFN_ResizeBuffers)pVTable[13];
                    oResizeBuffers1_FSR3 = (PFN_ResizeBuffers1)pVTable[39];

                    // Apply the detour
                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    if (oPresent_FSR3 != nullptr)
                        DetourAttach(&(PVOID&)oPresent_FSR3, hkPresent_FSR3);

                    if (oPresent1_Mod != nullptr)
                        DetourAttach(&(PVOID&)oPresent1_FSR3, hkPresent1_FSR3);

                    if (oResizeBuffers_Mod != nullptr)
                        DetourAttach(&(PVOID&)oResizeBuffers_FSR3, hkResizeBuffers_FSR3);

                    if (oResizeBuffers1_Mod != nullptr)
                        DetourAttach(&(PVOID&)oResizeBuffers1_FSR3, hkResizeBuffers1_FSR3);

                    DetourTransactionCommit();

                    spdlog::info("ImGuiOverlayDx12::hkFfxConfigure_FSR3 added swapchain hooks!");

                    break;
                }
            }

            header = (ffxCreateContextDescHeader*)header->pNext;
        }
    }

    return result;
}


#pragma endregion

#pragma region Hooks for ExecuteCommandLists 

static void WINAPI hkExecuteCommandLists_Dx12(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (_cqDx12 == nullptr && _dlssCommandList != nullptr)
    {
        // trying to find command list for dlss, 
        // it should be native dx12 cq
        for (size_t i = 0; i < NumCommandLists; i++)
        {
            if (ppCommandLists[i] == _dlssCommandList)
            {
                spdlog::info("ImGuiOverlayDx12::hkExecuteCommandLists_Dx12 new _cqDx12: {0:X}", (ULONG64)pCommandQueue);
                _cqDx12 = pCommandQueue;
                break;
            }
        }
    }

    return oExecuteCommandLists_Dx12(pCommandQueue, NumCommandLists, ppCommandLists);
}

// Not used anymore
static void WINAPI hkExecuteCommandLists_SL(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (_cqSL == nullptr)
    {
        auto desc = pCommandQueue->GetDesc();

        if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            spdlog::info("ImGuiOverlayDx12::hkExecuteCommandLists_SL new _cqSL: {0:X}", (ULONG64)pCommandQueue);
            _cqSL = pCommandQueue;
        }
    }

    return oExecuteCommandLists_SL(pCommandQueue, NumCommandLists, ppCommandLists);
}

#pragma endregion

#pragma region Hook for DXGIFactory for native DX12

static HRESULT WINAPI hkCreateSwapChain_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChain_Dx12(pFactory, pDevice, pDesc, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                    const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForHwnd_Dx12(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                          IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForCoreWindow_Dx12(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                           IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForComposition_Dx12(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
    return result;
}

#pragma endregion

#pragma region Hook for DXGIFactory for Early bindind

static HRESULT WINAPI hkCreateSwapChain_EB(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    CleanupRenderTarget(true);

    auto result = oCreateSwapChain_EB(pFactory, pDevice, pDesc, ppSwapChain);

    if (result == S_OK && !_dx12BindingActive)
    {
        spdlog::debug("ImGuiOverlayDx12::hkCreateSwapChain_EB created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_EB(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                  const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);

    auto result = oCreateSwapChainForHwnd_EB(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK && !_dx12BindingActive)
    {
        spdlog::debug("ImGuiOverlayDx12::hkCreateSwapChainForHwnd_EB created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_EB(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                        IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);

    auto result = oCreateSwapChainForCoreWindow_EB(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK && !_dx12BindingActive)
    {
        spdlog::debug("ImGuiOverlayDx12::hkCreateSwapChainForCoreWindow_EB created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_EB(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                         IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);

    auto result = oCreateSwapChainForComposition_EB(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK && !_dx12BindingActive)
    {
        spdlog::debug("ImGuiOverlayDx12::oCreateSwapChainForComposition_EB created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
    }

    return result;
}

#pragma endregion

#pragma region Hook for DXGIFactory for native StreamLine

static HRESULT WINAPI hkCreateSwapChain_SL(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChain_SL(pFactory, pDevice, pDesc, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_SL(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                  const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForHwnd_SL(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_SL(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                        IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForCoreWindow_SL(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
    return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_SL(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                         IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    CleanupRenderTarget(true);
    auto result = oCreateSwapChainForComposition_SL(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);;
    return result;
}

#pragma endregion

#pragma region Hooks for early binding

static ULONG WINAPI hkCommandQueueRelease(IUnknown* This)
{
    auto result = oCommandQueueRelease(This);

    if (result == 0)
    {
        if (This == _cqDx12)
            _cqDx12 = nullptr;
        else if (This == _cqFsr3Mod)
            _cqFsr3Mod = nullptr;
        else if (This == _cqFsr3Native)
            _cqFsr3Native = nullptr;
        else if (This == _cqSL)
            _cqSL = nullptr;
    }

    return result;
}

static HRESULT WINAPI hkCreateCommandQueue(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue)
{
    auto result = oCreateCommandQueue(This, pDesc, riid, ppCommandQueue);

    if (result == S_OK && oExecuteCommandLists_Dx12 == nullptr && pDesc->Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        auto cq = (ID3D12CommandQueue*)*ppCommandQueue;

        void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

        oCommandQueueRelease = (PFN_Release)pCommandQueueVTable[2];
        oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

        if (oExecuteCommandLists_Dx12 != nullptr)
        {
            spdlog::info("ImGuiOverlayDx12::hkCreateCommandQueue hooking CommandQueue");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);
            DetourAttach(&(PVOID&)oCommandQueueRelease, hkCommandQueueRelease);

            DetourTransactionCommit();
        }

        cq = nullptr;
    }

    return result;
}

static HRESULT WINAPI hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    auto result = oD3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (result == S_OK && oCreateCommandQueue == nullptr)
    {
        auto device = (ID3D12Device*)*ppDevice;

        HookToDevice(device);

        void** pCommandQueueVTable = *reinterpret_cast<void***>(device);

        oCreateCommandQueue = (PFN_CreateCommandQueue)pCommandQueueVTable[8];

        if (oCreateCommandQueue != nullptr)
        {
            spdlog::info("ImGuiOverlayDx12::hkD3D12CreateDevice hooking D3D12Device");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)oCreateCommandQueue, hkCreateCommandQueue);

            DetourTransactionCommit();
        }
    }

    return result;
}


static HRESULT WINAPI hkCreateDXGIFactory(REFIID riid, void** ppFactory)
{
    auto result = oCreateDXGIFactory1(riid, ppFactory);

    if (result == S_OK && oCreateSwapChain_EB == nullptr)
    {
        auto factory = (IDXGIFactory*)*ppFactory;


        void** pFactoryVTable = *reinterpret_cast<void***>(factory);

        oCreateSwapChain_EB = (PFN_CreateSwapChain)pFactoryVTable[10];

        if (oCreateSwapChain_EB != nullptr)
        {
            spdlog::info("ImGuiOverlayDx12::hkCreateDXGIFactory Hooking native DXGIFactory");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

            DetourTransactionCommit();
        }
    }

    return result;
}

static HRESULT WINAPI hkCreateDXGIFactory1(REFIID riid, void** ppFactory)
{
    auto result = oCreateDXGIFactory1(riid, ppFactory);

    if (result == S_OK && oCreateSwapChainForHwnd_EB == nullptr)
    {
        auto factory = (IDXGIFactory*)*ppFactory;
        IDXGIFactory4* factory4 = nullptr;

        if (factory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory4);

            bool skip = false;

            if (oCreateSwapChain_EB == nullptr)
                oCreateSwapChain_EB = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            oCreateSwapChainForHwnd_EB = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
            oCreateSwapChainForCoreWindow_EB = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
            oCreateSwapChainForComposition_EB = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

            if (oCreateSwapChainForHwnd_EB != nullptr)
            {
                spdlog::info("ImGuiOverlayDx12::hkCreateDXGIFactory1 Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);
                DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);
                DetourAttach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);

                DetourTransactionCommit();
            }

            factory4->Release();
            factory4 = nullptr;
        }
    }

    return result;
}

static HRESULT WINAPI hkCreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    auto result = oCreateDXGIFactory2(Flags, riid, ppFactory);

    if (result == S_OK && oCreateSwapChainForHwnd_EB == nullptr)
    {
        auto factory = (IDXGIFactory*)*ppFactory;
        IDXGIFactory4* factory4 = nullptr;

        if (factory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory4);

            bool skip = false;

            if (oCreateSwapChain_EB == nullptr)
                oCreateSwapChain_EB = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            oCreateSwapChainForHwnd_EB = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
            oCreateSwapChainForCoreWindow_EB = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
            oCreateSwapChainForComposition_EB = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

            if (oCreateSwapChainForHwnd_EB != nullptr)
            {
                spdlog::info("ImGuiOverlayDx12::hkCreateDXGIFactory1 Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);
                DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);
                DetourAttach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);

                DetourTransactionCommit();
            }

            factory4->Release();
            factory4 = nullptr;
        }
    }

    return result;
}

#pragma endregion

#pragma region Bindings

static bool CheckDx12(ID3D12Device* InDevice)
{
    HRESULT result;

    // Native DX12 CommandQueue Bind
    if (oExecuteCommandLists_Dx12 == nullptr)
    {
        ID3D12Device* device = InDevice;

        if (device == nullptr)
        {
            // Create device
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

            result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::CheckDx12 D3D12CreateDevice: {0:X}", (unsigned long)result);
                return false;
            }
        }

        // Create queue
        ID3D12CommandQueue* cq = nullptr;
        D3D12_COMMAND_QUEUE_DESC desc = { };
        result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
        if (result != S_OK)
        {
            spdlog::error("ImGuiOverlayDx12::CheckDx12 CreateCommandQueue: {0:X}", (unsigned long)result);
            return false;
        }

        if (cq != nullptr)
        {
            void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

            oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

            if (oExecuteCommandLists_Dx12 != nullptr)
            {
                spdlog::info("ImGuiOverlayDx12::CheckDx12 hooking CommandQueue");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

                DetourTransactionCommit();
            }

            cq->Release();
            cq = nullptr;
        }

        if (device != InDevice && device != nullptr)
        {
            device->Release();
            device = nullptr;
        }
    }

    return true;
}

static bool CheckFSR3()
{
    // Check for FSR3-FG & Mods
    if (Config::Instance()->HookFSR3Proxy.value_or(true))
    {
        if (offxCreateFrameinterpolationSwapchainForHwndDX12_Mod == nullptr)
        {
            // Check for Uniscaler
            offxCreateFrameinterpolationSwapchainForHwndDX12_Mod = (PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12)DetourFindFunction("Uniscaler.asi", "ffxCreateFrameinterpolationSwapchainForHwndDX12");
            offxFsr3ConfigureFrameGeneration_Mod = (PFN_ffxFsr3ConfigureFrameGeneration)DetourFindFunction("Uniscaler.asi", "ffxFsr3ConfigureFrameGeneration");

            if (offxCreateFrameinterpolationSwapchainForHwndDX12_Mod != nullptr)
            {
                spdlog::info("ImGuiOverlayDx12::CheckMods Uniscaler's ffxCreateFrameinterpolationSwapchainForHwndDX12 found");
                _bindedFSR3_Uniscaler = true;
                _bindedFSR3_Mod = true;
            }
            else
            {
                // Check for Nukem's
                offxCreateFrameinterpolationSwapchainForHwndDX12_Mod = (PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12)DetourFindFunction("dlssg_to_fsr3_amd_is_better.dll", "ffxCreateFrameinterpolationSwapchainForHwndDX12");
                offxFsr3ConfigureFrameGeneration_Mod = (PFN_ffxFsr3ConfigureFrameGeneration)DetourFindFunction("dlssg_to_fsr3_amd_is_better.dll", "ffxFsr3ConfigureFrameGeneration");

                if (offxCreateFrameinterpolationSwapchainForHwndDX12_Mod != nullptr)
                {
                    spdlog::info("ImGuiOverlayDx12::CheckMods Nukem's ffxCreateFrameinterpolationSwapchainForHwndDX12 found");
                    _bindedFSR3_Mod = true;
                }
            }

            // Hook FSR3 ffxGetDX12SwapchainPtr methods
            if (offxCreateFrameinterpolationSwapchainForHwndDX12_Mod != nullptr && _bindedFSR3_Mod)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)offxCreateFrameinterpolationSwapchainForHwndDX12_Mod, hkffxCreateFrameinterpolationSwapchainForHwndDX12_Mod);
                DetourAttach(&(PVOID&)offxFsr3ConfigureFrameGeneration_Mod, hkffxFsr3ConfigureFrameGeneration_Mod);

                DetourTransactionCommit();
            }
        }

        if (offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 == nullptr && oFfxCreateContext_FSR3 == nullptr)
        {
            // Check for native FSR3 (if game has native FSR3-FG)
            offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 = (PFN_ffxCreateFrameinterpolationSwapchainForHwndDX12)DetourFindFunction("ffx_backend_dx12_x64.dll", "ffxCreateFrameinterpolationSwapchainForHwndDX12");
            offxFsr3ConfigureFrameGeneration_FSR3 = (PFN_ffxFsr3ConfigureFrameGeneration)DetourFindFunction("ffx_fsr3_x64.dll", "ffxFsr3ConfigureFrameGeneration");

            // Hook FSR3 ffxGetDX12SwapchainPtr methods
            if (offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3)
            {
                spdlog::info("ImGuiOverlayDx12::CheckMods FSR3's ffxCreateFrameinterpolationSwapchainForHwndDX12 found");
                _bindedFSR3_Native = true;

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3, hkffxCreateFrameinterpolationSwapchainForHwndDX12_FSR3);
                DetourAttach(&(PVOID&)offxFsr3ConfigureFrameGeneration_FSR3, hkffxFsr3ConfigureFrameGeneration_FSR3);


                DetourTransactionCommit();
            }
            else
            {
                oFfxCreateContext_FSR3 = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxCreateContext");
                oFfxConfigure_FSR3 = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxConfigure");

                if (oFfxCreateContext_FSR3)
                {
                    spdlog::info("ImGuiOverlayDx12::CheckMods FSR3's ffxCreateContext found");
                    _bindedFSR3_Native = true;

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    if (oFfxCreateContext_FSR3 != nullptr)
                        DetourAttach(&(PVOID&)oFfxCreateContext_FSR3, hkFfxCreateContext_FSR3);

                    if (oFfxConfigure_FSR3 != nullptr)
                        DetourAttach(&(PVOID&)oFfxConfigure_FSR3, hkFfxConfigure_FSR3);

                    DetourTransactionCommit();
                }
            }
        }
    }

    return true;
}

static bool BindAll(HWND InHWnd, ID3D12Device* InDevice)
{
    spdlog::info("ImGuiOverlayDx12::BindAll Handle: {0:X}", (ULONG64)InHWnd);

    HRESULT result;

    _skipCleanup = true;

    do
    {

        if (!CheckDx12(InDevice))
            break;

        // Check for Uniscaler, Nukem's & FSR3 Native
        CheckFSR3();

        // Uniscaler captures and uses latest swapchain
        // Avoid creating and hooking swapchains to prevent crashes
        // If EarlyBind no need to bind again
        if (oPresent_Dx12 == nullptr && Config::Instance()->HookD3D12.value_or(true))
        {
            // Create device
            ID3D12Device* device = nullptr;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

            result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::BindAll Dx12 D3D12CreateDevice: {0:X}", (unsigned long)result);
                break;
            }

            // Create queue
            ID3D12CommandQueue* cq = nullptr;

            D3D12_COMMAND_QUEUE_DESC desc = { };
            result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateCommandQueue2: {0:X}", (unsigned long)result);
                break;
            }

            IDXGIFactory4* factory = nullptr;
            IDXGISwapChain1* swapChain1 = nullptr;
            IDXGISwapChain3* swapChain3 = nullptr;

            spdlog::info("ImGuiOverlayDx12::BindAll Dx12 Creating DXGIFactory for hooking");
            result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateDXGIFactory1: {0:X}", (unsigned long)result);
                break;
            }

            if (!_isEarlyBind && oCreateSwapChain_Dx12 == nullptr)
            {
                void** pFactoryVTable = *reinterpret_cast<void***>(factory);

                oCreateSwapChain_Dx12 = (PFN_CreateSwapChain)pFactoryVTable[10];
                oCreateSwapChainForHwnd_Dx12 = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
                oCreateSwapChainForCoreWindow_Dx12 = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
                oCreateSwapChainForComposition_Dx12 = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

                if (oCreateSwapChain_Dx12 != nullptr)
                {
                    spdlog::info("ImGuiOverlayDx12::BindAll Hooking native DXGIFactory");

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    DetourAttach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
                    DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
                    DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
                    DetourAttach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);
                    DetourTransactionCommit();
                }
            }

            // Hook DXGI Factory 
            if (!_bindedFSR3_Uniscaler && factory != nullptr && cq != nullptr && oPresent_Dx12 == nullptr)
            {
                // Setup swap chain
                DXGI_SWAP_CHAIN_DESC1 sd = { };
                sd.BufferCount = NUM_BACK_BUFFERS;
                sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.SampleDesc.Count = 1;
                sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

                // Create SwapChain
                if (_isEarlyBind)
                    result = factory->CreateSwapChainForHwnd(cq, InHWnd, &sd, NULL, NULL, &swapChain1);
                else
                    result = oCreateSwapChainForHwnd_Dx12(factory, cq, InHWnd, &sd, NULL, NULL, &swapChain1);

                if (result != S_OK)
                {
                    spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
                    break;
                }

                result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
                if (result != S_OK)
                {
                    spdlog::error("ImGuiOverlayDx12::BindAll Dx12 QueryInterface: {0:X}", (unsigned long)result);
                    break;
                }
            }

            if (swapChain3 != nullptr && oPresent_Dx12 == nullptr)
            {
                void** pVTable = *reinterpret_cast<void***>(swapChain3);

                oPresent_Dx12 = (PFN_Present)pVTable[8];
                oPresent1_Dx12 = (PFN_Present1)pVTable[22];

                oResizeBuffers_Dx12 = (PFN_ResizeBuffers)pVTable[13];
                oResizeBuffers1_Dx12 = (PFN_ResizeBuffers1)pVTable[39];

                if (oPresent_Dx12 != nullptr)
                {
                    spdlog::info("ImGuiOverlayDx12::BindAll Dx12 Hooking native SwapChain");

                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    DetourAttach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
                    DetourAttach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);
                    DetourAttach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
                    DetourAttach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);


                    DetourTransactionCommit();
                }
            }

            if (swapChain3 != nullptr)
            {
                swapChain3->Release();
                swapChain3 = nullptr;
            }

            if (swapChain1 != nullptr)
            {
                swapChain1->Release();
                swapChain1 = nullptr;
            }

            if (factory != nullptr)
            {
                factory->Release();
                factory = nullptr;
            }

            if (cq != nullptr)
            {
                cq->Release();
                cq = nullptr;
            }

            if (device != nullptr)
            {
                device->Release();
                device = nullptr;
            }

        }

        // If not using uniscaler, attaching to sl is needed
        // Try to create SL DXGIFactory
        if (oPresent_SL == nullptr && Config::Instance()->HookSLProxy.value_or(true))
        {
            // Create device
            ID3D12Device* device;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

            // check for sl.interposer
            PFN_D3D12_CREATE_DEVICE slCD = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("sl.interposer.dll", "D3D12CreateDevice");

            if (slCD != nullptr)
            {
                spdlog::info("ImGuiOverlayDx12::BindAll sl.interposer.dll D3D12CreateDevice found");

                result = slCD(NULL, featureLevel, IID_PPV_ARGS(&device));

                if (result != S_OK)
                    result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

                if (result != S_OK)
                {
                    spdlog::error("ImGuiOverlayDx12::BindAll SL D3D12CreateDevice: {0:X}", (unsigned long)result);
                    break;
                }

                // Create queue
                ID3D12CommandQueue* cq = nullptr;
                D3D12_COMMAND_QUEUE_DESC desc = { };
                result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
                if (result != S_OK)
                {
                    spdlog::error("ImGuiOverlayDx12::BindAll SL CreateCommandQueue1: {0:X}", (unsigned long)result);
                    break;
                }

                if (cq != nullptr)
                {
                    void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

                    oExecuteCommandLists_SL = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

                    if (oExecuteCommandLists_SL != nullptr)
                    {
                        spdlog::info("ImGuiOverlayDx12::BindAll SL hooking CommandQueue");

                        DetourTransactionBegin();
                        DetourUpdateThread(GetCurrentThread());

                        DetourAttach(&(PVOID&)oExecuteCommandLists_SL, hkExecuteCommandLists_SL);

                        DetourTransactionCommit();
                    }
                }

                if (!_bindedFSR3_Uniscaler)
                {
                    // Check for sl.interposer
                    PFN_CreateDXGIFactory1 slFactory = (PFN_CreateDXGIFactory1)DetourFindFunction("sl.interposer.dll", "CreateDXGIFactory1");

                    if (slFactory != nullptr)
                        spdlog::info("ImGuiOverlayDx12::BindAll sl.interposer.dll CreateDXGIFactory1 found");

                    IDXGIFactory4* factory = nullptr;
                    IDXGISwapChain1* swapChain1 = nullptr;
                    IDXGISwapChain3* swapChain3 = nullptr;

                    if (slFactory != nullptr)
                    {
                        result = slFactory(IID_PPV_ARGS(&factory));
                        if (result != S_OK)
                        {
                            spdlog::error("ImGuiOverlayDx12::BindAll SL CreateDXGIFactory1: {0:X}", (unsigned long)result);
                            break;
                        }

                        if (oCreateSwapChain_SL == nullptr)
                        {
                            void** pFactoryVTable = *reinterpret_cast<void***>(factory);

                            oCreateSwapChain_SL = (PFN_CreateSwapChain)pFactoryVTable[10];
                            oCreateSwapChainForHwnd_SL = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
                            oCreateSwapChainForCoreWindow_SL = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
                            oCreateSwapChainForComposition_SL = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

                            if (oCreateSwapChain_SL != nullptr)
                            {
                                spdlog::info("ImGuiOverlayDx12::BindAll hooking SL DXGIFactory");

                                DetourTransactionBegin();
                                DetourUpdateThread(GetCurrentThread());

                                if (oCreateSwapChain_SL != nullptr)
                                {
                                    DetourAttach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);
                                    spdlog::debug("ImGuiOverlayDx12::BindAll oCreateSwapChain_SL attached");
                                }

                                if (oCreateSwapChainForHwnd_SL != nullptr)
                                {
                                    DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);
                                    spdlog::debug("ImGuiOverlayDx12::BindAll oCreateSwapChainForHwnd_SL attached");
                                }

                                if (oCreateSwapChainForCoreWindow_SL != nullptr)
                                {
                                    DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);
                                    spdlog::debug("ImGuiOverlayDx12::BindAll oCreateSwapChainForCoreWindow_SL attached");
                                }

                                if (oCreateSwapChainForComposition_SL != nullptr)
                                {
                                    DetourAttach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);
                                    spdlog::debug("ImGuiOverlayDx12::BindAll oCreateSwapChainForComposition_SL attached");
                                }

                                DetourTransactionCommit();

                                spdlog::info("ImGuiOverlayDx12::BindAll attached to DXGIFactory");
                            }
                        }
                        else
                        {
                            spdlog::info("ImGuiOverlayDx12::BindAll oCreateSwapChain_SL != nullptr");
                        }

                        // Hook DXGI Factory 
                        if (factory != nullptr && cq != nullptr && oCreateSwapChainForHwnd_SL != nullptr)
                        {
                            // Setup swap chain
                            DXGI_SWAP_CHAIN_DESC1 sd = { };
                            sd.BufferCount = NUM_BACK_BUFFERS;
                            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                            sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                            sd.SampleDesc.Count = 1;
                            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

                            // Create SwapChain
                            result = oCreateSwapChainForHwnd_SL(factory, cq, InHWnd, &sd, NULL, NULL, &swapChain1);
                            if (result != S_OK)
                            {
                                spdlog::error("ImGuiOverlayDx12::BindAll SL CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
                                break;
                            }

                            spdlog::debug("ImGuiOverlayDx12::BindAll SL SwapChain created");

                            result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
                            if (result != S_OK)
                            {
                                spdlog::error("ImGuiOverlayDx12::BindAll SL QueryInterface: {0:X}", (unsigned long)result);
                                break;
                            }

                            spdlog::debug("ImGuiOverlayDx12::BindAll ISwapChain3 accuired from SL SwapChain");
                        }
                        else
                        {
                            spdlog::debug("ImGuiOverlayDx12::BindAll CreateSwapChainForHwnd_SL factory: {0}null, cq: {1}null, oCreateSwapChainForHwnd_SL: {2}null",
                                          !factory ? "" : "not ", !cq ? "" : "not ", !oCreateSwapChainForHwnd_SL ? "" : "not ");
                        }

                        if (swapChain3 != nullptr)
                        {
                            spdlog::debug("ImGuiOverlayDx12::BindAll Hooking SL SwapChain3 methods");

                            void** pVTable = *reinterpret_cast<void***>(swapChain3);

                            oPresent_SL = (PFN_Present)pVTable[8];
                            oPresent1_SL = (PFN_Present1)pVTable[22];

                            oResizeBuffers_SL = (PFN_ResizeBuffers)pVTable[13];
                            oResizeBuffers1_SL = (PFN_ResizeBuffers1)pVTable[39];

                            if (oPresent_SL != nullptr)
                            {
                                DetourTransactionBegin();
                                DetourUpdateThread(GetCurrentThread());

                                if (oPresent_SL != nullptr)
                                    DetourAttach(&(PVOID&)oPresent_SL, hkPresent_SL);

                                if (oPresent1_SL != nullptr)
                                    DetourAttach(&(PVOID&)oPresent1_SL, hkPresent1_SL);

                                if (oResizeBuffers_SL != nullptr)
                                    DetourAttach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);

                                if (oResizeBuffers1_SL != nullptr)
                                    DetourAttach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);

                                DetourTransactionCommit();

                                spdlog::info("ImGuiOverlayDx12::BindAll hooked SL SwapChain");
                            }
                        }
                        else
                        {
                            spdlog::debug("ImGuiOverlayDx12::BindAll Hook SwapChain3 swapChain3 == nullptr");
                        }

                        if (swapChain3 != nullptr)
                        {
                            swapChain3->Release();
                            swapChain3 = nullptr;
                        }

                        if (swapChain1 != nullptr)
                        {
                            swapChain1->Release();
                            swapChain1 = nullptr;
                        }

                        if (factory != nullptr)
                        {
                            factory->Release();
                            factory = nullptr;
                        }
                    }
                }

                if (cq != nullptr)
                {
                    cq->Release();
                    cq = nullptr;
                }

                if (device != nullptr)
                {
                    device->Release();
                    device = nullptr;
                }
            }
        }

        spdlog::debug("ImGuiOverlayDx12::BindAll completed successfully");

        _skipCleanup = false;
        return true;

    } while (false);

    spdlog::debug("ImGuiOverlayDx12::BindAll completed with errors!");

    _skipCleanup = false;
    return false;
}

#pragma endregion

static void CreateRenderTarget(ID3D12Device* device, IDXGISwapChain* pSwapChain)
{
    spdlog::info("ImGuiOverlayDx12::CreateRenderTarget");

    DXGI_SWAP_CHAIN_DESC desc;
    HRESULT hr = pSwapChain->GetDesc(&desc);

    if (hr != S_OK)
    {
        spdlog::error("ImGuiOverlayDx12::CreateRenderTarget pSwapChain->GetDesc: {0:X}", (unsigned long)hr);
        return;
    }

    for (UINT i = 0; i < desc.BufferCount; ++i)
    {
        ID3D12Resource* pBackBuffer = NULL;

        auto result = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));

        if (result != S_OK)
        {
            spdlog::error("ImGuiOverlayDx12::CreateRenderTarget pSwapChain->GetBuffer: {0:X}", (unsigned long)result);
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

    spdlog::info("ImGuiOverlayDx12::CreateRenderTarget done!");
}

static void CleanupDeviceD3D12(ID3D12Device* InDevice)
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
    {
        if (g_commandAllocators[i])
        {
            g_commandAllocators[i]->Release();
            g_commandAllocators[i] = NULL;
        }
    }

    if (g_pd3dCommandList)
    {
        g_pd3dCommandList->Release();
        g_pd3dCommandList = NULL;
    }

    if (g_pd3dRtvDescHeap)
    {
        g_pd3dRtvDescHeap->Release();
        g_pd3dRtvDescHeap = NULL;
    }

    if (g_pd3dSrvDescHeap)
    {
        g_pd3dSrvDescHeap->Release();
        g_pd3dSrvDescHeap = NULL;
    }
}

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain)
{
    bool drawMenu = false;

    do
    {
        // Wait until upscaler feature is ready
        if (Config::Instance()->CurrentFeature != nullptr &&
            Config::Instance()->CurrentFeature->FrameCount() <= Config::Instance()->MenuInitDelay.value_or(90) + 10)
        {
            if (ImGuiOverlayBase::IsVisible())
            {
                spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 Upscaler feature not created, closing menu");
                ImGuiOverlayBase::HideMenu();
            }

            break;
        }

        // If path is not selected yet
        if (_lastActiveSource == Unknown)
        {
            if (ImGuiOverlayBase::IsVisible())
            {
                spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 Swapchain not captured, closing menu");
                ImGuiOverlayBase::HideMenu();
            }

            break;
        }

        // Init base
        if (!ImGuiOverlayBase::IsInited())
            ImGuiOverlayBase::Init(Util::GetProcessWindow());

        // If handle is changed
        if (ImGuiOverlayBase::IsInited() && ImGuiOverlayBase::IsResetRequested())
        {
            auto hwnd = Util::GetProcessWindow();
            spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 Reset request detected, resetting ImGui, new handle {0:X}", (ULONG64)hwnd);
            ImGuiOverlayDx12::ReInitDx12(hwnd);
        }

        if (!ImGuiOverlayBase::IsInited())
            break;

        // Draw only when menu activated
        if (!ImGuiOverlayBase::IsVisible())
            break;

        if (g_pd3dCommandQueue == nullptr)
            break;

        drawMenu = true;

    } while (false);

    if (!drawMenu)
    {
        ImGuiOverlayBase::HideMenu();
        return;
    }

    // Get device from swapchain
    ID3D12Device* device = nullptr;
    auto result = pSwapChain->GetDevice(IID_PPV_ARGS(&device));

    if (result != S_OK)
    {
        spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 GetDevice: {0:X}", (unsigned long)result);
        ImGuiOverlayBase::HideMenu();
        return;
    }

    // Generate ImGui resources
    if (!ImGui::GetIO().BackendRendererUserData && g_pd3dCommandQueue != nullptr)
    {
        spdlog::debug("ImGuiOverlayDx12::RenderImGui_DX12 ImGui::GetIO().BackendRendererUserData == nullptr");

        HRESULT result;

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = { };
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;

            result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dRtvDescHeap): {0:X}", (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTarget(true);
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
            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dSrvDescHeap): {0:X}", (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTarget(true);
                return;
            }
        }

        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i]));

            if (result != S_OK)
            {
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateCommandAllocator[{0}]: {1:X}", i, (unsigned long)result);
                ImGuiOverlayBase::HideMenu();
                CleanupRenderTarget(true);
                return;
            }
        }

        result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList));
        if (result != S_OK)
        {
            spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateCommandList: {0:X}", (unsigned long)result);
            ImGuiOverlayBase::HideMenu();
            CleanupRenderTarget(true);
            return;
        }

        result = g_pd3dCommandList->Close();
        if (result != S_OK)
        {
            spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
            ImGuiOverlayBase::HideMenu();
            CleanupRenderTarget(false);
            return;
        }

        ImGui_ImplDX12_Init(device, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

        return;
    }

    if (ImGuiOverlayDx12::IsInitedDx12())
    {
        // Generate render targets
        if (!g_mainRenderTargetResource[0])
        {
            CreateRenderTarget(device, pSwapChain);
            return;
        }

        // If everything is ready render the frame
        if (ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])
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
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 commandAllocator->Reset: {0:X}", (unsigned long)result);
                CleanupRenderTarget(false);
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
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Reset: {0:X}", (unsigned long)result);
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
                spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
                CleanupRenderTarget(true);
                return;
            }

            ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };

            if (oExecuteCommandLists_Dx12 != nullptr)
                oExecuteCommandLists_Dx12(g_pd3dCommandQueue, 1, (ID3D12CommandList**)ppCommandLists);
            else
                g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);
        }
        else
        {
            if (_showRenderImGuiDebugOnce)
                spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 !(ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])");

            ImGuiOverlayBase::HideMenu();
            _showRenderImGuiDebugOnce = false;
        }
    }

    if (device != nullptr)
    {
        device->Release();
        device = nullptr;
    }
}

void DeatachAllHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (oD3D12CreateDevice != nullptr)
    {
        DetourDetach(&(PVOID&)oD3D12CreateDevice, hkD3D12CreateDevice);
        oD3D12CreateDevice = nullptr;
    }

    if (oCreateCommandQueue != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateCommandQueue, hkCreateCommandQueue);
        oCreateCommandQueue = nullptr;
    }

    if (oCreateDXGIFactory1 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateDXGIFactory1, hkCreateDXGIFactory1);
        oCreateDXGIFactory1 = nullptr;
    }

    if (oCreateDXGIFactory2 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateDXGIFactory2, hkCreateDXGIFactory2);
        oCreateDXGIFactory2 = nullptr;
    }

    if (oCommandQueueRelease != nullptr)
    {
        DetourDetach(&(PVOID&)oCommandQueueRelease, hkCommandQueueRelease);
        oCommandQueueRelease = nullptr;
    }

    if (oPresent_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
        oPresent_Dx12 = nullptr;
    }

    if (oPresent1_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);
        oPresent1_Dx12 = nullptr;
    }

    if (oResizeBuffers_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
        oResizeBuffers_Dx12 = nullptr;
    }

    if (oResizeBuffers1_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);
        oResizeBuffers1_Dx12 = nullptr;
    }

    if (oPresent_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent_SL, hkPresent_SL);
        oPresent_SL = nullptr;
    }

    if (oPresent1_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent1_SL, hkPresent1_SL);
        oPresent1_SL = nullptr;
    }

    if (oResizeBuffers_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);
        oResizeBuffers_SL = nullptr;
    }

    if (oResizeBuffers1_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);
        oResizeBuffers1_SL = nullptr;
    }

    if (oPresent_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent_FSR3, hkPresent_FSR3);
        oPresent_FSR3 = nullptr;
    }

    if (oPresent1_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent1_FSR3, hkPresent1_FSR3);
        oPresent1_FSR3 = nullptr;
    }

    if (oResizeBuffers_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers_FSR3, hkResizeBuffers_FSR3);
        oResizeBuffers_FSR3 = nullptr;
    }

    if (oResizeBuffers1_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers1_FSR3, hkResizeBuffers1_FSR3);
        oResizeBuffers1_FSR3 = nullptr;
    }

    if (oPresent_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent_Mod, hkPresent_Mod);
        oPresent_Mod = nullptr;
    }

    if (oPresent1_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)oPresent1_Mod, hkPresent1_Mod);
        oPresent1_Mod = nullptr;
    }

    if (oResizeBuffers_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers_Mod, hkResizeBuffers_Mod);
        oResizeBuffers_Mod = nullptr;
    }

    if (oResizeBuffers1_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)oResizeBuffers1_Mod, hkResizeBuffers1_Mod);
        oResizeBuffers1_Mod = nullptr;
    }

    if (oExecuteCommandLists_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);
        oExecuteCommandLists_Dx12 = nullptr;
    }

    if (oExecuteCommandLists_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oExecuteCommandLists_SL, hkExecuteCommandLists_SL);
        oExecuteCommandLists_SL = nullptr;
    }

    if (oCreateSwapChain_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
        oCreateSwapChain_Dx12 = nullptr;
    }

    if (oCreateSwapChainForHwnd_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
        oCreateSwapChainForHwnd_Dx12 = nullptr;
    }

    if (oCreateSwapChainForComposition_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);
        oCreateSwapChainForComposition_Dx12 = nullptr;
    }

    if (oCreateSwapChainForCoreWindow_Dx12 != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
        oCreateSwapChainForCoreWindow_Dx12 = nullptr;
    }

    if (oCreateSwapChain_EB != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);
        oCreateSwapChain_EB = nullptr;
    }

    if (oCreateSwapChainForHwnd_EB != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);
        oCreateSwapChainForHwnd_EB = nullptr;
    }

    if (oCreateSwapChainForComposition_EB != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);
        oCreateSwapChainForComposition_EB = nullptr;
    }

    if (oCreateSwapChainForCoreWindow_EB != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);
        oCreateSwapChainForCoreWindow_EB = nullptr;
    }

    if (oCreateSwapChain_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);
        oCreateSwapChain_SL = nullptr;
    }

    if (oCreateSwapChainForHwnd_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);
        oCreateSwapChainForHwnd_SL = nullptr;
    }

    if (oCreateSwapChainForComposition_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);
        oCreateSwapChainForComposition_SL = nullptr;
    }

    if (oCreateSwapChainForCoreWindow_SL != nullptr)
    {
        DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);
        oCreateSwapChainForCoreWindow_SL = nullptr;
    }

    if (offxCreateFrameinterpolationSwapchainForHwndDX12_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)offxCreateFrameinterpolationSwapchainForHwndDX12_Mod, hkffxCreateFrameinterpolationSwapchainForHwndDX12_Mod);
        offxCreateFrameinterpolationSwapchainForHwndDX12_Mod = nullptr;
    }

    if (offxFsr3ConfigureFrameGeneration_Mod != nullptr)
    {
        DetourDetach(&(PVOID&)offxFsr3ConfigureFrameGeneration_Mod, hkffxFsr3ConfigureFrameGeneration_Mod);
        offxFsr3ConfigureFrameGeneration_Mod = nullptr;
    }

    if (offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3, hkffxCreateFrameinterpolationSwapchainForHwndDX12_FSR3);
        offxCreateFrameinterpolationSwapchainForHwndDX12_FSR3 = nullptr;
    }

    if (offxFsr3ConfigureFrameGeneration_FSR3 != nullptr)
    {
        DetourDetach(&(PVOID&)offxFsr3ConfigureFrameGeneration_FSR3, hkffxFsr3ConfigureFrameGeneration_FSR3);
        offxFsr3ConfigureFrameGeneration_FSR3 = nullptr;
    }

    if (orgCreateSampler != nullptr)
    {
        DetourDetach(&(PVOID&)orgCreateSampler, hkCreateSampler);
        orgCreateSampler = nullptr;
    }

    DetourTransactionCommit();
}

bool ImGuiOverlayDx12::IsInitedDx12()
{
    return _isInited;
}

bool ImGuiOverlayDx12::IsEarlyBind()
{
    return _isEarlyBind;
}

HWND ImGuiOverlayDx12::Handle()
{
    return ImGuiOverlayBase::Handle();
}

void ImGuiOverlayDx12::InitDx12(HWND InHandle, ID3D12Device* InDevice)
{
    spdlog::info("ImGuiOverlayDx12::InitDx12 Handle: {0:X}", (ULONG64)InHandle);

    g_pd3dDeviceParam = InDevice;

    if (InDevice == nullptr || !_isInited)
    {
        if (!BindAll(InHandle, InDevice))
            spdlog::error("ImGuiOverlayDx12::InitDx12 BindAll failed!");

        ImGuiOverlayBase::Dx12Ready();
        _isInited = true;
        CleanupDeviceD3D12(InDevice);
    }
}

void ImGuiOverlayDx12::Dx12Bind()
{
    if (_isInited)
        return;

    auto d3d12Module = LoadLibrary(L"d3d12.dll");

    if (d3d12Module == nullptr)
        return;

    _skipCleanup = true;

    oD3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12Module, "D3D12CreateDevice");

    if (oD3D12CreateDevice != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)oD3D12CreateDevice, hkD3D12CreateDevice);

        DetourTransactionCommit();
    }

    auto dxgiModule = LoadLibrary(L"dxgi.dll");

    if (dxgiModule == nullptr)
    {
        _skipCleanup = false;
        return;
    }

    oCreateDXGIFactory = (PFN_CreateDXGIFactory)GetProcAddress(dxgiModule, "CreateDXGIFactory");
    oCreateDXGIFactory1 = (PFN_CreateDXGIFactory1)GetProcAddress(dxgiModule, "CreateDXGIFactory1");
    oCreateDXGIFactory2 = (PFN_CreateDXGIFactory2)GetProcAddress(dxgiModule, "CreateDXGIFactory2");

    if (oCreateDXGIFactory1 != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (oCreateDXGIFactory != nullptr)
            DetourAttach(&(PVOID&)oCreateDXGIFactory, hkCreateDXGIFactory);

        if (oCreateDXGIFactory1 != nullptr)
            DetourAttach(&(PVOID&)oCreateDXGIFactory1, hkCreateDXGIFactory1);

        if (oCreateDXGIFactory2 != nullptr)
            DetourAttach(&(PVOID&)oCreateDXGIFactory2, hkCreateDXGIFactory2);

        DetourTransactionCommit();
    }

    _isEarlyBind = oD3D12CreateDevice != nullptr && oCreateDXGIFactory1 != nullptr;
    _skipCleanup = false;
}

void ImGuiOverlayDx12::FSR3Bind()
{
    CheckFSR3();
}

// Gets upscalers command list to find correct command queue
void ImGuiOverlayDx12::CaptureQueue(ID3D12CommandList* InCommandList)
{
    _dlssCommandList = InCommandList;
}

void ImGuiOverlayDx12::ShutdownDx12()
{
    spdlog::info("ImGuiOverlayDx12::ShutdownDx12");

    if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
        ImGui_ImplDX12_Shutdown();

    ImGuiOverlayBase::Shutdown();

    if (_isInited)
        CleanupDeviceD3D12(g_pd3dDeviceParam);

    DeatachAllHooks();

    _isInited = false;
}

void ImGuiOverlayDx12::ReInitDx12(HWND InNewHwnd)
{
    if (!_isInited)
        return;

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    spdlog::info("ImGuiOverlayDx12::ReInitDx12 hwnd: {0:X}", (ULONG64)InNewHwnd);

    if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
        ImGui_ImplDX12_Shutdown();

    ImGuiOverlayBase::Shutdown();
    ImGuiOverlayBase::Init(InNewHwnd);

    ClearActivePathInfo();
}

