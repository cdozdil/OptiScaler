#pragma once

#include "../pch.h"
#include <d3d11.h>
#include <d3d12.h>

namespace ImGuiOverlayDx
{
	inline ID3D12QueryHeap* queryHeap = nullptr;
	inline ID3D12Resource* readbackBuffer = nullptr;
	inline bool dx12UpscaleTrig = false;

	inline ID3D11Query* disjointQuery = nullptr;
	inline ID3D11Query* timestampStartQuery = nullptr;
	inline ID3D11Query* timestampEndQuery = nullptr;
	inline bool dx11UpscaleTrig = false;

	void UnHookDx();
	void HookDx();
}
