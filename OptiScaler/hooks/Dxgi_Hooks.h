#pragma once
#include "../pch.h"

#include <dxgi1_6.h>

typedef void(*PFN_CleanCallback)(bool);
typedef void(*PFN_PresentCallback)(IDXGISwapChain*);
typedef void(*PFN_ReleaseCallback)(HWND);

namespace Hooks
{
    void AttachDxgiHooks();
    void DetachDxgiHooks();
    void AttachDxgiSwapchainHooks(IDXGIFactory* InFactory);

    void SetDxgiClean(PFN_CleanCallback);
    void SetDxgiPresent(PFN_PresentCallback);
    void SetDxgiRelease(PFN_ReleaseCallback);

    IUnknown* DxgiDevice();
    void EnumarateDxgiAdapters();
    IDXGIAdapter* GetDXGIAdapter(uint32_t no);
}
