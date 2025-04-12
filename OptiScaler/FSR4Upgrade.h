#pragma once

#include <pch.h>

#include <proxies/Dxgi_Proxy.h>
#include <proxies/Kernel32_Proxy.h>
#include <proxies/KernelBase_Proxy.h>

#include <Unknwn.h>
#include <Windows.h>

typedef HRESULT(__cdecl* PFN_AmdExtD3DCreateInterface)(IUnknown* pOuter, REFIID riid, void** ppvObject);

static HMODULE moduleAmdxc64 = nullptr;
static Kernel32Proxy::PFN_GetProcAddress o_K32_GetProcAddress = nullptr;
static KernelBaseProxy::PFN_GetProcAddress o_KB_GetProcAddress = nullptr;

#pragma region GDI32

// Manually define structures
typedef struct _D3DKMT_UMDFILENAMEINFO_L {
    UINT    Version;
    WCHAR   UmdFileName[MAX_PATH];
} D3DKMT_UMDFILENAMEINFO_L;

typedef struct _D3DKMT_ADAPTERINFO_L
{
    UINT    hAdapter;
    LUID    AdapterLuid;
    ULONG   NumOfSources;
    BOOL    bPrecisePresentRegionsPreferred;
} D3DKMT_ADAPTERINFO_L;

