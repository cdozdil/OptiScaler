#include "IFeature_Dx12.h"
#include "XeSSFeature.h"

class XeSSFeatureDx12 : public XeSSFeature, public IFeature_Dx12
{
private:

protected:

public:
	using XeSSFeature::GetConfig;

	XeSSFeatureDx12(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters, Config* config) : XeSSFeature(handleId, InParameters, config), IFeature_Dx12(handleId, InParameters, config)
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