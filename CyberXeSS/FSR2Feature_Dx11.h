#pragma once
#include "IFeature_Dx11.h"
#include "FSR2Feature.h"

#include <ffx_fsr2.h>
#include <dx11/ffx_fsr2_dx11.h>

class FSR2FeatureDx11 : public FSR2Feature, public IFeature_Dx11
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters) override;

public:
	FSR2FeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}


	// Inherited via IFeature_Dx11
	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* DeviceContext, const NVSDK_NGX_Parameter* InParameters) override;
};
