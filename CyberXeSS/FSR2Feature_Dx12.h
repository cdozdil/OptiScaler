#include "FSR2Feature.h"
#include "IFeature_Dx12.h"

#include "ffx3/include/FidelityFX/host/backends/dx12/ffx_dx12.h"

#ifdef _DEBUG
#pragma comment(lib, "ffx3/lib/ffx_backend_dx12_x64d.lib")
#else
#pragma comment(lib, "ffx3/lib/ffx_backend_dx12_x64.lib")
#endif // DEBUG

class FSR2FeatureDx12 : public FSR2Feature, public IFeature_Dx12
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureDx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters) override;
	void ReInit(const NVSDK_NGX_Parameter* InParameters) override;
};