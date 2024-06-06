#pragma once
#include <vulkan/vulkan.hpp>
#include "IFeature.h"

class IFeature_Vk : public virtual IFeature
{
private:

protected:
	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;
	PFN_vkGetInstanceProcAddr GIPA;
	PFN_vkGetDeviceProcAddr GDPA;

public:
	virtual bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA) = 0;
	virtual bool Evaluate(VkCommandBuffer InCmdBuffer, const IFeatureEvaluateParams* InParameters) = 0;

	IFeature_Vk(unsigned int InHandleId, const IFeatureCreateParams InParameters) : IFeature(InHandleId, InParameters)
	{
	}

	void Shutdown() final
	{
		//
	}

	virtual ~IFeature_Vk() {}
};
