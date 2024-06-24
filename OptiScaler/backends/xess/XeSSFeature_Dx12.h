#pragma once
#include "../IFeature_Dx12.h"
#include "XeSSFeature.h"
#include <string>

class XeSSFeatureDx12 : public XeSSFeature, public IFeature_Dx12
{
private:

protected:

public:
	XeSSFeatureDx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), XeSSFeature(InHandleId, InParameters)
	{
	}

	bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

	~XeSSFeatureDx12();
};