#pragma once

#include "IFeatureContext.h"

class IFeatureContextDx11 : public IFeatureContext
{
protected:
	ID3D11Device* Dx11Device = nullptr;
	ID3D11DeviceContext* Dx11DeviceContext = nullptr;

public:
	virtual bool Init(ID3D11Device* device, ID3D11DeviceContext* context, const NVSDK_NGX_Parameter* initParams) = 0;
	virtual bool Evaluate(ID3D11DeviceContext* deviceContext, const NVSDK_NGX_Parameter* initParams) = 0;

	IFeatureContextDx11() = default;
	virtual ~IFeatureContextDx11() {}
};
