#pragma once
#include "../IFeature_Vk.h"
#include "FSR31Feature.h"

#include <vk/ffx_api_vk.h>

class FSR31FeatureVk : public FSR31Feature, public IFeature_Vk
{
private:

protected:
	bool InitFSR3(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR31FeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters);

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;
};
