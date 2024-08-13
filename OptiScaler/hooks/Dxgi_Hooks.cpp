#include "Dxgi_Hooks.h"

#include "proxies/Wrapped_SwapChain.h"

#include "../exports/Dxgi.h"
#include "../detours/detours.h"

typedef HRESULT(*PFN_CreateSwapChain)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**);
typedef HRESULT(*PFN_CreateSwapChainForHwnd)(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**);
typedef HRESULT(*PFN_CreateSwapChainForCoreWindow)(IDXGIFactory*, IUnknown*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**);
typedef HRESULT(*PFN_CreateSwapChainForComposition)(IDXGIFactory*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**);

static PFN_CREATE_DXGI_FACTORY o_CreateDxgiFactory = nullptr;
static PFN_CREATE_DXGI_FACTORY o_CreateDxgiFactory1 = nullptr;
static PFN_CREATE_DXGI_FACTORY_2 o_CreateDxgiFactory2 = nullptr;

static PFN_CreateSwapChain o_CreateSwapChain = nullptr;
static PFN_CreateSwapChainForHwnd o_CreateSwapChainForHwnd = nullptr;
static PFN_CreateSwapChainForComposition o_CreateSwapChainForComposition = nullptr;
static PFN_CreateSwapChainForCoreWindow o_CreateSwapChainForCoreWindow = nullptr;

static std::function<void(SwapchainSource, IDXGISwapChain*)> presentCallback = nullptr;
static std::function<void(SwapchainSource, bool)> cleanCallback = nullptr;
static std::function<void(SwapchainSource, HWND)> releaseCallback = nullptr;

static void Preset(IDXGISwapChain* InSwapChain)
{
    if (presentCallback != nullptr)
        presentCallback(Dx12, InSwapChain);
}

static void Clean(bool InClearQueue)
{
    if (cleanCallback != nullptr)
        cleanCallback(Dx12, InClearQueue);
}

static void Release(HWND InHWND)
{
    if (releaseCallback != nullptr)
        releaseCallback(Dx12, InHWND);
}

static HRESULT WINAPI hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    if (cleanCallback != nullptr)
        cleanCallback(Dx12, true);

    auto result = o_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                               const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    if (cleanCallback != nullptr)
        cleanCallback(Dx12, true);

    auto result = o_CreateSwapChainForHwnd(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                     IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    if (cleanCallback != nullptr)
        cleanCallback(Dx12, true);

    auto result = o_CreateSwapChainForCoreWindow(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
                                                      IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    if (cleanCallback != nullptr)
        cleanCallback(Dx12, true);

    auto result = o_CreateSwapChainForComposition(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}


static void AttachSwapchainHooks(IDXGIFactory* InFactory)
{
    if (o_CreateSwapChain != nullptr)
        return;

    PVOID* pVTable = *(PVOID**)InFactory;
    o_CreateSwapChain = (PFN_CreateSwapChain)pVTable[10];
    o_CreateSwapChainForHwnd = (PFN_CreateSwapChainForHwnd)pVTable[15];
    o_CreateSwapChainForCoreWindow = (PFN_CreateSwapChainForCoreWindow)pVTable[16];
    o_CreateSwapChainForComposition = (PFN_CreateSwapChainForComposition)pVTable[24];

    LOG_INFO("Hooking native DXGIFactory");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_CreateSwapChain != nullptr)
        DetourAttach(&(PVOID&)o_CreateSwapChain, hkCreateSwapChain);

    if (o_CreateSwapChain != nullptr)
        DetourAttach(&(PVOID&)o_CreateSwapChainForHwnd, hkCreateSwapChainForHwnd);

    if (o_CreateSwapChain != nullptr)
        DetourAttach(&(PVOID&)o_CreateSwapChainForCoreWindow, hkCreateSwapChainForCoreWindow);

    if (o_CreateSwapChain != nullptr)
        DetourAttach(&(PVOID&)o_CreateSwapChainForComposition, hkCreateSwapChainForComposition);

    DetourTransactionCommit();
}

HRESULT static CreateDXGIFactory(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory* factory;
    HRESULT result = dxgi.CreateDxgiFactory(riid, (void**)&factory);

    if (result == S_OK)
        AttachToFactory(factory);

    *ppFactory = factory;

    return result;
}

HRESULT static CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory1* factory1;
    HRESULT result = dxgi.CreateDxgiFactory1(riid, (void**)&factory1);

    if (result == S_OK)
        AttachToFactory(factory1);

    *ppFactory = factory1;

    return result;
}

HRESULT static CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory* factory;
    HRESULT result = dxgi.CreateDxgiFactory2(Flags, riid, (void**)&factory);

    if (result == S_OK)
        AttachToFactory(factory);

    *ppFactory = factory;

    return result;
}

void Hooks::AttachDxgiHooks()
{
    if (o_CreateDxgiFactory != nullptr || o_CreateDxgiFactory1 != nullptr || o_CreateDxgiFactory2 != nullptr)
        return;

    LOG_INFO("DxgiSpoofing is enabled loading dxgi.dll");

    o_CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
    o_CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
    o_CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

    LOG_INFO("dxgi.dll found, hooking CreateDxgiFactory methods");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_CreateDxgiFactory != nullptr)
        DetourAttach(&(PVOID&)o_CreateDxgiFactory, CreateDXGIFactory);

    if (o_CreateDxgiFactory1 != nullptr)
        DetourAttach(&(PVOID&)o_CreateDxgiFactory1, CreateDXGIFactory1);

    if (o_CreateDxgiFactory2 != nullptr)
        DetourAttach(&(PVOID&)o_CreateDxgiFactory2, CreateDXGIFactory2);

    DetourTransactionCommit();
}

void Hooks::DetachDxgiHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_CreateDxgiFactory != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateDxgiFactory, _CreateDXGIFactory);
        o_CreateDxgiFactory = nullptr;
    }

    if (o_CreateDxgiFactory1 != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateDxgiFactory1, _CreateDXGIFactory1);
        o_CreateDxgiFactory1 = nullptr;
    }

    if (o_CreateDxgiFactory2 != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateDxgiFactory2, _CreateDXGIFactory2);
        o_CreateDxgiFactory2 = nullptr;
    }

    DetourTransactionCommit();
}

void Hooks::SetDxgiClean(std::function<void(SwapchainSource, bool)> InCallback)
{
    cleanCallback = InCallback;
}

void SetDxgiPresent(std::function<void(SwapchainSource, IDXGISwapChain*)> InCallback)
{
    presentCallback = InCallback;
}

void SetDxgiRelease(std::function<void(SwapchainSource, HWND)> InCallback)
{
    releaseCallback = InCallback;
}
