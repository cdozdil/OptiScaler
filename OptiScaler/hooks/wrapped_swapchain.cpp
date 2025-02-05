#include "wrapped_swapchain.h"

#include <Config.h>
#include <Util.h>
#include "HooksDx.h"

// Used RenderDoc's wrapped object as referance
// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/dxgi/dxgi_wrapped.cpp

static int scCount = 0;

WrappedIDXGISwapChain4::WrappedIDXGISwapChain4(IDXGISwapChain* real, IUnknown* pDevice, HWND hWnd, PFN_SC_Present renderTrig, PFN_SC_Clean clearTrig, PFN_SC_Release releaseTrig)
    : m_pReal(real), Device(pDevice), Handle(hWnd), RenderTrig(renderTrig), ClearTrig(clearTrig), ReleaseTrig(releaseTrig), m_iRefcount(1)
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
    LOG_DEBUG("");

#ifdef USE_LOCAL_MUTEX
    // dlssg calls this from present it seems
    // don't try to get a mutex when present owns it while dlssg mod is enabled
    if (!(_localMutex.getOwner() == 4 && Config::Instance()->FGType.value_or_default() == FGType::Nukems))
        OwnedLockGuard lock(_localMutex, 1);
#endif

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Waiting ffxMutex 3, current: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.lock(3);
        LOG_TRACE("Accuired ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
    }

    HRESULT result;
    DXGI_SWAP_CHAIN_DESC desc{};
    m_pReal->GetDesc(&desc);

    if (Config::Instance()->FGEnabled.value_or_default())
    {
        State::Instance().FGresetCapturedResources = true;
        State::Instance().FGonlyUseCapturedResources = false;

        if (State::Instance().currentFeature != nullptr)
            State::Instance().FGchanged = true;
    }

    if (ClearTrig != nullptr)
        ClearTrig(true, Handle);

    State::Instance().SCchanged = true;

    LOG_DEBUG("BufferCount: {0}, Width: {1}, Height: {2}, NewFormat: {3}, SwapChainFlags: {4:X}", BufferCount, Width, Height, (UINT)NewFormat, SwapChainFlags);

    result = m_pReal->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (result == S_OK && State::Instance().currentFeature == nullptr)
    {
        State::Instance().screenWidth = Width;
        State::Instance().screenHeight = Height;
        State::Instance().lastMipBias = 100.0f;
        State::Instance().lastMipBiasMax = -100.0f;
    }

    // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
    // https://github.com/EndlesslyFlowering/AutoHDR-ReShade
    if (Config::Instance()->ForceHDR.value_or_default())
    {
        LOG_INFO("Force HDR on");

        do
        {
            if (m_pReal3 == nullptr)
                break;

            NewFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
            DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

            if (Config::Instance()->UseHDR10.value_or_default())
            {
                NewFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
                hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            }

            UINT css = 0;

            result = m_pReal3->CheckColorSpaceSupport(hdrCS, &css);

            if (result != S_OK)
            {
                LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                break;
            }

            if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
            {
                result = m_pReal3->SetColorSpace1(hdrCS);

                if (result != S_OK)
                {
                    LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                    break;
                }
            }

            LOG_INFO("HDR format and color space are set");

        } while (false);
    }

    State::Instance().SCbuffers.clear();
    UINT bc = BufferCount;
    if (bc == 0 && m_pReal1 != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC1 desc{};

        if (m_pReal1->GetDesc1(&desc) == S_OK)
            bc = desc.BufferCount;
    }

    for (size_t i = 0; i < bc; i++)
    {
        IUnknown* buffer;

        if (m_pReal->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
        {
            State::Instance().SCbuffers.push_back(buffer);
            buffer->Release();
        }
    }

    LOG_DEBUG("result: {0:X}", (UINT)result);

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.unlockThis(3);
    }

    return result;
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetContainingOutput(IDXGIOutput** ppOutput)
{
    return m_pReal->GetContainingOutput(ppOutput);
}

HRESULT WrappedIDXGISwapChain4::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags,
                                               const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
    LOG_DEBUG("");

#ifdef USE_LOCAL_MUTEX
    // dlssg calls this from present it seems
    // don't try to get a mutex when present owns it while dlssg mod is enabled
    if (!(_localMutex.getOwner() == 4 && Config::Instance()->FGType.value_or_default() == FGType::Nukems))
        OwnedLockGuard lock(_localMutex, 2);
