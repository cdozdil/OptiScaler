#pragma once
#include <vulkan/vulkan.hpp>
#include "IFeature.h"

class IFeature_Vk : public IFeature
{
protected:
	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;

public:
	virtual bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual void ReInit(const NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Vk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
	}

	virtual ~IFeature_Vk() {}
};
