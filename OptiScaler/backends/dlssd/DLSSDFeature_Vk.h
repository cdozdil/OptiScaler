#pragma once
#include "../IFeature_Vk.h"
#include "DLSSDFeature.h"

#include <nvsdk_ngx_vk.h>

class DLSSDFeatureVk : public DLSSDFeature, public IFeature_Vk
{
private:

protected:


public:
	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(VkDevice InDevice);

	DLSSDFeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);
	~DLSSDFeatureVk();
};