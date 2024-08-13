#pragma once
#include "../pch.h"
#include "../enums/Enums.h"

#include <dxgi1_6.h>

namespace Hooks
{
    void AttachDxgiHooks();
    void DetachDxgiHooks();

    void SetDxgiClean(std::function<void(SwapchainSource, bool)>);
    void SetDxgiPresent(std::function<void(SwapchainSource, IDXGISwapChain)>);
    void SetDxgiRelease(std::function<void(SwapchainSource, HWND)>);
}
