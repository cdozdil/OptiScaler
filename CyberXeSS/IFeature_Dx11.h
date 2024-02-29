#pragma once

#include "IFeature.h"

class IFeature_Dx11 : public IFeature
{
protected:
	ID3D11Device* Dx11Device = nullptr;
	ID3D11DeviceContext* Dx11DeviceContext = nullptr;

public:
	virtual bool Init(ID3D11Device* device, ID3D11DeviceContext* context, const NVSDK_NGX_Parameter* initParams) = 0;
	virtual bool Evaluate(ID3D11DeviceContext* deviceContext, const NVSDK_NGX_Parameter* initParams) = 0;
	virtual void ReInit(const NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Dx11(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters, Config* config) : IFeature(handleId, InParameters, config)
	{
	}

	virtual ~IFeature_Dx11() {}
};
