#pragma once
#include <vulkan/vulkan.hpp>

#include <ffx_fsr2.h>
#include <vk/ffx_fsr2_vk.h>

#include "FSR2Feature.h"
#include "../IFeature_Vk.h"

class FSR2FeatureVk : public FSR2Feature, public IFeature_Vk
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters) override;
};
