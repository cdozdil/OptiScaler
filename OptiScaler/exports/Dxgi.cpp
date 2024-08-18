#include "Dxgi.h"

#include "../config.h"

#include "../detours/detours.h"

#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

typedef HRESULT(WINAPI* PFN_GetDesc)(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc1)(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc2)(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc3)(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc);

typedef HRESULT(WINAPI* PFN_EnumAdapterByGpuPreference)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, void** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapterByLuid)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, void** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters1)(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters)(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter);

static PFN_GetDesc o_GetDesc = nullptr;
static PFN_GetDesc1 o_GetDesc1 = nullptr;
static PFN_GetDesc2 o_GetDesc2 = nullptr;
static PFN_GetDesc3 o_GetDesc3 = nullptr;

static PFN_EnumAdapters o_EnumAdapters = nullptr;
static PFN_EnumAdapters1 o_EnumAdapters1 = nullptr;
static PFN_EnumAdapterByLuid o_EnumAdapterByLuid = nullptr;
static PFN_EnumAdapterByGpuPreference o_EnumAdapterByGpuPreference = nullptr;

static PFN_AttachDxgiSwapchainHooks attachDxgiSwapchainHooksCallback = nullptr;

#pragma region DXGI Adapter methods

static bool SkipSpoofing()
{
    auto result = !Config::Instance()->DxgiSpoofing.value_or(true) || (Config::Instance()->dxgiSkipSpoofing && Config::Instance()->DxgiSkipSpoofForUpscalers.value_or(true));

    if (result)
        LOG_TRACE("dxgiSkipSpoofing true, skipping spoofing");

    if (!result && Config::Instance()->DxgiBlacklist.has_value() && !Config::Instance()->IsRunningOnLinux)
    {
        result = true;

        // Walk the call stack to find the DLL that is calling the hooked function
        void* callers[100];
        unsigned short frames = CaptureStackBackTrace(0, 100, callers, NULL);
        HANDLE process = GetCurrentProcess();

        if (SymInitialize(process, NULL, TRUE))
        {
            SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);

            if (symbol != nullptr)
            {
                symbol->MaxNameLen = 255;
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

                for (unsigned int i = 0; i < frames; i++)
                {
                    if (SymFromAddr(process, (DWORD64)callers[i], 0, symbol))
                    {
                        auto sn = std::string(symbol->Name);
                        auto pos = Config::Instance()->DxgiBlacklist.value().rfind(sn);

                        LOG_DEBUG("checking for: {0} ({1})", sn, i);

                        if (pos != std::string::npos)
                        {
                            LOG_INFO("spoofing for: {0}", sn);
                            result = false;
                            break;
                        }
                    }
                }

                free(symbol);
            }

            SymCleanup(process);
        }

        if (result)
            LOG_DEBUG("skipping spoofing, blacklisting active");
    }

    return result;
}

