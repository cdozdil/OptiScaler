#pragma once

#include <pch.h>
#include <Unknwn.h>
#include <Windows.h>

//#include <d3dkmthk.h>

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

// Function pointers
typedef UINT(*PFN_D3DKMTQueryAdapterInfo_L)(D3DKMT_QUERYADAPTERINFO_L*);
typedef UINT(*PFN_D3DKMTEnumAdapters_L)(D3DKMT_ENUMADAPTERS_L*);

inline static std::vector<std::filesystem::path> GetDriverStore()
{
    std::vector<std::filesystem::path> result;

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
        auto o_D3DKMTQueryAdapterInfo = (PFN_D3DKMTQueryAdapterInfo_L)GetProcAddress(hGdi32, "D3DKMTQueryAdapterInfo");

        if (o_D3DKMTEnumAdapters == nullptr || o_D3DKMTQueryAdapterInfo == nullptr)
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
            }
        }
        else
        {
            LOG_ERROR("No adapters found.");
            break;
        }

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
            auto storePath = GetDriverStore();
            HMODULE fsr4Module = nullptr;

            for (size_t i = 0; i < storePath.size(); i++)
            {
                if (fsr4Module == nullptr)
                {
                    auto dllPath = storePath[i] / L"amdxcffx64.dll";
                    LOG_DEBUG("Trying to load: {}", wstring_to_string(dllPath.c_str()));
                    fsr4Module = LoadLibrary(dllPath.c_str());
                }
            }

            if (fsr4Module == nullptr)
                fsr4Module = LoadLibrary(L"amdxcffx64.dll");

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
