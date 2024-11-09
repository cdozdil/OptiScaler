#pragma once

#include "Dxgi.h"

#include <config.h>

#include <DbgHelp.h>
#include <include/detours/detours.h>

typedef HRESULT(*PFN_GetDesc)(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc);
typedef HRESULT(*PFN_GetDesc1)(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc);
typedef HRESULT(*PFN_GetDesc2)(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc);
typedef HRESULT(*PFN_GetDesc3)(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc);

typedef HRESULT(*PFN_EnumAdapterByGpuPreference)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IDXGIAdapter** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapterByLuid)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IDXGIAdapter** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapters1)(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter);
typedef HRESULT(*PFN_EnumAdapters)(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter);

static PFN_GetDesc o_GetDesc = nullptr;
static PFN_GetDesc1 o_GetDesc1 = nullptr;
static PFN_GetDesc2 o_GetDesc2 = nullptr;
static PFN_GetDesc3 o_GetDesc3 = nullptr;

static PFN_EnumAdapters o_EnumAdapters = nullptr;
static PFN_EnumAdapters1 o_EnumAdapters1 = nullptr;
static PFN_EnumAdapterByLuid o_EnumAdapterByLuid = nullptr;
static PFN_EnumAdapterByGpuPreference o_EnumAdapterByGpuPreference = nullptr;

void AttachToAdapter(IUnknown* unkAdapter);
void AttachToFactory(IUnknown* unkFactory);

#pragma region DXGI Adapter methods

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

    if (Config::Instance()->IsRunningOnDXVK)
        LOG_INFO("DXVK adapter detected");
}

bool SkipSpoofing()
{
    auto skip = !Config::Instance()->DxgiSpoofing.value_or(true) || Config::Instance()->dxgiSkipSpoofing;

    if (skip)
        LOG_TRACE("DxgiSpoofing: {}, dxgiSkipSpoofing: {}, skipping spoofing",
                  !Config::Instance()->DxgiSpoofing.value_or(true), Config::Instance()->dxgiSkipSpoofing);

    if (!skip && Config::Instance()->DxgiBlacklist.has_value())
    {
        skip = true;

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
                            skip = false;
                            break;
                        }
                    }
                }

                free(symbol);
            }

            SymCleanup(process);
        }

        if (skip)
            LOG_DEBUG("skipping spoofing, blacklisting active");
    }

    return skip;
}

static HRESULT hkGetDesc3(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc)
{
    auto result = o_GetDesc3(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        auto szName = Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090");
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());

        if (Config::Instance()->DxgiSpoofing.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiSpoofing.has_value() * 1024 * 1024 * 1024;

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT hkGetDesc2(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc)
{
    auto result = o_GetDesc2(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        auto szName = Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090");
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());

        if (Config::Instance()->DxgiSpoofing.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiSpoofing.has_value() * 1024 * 1024 * 1024;

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT hkGetDesc1(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc)
{
    auto result = o_GetDesc1(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        auto szName = Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090");
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());

        if (Config::Instance()->DxgiSpoofing.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiSpoofing.has_value() * 1024 * 1024 * 1024;

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

static HRESULT hkGetDesc(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc)
{
    auto result = o_GetDesc(This, pDesc);
    auto skip = SkipSpoofing();

    if (result == S_OK && !skip && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        pDesc->VendorId = 0x10de;
        pDesc->DeviceId = 0x2684;

        auto szName = Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090");
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());

        if (Config::Instance()->DxgiSpoofing.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiSpoofing.has_value() * 1024 * 1024 * 1024;

#ifdef _DEBUG
        LOG_DEBUG("spoofing");
#endif
    }

    AttachToAdapter(This);

    return result;
}

#pragma endregion

#pragma region DXGI Factory methods

static HRESULT WINAPI hkEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IDXGIAdapter** ppvAdapter)
{
    AttachToFactory(This);

    auto result = o_EnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, ppvAdapter);

    if (result == S_OK)
    {
        CheckAdapter(*ppvAdapter);
        AttachToAdapter(*ppvAdapter);
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IDXGIAdapter** ppvAdapter)
{
    AttachToFactory(This);

    auto result = o_EnumAdapterByLuid(This, AdapterLuid, riid, ppvAdapter);

    if (result == S_OK)
    {
        CheckAdapter(*ppvAdapter);
        AttachToAdapter(*ppvAdapter);
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter)
{
    AttachToFactory(This);

    auto result = o_EnumAdapters1(This, Adapter, ppAdapter);

    if (result == S_OK)
    {
        CheckAdapter(*ppAdapter);
        AttachToAdapter(*ppAdapter);
    }

    return result;
}

static HRESULT WINAPI hkEnumAdapters(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter)
{
    AttachToFactory(This);

    auto result = o_EnumAdapters(This, Adapter, ppAdapter);

    if (result == S_OK)
    {
        CheckAdapter(*ppAdapter);
        AttachToAdapter(*ppAdapter);
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

#pragma endregion 

#pragma region DXGI methods

HRESULT _CreateDXGIFactory(REFIID riid, _COM_Outptr_ IDXGIFactory** ppFactory)
{
    HRESULT result = dxgi.CreateDxgiFactory(riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

    return result;
}

HRESULT _CreateDXGIFactory1(REFIID riid, _COM_Outptr_ IDXGIFactory1** ppFactory)
{
    HRESULT result = dxgi.CreateDxgiFactory1(riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

    return result;
}

HRESULT _CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ IDXGIFactory2** ppFactory)
{
    HRESULT result = dxgi.CreateDxgiFactory2(Flags, riid, ppFactory);

    if (result == S_OK)
        AttachToFactory(*ppFactory);

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



