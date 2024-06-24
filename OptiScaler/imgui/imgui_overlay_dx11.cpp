#include "imgui_overlay_base.h"
#include "imgui_overlay_dx11.h"

#include "../Util.h"
#include "../pch.h"

#include <d3d11.h>

#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

static bool _isInited = false;

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static ID3D11RenderTargetView* g_pd3dRenderTarget = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;

static PFN_Present oPresent_Dx11 = nullptr;
static PFN_Present1 oPresent1_Dx11 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx11 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx11 = nullptr;
static PFN_CreateSwapChain oCreateSwapChain_Dx11 = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_Dx11 = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_Dx11 = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_Dx11 = nullptr;

static void CleanupRenderTarget();
static void CleanupDeviceD3D11();
static bool CreateDeviceD3D11(HWND hWnd);
static void CreateRenderTarget(IDXGISwapChain* pSwapChain);
static void RenderImGui_DX11(IDXGISwapChain* pSwapChain);

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
	switch (eCurrentFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return eCurrentFormat;
}

static HRESULT WINAPI hkPresent_Dx11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	RenderImGui_DX11(pSwapChain);
	return oPresent_Dx11(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	RenderImGui_DX11(pSwapChain);
	return oPresent1_Dx11(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx11(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	CleanupRenderTarget();
	return oResizeBuffers_Dx11(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1_Dx11(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	CleanupRenderTarget();
	return oResizeBuffers1_Dx11(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

static HRESULT WINAPI hkCreateSwapChain_Dx11(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChain_Dx11(pFactory, pDevice, pDesc, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_Dx11(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForHwnd_Dx11(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_Dx11(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForCoreWindow_Dx11(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForComposition_Dx11(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForComposition_Dx11(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

static void CleanupRenderTarget()
{
	spdlog::debug("ImGuiOverlayDx11::CleanupRenderTarget");
	
	if (g_pd3dRenderTarget)
	{
		g_pd3dRenderTarget->Release();
		g_pd3dRenderTarget = NULL;
	}
}

static void CleanupDeviceD3D11()
{
	spdlog::debug("ImGuiOverlayDx11::CleanupDeviceD3D11");

	CleanupRenderTarget();

	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		g_pSwapChain = nullptr;
	}

	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
	}

	if (g_pd3dDeviceContext)
	{
		g_pd3dDeviceContext->Release();
		g_pd3dDeviceContext = nullptr;
	}
}

static bool CreateDeviceD3D11(HWND hWnd)
{
	spdlog::info("ImGuiOverlayDx11::CreateDeviceD3D11({0:X})", (ULONG64)hWnd);

	// Create the D3DDevice
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { };
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;

	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, featureLevels, 2, D3D11_SDK_VERSION, &swapChainDesc, &g_pSwapChain, &g_pd3dDevice, nullptr, nullptr);
	if (hr != S_OK)
	{
		spdlog::error("ImGuiOverlayDx11::CreateDeviceD3D11 D3D11CreateDeviceAndSwapChain error: {0:X}", (ULONG64)hWnd);
		return false;
	}

	// Hook
	IDXGIDevice* pDXGIDevice = NULL;
	hr = g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
	if (hr != S_OK)
	{
		spdlog::error("ImGuiOverlayDx11::CreateDeviceD3D11 QueryInterface error: {0:X}", (ULONG64)hWnd);
		return false;
	}

	IDXGIAdapter* pDXGIAdapter = NULL;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if (hr != S_OK)
	{
		spdlog::error("ImGuiOverlayDx11::CreateDeviceD3D11 GetAdapter error: {0:X}", (ULONG64)hWnd);
		return false;
	}

	IDXGIFactory* pIDXGIFactory = NULL;
	hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));
	if (hr != S_OK)
	{
		spdlog::error("ImGuiOverlayDx11::CreateDeviceD3D11 GetParent error: {0:X}", (ULONG64)hWnd);
		return false;
	}

	if (!pIDXGIFactory)
	{
		spdlog::error("ImGuiOverlayDx11::CreateDeviceD3D11 pIDXGIFactory is null");
		return false;
	}

	void** pVTable = *reinterpret_cast<void***>(g_pSwapChain);
	void** pFactoryVTable = *reinterpret_cast<void***>(pIDXGIFactory);

	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pDXGIDevice->Release();
	pDXGIDevice = nullptr;

	CleanupDeviceD3D11();

	oCreateSwapChain_Dx11 = (PFN_CreateSwapChain)pFactoryVTable[10];
	oCreateSwapChainForHwnd_Dx11 = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
	oCreateSwapChainForCoreWindow_Dx11 = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
	oCreateSwapChainForComposition_Dx11 = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

	oPresent_Dx11 = (PFN_Present)pVTable[8];
	oPresent1_Dx11 = (PFN_Present1)pVTable[22];

	oResizeBuffers_Dx11 = (PFN_ResizeBuffers)pVTable[13];
	oResizeBuffers1_Dx11 = (PFN_ResizeBuffers1)pVTable[39];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)oCreateSwapChain_Dx11, hkCreateSwapChain_Dx11);
	DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_Dx11, hkCreateSwapChainForHwnd_Dx11);
	DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx11, hkCreateSwapChainForCoreWindow_Dx11);
	DetourAttach(&(PVOID&)oCreateSwapChainForComposition_Dx11, hkCreateSwapChainForComposition_Dx11);

	DetourAttach(&(PVOID&)oPresent_Dx11, hkPresent_Dx11);
	DetourAttach(&(PVOID&)oPresent1_Dx11, hkPresent1_Dx11);

	DetourAttach(&(PVOID&)oResizeBuffers_Dx11, hkResizeBuffers_Dx11);
	DetourAttach(&(PVOID&)oResizeBuffers1_Dx11, hkResizeBuffers1_Dx11);

	DetourTransactionCommit();

	return true;
}

static void CreateRenderTarget(IDXGISwapChain* pSwapChain)
{
	ID3D11Texture2D* pBackBuffer = NULL;
	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

	if (pBackBuffer)
	{
		DXGI_SWAP_CHAIN_DESC sd;
		pSwapChain->GetDesc(&sd);

		D3D11_RENDER_TARGET_VIEW_DESC desc = { };
		desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &desc, &g_pd3dRenderTarget);
		pBackBuffer->Release();
	}
}

static void RenderImGui_DX11(IDXGISwapChain* pSwapChain)
{
	auto hwnd = Util::GetProcessWindow();

	if (!ImGuiOverlayBase::IsInited())
		ImGuiOverlayBase::Init(hwnd);

	if (ImGuiOverlayBase::IsInited() && (ImGuiOverlayBase::IsResetRequested() || hwnd != ImGuiOverlayBase::Handle()))
	{
		spdlog::info("RenderImGui_DX12 Reset request detected, shutting down ImGui!");
		ImGuiOverlayDx11::ShutdownDx11();
		ImGuiOverlayDx11::InitDx11(hwnd);
		return;
	}

	if (!ImGuiOverlayBase::IsInited() || !ImGuiOverlayBase::IsVisible())
		return;

	if (!ImGui::GetIO().BackendRendererUserData)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice))))
		{
			g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
			ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
		}
	}

	if (_isInited)
	{
		if (!g_pd3dRenderTarget) 
			CreateRenderTarget(pSwapChain);

		if (ImGui::GetCurrentContext() && g_pd3dRenderTarget)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGuiOverlayBase::RenderMenu();

			ImGui::Render();

			g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pd3dRenderTarget, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}
}

