#pragma once
#include <upscalers/IFeature_Vk.h>
#include "DLSSFeature.h"
#include <string>
#include "nvsdk_ngx_vk.h"

class DLSSFeatureVk : public DLSSFeature, public IFeature_Vk
{
  private:
  protected:
  public:
    bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList,
              PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA,
              NVSDK_NGX_Parameter* InParameters) override;
    bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;

    static void Shutdown(VkDevice InDevice);

    DLSSFeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
    ~DLSSFeatureVk();
};
