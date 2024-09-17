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

/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */


 // Needs to be set whenever a DXGI call can end up in 'CDXGISwapChain::EnsureChildDeviceInternal', to avoid hooking internal D3D device creation
//thread_local bool g_in_dxgi_runtime = false;
//
//DXGISwapChain::DXGISwapChain(ID3D11Device* device, HWND handle, IDXGISwapChain* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback) :
//    _orig(original), _interface_version(0), _direct3d_device((IUnknown*)device), _direct3d_command_queue(nullptr), _direct3d_version(11),
//    _handle(handle), _cleanCallback(cleanCallback), _presentCallback(presentCallback)
//{
//    assert(_orig != nullptr && _direct3d_device != nullptr);
//    _direct3d_device->AddRef();
//}
//
//DXGISwapChain::DXGISwapChain(ID3D11Device* device, HWND handle, IDXGISwapChain1* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback) :
//    _orig(original), _interface_version(1), _direct3d_device((IUnknown*)device), _direct3d_command_queue(nullptr), _direct3d_version(11),
//    _handle(handle), _cleanCallback(cleanCallback), _presentCallback(presentCallback)
//{
//    assert(_orig != nullptr && _direct3d_device != nullptr);
//    _direct3d_device->AddRef();
//}
//
//DXGISwapChain::DXGISwapChain(ID3D12CommandQueue* command_queue, HWND handle, IDXGISwapChain3* original, PFN_SC_Present presentCallback, PFN_SC_Clean cleanCallback) :
//    _orig(original), _interface_version(3), _direct3d_command_queue(command_queue), _direct3d_version(12),
//    _handle(handle), _cleanCallback(cleanCallback), _presentCallback(presentCallback)
//
//{
//    // Add reference to command queue as well to ensure it is kept alive for the lifetime of the effect runtime
//    command_queue->GetDevice(__uuidof(ID3D12Device), (void**)&_direct3d_device);
//
//    for (size_t i = 0; i < std::size(_direct3d_command_queue_per_back_buffer); ++i)
//        _direct3d_command_queue_per_back_buffer[i] = _direct3d_command_queue;
//}
//
//DXGISwapChain::~DXGISwapChain()
//{
//    // Release the explicit reference to the device that was added in the 'DXGISwapChain' constructor above now that 
//    // the effect runtime was destroyed and is longer referencing it
//    if (_direct3d_command_queue != nullptr)
//        _direct3d_command_queue->Release();
//
//    _direct3d_device->Release();
//}
//
//bool DXGISwapChain::check_and_upgrade_interface(REFIID riid)
//{
//    if (riid == __uuidof(this) ||
//        riid == __uuidof(IUnknown) ||
//        riid == __uuidof(IDXGIObject) ||
//        riid == __uuidof(IDXGIDeviceSubObject))
//        return true;
//
//    static constexpr IID iid_lookup[] = {
//        __uuidof(IDXGISwapChain),
//        __uuidof(IDXGISwapChain1),
//        __uuidof(IDXGISwapChain2),
//        __uuidof(IDXGISwapChain3),
//        __uuidof(IDXGISwapChain4),
//    };
//
//    for (unsigned short version = 0; version < ARRAYSIZE(iid_lookup); ++version)
//    {
//        if (riid != iid_lookup[version])
//            continue;
//
//        if (version > _interface_version)
//        {
//            IUnknown* new_interface = nullptr;
//            if (FAILED(_orig->QueryInterface(riid, reinterpret_cast<void**>(&new_interface))))
//                return false;
//
//            _orig->Release();
//            _orig = static_cast<IDXGISwapChain*>(new_interface);
//            _interface_version = version;
//        }
//
//        return true;
//    }
//
//    return false;
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::QueryInterface(REFIID riid, void** ppvObj)
//{
//    if (ppvObj == nullptr)
//        return E_POINTER;
//
//    if (check_and_upgrade_interface(riid))
//    {
//        AddRef();
//        *ppvObj = this;
//        return S_OK;
//    }
//
//    return _orig->QueryInterface(riid, ppvObj);
//}
//
//ULONG   STDMETHODCALLTYPE DXGISwapChain::AddRef()
//{
//    _orig->AddRef();
//    return InterlockedIncrement(&_ref);
//}
//
//ULONG   STDMETHODCALLTYPE DXGISwapChain::Release()
//{
//    const ULONG ref = InterlockedDecrement(&_ref);
//
//    ULONG realRef = 0;
//    realRef = _orig->Release();
//
//    if (ref != 0)
//        return ref;
//
//    if (_cleanCallback != nullptr)
//        _cleanCallback(true, _handle);
//
//    delete this;
//
//    return 0;
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetPrivateData(REFGUID Name, UINT DataSize, const void* pData)
//{
//    return _orig->SetPrivateData(Name, DataSize, pData);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown)
//{
//    return _orig->SetPrivateDataInterface(Name, pUnknown);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData)
//{
//    return _orig->GetPrivateData(Name, pDataSize, pData);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetParent(REFIID riid, void** ppParent)
//{
//    return _orig->GetParent(riid, ppParent);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDevice(REFIID riid, void** ppDevice)
//{
//    return _direct3d_device->QueryInterface(riid, ppDevice);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::Present(UINT SyncInterval, UINT Flags)
//{
//    if (!(Flags & DXGI_PRESENT_TEST) && !(Flags & DXGI_PRESENT_RESTART) && _presentCallback != nullptr)
//    {
//        if (_direct3d_version == 11)
//            _presentCallback(_orig, _direct3d_device, _handle);
//        else
//            _presentCallback(_orig, _direct3d_command_queue, _handle);
//    }
//
//    assert(!g_in_dxgi_runtime);
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->Present(SyncInterval, Flags);
//    g_in_dxgi_runtime = false;
//
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
//{
//    return _orig->GetBuffer(Buffer, riid, ppSurface);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
//{
//    _current_fullscreen_state = -1;
//
//    const bool was_in_dxgi_runtime = g_in_dxgi_runtime;
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->SetFullscreenState(Fullscreen, pTarget);
//    g_in_dxgi_runtime = was_in_dxgi_runtime;
//    return hr;
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
//{
//    if (_current_fullscreen_state >= 0)
//    {
//        if (pFullscreen != nullptr)
//            *pFullscreen = _current_fullscreen_state;
//        if (ppTarget != nullptr)
//            _orig->GetContainingOutput(ppTarget);
//        return S_OK;
//    }
//
//    const bool was_in_dxgi_runtime = g_in_dxgi_runtime;
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->GetFullscreenState(pFullscreen, ppTarget);
//    g_in_dxgi_runtime = was_in_dxgi_runtime;
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc)
//{
//    const bool was_in_dxgi_runtime = g_in_dxgi_runtime;
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->GetDesc(pDesc);
//    g_in_dxgi_runtime = was_in_dxgi_runtime;
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
//{
//    assert(!g_in_dxgi_runtime);
//    // Handle update of the swap chain description
//
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
//    g_in_dxgi_runtime = false;
//
//    if (SUCCEEDED(hr) && _cleanCallback != nullptr)
//        _cleanCallback(false, _handle);
//
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters)
//{
//    const bool was_in_dxgi_runtime = g_in_dxgi_runtime;
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = _orig->ResizeTarget(pNewTargetParameters);
//    g_in_dxgi_runtime = was_in_dxgi_runtime;
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetContainingOutput(IDXGIOutput** ppOutput)
//{
//    return _orig->GetContainingOutput(ppOutput);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
//{
//    return _orig->GetFrameStatistics(pStats);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetLastPresentCount(UINT* pLastPresentCount)
//{
//    return _orig->GetLastPresentCount(pLastPresentCount);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc)
//{
//    assert(_interface_version >= 1);
//    const bool was_in_dxgi_runtime = g_in_dxgi_runtime;
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = static_cast<IDXGISwapChain1*>(_orig)->GetDesc1(pDesc);
//    g_in_dxgi_runtime = was_in_dxgi_runtime;
//    return hr;
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetFullscreenDesc(pDesc);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetHwnd(HWND* pHwnd)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetHwnd(pHwnd);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetCoreWindow(REFIID refiid, void** ppUnk)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetCoreWindow(refiid, ppUnk);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
//{
//    if (!(PresentFlags & DXGI_PRESENT_TEST) && !(PresentFlags & DXGI_PRESENT_RESTART) && _presentCallback != nullptr)
//    {
//        if (_direct3d_version == 11)
//            _presentCallback(_orig, _direct3d_device, _handle);
//        else
//            _presentCallback(_orig, _direct3d_command_queue, _handle);
//    }
//
//    assert(_interface_version >= 1);
//    assert(!g_in_dxgi_runtime);
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = static_cast<IDXGISwapChain1*>(_orig)->Present1(SyncInterval, PresentFlags, pPresentParameters);
//    g_in_dxgi_runtime = false;
//
//    return hr;
//}
//BOOL    STDMETHODCALLTYPE DXGISwapChain::IsTemporaryMonoSupported()
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->IsTemporaryMonoSupported();
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetRestrictToOutput(ppRestrictToOutput);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetBackgroundColor(const DXGI_RGBA* pColor)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->SetBackgroundColor(pColor);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetBackgroundColor(DXGI_RGBA* pColor)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetBackgroundColor(pColor);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetRotation(DXGI_MODE_ROTATION Rotation)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->SetRotation(Rotation);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetRotation(DXGI_MODE_ROTATION* pRotation)
//{
//    assert(_interface_version >= 1);
//    return static_cast<IDXGISwapChain1*>(_orig)->GetRotation(pRotation);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetSourceSize(UINT Width, UINT Height)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->SetSourceSize(Width, Height);
//}
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetSourceSize(UINT* pWidth, UINT* pHeight)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->GetSourceSize(pWidth, pHeight);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetMaximumFrameLatency(UINT MaxLatency)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->SetMaximumFrameLatency(MaxLatency);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetMaximumFrameLatency(UINT* pMaxLatency)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->GetMaximumFrameLatency(pMaxLatency);
//}
//
//HANDLE  STDMETHODCALLTYPE DXGISwapChain::GetFrameLatencyWaitableObject()
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->GetFrameLatencyWaitableObject();
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetMatrixTransform(const DXGI_MATRIX_3X2_F* pMatrix)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->SetMatrixTransform(pMatrix);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::GetMatrixTransform(DXGI_MATRIX_3X2_F* pMatrix)
//{
//    assert(_interface_version >= 2);
//    return static_cast<IDXGISwapChain2*>(_orig)->GetMatrixTransform(pMatrix);
//}
//
//UINT    STDMETHODCALLTYPE DXGISwapChain::GetCurrentBackBufferIndex()
//{
//    assert(_interface_version >= 3);
//    return static_cast<IDXGISwapChain3*>(_orig)->GetCurrentBackBufferIndex();
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE ColorSpace, UINT* pColorSpaceSupport)
//{
//    assert(_interface_version >= 3);
//    return static_cast<IDXGISwapChain3*>(_orig)->CheckColorSpaceSupport(ColorSpace, pColorSpaceSupport);
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetColorSpace1(DXGI_COLOR_SPACE_TYPE ColorSpace)
//{
//    assert(_interface_version >= 3);
//    assert(!g_in_dxgi_runtime);
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = static_cast<IDXGISwapChain3*>(_orig)->SetColorSpace1(ColorSpace);
//    g_in_dxgi_runtime = false;
//
//    return hr;
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
//{
//    assert(_interface_version >= 3);
//    assert(!g_in_dxgi_runtime);
//
//    g_in_dxgi_runtime = true;
//    const HRESULT hr = static_cast<IDXGISwapChain3*>(_orig)->ResizeBuffers1(BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
//    g_in_dxgi_runtime = false;
//
//    if (SUCCEEDED(hr) && _cleanCallback != nullptr)
//        _cleanCallback(false, _handle);
//
//    return hr;
//}
//
//HRESULT STDMETHODCALLTYPE DXGISwapChain::SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void* pMetaData)
//{
//    return static_cast<IDXGISwapChain4*>(_orig)->SetHDRMetaData(Type, Size, pMetaData);
//}
//
//class unique_direct3d_device_lock : std::unique_lock<std::shared_mutex>
//{
//public:
//    unique_direct3d_device_lock(IUnknown* direct3d_device, unsigned int direct3d_version, std::shared_mutex& mutex) : unique_lock(mutex)
//    {
//        ID3D11Device* d3d11Device = nullptr;
//
//        switch (direct3d_version)
//        {
//            case 11:
//                if (direct3d_device->QueryInterface(IID_PPV_ARGS(&d3d11Device)) == S_OK)
//                {
//                    ID3D11DeviceContext* context;
//                    d3d11Device->GetImmediateContext(&context);
//                    context->QueryInterface(IID_PPV_ARGS(&multithread));
//
//                    context->Release();
//                    d3d11Device->Release();
//                }
//
//                break;
//        }
//
//        if (multithread != nullptr)
//        {
//            was_protected = multithread->GetMultithreadProtected();
//            multithread->SetMultithreadProtected(TRUE);
//            multithread->Enter();
//        }
//    }
//    ~unique_direct3d_device_lock()
//    {
//        if (multithread != nullptr)
//        {
//            multithread->Leave();
//            multithread->SetMultithreadProtected(was_protected);
//            multithread->Release();
//        }
//    }
//
//private:
//    ID3D11Multithread* multithread = nullptr;
//    BOOL was_protected = FALSE;
//};

