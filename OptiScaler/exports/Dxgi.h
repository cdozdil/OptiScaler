#pragma once
#include "../pch.h"
#include "../config.h"

#include <dxgi1_6.h>
#include <DbgHelp.h>
#include "../detours/detours.h"

#pragma comment(lib, "Dbghelp.lib")

typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY)(REFIID riid, _COM_Outptr_ void** ppFactory);
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);
typedef HRESULT(WINAPI* PFN_DECLARE_ADAPTER_REMOVAL_SUPPORT)();
typedef HRESULT(WINAPI* PFN_GET_DEBUG_INTERFACE)(UINT Flags, REFIID riid, void** ppDebug);

typedef HRESULT(WINAPI* PFN_GetDesc)(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc1)(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc2)(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc);
typedef HRESULT(WINAPI* PFN_GetDesc3)(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc);

typedef HRESULT(WINAPI* PFN_EnumAdapterByGpuPreference)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, void** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapterByLuid)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, void** ppvAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters1)(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter);
typedef HRESULT(WINAPI* PFN_EnumAdapters)(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter);

inline static PFN_GetDesc ptrGetDesc = nullptr;
inline static PFN_GetDesc1 ptrGetDesc1 = nullptr;
inline static PFN_GetDesc2 ptrGetDesc2 = nullptr;
inline static PFN_GetDesc3 ptrGetDesc3 = nullptr;

inline static PFN_EnumAdapters ptrEnumAdapters = nullptr;
inline static PFN_EnumAdapters1 ptrEnumAdapters1 = nullptr;
inline static PFN_EnumAdapterByLuid ptrEnumAdapterByLuid = nullptr;
inline static PFN_EnumAdapterByGpuPreference ptrEnumAdapterByGpuPreference = nullptr;

void AttachToAdapter(IUnknown* unkAdapter);
void AttachToFactory(IUnknown* unkFactory);
bool SkipSpoofing();

struct dxgi_dll
{
    HMODULE dll = nullptr;

    PFN_CREATE_DXGI_FACTORY CreateDxgiFactory;
    PFN_CREATE_DXGI_FACTORY CreateDxgiFactory1;
    PFN_CREATE_DXGI_FACTORY_2 CreateDxgiFactory2;

    FARPROC dxgiDeclareAdapterRemovalSupport;
    FARPROC dxgiGetDebugInterface;
    FARPROC dxgiApplyCompatResolutionQuirking = nullptr;
    FARPROC dxgiCompatString = nullptr;
    FARPROC dxgiCompatValue = nullptr;
    FARPROC dxgiD3D10CreateDevice = nullptr;
    FARPROC dxgiD3D10CreateLayeredDevice = nullptr;
    FARPROC dxgiD3D10GetLayeredDeviceSize = nullptr;
    FARPROC dxgiD3D10RegisterLayers = nullptr;
    FARPROC dxgiD3D10ETWRundown = nullptr;
    FARPROC dxgiDumpJournal = nullptr;
    FARPROC dxgiReportAdapterConfiguration = nullptr;
    FARPROC dxgiPIXBeginCapture = nullptr;
    FARPROC dxgiPIXEndCapture = nullptr;
    FARPROC dxgiPIXGetCaptureState = nullptr;
    FARPROC dxgiSetAppCompatStringPointer = nullptr;
    FARPROC dxgiUpdateHMDEmulationStatus = nullptr;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(module, "CreateDXGIFactory");
        CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(module, "CreateDXGIFactory1");
        CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)GetProcAddress(module, "CreateDXGIFactory2");

        dxgiDeclareAdapterRemovalSupport = GetProcAddress(module, "DXGIDeclareAdapterRemovalSupport");
        dxgiGetDebugInterface = GetProcAddress(module, "DXGIGetDebugInterface1");
        dxgiApplyCompatResolutionQuirking = GetProcAddress(module, "ApplyCompatResolutionQuirking");
        dxgiCompatString = GetProcAddress(module, "CompatString");
        dxgiCompatValue = GetProcAddress(module, "CompatValue");
        dxgiD3D10CreateDevice = GetProcAddress(module, "DXGID3D10CreateDevice");
        dxgiD3D10CreateLayeredDevice = GetProcAddress(module, "DXGID3D10CreateLayeredDevice");
        dxgiD3D10GetLayeredDeviceSize = GetProcAddress(module, "DXGID3D10GetLayeredDeviceSize");
        dxgiD3D10RegisterLayers = GetProcAddress(module, "DXGID3D10RegisterLayers");
        dxgiD3D10ETWRundown = GetProcAddress(module, "DXGID3D10ETWRundown");
        dxgiDumpJournal = GetProcAddress(module, "DXGIDumpJournal");
        dxgiReportAdapterConfiguration = GetProcAddress(module, "DXGIReportAdapterConfiguration");
        dxgiPIXBeginCapture = GetProcAddress(module, "PIXBeginCapture");
        dxgiPIXEndCapture = GetProcAddress(module, "PIXEndCapture");
        dxgiPIXGetCaptureState = GetProcAddress(module, "PIXGetCaptureState");
        dxgiSetAppCompatStringPointer = GetProcAddress(module, "SetAppCompatStringPointer");
        dxgiUpdateHMDEmulationStatus = GetProcAddress(module, "UpdateHMDEmulationStatus");
    }
} dxgi;

