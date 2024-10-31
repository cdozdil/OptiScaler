#pragma once

#include <pch.h>

#include "IFeature_Dx12.h"

void IFeature_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState) const
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = InResource;
	barrier.Transition.StateBefore = InBeforeState;
	barrier.Transition.StateAfter = InAfterState;
	barrier.Transition.Subresource = 0;
	InCommandList->ResourceBarrier(1, &barrier);
}

IFeature_Dx12::IFeature_Dx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters)
{
}

void IFeature_Dx12::Shutdown()
{
	if (Imgui != nullptr && Imgui.get() != nullptr)
		Imgui.reset();
}

IFeature_Dx12::~IFeature_Dx12()
{
	if (OutputScaler != nullptr && OutputScaler.get() != nullptr)
		OutputScaler.reset();

	if (Device)
	{
		ID3D12Fence* d3d12Fence = nullptr;

		do
		{
			if (Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
				break;

			d3d12Fence->Signal(999);

			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
			{
				WaitForSingleObject(fenceEvent, INFINITE);
				CloseHandle(fenceEvent);
			}

		} while (false);

		if (d3d12Fence != nullptr)
		{
			d3d12Fence->Release();
			d3d12Fence = nullptr;
		}
	}
}
