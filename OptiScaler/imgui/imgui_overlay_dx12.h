#pragma once

#include "../pch.h"
#include <d3d12.h>

namespace ImGuiOverlayDx12
{
	bool IsInitedDx12();
	HWND Handle();

	void InitDx12(HWND InHandle, ID3D12Device* InDevice);
	void ShutdownDx12();
	void ReInitDx12(HWND InNewHwnd);
	void EarlyBind();
	void BindMods();
}