#pragma region DXGI Adapter methods

inline static bool SkipSpoofing()
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

HRESULT WINAPI detGetDesc3(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc)
{
    auto result = ptrGetDesc3(This, pDesc);
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

HRESULT WINAPI detGetDesc2(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc)
{
    auto result = ptrGetDesc2(This, pDesc);
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

HRESULT WINAPI detGetDesc1(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc)
{
    auto result = ptrGetDesc1(This, pDesc);
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

HRESULT WINAPI detGetDesc(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc)
{
    auto result = ptrGetDesc(This, pDesc);
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

HRESULT WINAPI detEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, void** ppvAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = ptrEnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, (void**)&adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppvAdapter = adapter;
    }

    return result;
}

HRESULT WINAPI detEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, void** ppvAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = ptrEnumAdapterByLuid(This, AdapterLuid, riid, (void**)&adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppvAdapter = adapter;
    }

    return result;
}

HRESULT WINAPI detEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter1* adapter = nullptr;
    auto result = ptrEnumAdapters1(This, Adapter, &adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppAdapter = adapter;
    }

    return result;
}

HRESULT WINAPI detEnumAdapters(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter)
{
    AttachToFactory(This);

    IDXGIAdapter* adapter = nullptr;
    auto result = ptrEnumAdapters(This, Adapter, &adapter);

    if (result == S_OK)
    {
        AttachToAdapter(adapter);
        *ppAdapter = adapter;
    }

    return result;
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
    dxgi.dxgiDeclareAdapterRemovalSupport();
}

void _DXGIGetDebugInterface1()
{
    LOG_FUNC();
    dxgi.dxgiGetDebugInterface();
}

void _ApplyCompatResolutionQuirking()
{
    LOG_FUNC();
    dxgi.dxgiApplyCompatResolutionQuirking();
}

void _CompatString()
{
    LOG_FUNC();
    dxgi.dxgiCompatString();
}

void _CompatValue()
{
    LOG_FUNC();
    dxgi.dxgiCompatValue();
}

void _DXGID3D10CreateDevice()
{
    LOG_FUNC();
    dxgi.dxgiD3D10CreateDevice();
}

void _DXGID3D10CreateLayeredDevice()
{
    LOG_FUNC();
    dxgi.dxgiD3D10CreateLayeredDevice();
}

void _DXGID3D10GetLayeredDeviceSize()
{
    LOG_FUNC();
    dxgi.dxgiD3D10GetLayeredDeviceSize();
}

void _DXGID3D10RegisterLayers()
{
    LOG_FUNC();
    dxgi.dxgiD3D10RegisterLayers();
}

