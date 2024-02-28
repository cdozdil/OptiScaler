#pragma once

#include "IFeatureContext.h"

class IFeatureContextDx12 : public IFeatureContext
{
protected:
	ID3D12Device* Device = nullptr;

public:
	virtual bool Init(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* InParameters) = 0;

	explicit IFeatureContextDx12(const NVSDK_NGX_Parameter* InParameters) : IFeatureContext(InParameters)
	{
	}

	virtual ~IFeatureContextDx12() {}
};
