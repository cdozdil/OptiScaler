#include "wrapped_swapchain.h"

#include "../Config.h"
#include "../Util.h"

// Used RenderDoc's wrapped object as referance
// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

static int scCount = 0;

WrappedIDXGISwapChain4::WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig)
    : m_pReal(real), Device(pDevice), Handle(hWnd), RenderTrig(renderTrig), ClearTrig(clearTrig), m_iRefcount(1)
{
    id = ++scCount;
    LOG_INFO("{0} created, real: {1:X}", id, (UINT64)real);
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal1));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal2));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal3));
    m_pReal->QueryInterface(IID_PPV_ARGS(&m_pReal4));
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

HRESULT WrappedIDXGISwapChain4::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    LOG_FUNC();

    HRESULT result;
    DXGI_SWAP_CHAIN_DESC desc{};
    GetDesc(&desc);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    if (Config::Instance()->CurrentFeature != nullptr)
        Config::Instance()->FGChanged = true;

    // recreate buffers only when needed
    if (desc.BufferDesc.Width != Width || desc.BufferDesc.Height != Height || desc.BufferDesc.Format != NewFormat)
    {
        if (ClearTrig != nullptr)
            ClearTrig(true, Handle);

        Config::Instance()->SCChanged = true;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    LOG_DEBUG("BufferCount: {0}, Width: {1}, Height: {2}, NewFormat: {3}, SwapChainFlags: {4:X}", BufferCount, Width, Height, (UINT)NewFormat, SwapChainFlags);

    result = m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (result == S_OK && Util::GetProcessWindow() == Handle)
    {
        Config::Instance()->ScreenWidth = Width;
        Config::Instance()->ScreenHeight = Height;
    }

    LOG_FUNC_RESULT(result);
    return result;
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetContainingOutput(IDXGIOutput** ppOutput)
{
    return m_pReal->GetContainingOutput(ppOutput);
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
                                               const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    LOG_FUNC();
    DXGI_SWAP_CHAIN_DESC desc{};
    GetDesc(&desc);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    if (Config::Instance()->CurrentFeature != nullptr)
        Config::Instance()->FGChanged = true;

    // recreate buffers only when needed
    if (desc.BufferDesc.Width != Width || desc.BufferDesc.Height != Height || desc.BufferDesc.Format != Format)
    {
        if (ClearTrig != nullptr)
            ClearTrig(true, Handle);

        Config::Instance()->SCChanged = true;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

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

HRESULT WrappedIDXGISwapChain4::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
    auto result = m_pReal->SetFullscreenState(Fullscreen, pTarget);

    LOG_DEBUG("Fullscreen: {}", Fullscreen);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    if (Config::Instance()->CurrentFeature != nullptr)
        Config::Instance()->FGChanged = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    LOG_DEBUG("result: {0:X}", (UINT)result);

    return result;
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

    HRESULT result;

    if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        result = RenderTrig(m_pReal, SyncInterval, Flags, nullptr, Device, Handle);
    else
        result = m_pReal->Present(SyncInterval, Flags);

    return result;
}

HRESULT WrappedIDXGISwapChain4::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
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

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
    return m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
}