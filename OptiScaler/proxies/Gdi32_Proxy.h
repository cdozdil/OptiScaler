#pragma once

#include "pch.h"

#include "Config.h"

#include "detours/detours.h"

typedef struct _D3DKMT_WDDM_2_7_CAPS
{
    union
    {
        struct
        {
            UINT HwSchSupported : 1;
            UINT HwSchEnabled : 1;
            UINT HwSchEnabledByDefault : 1;
            UINT IndependentVidPnVSyncControl : 1;
            UINT Reserved : 28;
        };
        UINT Value;
    };
} D3DKMT_WDDM_2_7_CAPS;

typedef enum _KMTQUERYADAPTERINFOTYPE
{
    KMTQAITYPE_WDDM_2_7_CAPS = 70,
} KMTQUERYADAPTERINFOTYPE;

typedef struct _D3DKMT_QUERYADAPTERINFO
{
    UINT hAdapter;
    KMTQUERYADAPTERINFOTYPE Type;
    VOID* pPrivateDriverData;
    UINT PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;

using D3DKMT_HANDLE = uint64_t;

typedef struct _D3DKMT_ADAPTERINFO {
    D3DKMT_HANDLE hAdapter;
    LUID AdapterLuid;
    ULONG NumOfSources;
    BOOL bPrecisePresentRegionsPreferred;
} D3DKMT_ADAPTERINFO;

typedef struct _D3DKMT_ENUMADAPTERS2 {
    ULONG NumAdapters;
    D3DKMT_ADAPTERINFO* pAdapters;
} D3DKMT_ENUMADAPTERS2;

typedef int(*PFN_D3DKMTQueryAdapterInfo)(const D3DKMT_QUERYADAPTERINFO* data);

static PFN_D3DKMTQueryAdapterInfo o_D3DKMTQueryAdapterInfo = nullptr;

static int hkD3DKMTQueryAdapterInfo(const D3DKMT_QUERYADAPTERINFO* data) {
    //LOG_INFO("Adapter into type: {}", (uint32_t)data->Type);
    if (data->Type == KMTQAITYPE_WDDM_2_7_CAPS) {
        LOG_INFO("Spoofing HAGS");

        if (data->pPrivateDriverData == nullptr) {
            LOG_ERROR("HAGS data nullptr");
            return 0xFFFFFFFF;
        }

        auto d3dkmt_wddm_2_7_caps = static_cast<D3DKMT_WDDM_2_7_CAPS*>(data->pPrivateDriverData);
        d3dkmt_wddm_2_7_caps->HwSchSupported = 1;
        d3dkmt_wddm_2_7_caps->HwSchEnabled = 1;
        d3dkmt_wddm_2_7_caps->HwSchEnabledByDefault = 0;
        d3dkmt_wddm_2_7_caps->IndependentVidPnVSyncControl = 0;

        return 0;
    }
    else
    {
        return o_D3DKMTQueryAdapterInfo(data);
    }
}

// Only for Linux, Wine doesn't have this function
static int customD3DKMTEnumAdapters2(const D3DKMT_ENUMADAPTERS2* data) {
    LOG_FUNC();

    // Try to detect streamline
    if (data->pAdapters != nullptr && data->NumAdapters == 8) {
        LOG_INFO("Streamline detected");

        static D3DKMT_ADAPTERINFO* adapters = []() -> D3DKMT_ADAPTERINFO* {
            static D3DKMT_ADAPTERINFO pAdapters[8]{};
            HRESULT hr = S_OK;

            IDXGIFactory* pFactory = nullptr;
            hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
            if (FAILED(hr)) {
                LOG_DEBUG("Couldn't create a dxgi factory");
                return pAdapters;
            }

            IDXGIAdapter* pAdapter = nullptr;
            for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                DXGI_ADAPTER_DESC desc;
                pAdapter->GetDesc(&desc);
                
                pAdapters[i].AdapterLuid.HighPart = desc.AdapterLuid.HighPart;
                pAdapters[i].AdapterLuid.LowPart = desc.AdapterLuid.LowPart;
                
                pAdapter->Release();
            }

            pFactory->Release();

            return pAdapters;
        }();

        // Something's not quite right in here
        // Can't just send all LUIDs from DXGI as streamline isn't happy
        // Can't send a true number of adapters as streamline isn't happy
        // 8th adapter sometimes has junk data so sending only 6 to be safe

        for (uint32_t i = 0; i < 6; i++) {
            data->pAdapters[i].AdapterLuid.HighPart = adapters[0].AdapterLuid.HighPart;
            data->pAdapters[i].AdapterLuid.LowPart  = adapters[0].AdapterLuid.LowPart;
            LOG_DEBUG("sent {}.{}", data->pAdapters[i].AdapterLuid.HighPart, data->pAdapters[i].AdapterLuid.LowPart);
        }

        return 0;
    }

    return 0xFFFFFFFF;
}

// for spoofing HAGS, call early
static void hookGdi32() {
    LOG_FUNC();

    if (Config::Instance()->SpoofHAGS.value_or_default() || Config::Instance()->FGType.value_or_default() == FGType::Nukems) {
        o_D3DKMTQueryAdapterInfo = reinterpret_cast<PFN_D3DKMTQueryAdapterInfo>(DetourFindFunction("gdi32.dll", "D3DKMTQueryAdapterInfo"));

        if (o_D3DKMTQueryAdapterInfo != nullptr) {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)o_D3DKMTQueryAdapterInfo, hkD3DKMTQueryAdapterInfo);

            DetourTransactionCommit();
        }
    }
}

static void unhookGdi32() {
    if (o_D3DKMTQueryAdapterInfo != nullptr) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&(PVOID&)o_D3DKMTQueryAdapterInfo, hkD3DKMTQueryAdapterInfo);
        o_D3DKMTQueryAdapterInfo = nullptr;

        DetourTransactionCommit();
    }
}
