#include "Dxgi.h"

#include <Util.h>
#include <Config.h>
#include <WorkingMode.h>
#include <exports/Dxgi.h>
#include <objects/WrappedSwapChain.h>

#include <d3d12.h>
#include <d3d11_4.h>
#include <include/detours/detours.h>

typedef HRESULT(*PFN_CreateSwapChain)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**);
typedef HRESULT(*PFN_CreateSwapChainForHwnd)(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**);

static PFN_CreateSwapChain o_CreateSwapChain = nullptr;
static PFN_CreateSwapChainForHwnd o_CreateSwapChainForHwnd = nullptr;

static bool dx11Device = false;
static bool dx12Device = false;

static void CleanupRenderTarget(bool clearQueue, HWND hWnd)
{
    LOG_FUNC();

    if (clearQueue)
        currentSCCommandQueue = nullptr;

    if (dx11Device)
        CleanupRenderTargetDx11(false);
    else
        CleanupRenderTargetDx12(clearQueue);

    // Releasing RTSS D3D11on12 device
    if (clearQueue && GetModuleHandle(L"RTSSHooks64.dll") != nullptr)
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

        LOG_FUNC_RESULT(presentResult);
        return presentResult;
    }

    ID3D12CommandQueue* cq = nullptr;
    ID3D11Device* device = nullptr;
    ID3D12Device* device12 = nullptr;

    // try to obtain directx objects and find the path
    if (pDevice->QueryInterface(IID_PPV_ARGS(&device)) == S_OK)
    {
        if (!dx11Device)
            LOG_DEBUG("D3D11Device captured");

        dx11Device = true;
    }
    else if (pDevice->QueryInterface(IID_PPV_ARGS(&cq)) == S_OK)
    {
        if (!dx12Device)
            LOG_DEBUG("D3D12CommandQueue captured");

        currentSCCommandQueue = pDevice;
        MenuDx::GameCommandQueue = (ID3D12CommandQueue*)pDevice;

        if (cq->GetDevice(IID_PPV_ARGS(&device12)) == S_OK)
        {
            if (!dx12Device)
                LOG_DEBUG("D3D12Device captured");

            dx12Device = true;
        }
    }

    // Upscaler GPU time computation
    if (MenuDx::dx12UpscaleTrig && MenuDx::readbackBuffer != nullptr && MenuDx::queryHeap != nullptr && cq != nullptr)
    {
        if (MenuBase::IsInited() && MenuBase::IsVisible())
        {
            UINT64* timestampData;
            MenuDx::readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&timestampData));

            // Get the GPU timestamp frequency (ticks per second)
            UINT64 gpuFrequency;
            cq->GetTimestampFrequency(&gpuFrequency);

            // Calculate elapsed time in milliseconds
            UINT64 startTime = timestampData[0];
            UINT64 endTime = timestampData[1];
            double elapsedTimeMs = (endTime - startTime) / static_cast<double>(gpuFrequency) * 1000.0;

            Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
            Config::Instance()->upscaleTimes.pop_front();

            // Unmap the buffer
            MenuDx::readbackBuffer->Unmap(0, nullptr);
        }

        MenuDx::dx12UpscaleTrig = false;
    }
    else if (MenuDx::dx11UpscaleTrig[MenuDx::currentFrameIndex] && device != nullptr && MenuDx::disjointQueries[0] != nullptr &&
             MenuDx::startQueries[0] != nullptr && MenuDx::endQueries[0] != nullptr)
    {
        if (g_pd3dDeviceContext == nullptr)
            device->GetImmediateContext(&g_pd3dDeviceContext);

        if (MenuBase::IsInited() && MenuBase::IsVisible())
        {
            // Retrieve the results from the previous frame
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
            if (g_pd3dDeviceContext->GetData(MenuDx::disjointQueries[MenuDx::previousFrameIndex], &disjointData, sizeof(disjointData), 0) == S_OK)
            {
                if (!disjointData.Disjoint && disjointData.Frequency > 0)
                {
                    UINT64 startTime = 0, endTime = 0;
                    if (g_pd3dDeviceContext->GetData(MenuDx::startQueries[MenuDx::previousFrameIndex], &startTime, sizeof(UINT64), 0) == S_OK &&
                        g_pd3dDeviceContext->GetData(MenuDx::endQueries[MenuDx::previousFrameIndex], &endTime, sizeof(UINT64), 0) == S_OK)
                    {
                        double elapsedTimeMs = (endTime - startTime) / static_cast<double>(disjointData.Frequency) * 1000.0;
                        Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
                        Config::Instance()->upscaleTimes.pop_front();
                    }
                }
            }
        }


        MenuDx::dx11UpscaleTrig[MenuDx::currentFrameIndex] = false;
        MenuDx::currentFrameIndex = (MenuDx::currentFrameIndex + 1) % MenuDx::QUERY_BUFFER_COUNT;
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

    // Process window handle changed, update base
    if (MenuBase::Handle() != hWnd)
    {
        LOG_DEBUG("Handle changed");

        if (MenuBase::IsInited())
            MenuBase::Shutdown();

        MenuBase::Init(hWnd);

        _isInited = false;
    }

    // Init
    if (!_isInited)
    {
        if (dx11Device)
        {
            CleanupRenderTargetDx11(false);

            g_pd3dDevice = device;
            g_pd3dDevice->AddRef();

            CreateRenderTargetDx11(pSwapChain);
            MenuBase::Dx11Ready();
            _isInited = true;
        }
        else if (dx12Device && (g_pd3dDeviceParam != nullptr || device12 != nullptr))
        {
            if (g_pd3dDeviceParam != nullptr && device12 == nullptr)
                device12 = g_pd3dDeviceParam;

            CleanupRenderTargetDx12(true);

            g_pd3dCommandQueue = cq;
            g_pd3dDeviceParam = device12;

            g_pd3dCommandQueue->AddRef();
            g_pd3dDeviceParam->AddRef();

            MenuBase::Dx12Ready();
            _isInited = true;
        }
    }

    // dx11 multi thread safety
    ID3D11Multithread* dx11MultiThread = nullptr;
    ID3D11DeviceContext* dx11Context = nullptr;
    bool mtState = false;

    if (dx11Device)
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
    if (dx11Device)
        RenderImGui_DX11(pSwapChain);
    else if (dx12Device)
        RenderImGui_DX12(pSwapChain);

    // swapchain present
    if (pPresentParameters == nullptr)
        presentResult = pSwapChain->Present(SyncInterval, Flags);
    else
        presentResult = ((IDXGISwapChain1*)pSwapChain)->Present1(SyncInterval, Flags, pPresentParameters);

    // dx11 multi thread safety
    if (dx11Device && dx11MultiThread != nullptr)
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

    return presentResult;
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

        return o_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return o_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    }

    auto result = o_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Windowed: {4}", pDesc->BufferDesc.Width, pDesc->BufferDesc.Height, (UINT)pDesc->BufferDesc.Format, pDesc->BufferCount, pDesc->Windowed);

        if (Util::GetProcessWindow() == pDesc->OutputWindow)
        {
            Config::Instance()->ScreenWidth = pDesc->BufferDesc.Width;
            Config::Instance()->ScreenHeight = pDesc->BufferDesc.Height;
        }

        if (Config::Instance()->OverlayMenu.value_or(true))
        {
            LOG_DEBUG("created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDesc->OutputWindow);
            *ppSwapChain = new WrappedSwapChain(*ppSwapChain, pDevice, pDesc->OutputWindow, Present, CleanupRenderTarget);
            LOG_DEBUG("created new WrappedSwapChain: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);
        }
    }

    return result;
}

