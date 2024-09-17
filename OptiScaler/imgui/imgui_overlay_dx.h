#pragma once

#include "../pch.h"
#include <d3d11_4.h>
#include <d3d12.h>

namespace ImGuiOverlayDx
{
	inline ID3D12QueryHeap* queryHeap = nullptr;
	inline ID3D12Resource* readbackBuffer = nullptr;
	inline bool dx12UpscaleTrig = false;

	inline const int QUERY_BUFFER_COUNT = 3;
	inline ID3D11Query* disjointQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline ID3D11Query* startQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline ID3D11Query* endQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline bool dx11UpscaleTrig[QUERY_BUFFER_COUNT] = { false, false, false };

	inline int currentFrameIndex = 0;
	inline int previousFrameIndex = 0;

	void UnHookDx();
	void HookDx();
}
