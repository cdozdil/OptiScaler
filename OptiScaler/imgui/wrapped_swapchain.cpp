#include "wrapped_swapchain.h"

// Used RenderDoc's wrapped object as referance
// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

WrappedIDXGISwapChain4::WrappedIDXGISwapChain4(IDXGISwapChain* real, 
	std::function<HRESULT(IDXGISwapChain3*, UINT, UINT)> renderTrig,
	std::function<HRESULT(IDXGISwapChain3*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*)> renderTrig1, 
	std::function<void(bool)> clearTrig) : m_pReal(real), RenderTrig(renderTrig), RenderTrig1(renderTrig1), ClearTrig(clearTrig), m_iRefcount(1)
{
	real->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&m_pReal1);
	real->QueryInterface(__uuidof(IDXGISwapChain2), (void**)&m_pReal2);
	real->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_pReal3);
	real->QueryInterface(__uuidof(IDXGISwapChain4), (void**)&m_pReal4);
}

WrappedIDXGISwapChain4::~WrappedIDXGISwapChain4()
{
	if (m_pReal1 != nullptr)
	{
		m_pReal1->Release();
		m_pReal1 = nullptr;
	}

	if (m_pReal2 != nullptr)
	{
		m_pReal2->Release();
		m_pReal2 = nullptr;
	}

	if (m_pReal3 != nullptr)
	{
		m_pReal3->Release();
		m_pReal3 = nullptr;
	}

	if (m_pReal4 != nullptr)
	{
		m_pReal4->Release();
		m_pReal4 = nullptr;
	}

	if (m_pReal != nullptr)
	{
		m_pReal->Release();
		m_pReal = nullptr;
	}
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
	spdlog::debug("WrappedIDXGISwapChain4::ResizeBuffers");

	if (ClearTrig != nullptr)
		ClearTrig(false);

	HRESULT ret = m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);

	return ret;
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetContainingOutput(IDXGIOutput** ppOutput)
{
	HRESULT ret = m_pReal->GetContainingOutput(ppOutput);
	return ret;
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
	_In_reads_(BufferCount) const UINT* pCreationNodeMask, _In_reads_(BufferCount) IUnknown* const* ppPresentQueue)
{
	spdlog::debug("WrappedIDXGISwapChain4::ResizeBuffers1");

	if (ClearTrig != nullptr)
		ClearTrig(false);

	HRESULT ret = m_pReal3->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
	return ret;
}

HRESULT WrappedIDXGISwapChain4::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
	spdlog::debug("WrappedIDXGISwapChain4::SetFullscreenState");

	if (ClearTrig != nullptr)
		ClearTrig(true);

	return m_pReal->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT WrappedIDXGISwapChain4::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
{
	HRESULT ret = m_pReal->GetFullscreenState(pFullscreen, ppTarget);
	return ret;
}

HRESULT WrappedIDXGISwapChain4::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
{
	HRESULT ret = m_pReal->GetBuffer(Buffer, riid, ppSurface);
	return ret;
}

HRESULT WrappedIDXGISwapChain4::GetDevice(REFIID riid, void** ppDevice)
{
	HRESULT ret = m_pReal->GetDevice(riid, ppDevice);
	return ret;
}

HRESULT WrappedIDXGISwapChain4::Present(UINT SyncInterval, UINT Flags)
{
	if (RenderTrig != nullptr && m_pReal3 != nullptr)
		RenderTrig(m_pReal3, SyncInterval, Flags);

	return m_pReal->Present(SyncInterval, Flags);
}

HRESULT WrappedIDXGISwapChain4::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if (RenderTrig1 != nullptr && m_pReal3 != nullptr)
		RenderTrig1(m_pReal3, SyncInterval, Flags, pPresentParameters);

	return m_pReal1->Present1(SyncInterval, Flags, pPresentParameters);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
	HRESULT ret = m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
	return ret;
}