#endif

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Waiting ffxMutex 3, current: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.lock(3);
        LOG_TRACE("Accuired ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
    }

    HRESULT result;
    DXGI_SWAP_CHAIN_DESC desc{};
    m_pReal->GetDesc(&desc);

    if (Config::Instance()->FGEnabled.value_or_default())
    {
        State::Instance().FGresetCapturedResources = true;
        State::Instance().FGonlyUseCapturedResources = false;

        if (State::Instance().currentFeature != nullptr)
            State::Instance().FGchanged = true;
    }

    if (ClearTrig != nullptr)
        ClearTrig(true, Handle);

    State::Instance().SCchanged = true;

    LOG_DEBUG("BufferCount: {0}, Width: {1}, Height: {2}, NewFormat: {3}, SwapChainFlags: {4:X}, pCreationNodeMask: {5}", BufferCount, Width, Height, (UINT)Format, SwapChainFlags, *pCreationNodeMask);

    result = m_pReal3->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
    if (result == S_OK && State::Instance().currentFeature == nullptr)
    {
        State::Instance().screenWidth = Width;
        State::Instance().screenHeight = Height;
        State::Instance().lastMipBias = 100.0f;
        State::Instance().lastMipBiasMax = -100.0f;
    }

    // Crude implementation of EndlesslyFlowering's AutoHDR-ReShade
    // https://github.com/EndlesslyFlowering/AutoHDR-ReShade
    if (Config::Instance()->ForceHDR.value_or_default())
    {
        LOG_INFO("Force HDR on");

        do
        {
            Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            DXGI_COLOR_SPACE_TYPE hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;

            if (Config::Instance()->UseHDR10.value_or_default())
            {
                Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                hdrCS = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            }

            UINT css = 0;

            auto result = m_pReal3->CheckColorSpaceSupport(hdrCS, &css);

            if (result != S_OK)
            {
                LOG_ERROR("CheckColorSpaceSupport error: {:X}", (UINT)result);
                break;
            }

            if (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT & css)
            {
                result = m_pReal3->SetColorSpace1(hdrCS);

                if (result != S_OK)
                {
                    LOG_ERROR("SetColorSpace1 error: {:X}", (UINT)result);
                    break;
                }
            }

            LOG_INFO("HDR format and color space are set");

        } while (false);
    }

    State::Instance().SCbuffers.clear();
    UINT bc = BufferCount;
    if (bc == 0 && m_pReal1 != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC1 desc{};

        if (m_pReal1->GetDesc1(&desc) == S_OK)
            bc = desc.BufferCount;
    }

    for (size_t i = 0; i < bc; i++)
    {
        IUnknown* buffer;

        if (m_pReal->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
        {
            State::Instance().SCbuffers.push_back(buffer);
            buffer->Release();
        }
    }

    LOG_DEBUG("result: {0:X}", (UINT)result);

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.unlockThis(3);
    }

    return result;
}

HRESULT WrappedIDXGISwapChain4::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
    LOG_DEBUG("Fullscreen: {}, pTarget: {:X}", Fullscreen, (size_t)pTarget);
    HRESULT result = S_OK;

    {
#ifdef USE_LOCAL_MUTEX
        // dlssg calls this from present it seems
        // don't try to get a mutex when present owns it while dlssg mod is enabled
        if (!(_localMutex.getOwner() == 4 && Config::Instance()->FGType.value_or_default() == FGType::Nukems))
            OwnedLockGuard lock(_localMutex, 3);
#endif

        if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
        {
            LOG_TRACE("Waiting ffxMutex 3, current: {}", FrameGen_Dx12::ffxMutex.getOwner());
            FrameGen_Dx12::ffxMutex.lock(3);
            LOG_TRACE("Accuired ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
        }

        result = m_pReal->SetFullscreenState(Fullscreen, pTarget);

        if (result != S_OK)
            LOG_ERROR("result: {:X}", (UINT)result);
        else
            LOG_DEBUG("result: {:X}", result);

        if (Config::Instance()->FGEnabled.value_or_default())
        {
            State::Instance().FGresetCapturedResources = true;
            State::Instance().FGonlyUseCapturedResources = false;

            if (State::Instance().currentFeature != nullptr)
                State::Instance().FGchanged = true;
        }

        if (ClearTrig != nullptr)
            ClearTrig(true, Handle);

        State::Instance().SCchanged = true;
        State::Instance().SCbuffers.clear();

        UINT bc = 0;
        if (m_pReal1 != nullptr)
        {
            DXGI_SWAP_CHAIN_DESC1 desc{};

            if (m_pReal1->GetDesc1(&desc) == S_OK)
                bc = desc.BufferCount;
        }

        for (size_t i = 0; i < bc; i++)
        {
            IUnknown* buffer;

            if (m_pReal->GetBuffer(i, IID_PPV_ARGS(&buffer)) == S_OK)
            {
                State::Instance().SCbuffers.push_back(buffer);
                buffer->Release();
            }
        }
    }

    if (Config::Instance()->FGUseMutexForSwaphain.value_or_default())
    {
        LOG_TRACE("Releasing ffxMutex: {}", FrameGen_Dx12::ffxMutex.getOwner());
        FrameGen_Dx12::ffxMutex.unlockThis(3);
    }

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

#ifdef USE_LOCAL_MUTEX
    OwnedLockGuard lock(_localMutex, 4);
#endif

    HRESULT result;

    if (!(Flags & DXGI_PRESENT_TEST || Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        result = RenderTrig(m_pReal, SyncInterval, Flags, nullptr, Device, Handle);
    else
        result = m_pReal->Present(SyncInterval, Flags);

    return result;
}

HRESULT WrappedIDXGISwapChain4::Present1(UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
    if (m_pReal1 == nullptr)
        return DXGI_ERROR_DEVICE_REMOVED;

#ifdef USE_LOCAL_MUTEX
    OwnedLockGuard lock(_localMutex, 5);
#endif

    HRESULT result;

    if (!(Flags & DXGI_PRESENT_TEST || Flags & DXGI_PRESENT_RESTART) && RenderTrig != nullptr)
        result = RenderTrig(m_pReal1, SyncInterval, Flags, pPresentParameters, Device, Handle);
    else
        result = m_pReal1->Present1(SyncInterval, Flags, pPresentParameters);

    return result;
}

HRESULT STDMETHODCALLTYPE WrappedIDXGISwapChain4::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
    return m_pReal1->GetRestrictToOutput(ppRestrictToOutput);
}