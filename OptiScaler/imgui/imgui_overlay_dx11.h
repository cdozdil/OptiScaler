#pragma once

#include <pch.h>

namespace ImGuiOverlayDx11
{
	bool IsInitedDx11();
	HWND Handle();

	void InitDx11(HWND InHandle);
	void ShutdownDx11();
	void ReInitDx11(HWND InNewHwnd);
}
