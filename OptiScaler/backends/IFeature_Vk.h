#pragma once
#include "IFeature.h"

#include <Config.h>

#include <vulkan/vulkan.hpp>

class IFeature_Vk : public virtual IFeature
{
private:

protected:
	VkInstance Instance = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
	VkDevice Device = nullptr;
	PFN_vkGetInstanceProcAddr GIPA = nullptr;
	PFN_vkGetDeviceProcAddr GDPA = nullptr;

public:
	virtual bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Vk(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters)
	{
	}

	void Shutdown() final
	{
		//
	}

	virtual ~IFeature_Vk() {}
};
