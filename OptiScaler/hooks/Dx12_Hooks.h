#pragma once
#include "../pch.h"

#include <d3d12.h>

namespace Hooks
{
    void AttachDx12Hooks();
    void DetachDx12Hooks();

    void ContextEvaluateStart();
    void ContextEvaluateStop(ID3D12GraphicsCommandList* InCmdList, bool InRestore);

    bool IsDeviceCaptured();

    void SetCommandQueueReleaseCallback(std::function<ULONG(IUnknown*)> InCallback);
    void SetExecuteCommandListCallback(std::function<void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)> InCallback);
}