typedef struct _D3DKMT_QUERYADAPTERINFO_L
{
    UINT    hAdapter;
    UINT    Type;
    VOID* pPrivateDriverData;
    UINT    PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO_L;

typedef struct _D3DKMT_ENUMADAPTERS_L
{
    ULONG               NumAdapters;
    D3DKMT_ADAPTERINFO_L  Adapters[16];
} D3DKMT_ENUMADAPTERS_L;

typedef struct _D3DKMT_CLOSEADAPTER_L
{
    UINT   hAdapter;   // in: adapter handle
} D3DKMT_CLOSEADAPTER_L;

// Function pointers
typedef UINT(*PFN_D3DKMTQueryAdapterInfo_L)(D3DKMT_QUERYADAPTERINFO_L*);
typedef UINT(*PFN_D3DKMTEnumAdapters_L)(D3DKMT_ENUMADAPTERS_L*);
typedef UINT(*PFN_D3DKMTCloseAdapter)(D3DKMT_CLOSEADAPTER_L*);

inline static std::vector<std::filesystem::path> GetDriverStore()
{
    std::vector<std::filesystem::path> result;

    // Load D3DKMT functions dynamically
    bool libraryLoaded = false;
    HMODULE hGdi32 = KernelBaseProxy::GetModuleHandleW_()(L"Gdi32.dll");

    if (hGdi32 == nullptr)
    {
        hGdi32 = KernelBaseProxy::LoadLibraryExW_()(L"Gdi32.dll", NULL, 0);
        libraryLoaded = hGdi32 != nullptr;
    }

    if (hGdi32 == nullptr)
    {
        LOG_ERROR("Failed to load Gdi32.dll");
        return result;
    }

    do
    {
        auto o_D3DKMTEnumAdapters = (PFN_D3DKMTEnumAdapters_L)KernelBaseProxy::GetProcAddress_()(hGdi32, "D3DKMTEnumAdapters");
        auto o_D3DKMTQueryAdapterInfo = (PFN_D3DKMTQueryAdapterInfo_L)KernelBaseProxy::GetProcAddress_()(hGdi32, "D3DKMTQueryAdapterInfo");
        auto o_D3DKMTCloseAdapter = (PFN_D3DKMTCloseAdapter)KernelBaseProxy::GetProcAddress_()(hGdi32, "D3DKMTCloseAdapter");

        if (o_D3DKMTEnumAdapters == nullptr || o_D3DKMTQueryAdapterInfo == nullptr || o_D3DKMTCloseAdapter == nullptr)
        {
            LOG_ERROR("Failed to resolve D3DKMT functions");
            break;
        }

        D3DKMT_UMDFILENAMEINFO_L umdFileInfo = {};
        D3DKMT_QUERYADAPTERINFO_L queryAdapterInfo = {};

        queryAdapterInfo.Type = 1; // KMTQAITYPE_UMDRIVERNAME
        queryAdapterInfo.pPrivateDriverData = &umdFileInfo;
        queryAdapterInfo.PrivateDriverDataSize = sizeof(umdFileInfo);

        D3DKMT_ENUMADAPTERS_L enumAdapters = {};

        // Query the number of adapters first
        if (o_D3DKMTEnumAdapters(&enumAdapters) != 0)
        {
            LOG_ERROR("Failed to enumerate adapters.");
            break;
        }

        // If there are any adapters, the first one should be in the list
        if (enumAdapters.NumAdapters > 0)
        {
            for (size_t i = 0; i < enumAdapters.NumAdapters; i++)
            {
                D3DKMT_ADAPTERINFO_L adapter = enumAdapters.Adapters[i];
                queryAdapterInfo.hAdapter = adapter.hAdapter;

                auto hr = o_D3DKMTQueryAdapterInfo(&queryAdapterInfo);

                if (hr != 0)
                    LOG_ERROR("Failed to query adapter info {:X}", hr);
                else
                    result.push_back(std::filesystem::path(umdFileInfo.UmdFileName).parent_path());


                D3DKMT_CLOSEADAPTER_L closeAdapter = {};
                closeAdapter.hAdapter = adapter.hAdapter;
                auto closeResult = o_D3DKMTCloseAdapter(&closeAdapter);
                if (closeResult != 0)
                    LOG_ERROR("D3DKMTCloseAdapter error: {:X}", closeResult);
            }
        }
        else
        {
            LOG_ERROR("No adapters found.");
            break;
        }

    } while (false);

    if (libraryLoaded)
        KernelBaseProxy::FreeLibrary_()(hGdi32);

    return result;
}

#pragma endregion

/* Potato_of_Doom's Implementation */
#pragma region IAmdExtFfxApi

MIDL_INTERFACE("b58d6601-7401-4234-8180-6febfc0e484c")
IAmdExtFfxApi : public IUnknown
{
public:
    virtual HRESULT UpdateFfxApiProvider(void* pData, uint32_t dataSizeInBytes) = 0;
};

typedef HRESULT(STDMETHODCALLTYPE* PFN_UpdateFfxApiProvider)(void* pData, uint32_t dataSizeInBytes);

struct AmdExtFfxApi : public IAmdExtFfxApi
{
    PFN_UpdateFfxApiProvider pfnUpdateFfxApiProvider = nullptr;

    HRESULT STDMETHODCALLTYPE UpdateFfxApiProvider(void* pData, uint32_t dataSizeInBytes) override
    {
        LOG_INFO("UpdateFfxApiProvider called");

        if (pfnUpdateFfxApiProvider == nullptr)
        {
            auto storePath = GetDriverStore();
            HMODULE fsr4Module = nullptr;

            for (size_t i = 0; i < storePath.size(); i++)
            {
                if (fsr4Module == nullptr)
                {
                    auto dllPath = storePath[i] / L"amdxcffx64.dll";
                    LOG_DEBUG("Trying to load: {}", wstring_to_string(dllPath.c_str()));
                    fsr4Module = KernelBaseProxy::LoadLibraryExW_()(dllPath.c_str(), NULL, 0);
                }
            }

            if (fsr4Module == nullptr)
                fsr4Module = KernelBaseProxy::LoadLibraryExW_()(L"amdxcffx64.dll", NULL, 0);

            if (fsr4Module == nullptr)
            {
                LOG_ERROR("Failed to load amdxcffx64.dll");
                return E_NOINTERFACE;
            }

            pfnUpdateFfxApiProvider = (PFN_UpdateFfxApiProvider)KernelBaseProxy::GetProcAddress_()(fsr4Module, "UpdateFfxApiProvider");

            if (pfnUpdateFfxApiProvider == nullptr)
            {
                LOG_ERROR("Failed to get UpdateFfxApiProvider");
                return E_NOINTERFACE;
            }
        }

        if (pfnUpdateFfxApiProvider != nullptr)
        {
            State::DisableChecks(1);
            auto result = pfnUpdateFfxApiProvider(pData, dataSizeInBytes);
            State::EnableChecks(1);
            return result;
        }

        return E_NOINTERFACE;
    }

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
    {
        return E_NOTIMPL;
    }

    ULONG __stdcall AddRef(void) override
    {
        return 0;
    }

    ULONG __stdcall Release(void) override
    {
        return 0;
    }
};

#pragma endregion

static AmdExtFfxApi* _amdExtFfxApi = nullptr;
static PFN_AmdExtD3DCreateInterface o_AmdExtD3DCreateInterface = nullptr;

/// <summary>
/// Sets Config::Instance()->Fsr4Update if GPU is RDNA4
/// </summary>
static inline void CheckForGPU()
{
    if (Config::Instance()->Fsr4Update.has_value())
        return;

    // Call init for any case
    DxgiProxy::Init();

    IDXGIFactory* factory;
    HRESULT result = DxgiProxy::CreateDxgiFactory_()(__uuidof(factory), &factory);

    UINT adapterIndex = 0;
    DXGI_ADAPTER_DESC adapterDesc{};
    IDXGIAdapter* adapter;

    while (factory->EnumAdapters(adapterIndex, &adapter) == S_OK)
    {
        if (adapter == nullptr)
        {
            adapterIndex++;
            continue;
        }

        State::Instance().skipSpoofing = true;
        result = adapter->GetDesc(&adapterDesc);
        State::Instance().skipSpoofing = false;

        if (result == S_OK && adapterDesc.VendorId != 0x00001414)
        {
            std::wstring szName(adapterDesc.Description);
            std::string descStr = std::format("Adapter: {}, VRAM: {} MB", wstring_to_string(szName), adapterDesc.DedicatedVideoMemory / (1024 * 1024));
            LOG_INFO("{}", descStr);

            // If GPU is AMD
            if (adapterDesc.VendorId == 0x1002)
            {
                // If GPU Name contains 90XX or GFX12 (Linux) always set it to true
                if (szName.find(L" 90") != std::wstring::npos || szName.find(L" GFX12") != std::wstring::npos)
                    Config::Instance()->Fsr4Update = true;
            }
        }
        else
        {
            LOG_DEBUG("Can't get description of adapter: {}", adapterIndex);
        }

        adapter->Release();
        adapter = nullptr;
        adapterIndex++;
    }

    factory->Release();
    factory = nullptr;

    if (!Config::Instance()->Fsr4Update.has_value())
        Config::Instance()->Fsr4Update = false;

    LOG_INFO("Fsr4Update: {}", Config::Instance()->Fsr4Update.value_or_default());
}

inline static HRESULT STDMETHODCALLTYPE hkAmdExtD3DCreateInterface(IUnknown* pOuter, REFIID riid, void** ppvObject)
{
    // If querying IAmdExtFfxApi 
    if (riid == __uuidof(IAmdExtFfxApi))
    {
        if (_amdExtFfxApi == nullptr)
            _amdExtFfxApi = new AmdExtFfxApi();

        // Return custom one
        *ppvObject = _amdExtFfxApi;

        return S_OK;
    }

    if (o_AmdExtD3DCreateInterface != nullptr)
        return o_AmdExtD3DCreateInterface(pOuter, riid, ppvObject);

    return E_NOINTERFACE;
}

inline static FARPROC hk_K32_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == Kernel32Proxy::Module() || hModule == KernelBaseProxy::Module())
        return o_K32_GetProcAddress(hModule, lpProcName);

    if ((size_t)lpProcName < 0x0000000010000000)
        return o_K32_GetProcAddress(hModule, lpProcName);

    // For FSR4 Upgrade
    if (hModule == moduleAmdxc64 && o_AmdExtD3DCreateInterface != nullptr && lpProcName != nullptr && strcmp(lpProcName, "AmdExtD3DCreateInterface") == 0)
    {
        CheckForGPU();

        // Return custom method for upgrade for RDNA4
        if (Config::Instance()->Fsr4Update.value_or_default())
            return (FARPROC)hkAmdExtD3DCreateInterface;
    }

    return o_K32_GetProcAddress(hModule, lpProcName);
}

