#pragma once

#include "../pch.h"

namespace ImGuiOverlayDx12
{
	bool IsInitedDx12();
	HWND Handle();

	void InitDx12(HWND InHandle);
	void ShutdownDx12();
	void ReInitDx12(HWND InNewHwnd);
}
