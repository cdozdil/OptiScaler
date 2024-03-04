#pragma once
#include "vulkan/vulkan.hpp"

typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceProperties)(VkPhysicalDevice, VkPhysicalDeviceProperties*);
typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceProperties2)(VkPhysicalDevice, VkPhysicalDeviceProperties2*);
typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceProperties2KHR)(VkPhysicalDevice, VkPhysicalDeviceProperties2*);