void _DXGID3D10ETWRundown()
{
    LOG_FUNC();
    dxgi.dxgiD3D10ETWRundown();
}

void _DXGIDumpJournal()
{
    LOG_FUNC();
    dxgi.dxgiDumpJournal();
}

void _DXGIReportAdapterConfiguration()
{
    LOG_FUNC();
    dxgi.dxgiReportAdapterConfiguration();
}

void _PIXBeginCapture()
{
    LOG_FUNC();
    dxgi.dxgiPIXBeginCapture();
}

void _PIXEndCapture()
{
    LOG_FUNC();
    dxgi.dxgiPIXEndCapture();
}

void _PIXGetCaptureState()
{
    LOG_FUNC();
    dxgi.dxgiPIXGetCaptureState();
}

void _SetAppCompatStringPointer()
{
    LOG_FUNC();
    dxgi.dxgiSetAppCompatStringPointer();
}

void _UpdateHMDEmulationStatus()
{
    LOG_FUNC();
    dxgi.dxgiUpdateHMDEmulationStatus();
}

#pragma endregion

#pragma region DXGI Attach methods

inline static void AttachToAdapter(IUnknown* unkAdapter)
{
    PVOID* pVTable = *(PVOID**)unkAdapter;

    IDXGIAdapter* adapter = nullptr;
    if (ptrGetDesc == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter), (void**)&adapter) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc = (PFN_GetDesc)pVTable[8];

        DetourAttach(&(PVOID&)ptrGetDesc, detGetDesc);

        DetourTransactionCommit();
    }

    if (adapter != nullptr)
        adapter->Release();

    IDXGIAdapter1* adapter1 = nullptr;
    if (ptrGetDesc1 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter1), (void**)&adapter1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc1 = (PFN_GetDesc1)pVTable[10];

        DetourAttach(&(PVOID&)ptrGetDesc1, detGetDesc1);

        DetourTransactionCommit();
    }

    if (adapter1 != nullptr)
        adapter1->Release();

    IDXGIAdapter2* adapter2 = nullptr;
    if (ptrGetDesc2 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter2), (void**)&adapter2) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc2 = (PFN_GetDesc2)pVTable[11];

        DetourAttach(&(PVOID&)ptrGetDesc2, detGetDesc2);

        DetourTransactionCommit();
    }

    if (adapter2 != nullptr)
        adapter2->Release();

    IDXGIAdapter4* adapter4 = nullptr;
    if (ptrGetDesc3 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter4), (void**)&adapter4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc3 = (PFN_GetDesc3)pVTable[18];

        DetourAttach(&(PVOID&)ptrGetDesc3, detGetDesc3);

        DetourTransactionCommit();
    }

    if (adapter4 != nullptr)
        adapter4->Release();
}

inline static void AttachToFactory(IUnknown* unkFactory)
{
    PVOID* pVTable = *(PVOID**)unkFactory;

    IDXGIFactory* factory;
    if (ptrEnumAdapters == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory), (void**)&factory) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters = (PFN_EnumAdapters)pVTable[7];

        DetourAttach(&(PVOID&)ptrEnumAdapters, detEnumAdapters);

        DetourTransactionCommit();

        factory->Release();
    }

    IDXGIFactory1* factory1;
    if (ptrEnumAdapters1 == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory1), (void**)&factory1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters1 = (PFN_EnumAdapters1)pVTable[12];

        DetourAttach(&(PVOID&)ptrEnumAdapters1, detEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (ptrEnumAdapterByLuid == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory4), (void**)&factory4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByLuid = (PFN_EnumAdapterByLuid)pVTable[26];

        DetourAttach(&(PVOID&)ptrEnumAdapterByLuid, detEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (ptrEnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory6), (void**)&factory6) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference)pVTable[29];

        DetourAttach(&(PVOID&)ptrEnumAdapterByGpuPreference, detEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

#pragma endregion 