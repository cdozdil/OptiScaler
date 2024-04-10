#pragma once
#include "../IFeature_Dx11wDx12.h"
#include "FSR2Feature.h"

#include "../../fsr2/include/ffx_fsr2.h"
#include "../../fsr2/include/dx12/ffx_fsr2_dx12.h"

class FSR2FeatureDx11on12 : public FSR2Feature, public IFeature_Dx11wDx12
{
private:
	bool _baseInit = false;

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	const char* Name() override { return "FSR w/Dx12"; }

	FSR2FeatureDx11on12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature_Dx11wDx12(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters) override;

	~FSR2FeatureDx11on12();
};