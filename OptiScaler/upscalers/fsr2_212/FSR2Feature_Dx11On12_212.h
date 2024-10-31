#pragma once

#include "FSR2Feature_212.h"
#include <upscalers/IFeature_Dx11wDx12.h>

#include "include/ffx_fsr2.h"
#include "include/dx12/ffx_fsr2_dx12.h"

class FSR2FeatureDx11on12_212 : public FSR2Feature212, public IFeature_Dx11wDx12
{
private:
	bool _baseInit = false;

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	const char* Name() final { return "FSR w/Dx12"; }

	FSR2FeatureDx11on12_212(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR2Feature212(InHandleId, InParameters), IFeature_Dx11wDx12(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters) override;

	~FSR2FeatureDx11on12_212();
};