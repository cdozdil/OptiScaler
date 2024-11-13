#pragma once

#include <pch.h>

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

	inline static int GetCorrectDXGIFormat(int eCurrentFormat)
	{
		switch (eCurrentFormat)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return eCurrentFormat;
	}

public:
	static void Init(IDXGISwapChain* pSwapChain, ID3D11DeviceContext* InDevCtx);
	static void Render(IDXGISwapChain* pSwapChain);
	static void Cleanup(bool shutdown);
	static void ShutDown();

	static void PrepareTimeObjects(ID3D11Device* InDevice);
	static void BeforeUpscale(ID3D11DeviceContext* InDevCtx);
	static void AfterUpscale(ID3D11DeviceContext* InDevCtx);
	static void CalculateTime();
};
