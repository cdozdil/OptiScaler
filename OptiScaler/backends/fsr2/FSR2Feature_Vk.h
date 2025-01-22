#pragma once


#include <fsr2/ffx_fsr2.h>
#include <fsr2/vk/ffx_fsr2_vk.h>

#include "FSR2Feature.h"
#include <backends/IFeature_Vk.h>

class FSR2FeatureVk : public FSR2Feature, public IFeature_Vk
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureVk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;
};