inline static FARPROC hk_KB_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == dllModule && lpProcName != nullptr)
        LOG_DEBUG("Trying to get process address of {0}", lpProcName);

    // For FSR4 Upgrade
    if (hModule == moduleAmdxc64 && o_AmdExtD3DCreateInterface != nullptr && lpProcName != nullptr && strcmp(lpProcName, "AmdExtD3DCreateInterface") == 0)
    {
        CheckForGPU();

        // Return custom method for upgrade for RDNA4
        if (Config::Instance()->Fsr4Update.value_or_default())
            return (FARPROC)hkAmdExtD3DCreateInterface;
    }

    return o_KB_GetProcAddress(hModule, lpProcName);
}

inline void InitFSR4Update()
{
    LOG_DEBUG("");

    // For FSR4 Upgrade
    moduleAmdxc64 = KernelBaseProxy::GetModuleHandleW_()(L"amdxc64.dll");
    if (moduleAmdxc64 == nullptr)
        moduleAmdxc64 = KernelBaseProxy::LoadLibraryExW_()(L"amdxc64.dll", NULL, 0);

    if (moduleAmdxc64 != nullptr)
    {
        LOG_DEBUG("Found amdxc64.dll");
        o_AmdExtD3DCreateInterface = (PFN_AmdExtD3DCreateInterface)Kernel32Proxy::GetProcAddress_()(moduleAmdxc64, "AmdExtD3DCreateInterface");
    }

    o_K32_GetProcAddress = Kernel32Proxy::Hook_GetProcAddress(hk_K32_GetProcAddress);

    // Disabled 
    //o_KB_GetProcAddress = KernelBaseProxy::Hook_GetProcAddress(hk_KB_GetProcAddress);
}