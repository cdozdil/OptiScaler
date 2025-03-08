#pragma once

#include <pch.h>
#include <Unknwn.h>
#include <Windows.h>

//#include <d3dkmthk.h>

// Manually define structures

typedef struct _D3DKMT_OPENADAPTERFROMLUID_L
{
    LUID            AdapterLuid;
    UINT   hAdapter;
} D3DKMT_OPENADAPTERFROMLUID_L;

typedef struct _D3DKMT_ADAPTERINFO_L {
    uint64_t hAdapter;
    LUID AdapterLuid;
    ULONG NumOfSources;
    BOOL bPrecisePresentRegionsPreferred;
} D3DKMT_ADAPTERINFO_L;

typedef struct _D3DKMT_ENUMADAPTERS_L
{
    ULONG  NumAdapters;
    D3DKMT_ADAPTERINFO_L Adapters[16];
} D3DKMT_ENUMADAPTERS_L;

typedef struct _D3DKMT_UMDFILENAMEINFO_L {
    ULONG Version;
    WCHAR UmdFileName[MAX_PATH];
    WCHAR UmdResourceFileName[MAX_PATH];
} D3DKMT_UMDFILENAMEINFO_L;

typedef struct _D3DKMT_QUERYADAPTERINFO_L {
    uint64_t  hAdapter;
    ULONG   Type;
    VOID* pPrivateDriverData;
    UINT   PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO_L;

// Function pointers
typedef UINT(*PFN_D3DKMTQueryAdapterInfo_L)(D3DKMT_QUERYADAPTERINFO_L*);
typedef UINT(*PFN_D3DKMTEnumAdapters_L)(D3DKMT_ENUMADAPTERS_L*);

inline static std::optional<std::filesystem::path> GetDriverStore()
{
    std::optional<std::filesystem::path> result;

    // Load D3DKMT functions dynamically
    HMODULE hGdi32 = LoadLibrary(L"Gdi32.dll");
    if (hGdi32 == nullptr)
    {
        LOG_ERROR("Failed to load Gdi32.dll");
        return result;
    }

    do
    {
        auto o_D3DKMTEnumAdapters = (PFN_D3DKMTEnumAdapters_L)GetProcAddress(hGdi32, "D3DKMTEnumAdapters");
        //auto o_D3DKMTOpenAdapterFromLuid = (PFN_D3DKMTOpenAdapterFromLuid_L)GetProcAddress(hGdi32, "D3DKMTOpenAdapterFromLuid");
        auto o_D3DKMTQueryAdapterInfo = (PFN_D3DKMTQueryAdapterInfo_L)GetProcAddress(hGdi32, "D3DKMTQueryAdapterInfo");

        if (o_D3DKMTEnumAdapters == nullptr /*|| o_D3DKMTOpenAdapterFromLuid == nullptr*/ || o_D3DKMTQueryAdapterInfo == nullptr)
        {
            LOG_ERROR("Failed to resolve D3DKMT functions");
            break;
        }

        D3DKMT_UMDFILENAMEINFO_L umdFileInfo = {};
        D3DKMT_QUERYADAPTERINFO_L queryAdapterInfo = {};

        queryAdapterInfo.Type = 1;
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
            D3DKMT_ADAPTERINFO_L adapter = enumAdapters.Adapters[0];

            // Now you have the first adapter with a D3DKMT_HANDLE
            queryAdapterInfo.hAdapter = adapter.hAdapter;

            LOG_INFO("Found adapter with handle: {:X}", (size_t)adapter.hAdapter);
        }
        else 
        {
            LOG_ERROR("No adapters found.");
            break;
        }

        auto hr = o_D3DKMTQueryAdapterInfo(&queryAdapterInfo);

        if (hr != 0)
        {
            LOG_ERROR("Failed to query adapter info {:X}", hr);
            break;
        }

        result = std::filesystem::path(umdFileInfo.UmdFileName);

    } while (false);

    FreeLibrary(hGdi32);
    return result;
}

/* Potato_of_Doom's Implementation */

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
            auto dllPath = GetDriverStore();

            // TODO: Add ini option for it
            auto fsr4Module = LoadLibrary(L"amdxcffx64.dll");

            if (fsr4Module == nullptr)
            {
                LOG_ERROR("Failed to load amdxcffx64.dll");
                return E_NOINTERFACE;
            }

            pfnUpdateFfxApiProvider = (PFN_UpdateFfxApiProvider)GetProcAddress(fsr4Module, "UpdateFfxApiProvider");

            if (pfnUpdateFfxApiProvider == nullptr)
            {
                LOG_ERROR("Failed to get UpdateFfxApiProvider");
                return E_NOINTERFACE;
            }
        }

        if (pfnUpdateFfxApiProvider != nullptr)
            return pfnUpdateFfxApiProvider(pData, dataSizeInBytes);

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
