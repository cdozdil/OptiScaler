//#pragma once
#include "../pch.h"
#include "dxgi1_6.h"

typedef HRESULT(*PFN_SC_Present)(IDXGISwapChain*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*, IUnknown*, HWND);
typedef void(*PFN_SC_Clean)(bool, HWND);
typedef void(*PFN_SC_Release)(HWND);

struct DECLSPEC_UUID("3af622a3-82d0-49cd-994f-cce05122c222") WrappedIDXGISwapChain4 : public IDXGISwapChain4
{
    WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig, PFN_SC_Release releaseTrig);

    virtual ~WrappedIDXGISwapChain4();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

    ULONG STDMETHODCALLTYPE AddRef()
    {
        InterlockedIncrement(&m_iRefcount);
        LOG_TRACE("Count: {}", m_iRefcount);
        return m_iRefcount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        auto ret = InterlockedDecrement(&m_iRefcount);
        LOG_TRACE("Count: {}", ret);

        ULONG relCount = 0;

        if (ret == 0)
        {
            if (ClearTrig != nullptr)
                ClearTrig(true, Handle);

            if (m_pReal4 != nullptr)
                relCount = m_pReal4->Release();

            if (m_pReal3 != nullptr)
                relCount = m_pReal3->Release();

            if (m_pReal2 != nullptr)
                relCount = m_pReal2->Release();

            if (m_pReal1 != nullptr)
                relCount = m_pReal1->Release();

            if (m_pReal != nullptr)
                relCount = m_pReal->Release();

            if (ReleaseTrig != nullptr)
                ReleaseTrig(Handle);

            LOG_INFO("{} released", id);

            delete this;
        }

        return ret;
    }

    //////////////////////////////
    // implement IDXGIObject

    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID Name, UINT DataSize, const void* pData)
    {
        return m_pReal->SetPrivateData(Name, DataSize, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown)
    {
        return m_pReal->SetPrivateDataInterface(Name, pUnknown);
    }

    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData)
    {
        return m_pReal->GetPrivateData(Name, pDataSize, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void** ppParent)
    {
        return m_pReal->GetParent(riid, ppParent);
    }

    //////////////////////////////
    // implement IDXGIDeviceSubObject

    virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice);

    //////////////////////////////
    // implement IDXGISwapChain

    virtual HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval, UINT Flags);

    virtual HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void** ppSurface);

    virtual HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget);

    virtual HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget);

    virtual HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc)
    {
        return m_pReal->GetDesc(pDesc);
    }

    virtual HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

    virtual HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters)
    {
        return m_pReal->ResizeTarget(pNewTargetParameters);
    }

    virtual HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput** ppOutput);

    virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
    {
        return m_pReal->GetFrameStatistics(pStats);
    }

    virtual HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount)
    {
        return m_pReal->GetLastPresentCount(pLastPresentCount);
    }

    //////////////////////////////
    // implement IDXGISwapChain1

    virtual HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc)
    {
        return m_pReal1->GetDesc1(pDesc);
    }

    virtual HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc)
    {
        return m_pReal1->GetFullscreenDesc(pDesc);
    }

    virtual HRESULT STDMETHODCALLTYPE GetHwnd(HWND* pHwnd)
    {
        return m_pReal1->GetHwnd(pHwnd);
    }

    virtual HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID refiid, void** ppUnk)
    {
        return m_pReal1->GetCoreWindow(refiid, ppUnk);
    }

    virtual HRESULT STDMETHODCALLTYPE Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);

    virtual BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported(void)
    {
        return m_pReal1->IsTemporaryMonoSupported();
    }

    virtual HRESULT STDMETHODCALLTYPE GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput);

    virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(const DXGI_RGBA* pColor)
    {
        return m_pReal1->SetBackgroundColor(pColor);
    }

    virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(DXGI_RGBA* pColor)
    {
        return m_pReal1->GetBackgroundColor(pColor);
    }

    virtual HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION Rotation)
    {
        return m_pReal1->SetRotation(Rotation);
    }

    virtual HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION* pRotation)
    {
        return m_pReal1->GetRotation(pRotation);
    }

    //////////////////////////////
    // implement IDXGISwapChain2

    virtual HRESULT STDMETHODCALLTYPE SetSourceSize(UINT Width, UINT Height)
    {
        return m_pReal2->SetSourceSize(Width, Height);
    }

    virtual HRESULT STDMETHODCALLTYPE GetSourceSize(UINT* pWidth, UINT* pHeight)
    {
        return m_pReal2->GetSourceSize(pWidth, pHeight);
    }

    virtual HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(UINT MaxLatency)
    {
        return m_pReal2->SetMaximumFrameLatency(MaxLatency);
    }

    virtual HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(UINT* pMaxLatency)
    {
        return m_pReal2->GetMaximumFrameLatency(pMaxLatency);
    }

    virtual HANDLE STDMETHODCALLTYPE GetFrameLatencyWaitableObject(void)
    {
        return m_pReal2->GetFrameLatencyWaitableObject();
    }

    virtual HRESULT STDMETHODCALLTYPE SetMatrixTransform(const DXGI_MATRIX_3X2_F* pMatrix)
    {
        return m_pReal2->SetMatrixTransform(pMatrix);
    }

    virtual HRESULT STDMETHODCALLTYPE GetMatrixTransform(DXGI_MATRIX_3X2_F* pMatrix)
    {
        return m_pReal2->GetMatrixTransform(pMatrix);
    }

    //////////////////////////////
    // implement IDXGISwapChain3

    virtual UINT STDMETHODCALLTYPE GetCurrentBackBufferIndex(void)
    {
        return m_pReal3->GetCurrentBackBufferIndex();
    }

    virtual HRESULT STDMETHODCALLTYPE CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE ColorSpace, UINT* pColorSpaceSupport)
    {
        return m_pReal3->CheckColorSpaceSupport(ColorSpace, pColorSpaceSupport);
    }

    virtual HRESULT STDMETHODCALLTYPE SetColorSpace1(DXGI_COLOR_SPACE_TYPE ColorSpace)
    {
        return m_pReal3->SetColorSpace1(ColorSpace);
    }

    virtual HRESULT STDMETHODCALLTYPE ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
                                                     const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue);

    //////////////////////////////
    // implement IDXGISwapChain4

    virtual HRESULT STDMETHODCALLTYPE SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void* pMetaData)
    {
        return m_pReal4->SetHDRMetaData(Type, Size, pMetaData);
    }


    IDXGISwapChain* m_pReal = nullptr;
    LONG m_iRefcount;

    IUnknown* Device = nullptr;

    IDXGISwapChain1* m_pReal1 = nullptr;
    IDXGISwapChain2* m_pReal2 = nullptr;
    IDXGISwapChain3* m_pReal3 = nullptr;
    IDXGISwapChain4* m_pReal4 = nullptr;

    PFN_SC_Present RenderTrig = nullptr;
    PFN_SC_Clean ClearTrig = nullptr;
    PFN_SC_Release ReleaseTrig = nullptr;
    HWND Handle = nullptr;

    int id = 0;
};
