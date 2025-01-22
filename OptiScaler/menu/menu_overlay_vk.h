#pragma once

#include <pch.h>
#include <vulkan/vulkan.hpp>

namespace MenuOverlayVk
{
    void CreateSwapchain(VkDevice device, VkPhysicalDevice pd, VkInstance instance, HWND hwnd, const VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
    bool QueuePresent(VkQueue queue, VkPresentInfoKHR* pPresentInfo);
    void DestroyVulkanObjects(bool shutdown);
}
