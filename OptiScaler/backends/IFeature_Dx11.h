#pragma once
#include "IFeature.h"
#include "../imgui/Imgui_Dx11.h"

class IFeature_Dx11 : public virtual IFeature
{
protected:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	inline static std::unique_ptr<Imgui_Dx11> Imgui = nullptr;

public:
	virtual bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext) = 0;
	virtual bool Evaluate(ID3D11DeviceContext* DeviceContext, const IFeatureEvaluateParams* InParameters) = 0;

	IFeature_Dx11(unsigned int InHandleId, const IFeatureCreateParams InParams) : IFeature(InHandleId, InParams)
	{
	}

	void Shutdown() final;

	~IFeature_Dx11();
};
