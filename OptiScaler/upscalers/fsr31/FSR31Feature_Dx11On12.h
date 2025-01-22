#pragma once
#include "FSR31Feature.h"
#include <upscalers/IFeature_Dx11wDx12.h>

#include "dx12/ffx_api_dx12.h"


class FSR31FeatureDx11on12 : public FSR31Feature, public IFeature_Dx11wDx12
{
private:
	bool _baseInit = false;

protected:
	bool InitFSR3(const NVSDK_NGX_Parameter* InParameters);

public:
	const char* Name() final { return "FSR3 w/Dx12"; }

	FSR31FeatureDx11on12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);

	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters) override;

	~FSR31FeatureDx11on12()
	{
		if (_context != nullptr)
			FfxApiProxy::D3D12_DestroyContext()(&_context, NULL);
	}
};