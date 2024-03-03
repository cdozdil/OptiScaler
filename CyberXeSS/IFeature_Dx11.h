#pragma once
#include "IFeature.h"

class IFeature_Dx11 : public virtual IFeature
{
protected:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;

public:
	virtual bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(ID3D11DeviceContext* DeviceContext, const NVSDK_NGX_Parameter* InParameters) = 0;
	//virtual void ReInit(const NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Dx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
	}

	virtual ~IFeature_Dx11() {}
};
