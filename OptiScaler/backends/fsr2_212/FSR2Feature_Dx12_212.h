#pragma once
#include "FSR2Feature_212.h"
#include <backends/IFeature_Dx12.h>

#include <fsr2_212/include/ffx_fsr2.h>
#include <fsr2_212/include/dx12/ffx_fsr2_dx12.h>

class FSR2FeatureDx12_212 : public FSR2Feature212, public IFeature_Dx12
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureDx12_212(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR2Feature212(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

	~FSR2FeatureDx12_212();
};