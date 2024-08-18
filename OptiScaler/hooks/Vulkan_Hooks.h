#pragma once

#include <vulkan/vulkan_core.h>

namespace Hooks
{
    void AttachVulkanHooks();
    void DetachVulkanHooks();
    void AttachVulkanDeviceSpoofingHooks();
    void DetachVulkanDeviceSpoofingHooks();
    void AttachVulkanExtensionSpoofingHooks();
    void DetachVulkanExtensionSpoofingHooks();

    VkDevice VulkanDevice();
    VkInstance VulkanInstance();
    VkPhysicalDevice VulkanPD();
}