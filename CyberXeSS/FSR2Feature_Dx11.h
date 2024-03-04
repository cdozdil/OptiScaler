#pragma once
#include "IFeature_Dx11.h"
#include "FSR2Feature.h"

#include "ffx3dx11/include/FidelityFX/host/backends/dx11/ffx_dx11.h"

#ifdef _DEBUG
#pragma comment(lib, "ffx3dx11/lib/ffx_x64d.lib")
#else
#pragma comment(lib, "ffx3dx11/lib/ffx_x64.lib")
#endif // DEBUG

#ifdef _DEBUG
#pragma comment(lib, "ffx3dx11/lib/ffx_fsr2_x64d.lib")
#else
#pragma comment(lib, "ffx3dx11/lib/ffx_fsr2_x64.lib")
#endif // DEBUG

class FSR2FeatureDx11 : public FSR2Feature, public IFeature_Dx11
{
private:

protected:

public:
	FSR2FeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	// Inherited via FSR2Feature
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters) override;

	// Inherited via IFeature_Dx11
	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* DeviceContext, const NVSDK_NGX_Parameter* InParameters) override;

	~FSR2FeatureDx11();
};