static HRESULT hkCreateSwapChainForHwnd(IDXGIFactory* pCommandQueue, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                        const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    LOG_FUNC();

    *ppSwapChain = nullptr;

    if (Config::Instance()->VulkanCreatingSC)
    {
        LOG_WARN("Vulkan is creating swapchain!");

        if (pDesc != nullptr)
            LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Flags: {4:X}", pDesc->Width, pDesc->Height, (UINT)pDesc->Format, pDesc->BufferCount, pDesc->Flags);

        return o_CreateSwapChainForHwnd(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    if (pDevice == nullptr)
    {
        LOG_WARN("pDevice is nullptr!");
        return o_CreateSwapChainForHwnd(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    }

    auto result = o_CreateSwapChainForHwnd(pCommandQueue, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("Width: {0}, Height: {1}, Format: {2:X}, Count: {3}, Flags: {4:X}", pDesc->Width, pDesc->Height, (UINT)pDesc->Format, pDesc->BufferCount, pDesc->Flags);

        if (Util::GetProcessWindow() == hWnd)
        {
            Config::Instance()->ScreenWidth = pDesc->Width;
            Config::Instance()->ScreenHeight = pDesc->Height;
        }

        if (Config::Instance()->OverlayMenu.value_or(true))
        {
            LOG_DEBUG("created new swapchain: {0:X}, hWnd: {1:X}", (UINT64)*ppSwapChain, (UINT64)hWnd);
            *ppSwapChain = new WrappedSwapChain(*ppSwapChain, pDevice, hWnd, Present, CleanupRenderTarget);
            LOG_DEBUG("created new WrappedSwapChain: {0:X}, pDevice: {1:X}", (UINT64)*ppSwapChain, (UINT64)pDevice);
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory(REFIID riid, IDXGIFactory** ppFactory)
{
    HRESULT result;

    if (Config::Instance()->DxgiSpoofing.value_or(true))
        result = _CreateDXGIFactory(riid, ppFactory);
    else
        result = dxgi.CreateDxgiFactory(riid, ppFactory);

    if (result == S_OK && o_CreateSwapChain == nullptr)
    {
        void** pFactoryVTable = *reinterpret_cast<void***>(*ppFactory);

        o_CreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];

        if (o_CreateSwapChain != nullptr)
        {
            LOG_INFO("Hooking native DXGIFactory");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)o_CreateSwapChain, hkCreateSwapChain);

            DetourTransactionCommit();
        }
    }

    return result;
}

static HRESULT hkCreateDXGIFactory1(REFIID riid, IDXGIFactory1** ppFactory)
{
    HRESULT result;

    if (Config::Instance()->DxgiSpoofing.value_or(true))
        result = _CreateDXGIFactory1(riid, ppFactory);
    else
        result = dxgi.CreateDxgiFactory1(riid, ppFactory);

    if (result == S_OK && o_CreateSwapChainForHwnd == nullptr)
    {
        IDXGIFactory2* factory2 = nullptr;

        if ((*ppFactory)->QueryInterface(IID_PPV_ARGS(&factory2)) == S_OK && factory2 != nullptr)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory2);

            bool skip = false;

            if (o_CreateSwapChain == nullptr)
                o_CreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            o_CreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

            if (o_CreateSwapChainForHwnd != nullptr)
            {
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)o_CreateSwapChain, hkCreateSwapChain);

                DetourAttach(&(PVOID&)o_CreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

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
    HRESULT result;

    if (Config::Instance()->DxgiSpoofing.value_or(true))
        result = _CreateDXGIFactory2(Flags, riid, ppFactory);
    else
        result = dxgi.CreateDxgiFactory2(Flags, riid, ppFactory);

    if (result == S_OK && o_CreateSwapChainForHwnd == nullptr)
    {
        IDXGIFactory2* factory2 = nullptr;

        if ((*ppFactory)->QueryInterface(IID_PPV_ARGS(&factory2)) == S_OK && factory2 != nullptr)
        {
            void** pFactoryVTable = *reinterpret_cast<void***>(factory2);

            bool skip = false;

            if (o_CreateSwapChain == nullptr)
                o_CreateSwapChain = (PFN_CreateSwapChain)pFactoryVTable[10];
            else
                skip = true;

            o_CreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];

            if (o_CreateSwapChainForHwnd != nullptr)
            {
                LOG_INFO("Hooking native DXGIFactory");

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                if (!skip)
                    DetourAttach(&(PVOID&)o_CreateSwapChain, hkCreateSwapChain);

                DetourAttach(&(PVOID&)o_CreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

                DetourTransactionCommit();
            }

            factory2->Release();
            factory2 = nullptr;
        }
    }

    return result;
}

inline static void HookForDxgiSpoofing()
{
    // hook dxgi when not working as dxgi.dll
    if (dxgi.CreateDxgiFactory == nullptr && !WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->IsDxgiMode)
    {
        LOG_INFO("DxgiSpoofing is enabled loading dxgi.dll");

        dxgi.CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
        dxgi.CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY_1)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
        dxgi.CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

        if (dxgi.CreateDxgiFactory != nullptr || dxgi.CreateDxgiFactory1 != nullptr || dxgi.CreateDxgiFactory2 != nullptr)
        {
            LOG_INFO("dxgi.dll found, hooking CreateDxgiFactory methods");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (dxgi.CreateDxgiFactory != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory, hkCreateDXGIFactory);

            if (dxgi.CreateDxgiFactory1 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory1, hkCreateDXGIFactory1);

            if (dxgi.CreateDxgiFactory2 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory2, hkCreateDXGIFactory2);

            DetourTransactionCommit();
        }
    }
}

void DxgiHooks::Hook()
{
    HookForDxgiSpoofing();
}

void DxgiHooks::Unhook()
{
}
