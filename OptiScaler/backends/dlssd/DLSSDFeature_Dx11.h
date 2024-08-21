#pragma once
#include "../IFeature_Dx11.h"
#include "DLSSDFeature.h"

class DLSSDFeatureDx11 : public DLSSDFeature, public IFeature_Dx11
{
private:

protected:

public:
	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(ID3D11Device* InDevice);

	DLSSDFeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
	~DLSSDFeatureDx11();
};