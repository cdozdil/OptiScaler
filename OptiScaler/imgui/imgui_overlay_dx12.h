#pragma once

#include "../pch.h"

namespace ImGuiOverlayDx12
{
	bool IsInitedDx12();

	void InitDx12(HWND InHandle);
	void ShutdownDx12();
}
