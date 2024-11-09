#include "WrappedSwapChain.h"

#include <Util.h>
#include <Config.h>

// Used RenderDoc's wrapped object as referance
// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

static int scCount = 0;

WrappedSwapChain::WrappedSwapChain(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig)
    : m_pReal(real), Device(pDevice), Handle(hWnd), RenderTrig(renderTrig), ClearTrig(clearTrig), m_iRefcount(1)
{
    id = ++scCount;
    LOG_INFO("{0} created, real: {1:X}", id, (UINT64)real);
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal1));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal2));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal3));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal4));
}

WrappedSwapChain::~WrappedSwapChain()
{

}

HRESULT STDMETHODCALLTYPE WrappedSwapChain::QueryInterface(REFIID riid, void** ppvObject)
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
    else if (riid == __uuidof(this))
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else if (riid == __uuidof(IUnknown))
    {
        AddRef();
        *ppvObject = (IUnknown*)this;
        return S_OK;
    }
    else if (riid == __uuidof(IDXGIObject))
    {
        AddRef();
        *ppvObject = (IDXGIObject*)this;
        return S_OK;
    }
    else if (riid == __uuidof(IDXGIDeviceSubObject))
    {
        AddRef();
        *ppvObject = (IDXGIDeviceSubObject*)this;
        return S_OK;
    }

    return E_NOINTERFACE;
}

HRESULT WrappedSwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    LOG_FUNC();

    if (ClearTrig != nullptr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        ClearTrig(true, Handle);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    LOG_DEBUG("BufferCount: {0}, Width: {1}, Height: {2}, NewFormat: {3}, SwapChainFlags: {4:X}", BufferCount, Width, Height, (UINT)NewFormat, SwapChainFlags);

    auto result = m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (result == S_OK && Util::GetProcessWindow() == Handle)
    {
        Config::Instance()->ScreenWidth = Width;
        Config::Instance()->ScreenHeight = Height;
    }

    LOG_FUNC_RESULT(result);
    return result;
}

HRESULT STDMETHODCALLTYPE WrappedSwapChain::GetContainingOutput(IDXGIOutput** ppOutput)
{
    return m_pReal->GetContainingOutput(ppOutput);
}

HRESULT WrappedSwapChain::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
                                               const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    LOG_FUNC();

    if (ClearTrig != nullptr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        ClearTrig(true, Handle);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    LOG_DEBUG("BufferCount: {0}, Width: {1}, Height: {2}, NewFormat: {3}, SwapChainFlags: {4:X}, pCreationNodeMask: {5}", BufferCount, Width, Height, (UINT)Format, SwapChainFlags, *pCreationNodeMask);

    auto result = m_pReal3->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
    if (result == S_OK && Util::GetProcessWindow() == Handle)
    {
        Config::Instance()->ScreenWidth = Width;
        Config::Instance()->ScreenHeight = Height;
    }

    LOG_FUNC_RESULT(result);
    return result;
}

HRESULT WrappedSwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
    return m_pReal->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT WrappedSwapChain::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
{
    return m_pReal->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT WrappedSwapChain::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
{
    return m_pReal->GetBuffer(Buffer, riid, ppSurface);
}

HRESULT WrappedSwapChain::GetDevice(REFIID riid, void** ppDevice)
{
    return m_pReal->GetDevice(riid, ppDevice);
}

HRESULT WrappedSwapChain::Present(UINT SyncInterval, UINT Flags)
{
    if (m_pReal == nullptr)
        return DXGI_ERROR_DEVICE_REMOVED;

    HRESULT result;

    if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        result = RenderTrig(m_pReal, SyncInterval, Flags, nullptr, Device, Handle);
    else
        result = m_pReal->Present(SyncInterval, Flags);

    return result;
}

HRESULT WrappedSwapChain::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if (m_pReal1 == nullptr)
        return DXGI_ERROR_DEVICE_REMOVED;

    HRESULT result;

    if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        result = RenderTrig(m_pReal1, SyncInterval, Flags, pPresentParameters, Device, Handle);
    else
        result = m_pReal1->Present1(SyncInterval, Flags, pPresentParameters);

    return result;
}

HRESULT STDMETHODCALLTYPE WrappedSwapChain::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
    return m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
}