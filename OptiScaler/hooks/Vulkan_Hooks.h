#pragma once

#include <vulkan/vulkan_core.h>

typedef void(*PFN_VulkanCreateSwapchainCallback)(VkDevice, const VkSwapchainCreateInfoKHR*, VkSwapchainKHR*);
typedef void(*PFN_VulkanPresentCallback)(VkPresentInfoKHR*);

namespace Hooks
{
    void AttachVulkanHooks();
    void DetachVulkanHooks();
    void AttachVulkanDeviceSpoofingHooks();
    void DetachVulkanDeviceSpoofingHooks();
    void AttachVulkanExtensionSpoofingHooks();
    void DetachVulkanExtensionSpoofingHooks();

    void SetVulkanCreateSwapchain(PFN_VulkanCreateSwapchainCallback);
    void SetVulkanPresent(PFN_VulkanPresentCallback);

    VkDevice VulkanDevice();
    VkInstance VulkanInstance();
    VkPhysicalDevice VulkanPD();
}