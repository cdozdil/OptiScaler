#pragma once

#include "../pch.h"
#include <vulkan/vulkan.hpp>

namespace ImGuiOverlayVk
{
	bool IsInitedVk();

	void InitVk(HWND InHwnd, VkDevice InDevice, VkInstance InInstance, VkPhysicalDevice InPD);
	void ShutdownVk();
	void ReInitDx12(HWND InNewHwnd);
}
