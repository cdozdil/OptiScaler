#pragma once

#include <pch.h>

#include <d3d12.h>
#include <d3d11_4.h>

class MenuDx11
{
private:
	inline static const int QUERY_BUFFER_COUNT = 3;
	inline static ID3D11Query* disjointQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline static ID3D11Query* startQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline static ID3D11Query* endQueries[QUERY_BUFFER_COUNT] = { nullptr, nullptr, nullptr };
	inline static bool dx11UpscaleTrig[QUERY_BUFFER_COUNT] = { false, false, false };

	inline static int currentFrameIndex = 0;
	inline static int previousFrameIndex = 0;

public:
	
	static void PrepareTimeObjects(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice);
	static void BeforeUpscale(VkCommandBuffer InCmdBuffer);
	static void AfterUpscale(VkCommandBuffer InCmdBuffer);
};
