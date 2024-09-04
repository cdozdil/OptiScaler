#include "imgui_overlay_base.h"
#include "imgui_overlay_dx.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"
#include <d3d11.h>
#include <dxgi1_6.h>

#include "wrapped_swapchain.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

// dxgi stuff
typedef HRESULT(WINAPI* PFN_EnumAdapterByGpuPreference2)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapterByLuid2)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters12)(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters2)(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter);

inline static PFN_EnumAdapters2 ptrEnumAdapters = nullptr;
inline static PFN_EnumAdapters12 ptrEnumAdapters1 = nullptr;
inline static PFN_EnumAdapterByLuid2 ptrEnumAdapterByLuid = nullptr;
inline static PFN_EnumAdapterByGpuPreference2 ptrEnumAdapterByGpuPreference = nullptr;

// MipMap hooks
typedef void(*PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static PFN_CreateSampler o_CreateSampler = nullptr;


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

typedef HRESULT(*PFN_CreateDXGIFactory)(REFIID riid, void** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef HRESULT(*PFN_CreateDXGIFactory2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

// Dx12 early binding
static PFN_D3D12_CREATE_DEVICE o_D3D12CreateDevice = nullptr;
static PFN_CreateDXGIFactory o_CreateDXGIFactory = nullptr;
static PFN_CreateDXGIFactory1 o_CreateDXGIFactory1 = nullptr;
static PFN_CreateDXGIFactory2 o_CreateDXGIFactory2 = nullptr;

// Dx12 early binding
static PFN_CreateSwapChain oCreateSwapChain_EB = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_EB = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_EB = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_EB = nullptr;

static bool _isInited = false;

// for showing 
static bool _showRenderImGuiDebugOnce = true;

static std::mutex _dx11CleanMutex;
static std::mutex _dx12CleanMutex;

static void RenderImGui_DX12(IDXGISwapChain* pSwapChain);
static void RenderImGui_DX11(IDXGISwapChain* pSwapChain);
static void DeatachAllHooks();

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
    switch (eCurrentFormat)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

static void CleanupRenderTargetDx12(bool clearQueue)
{
    if (!_isInited || !_dx12Device)
        return;

    LOG_DEBUG("clearQueue: {0}!", clearQueue);

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
    }

    _dx12CleanMutex.unlock();
}

static void CreateRenderTarget(ID3D12Device* device, IDXGISwapChain* pSwapChain)
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

static void CleanupDeviceD3D12()
{
    _dx12CleanMutex.lock();

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

    _dx12CleanMutex.unlock();
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

static void CleanupRenderTargetDx11()
{
    LOG_FUNC();

    _dx11CleanMutex.lock();

    if (g_pd3dRenderTarget)
    {
        g_pd3dRenderTarget->Release();
        g_pd3dRenderTarget = NULL;
    }

    _dx11CleanMutex.unlock();
}

static void CleanupRenderTarget(bool clearQueue)
{
    if (_dx11Device)
        CleanupRenderTargetDx11();
    else
        CleanupRenderTargetDx12(clearQueue);
}

#pragma region Mipmap Hook

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


static void HookToDevice(ID3D12Device* InDevice)
{
    if (o_CreateSampler != nullptr || InDevice == nullptr)
        return;

    // Get the vtable pointer
    PVOID* pVTable = *(PVOID**)InDevice;

    // Get the address of the SetComputeRootSignature function from the vtable
    o_CreateSampler = (PFN_CreateSampler)pVTable[22];

    // Apply the detour
    if (o_CreateSampler != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_CreateSampler, hkCreateSampler);

        DetourTransactionCommit();
    }
}

#pragma endregion

#pragma region Hooks for native EB Prensent Methods

static void Present(IDXGISwapChain* pSwapChain)
{
    if (Config::Instance()->IsRunningOnDXVK)
        return;

    if (g_pd3dDevice != nullptr)
        RenderImGui_DX11(pSwapChain);
    else
        RenderImGui_DX12(pSwapChain);
}

#pragma endregion

#pragma region Hook for DXGIFactory for Early bindind

static HRESULT WINAPI hkCreateSwapChain_EB(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    LOG_FUNC();

    if (Config::Instance()->VulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");
        return oCreateSwapChain_EB(pFactory, pDevice, pDesc, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return oCreateSwapChain_EB(pFactory, pDevice, pDesc, ppSwapChain);
    }

    if (pDevice->QueryInterface(IID_PPV_ARGS(&g_pd3dCommandQueue)) == S_OK)
    {
        g_pd3dCommandQueue->Release();
        LOG_DEBUG("D3D12 Command Queue captured");
    }
    else if (pDevice->QueryInterface(IID_PPV_ARGS(&g_pd3dDevice)) == S_OK)
    {
        g_pd3dDevice->Release();
        LOG_DEBUG("D3D11 Device captured");
        _dx11Device = true;
    }

    auto result = oCreateSwapChain_EB(pFactory, pDevice, pDesc, ppSwapChain);

    if (result == S_OK)
    {
        Config::Instance()->ScreenWidth = pDesc->BufferDesc.Width;
        Config::Instance()->ScreenHeight = pDesc->BufferDesc.Height;

        if (ImGuiOverlayBase::Handle() != pDesc->OutputWindow)
        {
            LOG_DEBUG("Handle changed");

            if (ImGuiOverlayBase::IsInited())
                ImGuiOverlayBase::Shutdown();

            ImGuiOverlayBase::Init(pDesc->OutputWindow);
        }

        if (_dx11Device)
        {
            LOG_DEBUG("_dx11Device");
            CleanupRenderTargetDx11();
            CreateRenderTargetDx11(*ppSwapChain);
            ImGuiOverlayBase::Dx11Ready();
            _isInited = true;
        }
        else if (g_pd3dCommandQueue != nullptr)
        {
            LOG_DEBUG("_dx12Device");
            CleanupRenderTargetDx12(true);
            ImGuiOverlayBase::Dx12Ready();
            _isInited = true;
        }

        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Present, CleanupRenderTarget);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_EB(IDXGIFactory* pCommandQueue, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                  const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    LOG_FUNC();

    if (Config::Instance()->VulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");
        return oCreateSwapChainForHwnd_EB(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return oCreateSwapChainForHwnd_EB(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    if (pDevice->QueryInterface(IID_PPV_ARGS(&g_pd3dCommandQueue)) == S_OK)
    {
        LOG_DEBUG("D3D12 Command Queue captured");
        g_pd3dCommandQueue->Release();
    }
    else if (pDevice->QueryInterface(IID_PPV_ARGS(&g_pd3dDevice)) == S_OK)
    {
        LOG_DEBUG("D3D11 Device captured");
        g_pd3dDevice->Release();
        _dx11Device = true;
    }
    else
    {
        return oCreateSwapChainForHwnd_EB(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    auto result = oCreateSwapChainForHwnd_EB(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        Config::Instance()->ScreenWidth = pDesc->Width;
        Config::Instance()->ScreenHeight = pDesc->Height;

        if (ImGuiOverlayBase::Handle() != hWnd)
        {
            LOG_DEBUG("Handle changed");
            if (ImGuiOverlayBase::IsInited())
                ImGuiOverlayBase::Shutdown();

            ImGuiOverlayBase::Init(hWnd);
        }

        if (_dx11Device)
        {
            LOG_DEBUG("_dx11Device");
            CleanupRenderTargetDx11();
            CreateRenderTargetDx11(*ppSwapChain);
            ImGuiOverlayBase::Dx11Ready();
            _isInited = true;
        }
        else if (g_pd3dCommandQueue != nullptr)
        {
            LOG_DEBUG("_dx12Device");
            CleanupRenderTargetDx12(true);
            ImGuiOverlayBase::Dx12Ready();
            _isInited = true;
        }

        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Present, CleanupRenderTarget);
    }

    return result;
}

#pragma endregion

#pragma region Hooks for early binding

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

    adapter->Release();
}

static HRESULT WINAPI detEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IUnknown** ppvAdapter)
{
    auto result = ptrEnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, ppvAdapter);

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT WINAPI detEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IUnknown** ppvAdapter)
{
    auto result = ptrEnumAdapterByLuid(This, AdapterLuid, riid, ppvAdapter);

    if (result == S_OK)
        CheckAdapter(*ppvAdapter);

    return result;
}

static HRESULT WINAPI detEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IUnknown** ppAdapter)
{
    auto result = ptrEnumAdapters1(This, Adapter, ppAdapter);

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
}

static HRESULT WINAPI detEnumAdapters(IDXGIFactory* This, UINT Adapter, IUnknown** ppAdapter)
{
    auto result = ptrEnumAdapters(This, Adapter, ppAdapter);

    if (result == S_OK)
        CheckAdapter(*ppAdapter);

    return result;
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

        DetourAttach(&(PVOID&)ptrEnumAdapters, detEnumAdapters);

        DetourTransactionCommit();

        factory->Release();
    }

    IDXGIFactory1* factory1;
    if (ptrEnumAdapters1 == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory1)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters1 = (PFN_EnumAdapters12)pVTable[12];

        DetourAttach(&(PVOID&)ptrEnumAdapters1, detEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (ptrEnumAdapterByLuid == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByLuid = (PFN_EnumAdapterByLuid2)pVTable[26];

        DetourAttach(&(PVOID&)ptrEnumAdapterByLuid, detEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (ptrEnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference2)pVTable[29];

        DetourAttach(&(PVOID&)ptrEnumAdapterByGpuPreference, detEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

static HRESULT WINAPI hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    auto result = o_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (result == S_OK)
    {
        g_pd3dDeviceParam = (ID3D12Device*)*ppDevice;
        _dx12Device = true;
        HookToDevice(g_pd3dDeviceParam);
    }

    return result;
}

static HRESULT WINAPI hkCreateDXGIFactory(REFIID riid, void** ppFactory)
{
    auto result = o_CreateDXGIFactory1(riid, ppFactory);

    if (result == S_OK)
    {
        auto factory = (IDXGIFactory*)*ppFactory;
        AttachToFactory(factory);
    }

    if (result == S_OK && oCreateSwapChain_EB == nullptr)
    {
        auto factory = (IDXGIFactory*)*ppFactory;

        void** pFactoryVTable = *reinterpret_cast<void***>(factory);

        oCreateSwapChain_EB = (PFN_CreateSwapChain)pFactoryVTable[10];

        if (oCreateSwapChain_EB != nullptr)
        {
            LOG_INFO("Hooking native DXGIFactory");

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
    auto result = o_CreateDXGIFactory1(riid, ppFactory);

    if (result == S_OK)
    {
        auto factory = (IDXGIFactory*)*ppFactory;
        AttachToFactory(factory);
    }

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
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);
                //DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);
                //DetourAttach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);

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
    auto result = o_CreateDXGIFactory2(Flags, riid, ppFactory);

    if (result == S_OK)
    {
        auto factory = (IDXGIFactory*)*ppFactory;
        AttachToFactory(factory);
    }

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
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

                DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);

                DetourTransactionCommit();
            }

            factory4->Release();
            factory4 = nullptr;
        }
    }

    return result;
}

#pragma endregion

static void RenderImGui_DX12(IDXGISwapChain* pSwapChainPlain)
{
    LOG_FUNC();

    bool drawMenu = false;
    IDXGISwapChain3* pSwapChain = nullptr;

    do
    {
        if (pSwapChainPlain->QueryInterface(IID_PPV_ARGS(&pSwapChain)) != S_OK)
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
        pSwapChain->Release();
        return;
    }

    LOG_DEBUG("start drawing");

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
            CreateRenderTarget(device, pSwapChain);
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

static void RenderImGui_DX11(IDXGISwapChain* pSwapChain)
{
    LOG_FUNC();

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

    LOG_DEBUG("start drawing");

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

void DeatachAllHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

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

void ImGuiOverlayDx::UnHookDx()
{
    if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
        ImGui_ImplDX12_Shutdown();

    ImGuiOverlayBase::Shutdown();

    if (_isInited)
        CleanupDeviceD3D12();

    DeatachAllHooks();

    _isInited = false;
}
