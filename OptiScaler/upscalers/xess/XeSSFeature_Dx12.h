#pragma once
#include "XeSSFeature.h"

#include <upscalers/IFeature_Dx12.h>

class XeSSFeatureDx12 : public XeSSFeature, public IFeature_Dx12
{
private:

protected:

public:
    XeSSFeatureDx12(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), XeSSFeature(InHandleId, InParameters)
    {
        if (XeSSProxy::Module() == nullptr && XeSSProxy::InitXeSS())
            XeSSProxy::HookXeSS();

        _moduleLoaded = XeSSProxy::Module() != nullptr && XeSSProxy::D3D12CreateContext() != nullptr;
    }

    bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;
    bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

    ~XeSSFeatureDx12();
};