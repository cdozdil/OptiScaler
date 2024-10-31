#pragma once
#include <backends/IFeature_Dx12.h>
#include "DLSSDFeature.h"

class DLSSDFeatureDx12 : public DLSSDFeature, public IFeature_Dx12
{
private:

protected:


public:
	bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(ID3D12Device* InDevice);

	DLSSDFeatureDx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
	~DLSSDFeatureDx12();
};