//void DXGISwapChain::on_init()
//{
//    assert(!_is_initialized);
//
//    const unique_direct3d_device_lock lock(_direct3d_device, _direct3d_version, _direct3d_version == 12 ? static_cast<D3D12CommandQueue*>(_direct3d_command_queue)->_mutex : _impl_mutex);
//
//
//    _is_initialized = true;
//}
//
//void DXGISwapChain::on_reset()
//{
//    if (!_is_initialized)
//        return;
//
//    const unique_direct3d_device_lock lock(_direct3d_device, _direct3d_version, _direct3d_version == 12 ? static_cast<D3D12CommandQueue*>(_direct3d_command_queue)->_mutex : _impl_mutex);
//
//    _is_initialized = false;
//}
//
//void DXGISwapChain::on_present(UINT flags, [[maybe_unused]] const DXGI_PRESENT_PARAMETERS* params)
//{
//    // Some D3D11 games test presentation for timing and composition purposes
//    // These calls are not rendering related, but rather a status request for the D3D runtime and as such should be ignored
//    if ((flags & DXGI_PRESENT_TEST) != 0)
//        return;
//
//    // Also skip when the same frame is presented multiple times
//    if ((flags & DXGI_PRESENT_DO_NOT_WAIT) != 0 && _was_still_drawing_last_frame)
//        return;
//    assert(!_was_still_drawing_last_frame);
//
//    assert(_is_initialized);
//
//    // Synchronize access to effect runtime to avoid race conditions between 'load_effects' and 'destroy_effects' causing crashes
//    // This is necessary because Resident Evil 3 calls D3D11 and DXGI functions simultaneously from multiple threads
//    // In case of D3D12, also synchronize access to the command queue while events are invoked and the immediate command list may be accessed
//    const unique_direct3d_device_lock lock(_direct3d_device, _direct3d_version, _direct3d_version == 12 ? static_cast<D3D12CommandQueue*>(_direct3d_command_queue)->_mutex : _impl_mutex);
//
//
//}
//
//void DXGISwapChain::handle_device_loss(HRESULT hr)
//{
//    _was_still_drawing_last_frame = (hr == DXGI_ERROR_WAS_STILL_DRAWING);
//
//    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
//    {
//        if (hr == DXGI_ERROR_DEVICE_REMOVED)
//        {
//            HRESULT reason = DXGI_ERROR_INVALID_CALL;
//            switch (_direct3d_version)
//            {
//                case 10:
//                    reason = static_cast<ID3D10Device*>(_direct3d_device)->GetDeviceRemovedReason();
//                    break;
//                case 11:
//                    reason = static_cast<ID3D11Device*>(_direct3d_device)->GetDeviceRemovedReason();
//                    break;
//                case 12:
//                    reason = static_cast<ID3D12Device*>(_direct3d_device)->GetDeviceRemovedReason();
//                    break;
//            }
//
//        }
//    }
//}