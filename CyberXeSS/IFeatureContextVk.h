#pragma once

#include "IFeatureContext.h"
#include <vulkan/vulkan.hpp>

class IFeatureContextVk : public IFeatureContext
{
protected:
	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;

public:
	virtual bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, const NVSDK_NGX_Parameter* initParams) = 0;
	virtual bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* initParams) = 0;

	IFeatureContextVk() = default;
	virtual ~IFeatureContextVk() {}
};
