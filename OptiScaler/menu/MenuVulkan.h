#pragma once

#include <pch.h>

#include <vulkan/vulkan.hpp>

class MenuVulkan
{
private:
    inline static VkQueryPool queryPool = VK_NULL_HANDLE;
    inline static double timeStampPeriod = 1.0;
    inline static bool vkUpscaleTrig = false;

public:
    static void CreateSurface(VkInstance instance, HWND hwnd);
    static void CreateInstance(VkInstance* pInstance);
    static void CreateDevice(VkPhysicalDevice physicalDevice, VkDevice* pDevice);
    static VkResult QueuePresent(VkQueue queue, VkPresentInfoKHR* pPresentInfo);
    static void CreateSwapchain(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain);
};
