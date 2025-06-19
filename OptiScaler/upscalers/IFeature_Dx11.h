#pragma once
#include "IFeature.h"
#include <menu/menu_dx11.h>
#include <shaders/rcas/RCAS_Dx11.h>
#include <shaders/output_scaling/OS_Dx11.h>
#include <shaders/bias/Bias_Dx11.h>

class IFeature_Dx11 : public virtual IFeature
{
  protected:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    inline static std::unique_ptr<Menu_Dx11> Imgui = nullptr;

    std::unique_ptr<OS_Dx11> OutputScaler = nullptr;
    std::unique_ptr<RCAS_Dx11> RCAS = nullptr;
    std::unique_ptr<Bias_Dx11> Bias = nullptr;

  public:
    virtual bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters) = 0;
    virtual bool Evaluate(ID3D11DeviceContext* DeviceContext, NVSDK_NGX_Parameter* InParameters) = 0;

    IFeature_Dx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters) {}

    void Shutdown() final;

    ~IFeature_Dx11();
};
