#pragma once
#include "DLSSFeature.h"
#include <backends/IFeature_Dx12.h>
#include <rcas/RCAS_Dx12.h>
#include <string>

class DLSSFeatureDx12 : public DLSSFeature, public IFeature_Dx12
{
private:

protected:


public:
	bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(ID3D12Device* InDevice);

	DLSSFeatureDx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
	~DLSSFeatureDx12();
};