static HRESULT WINAPI hkGetDesc3(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc)
{
    auto result = o_GetDesc3(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        std::wstring name(L"NVIDIA GeForce RTX 4090");
        const wchar_t* szName = name.c_str();
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::memcpy(pDesc->Description, szName, 50);

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT WINAPI hkGetDesc2(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc)
{
    auto result = o_GetDesc2(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        std::wstring name(L"NVIDIA GeForce RTX 4090");
        const wchar_t* szName = name.c_str();
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::memcpy(pDesc->Description, szName, 50);

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT WINAPI hkGetDesc1(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc)
{
    auto result = o_GetDesc1(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        std::wstring name(L"NVIDIA GeForce RTX 4090");
        const wchar_t* szName = name.c_str();
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::memcpy(pDesc->Description, szName, 50);

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT WINAPI hkGetDesc(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc)
{
    auto result = o_GetDesc(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        std::wstring name(L"NVIDIA GeForce RTX 4090");
        const wchar_t* szName = name.c_str();
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::memcpy(pDesc->Description, szName, 50);

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

#pragma endregion

#pragma region DXGI Factory methods

static HRESULT WINAPI hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, void** ppvAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = o_EnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, (void**)&adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppvAdapter = adapter;
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, void** ppvAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = o_EnumAdapterByLuid(This, AdapterLuid, riid, (void**)&adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppvAdapter = adapter;
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter1* adapter = nullptr;
    auto result = o_EnumAdapters1(This, Adapter, &adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppAdapter = adapter;
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = o_EnumAdapters(This, Adapter, &adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppAdapter = adapter;
    }

    return result;
}

#pragma endregion

#pragma region DXGI Attach methods

static void AttachToAdapter(IUnknown* unkAdapter)
{
    PVOID* pVTable = *(PVOID**)unkAdapter;

    IDXGIAdapter* adapter = nullptr;
    if (o_GetDesc == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter), (void**)&adapter) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_GetDesc = (PFN_GetDesc)pVTable[8];

        DetourAttach(&(PVOID&)o_GetDesc, hkGetDesc);

        DetourTransactionCommit();
    }

    if (adapter != nullptr)
        adapter->Release();

    IDXGIAdapter1* adapter1 = nullptr;
    if (o_GetDesc1 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter1), (void**)&adapter1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_GetDesc1 = (PFN_GetDesc1)pVTable[10];

        DetourAttach(&(PVOID&)o_GetDesc1, hkGetDesc1);

        DetourTransactionCommit();
    }

    if (adapter1 != nullptr)
        adapter1->Release();

    IDXGIAdapter2* adapter2 = nullptr;
    if (o_GetDesc2 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter2), (void**)&adapter2) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_GetDesc2 = (PFN_GetDesc2)pVTable[11];

        DetourAttach(&(PVOID&)o_GetDesc2, hkGetDesc2);

        DetourTransactionCommit();
    }

    if (adapter2 != nullptr)
        adapter2->Release();

    IDXGIAdapter4* adapter4 = nullptr;
    if (o_GetDesc3 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter4), (void**)&adapter4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_GetDesc3 = (PFN_GetDesc3)pVTable[18];

        DetourAttach(&(PVOID&)o_GetDesc3, hkGetDesc3);

        DetourTransactionCommit();
    }

    if (adapter4 != nullptr)
        adapter4->Release();
}

static void AttachToFactory(IUnknown* unkFactory)
{
    PVOID* pVTable = *(PVOID**)unkFactory;

    IDXGIFactory* factory;
    if (unkFactory->QueryInterface(IID_PPV_ARGS(&factory)) != S_OK)
        return;

    if (attachDxgiSwapchainHooksCallback != nullptr)
        attachDxgiSwapchainHooksCallback(factory);

    if (o_EnumAdapters == nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_EnumAdapters = (PFN_EnumAdapters)pVTable[7];

        DetourAttach(&(PVOID&)o_EnumAdapters, hkEnumAdapters);

        DetourTransactionCommit();
    }
    
    factory->Release();

    IDXGIFactory1* factory1;
    if (o_EnumAdapters1 == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory1), (void**)&factory1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_EnumAdapters1 = (PFN_EnumAdapters1)pVTable[12];

        DetourAttach(&(PVOID&)o_EnumAdapters1, hkEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (o_EnumAdapterByLuid == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory4), (void**)&factory4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_EnumAdapterByLuid = (PFN_EnumAdapterByLuid)pVTable[26];

        DetourAttach(&(PVOID&)o_EnumAdapterByLuid, hkEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (o_EnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory6), (void**)&factory6) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        o_EnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference)pVTable[29];

        DetourAttach(&(PVOID&)o_EnumAdapterByGpuPreference, hkEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

void SetSwapchainHookMethod(PFN_AttachDxgiSwapchainHooks InCallback)
{
}

#pragma endregion 

#pragma region DXGI methods

HRESULT _CreateDXGIFactory(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory* factory;
    HRESULT result = dxgi.CreateDxgiFactory(riid, (void**)&factory);

    if (result == S_OK)
        AttachToFactory(factory);

    *ppFactory = factory;

    return result;
}

HRESULT _CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory1* factory1;
    HRESULT result = dxgi.CreateDxgiFactory1(riid, (void**)&factory1);

    if (result == S_OK)
        AttachToFactory(factory1);

    *ppFactory = factory1;

    return result;
}

HRESULT _CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    IDXGIFactory* factory;
    HRESULT result = dxgi.CreateDxgiFactory2(Flags, riid, (void**)&factory);

    if (result == S_OK)
        AttachToFactory(factory);

    *ppFactory = factory;

    return result;
}

void _DXGIDeclareAdapterRemovalSupport()
{
    LOG_FUNC();
    dxgi.DeclareAdapterRemovalSupport();
}

void _DXGIGetDebugInterface1()
{
    LOG_FUNC();
    dxgi.GetDebugInterface();
}

void _ApplyCompatResolutionQuirking()
{
    LOG_FUNC();
    dxgi.ApplyCompatResolutionQuirking();
}

void _CompatString()
{
    LOG_FUNC();
    dxgi.CompatString();
}

void _CompatValue()
{
    LOG_FUNC();
    dxgi.CompatValue();
}

void _DXGID3D10CreateDevice()
{
    LOG_FUNC();
    dxgi.D3D10CreateDevice();
}

void _DXGID3D10CreateLayeredDevice()
{
    LOG_FUNC();
    dxgi.D3D10CreateLayeredDevice();
}

void _DXGID3D10GetLayeredDeviceSize()
{
    LOG_FUNC();
    dxgi.D3D10GetLayeredDeviceSize();
}

void _DXGID3D10RegisterLayers()
{
    LOG_FUNC();
    dxgi.D3D10RegisterLayers();
}

void _DXGID3D10ETWRundown()
{
    LOG_FUNC();
    dxgi.D3D10ETWRundown();
}

void _DXGIDumpJournal()
{
    LOG_FUNC();
    dxgi.DumpJournal();
}

void _DXGIReportAdapterConfiguration()
{
    LOG_FUNC();
    dxgi.ReportAdapterConfiguration();
}

void _PIXBeginCapture()
{
    LOG_FUNC();
    dxgi.PIXBeginCapture();
}

void _PIXEndCapture()
{
    LOG_FUNC();
    dxgi.PIXEndCapture();
}

void _PIXGetCaptureState()
{
    LOG_FUNC();
    dxgi.PIXGetCaptureState();
}

void _SetAppCompatStringPointer()
{
    LOG_FUNC();
    dxgi.SetAppCompatStringPointer();
}

void _UpdateHMDEmulationStatus()
{
    LOG_FUNC();
    dxgi.UpdateHMDEmulationStatus();
}

#pragma endregion


