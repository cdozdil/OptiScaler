#include "wrapped_swapchain.h"

// Used RenderDoc's wrapped object as referance
// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

static int scCount = 0;

WrappedIDXGISwapChain4::WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_Prensent renderTrig, PFN_Clean clearTrig) : m_pReal(real), Device(pDevice), Handle(hWnd), RenderTrig(renderTrig), ClearTrig(clearTrig), m_iRefcount(1)
{
	id = ++scCount;
	LOG_INFO("{0} created, real: {1:X}", id, (UINT64)real);
	m_pReal->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&m_pReal1);
	m_pReal->QueryInterface(__uuidof(IDXGISwapChain2), (void**)&m_pReal2);
	m_pReal->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_pReal3);
	m_pReal->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_pReal4);
}

WrappedIDXGISwapChain4::~WrappedIDXGISwapChain4()
{
//	if (m_pReal4 != nullptr)
//	{
//		m_pReal4->Release();
//		m_pReal4 = nullptr;
//	}
//
//	if (m_pReal3 != nullptr)
//	{
//		m_pReal3->Release();
//		m_pReal3 = nullptr;
//	}
//
//	if (m_pReal2 != nullptr)
//	{
//		m_pReal2->Release();
//		m_pReal2 = nullptr;
//	}
//
//	if (m_pReal1 != nullptr)
//	{
//		m_pReal1->Release();
//		m_pReal1 = nullptr;
//	}
//
//	if (m_pReal != nullptr)
//	{
//		m_pReal->Release();
//		m_pReal = nullptr;
//	}
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == __uuidof(IDXGISwapChain))
	{
		AddRef();
		*ppvObject = (IDXGISwapChain*)this;
		return S_OK;
	}
	else if (riid == __uuidof(IDXGISwapChain1))
	{
		if (m_pReal1)
		{
			AddRef();
			*ppvObject = (IDXGISwapChain1*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGISwapChain2))
	{
		if (m_pReal2)
		{
			AddRef();
			*ppvObject = (IDXGISwapChain2*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGISwapChain3))
	{
		if (m_pReal3)
		{
			AddRef();
			*ppvObject = (IDXGISwapChain3*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGISwapChain4))
	{
		if (m_pReal4)
		{
			AddRef();
			*ppvObject = (IDXGISwapChain4*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}

	return E_NOINTERFACE;
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	LOG_FUNC();

	if (ClearTrig != nullptr)
		ClearTrig(false, Handle);

	return m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetContainingOutput(IDXGIOutput** ppOutput)
{
	return m_pReal->GetContainingOutput(ppOutput);
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
	_In_reads_(BufferCount) const UINT* pCreationNodeMask, _In_reads_(BufferCount) IUnknown* const* ppPresentQueue)
{
	LOG_FUNC();

	if (ClearTrig != nullptr)
		ClearTrig(false, Handle);

	return m_pReal3->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

HRESULT WrappedIDXGISwapChain4::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
	return m_pReal->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT WrappedIDXGISwapChain4::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
{
	return m_pReal->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT WrappedIDXGISwapChain4::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
{
	return m_pReal->GetBuffer(Buffer, riid, ppSurface);
}

HRESULT WrappedIDXGISwapChain4::GetDevice(REFIID riid, void** ppDevice)
{
	return m_pReal->GetDevice(riid, ppDevice);
}

HRESULT WrappedIDXGISwapChain4::Present(UINT SyncInterval, UINT Flags)
{
	if (m_pReal == nullptr)
		return DXGI_ERROR_DEVICE_REMOVED;

	if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
		RenderTrig(m_pReal, Device, Handle);

	return m_pReal->Present(SyncInterval, Flags);
}

HRESULT WrappedIDXGISwapChain4::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if (m_pReal1 == nullptr)
		return DXGI_ERROR_DEVICE_REMOVED;

	if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
		RenderTrig(m_pReal1, Device, Handle);

	return m_pReal1->Present1(SyncInterval, Flags, pPresentParameters);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
	return m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
}