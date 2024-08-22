#pragma once
#include <backends/IFeature_Vk.h>
#include "FSR2Feature_212.h"

#include <fsr2_212/include/vk/ffx_fsr2_vk.h>

class FSR2FeatureVk212 : public FSR2Feature212, public IFeature_Vk
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureVk212(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : FSR2Feature212(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;
};
