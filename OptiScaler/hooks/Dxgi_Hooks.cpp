#include "Dxgi_Hooks.h"

#include "../Config.h"

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

static PFN_DxgiPresentCallback presentCallback[2] = { nullptr, nullptr };
static PFN_DxgiCleanCallback cleanCallback[2] = { nullptr, nullptr };
static PFN_DxgiReleaseCallback releaseCallback[2] = { nullptr, nullptr };

static IUnknown* device = nullptr;
static IDXGIAdapter* adapters[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

static void Hooks::EnumarateDxgiAdapters()
{
    if (adapters[0] != nullptr)
        return;

    Config::Instance()->dxgiSkipSpoofing = true;

    IDXGIFactory* dxgiFactory;
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        LOG_ERROR("CreateDXGIFactory error: {0:X}", (UINT)hr);
        return;
    }

    UINT adapterIndex = 0;
    while (dxgiFactory->EnumAdapters(adapterIndex, &adapters[adapterIndex]) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapters[adapterIndex]->GetDesc(&desc);

        LOG_INFO("Adapter #{0}, Vendor ID:{1:X}, Device ID:{2:X}, DVM: {3}, DSM: {4}, SSM: {5}", adapterIndex, desc.VendorId, desc.DeviceId,
                 desc.DedicatedVideoMemory / (1024 * 1024), desc.DedicatedSystemMemory / (1024 * 1024), desc.SharedSystemMemory / (1024 * 1024));

        adapterIndex++;
    }

    Config::Instance()->dxgiSkipSpoofing = false;
}

static void Preset(IDXGISwapChain* InSwapChain)
{
    for (size_t i = 0; i < 2; i++)
    {
        if (presentCallback[i] != nullptr)
            presentCallback[i](InSwapChain);
    }
}

static void Clean(bool InClearQueue)
{
    for (size_t i = 0; i < 2; i++)
    {
        if (cleanCallback[i] != nullptr)
            cleanCallback[i](InClearQueue);
    }
}

static void Release(HWND InHWND)
{
    for (size_t i = 0; i < 2; i++)
    {
        if (releaseCallback[i] != nullptr)
            releaseCallback[i](InHWND);
    }
}

static HRESULT WINAPI hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    Clean(true);
    device = pDevice;

    auto result = o_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    Clean(true);
    device = pDevice;

    auto result = o_CreateSwapChainForHwnd(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    Clean(true);
    device = pDevice;

    auto result = o_CreateSwapChainForCoreWindow(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
    Clean(true);
    device = pDevice;

    auto result = o_CreateSwapChainForComposition(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);

    if (result == S_OK)
    {
        LOG_DEBUG("created new WrappedIDXGISwapChain4");
        *ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), Preset, Clean, Release);
    }

    return result;
}

HRESULT static CreateDXGIFactory(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory* factory;
    HRESULT result = dxgi.CreateDxgiFactory(riid, (void**)&factory);

    if (result == S_OK)
    {
        AttachToFactory(factory);
        Hooks::AttachDxgiSwapchainHooks(factory);
    }

    *ppFactory = factory;

    return result;
}

HRESULT static CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory1* factory1;
    HRESULT result = dxgi.CreateDxgiFactory1(riid, (void**)&factory1);

    if (result == S_OK)
    {
        AttachToFactory(factory1);
        Hooks::AttachDxgiSwapchainHooks(factory1);
    }

    *ppFactory = factory1;

    return result;
}

HRESULT static CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory2* factory2;
    HRESULT result = dxgi.CreateDxgiFactory2(Flags, riid, (void**)&factory2);

    if (result == S_OK)
    {
        AttachToFactory(factory2);
        Hooks::AttachDxgiSwapchainHooks(factory2);
    }

    *ppFactory = factory2;

    return result;
}

void Hooks::AttachDxgiHooks()
{
    if (o_CreateDxgiFactory != nullptr || o_CreateDxgiFactory1 != nullptr || o_CreateDxgiFactory2 != nullptr)
        return;

    LOG_INFO("DxgiSpoofing is enabled loading dxgi.dll");

    Hooks::EnumarateDxgiAdapters();

    o_CreateDxgiFactory = reinterpret_cast<PFN_CREATE_DXGI_FACTORY>(DetourFindFunction("dxgi.dll", "CreateDXGIFactory"));
    o_CreateDxgiFactory1 = reinterpret_cast<PFN_CREATE_DXGI_FACTORY>(DetourFindFunction("dxgi.dll", "CreateDXGIFactory1"));
    o_CreateDxgiFactory2 = reinterpret_cast<PFN_CREATE_DXGI_FACTORY_2>(DetourFindFunction("dxgi.dll", "CreateDXGIFactory2"));

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

void Hooks::AttachDxgiSwapchainHooks(IDXGIFactory* InFactory)
{
    if (o_CreateSwapChain != nullptr)
        return;

    PVOID* pVTable = *reinterpret_cast<PVOID**>(InFactory);
    o_CreateSwapChain = reinterpret_cast<PFN_CreateSwapChain>(pVTable[10]);
    o_CreateSwapChainForHwnd = reinterpret_cast<PFN_CreateSwapChainForHwnd>(pVTable[15]);
    o_CreateSwapChainForCoreWindow = reinterpret_cast<PFN_CreateSwapChainForCoreWindow>(pVTable[16]);
    o_CreateSwapChainForComposition = reinterpret_cast<PFN_CreateSwapChainForComposition>(pVTable[24]);

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

void Hooks::SetDxgiClean(PFN_DxgiCleanCallback InCallback)
{
    if(cleanCallback == nullptr)
        cleanCallback[0] = InCallback;
    else
        cleanCallback[1] = InCallback;
}

void Hooks::SetDxgiPresent(PFN_DxgiPresentCallback InCallback)
{
    if (presentCallback == nullptr)
        presentCallback[0] = InCallback;
    else
        presentCallback[1] = InCallback;
}

void Hooks::SetDxgiRelease(PFN_DxgiReleaseCallback InCallback)
{
    if (releaseCallback == nullptr)
        releaseCallback[0] = InCallback;
    else
        releaseCallback[1] = InCallback;
}

IUnknown* Hooks::DxgiDevice()
{
    return device;
}

IDXGIAdapter* Hooks::GetDXGIAdapter(uint32_t index)
{
    if (index >= 0 && index < 8)
        return adapters[index];

    return nullptr;
}