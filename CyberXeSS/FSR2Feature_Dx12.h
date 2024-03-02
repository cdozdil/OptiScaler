#include "IFeature_Dx12.h"
#include "FSR2Feature.h"
#include <string>

class FSR2FeatureDx12 : public FSR2Feature, public IFeature_Dx12
{
private:

protected:

public:
	FSR2FeatureDx12(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(handleId, InParameters), IFeature_Dx12(handleId, InParameters)
	{
	}

	// Inherited via IFeatureContextDx12
	bool Init(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* InParameters) override;
	void ReInit(const NVSDK_NGX_Parameter* InParameters) override;

	// Inherited via XeSSContext
	void SetInit(bool value) override;
	bool IsInited() override;
};