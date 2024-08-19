#pragma once
#include "../pch.h"

#include <dxgi1_6.h>

typedef void(*PFN_DxgiCleanCallback)(bool);
typedef void(*PFN_DxgiPresentCallback)(IDXGISwapChain*);
typedef void(*PFN_DxgiReleaseCallback)(HWND);

namespace Hooks
{
    void AttachDxgiHooks();
    void DetachDxgiHooks();
    void AttachDxgiSwapchainHooks(IDXGIFactory* InFactory);

    void SetDxgiClean(PFN_DxgiCleanCallback);
    void SetDxgiPresent(PFN_DxgiPresentCallback);
    void SetDxgiRelease(PFN_DxgiReleaseCallback);

    IUnknown* DxgiDevice();
    void EnumarateDxgiAdapters();
    IDXGIAdapter* GetDXGIAdapter(uint32_t no);
}
