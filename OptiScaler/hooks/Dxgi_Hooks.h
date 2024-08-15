#pragma once
#include "../pch.h"
#include "../enums/Enums.h"

#include <dxgi1_6.h>

typedef void(*CleanCallback)(SwapchainSource, bool);
typedef void(*PresentCallback)(SwapchainSource, IDXGISwapChain*);
typedef void(*ReleaseCallback)(SwapchainSource, HWND);

namespace Hooks
{
    void AttachDxgiHooks();
    void DetachDxgiHooks();

    void SetDxgiClean(CleanCallback);
    void SetDxgiPresent(PresentCallback);
    void SetDxgiRelease(ReleaseCallback);
}