bool ImGuiOverlayDx11::IsInitedDx11()
{
	return _isInited;
}

HWND ImGuiOverlayDx11::Handle()
{
	return ImGuiOverlayBase::Handle();
}

void ImGuiOverlayDx11::InitDx11(HWND InHandle)
{
	if (!_isInited && g_pd3dDevice == nullptr && CreateDeviceD3D11(InHandle))
	{
		ImGuiOverlayBase::Dx11Ready();
		_isInited = true;
	}
}

void ImGuiOverlayDx11::ShutdownDx11()
{
	if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX11_Shutdown();

	ImGuiOverlayBase::Shutdown();

	if (_isInited)
	{
		CleanupDeviceD3D11();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID&)oCreateSwapChain_Dx11, hkCreateSwapChain_Dx11);
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_Dx11, hkCreateSwapChainForHwnd_Dx11);
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx11, hkCreateSwapChainForCoreWindow_Dx11);
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_Dx11, hkCreateSwapChainForComposition_Dx11);

		DetourDetach(&(PVOID&)oPresent_Dx11, hkPresent_Dx11);
		DetourDetach(&(PVOID&)oPresent1_Dx11, hkPresent1_Dx11);

		DetourDetach(&(PVOID&)oResizeBuffers_Dx11, hkResizeBuffers_Dx11);
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx11, hkResizeBuffers1_Dx11);

		DetourTransactionCommit();
	}

	_isInited = false;
}

void ImGuiOverlayDx11::ReInitDx11(HWND InNewHwnd)
{
	if (!_isInited)
		return;

	spdlog::debug("ImGuiOverlayDx11::ReInitDx11 hwnd: {0:X}", (ULONG64)InNewHwnd);

	if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX11_Shutdown();

	ImGuiOverlayBase::Shutdown();
	ImGuiOverlayBase::Init(InNewHwnd);

	CleanupRenderTarget();
}
