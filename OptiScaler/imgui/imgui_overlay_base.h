#pragma once

#include "../pch.h"

#include <d3d12.h>
#include <dxgi1_6.h>


// LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//typedef HRESULT(__fastcall* PFN_Present)(IDXGISwapChain*, UINT, UINT);
//typedef HRESULT(__fastcall* PFN_Present1)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
//typedef HRESULT(__fastcall* PFN_ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
//typedef HRESULT(__fastcall* PFN_ResizeBuffers1)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT, const UINT*, IUnknown* const*);
//typedef HRESULT(__fastcall* PFN_CreateSwapChain)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**);
//typedef HRESULT(__fastcall* PFN_CreateSwapChainForHwnd)(IDXGIFactory*, IUnknown*, HWND, const DXGI_SWAP_CHAIN_DESC1*, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGIOutput*, IDXGISwapChain1**);
//typedef HRESULT(__fastcall* PFN_CreateSwapChainForCoreWindow)(IDXGIFactory*, IUnknown*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**);
//typedef HRESULT(__fastcall* PFN_CreateSwapChainForComposition)(IDXGIFactory*, IUnknown*, const DXGI_SWAP_CHAIN_DESC1*, IDXGIOutput*, IDXGISwapChain1**);

class ImGuiOverlayBase
{
public:
	static HWND Handle();

	static void Dx11Ready();
	static void Dx12Ready();
	static void VulkanReady();

	static bool IsInited();
	static bool IsVisible();
	static bool IsResetRequested();

	static void Init(HWND InHandle);
	static void RenderMenu();
	static void Shutdown();
	static void HideMenu();
};
