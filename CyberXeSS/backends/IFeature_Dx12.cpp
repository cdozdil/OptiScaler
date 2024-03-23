#pragma once
#include "IFeature_Dx12.h"

void IFeature_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	barrier.Transition.Subresource = 0;
	commandList->ResourceBarrier(1, &barrier);
}

IFeature_Dx12::~IFeature_Dx12()
{
	if (Device)
	{
		ID3D12Fence* d3d12Fence;
		Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));

		d3d12Fence->Signal(999);

		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
		{
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}
		else
			spdlog::warn("XeSSFeatureDx12::~XeSSFeatureDx12 can't get fenceEvent handle");

		d3d12Fence->Release();
	}

	if (Imgui)
		Imgui.reset();
}
