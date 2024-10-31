#pragma once

#include <pch.h>

#include <vulkan/vulkan.hpp>

namespace ImGuiOverlayVk
{
	inline VkQueryPool queryPool = VK_NULL_HANDLE;
	inline double timeStampPeriod = 1.0;
	inline bool vkUpscaleTrig = false;

	void HookVk();
	void UnHookVk();
}
