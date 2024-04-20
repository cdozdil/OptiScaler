#pragma once
#include "../IFeature_Dx11wDx12.h"
#include "XeSSFeature.h"
#include <string>

class XeSSFeatureDx11 : public XeSSFeature, public IFeature_Dx11wDx12
{
private:
	ID3D12Resource* _outBuffer;
	bool _baseInit = false;

protected:

public:
	XeSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters);
	const char* Name() override { return "XeSS w/Dx12"; }

	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters) override;

	~XeSSFeatureDx11();
};