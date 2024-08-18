#pragma once
#include "../pch.h"

#include <d3d12.h>

namespace Hooks
{
    void AttachDx12Hooks();
    void DetachDx12Hooks();

    void ContextEvaluateStart();
    void ContextEvaluateStop(ID3D12GraphicsCommandList* InCmdList, bool InRestore);

    ID3D12Device* Dx12Device();
}