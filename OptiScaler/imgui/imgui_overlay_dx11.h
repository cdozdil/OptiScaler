#pragma once

#include "../pch.h"

namespace ImGuiOverlayDx11
{
	bool IsInitedDx11();

	void InitDx11(HWND InHandle);
	void ShutdownDx11();
}
