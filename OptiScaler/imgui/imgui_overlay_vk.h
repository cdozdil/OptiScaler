#pragma once

#include "../pch.h"
#include <vulkan/vulkan.hpp>

namespace ImGuiOverlayVk
{
	bool IsInitedVk();
	HWND Handle();

	void InitVk(HWND InHwnd, VkDevice InDevice, VkInstance InInstance, VkPhysicalDevice InPD);
	void HookVK();
	void ShutdownVk();
	void ReInitVk(HWND InNewHwnd);
}
