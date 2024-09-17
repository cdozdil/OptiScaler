//#pragma once
#include "../pch.h"
#include "dxgi1_6.h"
#include "imgui_overlay_base.h"

typedef HRESULT(*PFN_SC_Present)(IDXGISwapChain*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*, IUnknown*, HWND);
typedef void(*PFN_SC_Clean)(bool, HWND);

struct DECLSPEC_UUID("3af622a3-82d0-49cd-994f-cce05122c222") WrappedIDXGISwapChain4 : public IDXGISwapChain4
{
    WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig);

    virtual ~WrappedIDXGISwapChain4();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

    ULONG STDMETHODCALLTYPE AddRef()
    {
        InterlockedIncrement(&m_iRefcount);
        return m_iRefcount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        auto ret = InterlockedDecrement(&m_iRefcount);

        ULONG relCount = 0;

        if (ret == 0)
        {
            LOG_INFO("{} released", id);

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
                                                     _In_reads_(BufferCount) const UINT* pCreationNodeMask, _In_reads_(BufferCount) IUnknown* const* ppPresentQueue);

    //////////////////////////////
    // implement IDXGISwapChain4

    virtual HRESULT STDMETHODCALLTYPE SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, _In_reads_opt_(Size) void* pMetaData)
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
    HWND Handle = nullptr;

    int id = 0;
};

/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

//#pragma once
//#include <dxgi1_5.h>
//#include <shared_mutex>
//
//struct DECLSPEC_UUID("1F445F9F-9887-4C4C-9055-4E3BADAFCCA8") DXGISwapChain final : IDXGISwapChain4
//{
//	DXGISwapChain(ID3D11Device* device, HWND handle, IDXGISwapChain* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback);
//	DXGISwapChain(ID3D11Device* device, HWND handle, IDXGISwapChain1* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback);
//	DXGISwapChain(ID3D12CommandQueue* command_queue, HWND handle, IDXGISwapChain3* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback);
//	~DXGISwapChain();
//
//	DXGISwapChain(const DXGISwapChain&) = delete;
//	DXGISwapChain& operator=(const DXGISwapChain&) = delete;
//
//	#pragma region IUnknown
//	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
//	ULONG   STDMETHODCALLTYPE AddRef() override;
//	ULONG   STDMETHODCALLTYPE Release() override;
//	#pragma endregion
//	#pragma region IDXGIObject
//	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override;
//	HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override;
//	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override;
//	HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void** ppParent) override;
//	#pragma endregion
//	#pragma region IDXGIDeviceSubObject
//	HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;
//	#pragma endregion
//	#pragma region IDXGISwapChain
//	HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval, UINT Flags) override;
//	HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) override;
//	HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) override;
//	HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) override;
//	HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) override;
//	HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override;
//	HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) override;
//	HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput** ppOutput) override;
//	HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) override;
//	HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) override;
//	#pragma endregion
//	#pragma region IDXGISwapChain1
//	HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) override;
//	HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) override;
//	HRESULT STDMETHODCALLTYPE GetHwnd(HWND* pHwnd) override;
//	HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID refiid, void** ppUnk) override;
//	HRESULT STDMETHODCALLTYPE Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters) override;
//	BOOL    STDMETHODCALLTYPE IsTemporaryMonoSupported() override;
//	HRESULT STDMETHODCALLTYPE GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput) override;
//	HRESULT STDMETHODCALLTYPE SetBackgroundColor(const DXGI_RGBA* pColor) override;
//	HRESULT STDMETHODCALLTYPE GetBackgroundColor(DXGI_RGBA* pColor) override;
//	HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION Rotation) override;
//	HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION* pRotation) override;
//	#pragma endregion
//	#pragma region IDXGISwapChain2
//	HRESULT STDMETHODCALLTYPE SetSourceSize(UINT Width, UINT Height) override;
//	HRESULT STDMETHODCALLTYPE GetSourceSize(UINT* pWidth, UINT* pHeight) override;
//	HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(UINT MaxLatency) override;
//	HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(UINT* pMaxLatency) override;
//	HANDLE  STDMETHODCALLTYPE GetFrameLatencyWaitableObject() override;
//	HRESULT STDMETHODCALLTYPE SetMatrixTransform(const DXGI_MATRIX_3X2_F* pMatrix) override;
//	HRESULT STDMETHODCALLTYPE GetMatrixTransform(DXGI_MATRIX_3X2_F* pMatrix) override;
//	#pragma endregion
//	#pragma region IDXGISwapChain3
//	UINT    STDMETHODCALLTYPE GetCurrentBackBufferIndex() override;
//	HRESULT STDMETHODCALLTYPE CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE ColorSpace, UINT* pColorSpaceSupport) override;
//	HRESULT STDMETHODCALLTYPE SetColorSpace1(DXGI_COLOR_SPACE_TYPE ColorSpace) override;
//	HRESULT STDMETHODCALLTYPE ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue) override;
//	#pragma endregion
//	#pragma region IDXGISwapChain4
//	HRESULT STDMETHODCALLTYPE SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void* pMetaData) override;
//	#pragma endregion
//
//	//void on_init();
//	//void on_reset();
//	//void on_present(UINT flags, [[maybe_unused]] const DXGI_PRESENT_PARAMETERS* params = nullptr);
//	//void handle_device_loss(HRESULT hr);
//
//	bool check_and_upgrade_interface(REFIID riid);
//
//	IDXGISwapChain* _orig;
//	LONG _ref = 1;
//	unsigned short _interface_version;
//
//	IUnknown* _direct3d_device = nullptr;
//	// The GOG Galaxy overlay scans the swap chain object memory for the D3D12 command queue, but fails if it cannot find at least two occurences of it.
//	// In that case it falls back to using the first (normal priority) direct command queue that 'ID3D12CommandQueue::ExecuteCommandLists' is called on,
//	// but if this is not the queue the swap chain was created with (DLSS Frame Generation e.g. creates a separate high priority one for presentation), D3D12 removes the device.
//	// Instead spoof a more similar layout to the original 'CDXGISwapChain' implementation, so that the GOG Galaxy overlay successfully extracts and
//	// later uses these command queue pointer offsets directly (the second of which is indexed with the back buffer index), ensuring the correct queue is used.
//	IUnknown* const _direct3d_command_queue,* _direct3d_command_queue_per_back_buffer[DXGI_MAX_SWAP_CHAIN_BUFFERS] = {};
//	const unsigned int _direct3d_version;
//
//	std::shared_mutex _impl_mutex;
//
//	bool _is_initialized = false;
//	bool _was_still_drawing_last_frame = false;
//	BOOL _current_fullscreen_state = -1;
//	HWND _handle = nullptr;
//	PFN_SC_Clean _cleanCallback = nullptr;
//	PFN_SC_Present _presentCallback = nullptr;
//};