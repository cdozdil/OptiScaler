#include "Dxgi_Hooks.h"

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

static std::function<void> createSwapChainCallback = nullptr;

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
        DetourAttach(&(PVOID&)o_CreateDxgiFactory, _CreateDXGIFactory);

    if (o_CreateDxgiFactory1 != nullptr)
        DetourAttach(&(PVOID&)o_CreateDxgiFactory1, _CreateDXGIFactory1);

    if (o_CreateDxgiFactory2 != nullptr)
        DetourAttach(&(PVOID&)o_CreateDxgiFactory2, _CreateDXGIFactory2);

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
