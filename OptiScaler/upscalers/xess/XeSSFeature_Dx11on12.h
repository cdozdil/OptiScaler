#pragma once
#include "XeSSFeature.h"
#include <upscalers/IFeature_Dx11wDx12.h>
#include <string>

class XeSSFeatureDx11on12 : public XeSSFeature, public IFeature_Dx11wDx12
{
private:
	bool _baseInit = false;

protected:

public:
	XeSSFeatureDx11on12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
	std::string Name() const { return "XeSS w/Dx12"; }

	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters) override;

	~XeSSFeatureDx11on12();
};