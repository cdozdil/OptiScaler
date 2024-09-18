#include "wrapped_swapchain.h"
#include <d3d11_4.h>

//// Used RenderDoc's wrapped object as referance
//// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

static int scCount = 0;

WrappedIDXGISwapChain4::WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig)
    : m_pReal(real), Device(pDevice), Handle(hWnd), RenderTrig(renderTrig), ClearTrig(clearTrig), m_iRefcount(1)
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
    else if (riid == __uuidof(this))
    {
        AddRef();
        *ppvObject = (IUnknown*)this;
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

HRESULT WrappedIDXGISwapChain4::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    LOG_FUNC();

    _mutex.lock();

    if (ClearTrig != nullptr)
        ClearTrig(true, Handle);

    auto result = m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);

    LOG_FUNC_RESULT(result);

    _mutex.unlock();

    return result;
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetContainingOutput(IDXGIOutput** ppOutput)
{
    return m_pReal->GetContainingOutput(ppOutput);
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
                                               _In_reads_(BufferCount) const UINT* pCreationNodeMask, _In_reads_(BufferCount) IUnknown* const* ppPresentQueue)
{
    LOG_FUNC();

    _mutex.lock();

    if (ClearTrig != nullptr)
        ClearTrig(true, Handle);


    auto result = m_pReal3->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

    _mutex.unlock();

    return result;
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
        return RenderTrig(m_pReal, SyncInterval, Flags, nullptr, Device, Handle);

    return m_pReal->Present(SyncInterval, Flags);
}

HRESULT WrappedIDXGISwapChain4::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if (m_pReal1 == nullptr)
        return DXGI_ERROR_DEVICE_REMOVED;

    if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        return RenderTrig(m_pReal1, SyncInterval, Flags, pPresentParameters, Device, Handle);

    return m_pReal1->Present1(SyncInterval, Flags, pPresentParameters);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
    return m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
}