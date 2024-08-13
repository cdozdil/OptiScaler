#pragma once
#include "../../pch.h"

#include <dxgi1_6.h>

class WrappedIDXGISwapChain4 : public IDXGISwapChain4
{
	IDXGISwapChain* m_pReal = nullptr;
	IDXGISwapChain1* m_pReal1 = nullptr;
	IDXGISwapChain2* m_pReal2 = nullptr;
	IDXGISwapChain3* m_pReal3 = nullptr;
	IDXGISwapChain4* m_pReal4 = nullptr;

	std::function<void(IDXGISwapChain*)> PresentCallback = nullptr;
	std::function<void(bool)> ClearCallback = nullptr;
	std::function<void(HWND)> ReleaseCallback = nullptr;

	unsigned int m_iRefcount;

public:
	WrappedIDXGISwapChain4(IDXGISwapChain* InRealSwapChain,
						   std::function<void(IDXGISwapChain*)> InPresentCallback,
						   std::function<void(bool)> InClearCallback,
						   std::function<void(HWND)> InReleaseCallback);

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

		if (ret == 0)
		{
			if (m_pReal1 != nullptr)
			{
				HWND hwnd = nullptr;
				m_pReal1->GetHwnd(&hwnd);
				ReleaseCallback(hwnd);
			}

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
};
