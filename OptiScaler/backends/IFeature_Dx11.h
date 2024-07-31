#pragma once
#include "IFeature.h"
#include "../imgui/Imgui_Dx11.h"
#include "../rcas/RCAS_Dx11.h"

class IFeature_Dx11 : public virtual IFeature
{
protected:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	inline static std::unique_ptr<Imgui_Dx11> Imgui = nullptr;
	std::unique_ptr<RCAS_Dx11> RCAS = nullptr;


public:
	virtual bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(ID3D11DeviceContext* DeviceContext, NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Dx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
	}

	void Shutdown() final;

	~IFeature_Dx11();
};
