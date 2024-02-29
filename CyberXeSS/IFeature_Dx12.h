#pragma once
#include "d3d12.h"

#include "IFeature.h"

class IFeature_Dx12 : public IFeature
{
protected:
	ID3D12Device* Device = nullptr;

	void ResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = beforeState;
		barrier.Transition.StateAfter = afterState;
		barrier.Transition.Subresource = 0;
		commandList->ResourceBarrier(1, &barrier);
	}

public:
	virtual bool Init(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual void ReInit(const NVSDK_NGX_Parameter* InParameters) = 0;

	explicit IFeature_Dx12(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters, Config* config) : IFeature(handleId, InParameters, config)
	{
	}

	virtual ~IFeature_Dx12() {}
};
