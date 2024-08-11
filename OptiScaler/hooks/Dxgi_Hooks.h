#pragma once
#include "../pch.h"
#include "../enums/Enums.h"

#include <dxgi1_6.h>

namespace Hooks
{
    void AttachDxgiHooks();
    void DetachDxgiHooks();

    void DxgiClean(std::function<void(SwapchainSource, bool)>);
    void DxgiPresent(std::function<void(SwapchainSource, IDXGISwapChain3*, UINT)>);
}
