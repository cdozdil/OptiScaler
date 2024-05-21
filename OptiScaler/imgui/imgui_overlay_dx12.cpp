#include "imgui_overlay_base.h"
#include "imgui_overlay_dx12.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

static int const NUM_BACK_BUFFERS = 4;
static IDXGIFactory4* g_dxgiFactory = nullptr;
static ID3D12Device* g_pd3dDeviceParam = nullptr;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };

typedef void(WINAPI* PFN_ExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList*);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef void* (WINAPI* PFN_ffxGetDX12SwapchainPtr)(void* ffxSwapChain);

static PFN_Present oPresent_Dx12 = nullptr;
static PFN_Present1 oPresent1_Dx12 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12 = nullptr;

static PFN_Present oPresent_SL = nullptr;
static PFN_Present1 oPresent1_SL = nullptr;
static PFN_ResizeBuffers oResizeBuffers_SL = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_SL = nullptr;

static PFN_Present oPresent_Dx12_FSR3 = nullptr;
static PFN_Present1 oPresent1_Dx12_FSR3 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12_FSR3 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12_FSR3 = nullptr;
static IDXGISwapChain3* _swapChain_FSR3 = nullptr;

static PFN_Present oPresent_Dx12_Mod = nullptr;
static PFN_Present1 oPresent1_Dx12_Mod = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12_Mod = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12_Mod = nullptr;
static IDXGISwapChain3* _swapChain_Mod = nullptr;

static PFN_ExecuteCommandLists oExecuteCommandLists_Dx12 = nullptr;

static PFN_CreateSwapChain oCreateSwapChain_Dx12 = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_Dx12 = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_Dx12 = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_Dx12 = nullptr;

static PFN_CreateSwapChain oCreateSwapChain_SL = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_SL = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_SL = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_SL = nullptr;

static PFN_ffxGetDX12SwapchainPtr offxGetDX12Swapchain_Mod = nullptr;
static PFN_ffxGetDX12SwapchainPtr offxGetDX12Swapchain_FSR3 = nullptr;

static bool _isInited = false;
static bool _changeQueue = false;
static int _changeQueueCount = 0;
static bool _showRenderImGuiDebugOnce = true;

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain);

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
	switch (eCurrentFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return eCurrentFormat;
}

static void CleanupRenderTarget()
{
	for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
	{
		if (g_mainRenderTargetResource[i])
		{
			g_mainRenderTargetResource[i]->Release();
			g_mainRenderTargetResource[i] = NULL;
		}
	}
}

// Hooks for native Dx12
static HRESULT WINAPI hkPresent_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	RenderImGui_DX12(pSwapChain);
	return oPresent_Dx12(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	RenderImGui_DX12(pSwapChain);
	return oPresent1_Dx12(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	CleanupRenderTarget();
	return oResizeBuffers_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1_Dx12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	CleanupRenderTarget();
	return oResizeBuffers1_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

// Hooks for native StreamLine
static HRESULT WINAPI hkPresent_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	HRESULT result;

	if (oPresent_SL == oPresent_Dx12)
		return oPresent_SL(pSwapChain, SyncInterval, Flags);

	RenderImGui_DX12(pSwapChain);
	return oPresent_SL(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	HRESULT result;

	if (oPresent1_SL == oPresent1_Dx12)
		return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	RenderImGui_DX12(pSwapChain);
	return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_SL(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	CleanupRenderTarget();
	return oResizeBuffers_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1_SL(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	CleanupRenderTarget();
	return oResizeBuffers1_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

// Hooks for native Native FSR3
static HRESULT WINAPI hkPresent_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	HRESULT result;

	if (oPresent_Dx12_FSR3 == oPresent_Dx12)
		return oPresent_Dx12_FSR3(pSwapChain, SyncInterval, Flags);

	if (_swapChain_FSR3)
		RenderImGui_DX12(_swapChain_FSR3);
	else
		RenderImGui_DX12(pSwapChain);

	return oPresent_Dx12_FSR3(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if (oPresent1_Dx12_FSR3 == oPresent1_Dx12)
		return oPresent1_Dx12_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (_swapChain_FSR3)
		RenderImGui_DX12(_swapChain_FSR3);
	else
		RenderImGui_DX12(pSwapChain);

	return oPresent1_Dx12_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12_FSR3(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	CleanupRenderTarget();
	return oResizeBuffers_Dx12_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	CleanupRenderTarget();
	return oResizeBuffers1_Dx12_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

// Hooks for native Native FSR3 Mods
static HRESULT WINAPI hkPresent_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (oPresent_Dx12_Mod == oPresent_Dx12)
		return oPresent_Dx12_Mod(pSwapChain, SyncInterval, Flags);

	if (_swapChain_Mod)
		RenderImGui_DX12(_swapChain_Mod);
	else
		RenderImGui_DX12(pSwapChain);

	return oPresent_Dx12_Mod(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if (oPresent1_Dx12_Mod == oPresent1_Dx12)
		return oPresent1_Dx12_Mod(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (_swapChain_Mod)
		RenderImGui_DX12(_swapChain_Mod);
	else
		RenderImGui_DX12(pSwapChain);

	return oPresent1_Dx12_Mod(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12_Mod(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	CleanupRenderTarget();
	return oResizeBuffers_Dx12_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static HRESULT WINAPI hkResizeBuffers1_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	CleanupRenderTarget();
	return oResizeBuffers1_Dx12_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

bool IsIDXGISwapChain4(void* ptr) {
	if (ptr == nullptr) {
		return false;
	}

	// Use a smart pointer to manage the queried interface
	IDXGISwapChain4* swapChain4;

	// Use structured exception handling to catch access violations
	__try {
		HRESULT hr = static_cast<IUnknown*>(ptr)->QueryInterface(IID_PPV_ARGS(&swapChain4));
		return SUCCEEDED(hr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool IsCommandQueue(void* ptr) {
	if (ptr == nullptr) {
		return false;
	}

	// Use a smart pointer to manage the queried interface
	ID3D12CommandQueue* queue;

	// Use structured exception handling to catch access violations
	__try {
		HRESULT hr = static_cast<IUnknown*>(ptr)->QueryInterface(IID_PPV_ARGS(&queue));
		return SUCCEEDED(hr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

// Hook for ffxGetDX12SwapchainPtr with FSR3 Mods
static void* WINAPI hkffxGetDX12Swapchain_Mod(void* InSwapChain)
{
	spdlog::debug("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_Mod!");

	if (oPresent_Dx12_Mod == nullptr && IsIDXGISwapChain4(InSwapChain))
	{
		void* result = nullptr;
		IDXGISwapChain3* swapChain3 = nullptr;

		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(InSwapChain);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			_swapChain_Mod = swapChain3;
			spdlog::debug("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_Mod swapchain captured: {0:X}", (unsigned long)swapChain3);

			void** pVTable = *reinterpret_cast<void***>(swapChain3);

			oPresent_Dx12_Mod = (PFN_Present)pVTable[8];
			oPresent1_Dx12_Mod = (PFN_Present1)pVTable[22];
			oResizeBuffers_Dx12_Mod = (PFN_ResizeBuffers)pVTable[13];
			oResizeBuffers1_Dx12_Mod = (PFN_ResizeBuffers1)pVTable[39];

			// Apply the detour
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			if (oPresent_Dx12_Mod != oPresent_Dx12 && oPresent_Dx12_Mod != oPresent_Dx12_FSR3)
				DetourAttach(&(PVOID&)oPresent_Dx12_Mod, hkPresent_Dx12_Mod);

			if (oPresent1_Dx12_Mod != oPresent1_Dx12 && oPresent1_Dx12_Mod != oPresent1_Dx12_FSR3)
				DetourAttach(&(PVOID&)oPresent1_Dx12_Mod, hkPresent1_Dx12_Mod);

			if (oResizeBuffers_Dx12_Mod != oResizeBuffers_Dx12 && oResizeBuffers_Dx12_Mod != oResizeBuffers_Dx12_FSR3)
				DetourAttach(&(PVOID&)oResizeBuffers_Dx12_Mod, hkResizeBuffers_Dx12_Mod);

			if (oResizeBuffers1_Dx12_Mod != oResizeBuffers1_Dx12 && oResizeBuffers1_Dx12_Mod != oResizeBuffers1_Dx12_FSR3)
				DetourAttach(&(PVOID&)oResizeBuffers1_Dx12_Mod, hkResizeBuffers1_Dx12_Mod);

			DetourTransactionCommit();

			spdlog::info("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_Mod added swapchain hooks!");

			// get result of this call
			result = offxGetDX12Swapchain_Mod(InSwapChain);

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			DetourDetach(&(PVOID&)offxGetDX12Swapchain_Mod, hkffxGetDX12Swapchain_Mod);
			offxGetDX12Swapchain_Mod = nullptr;

			DetourTransactionCommit();

			spdlog::info("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_Mod disabled hkffxGetDX12SwapchainPtr hook!");
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();

		if (offxGetDX12Swapchain_Mod == nullptr)
			return result;
	}

	return offxGetDX12Swapchain_Mod(InSwapChain);
}

// Hook for ffxGetDX12SwapchainPtr with native FSR3 
static void* WINAPI hkffxGetDX12Swapchain_FSR3(void* InSwapChain)
{
	spdlog::debug("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_FSR3!");

	if (oPresent_Dx12_FSR3 == nullptr && IsIDXGISwapChain4(InSwapChain))
	{
		void* result = nullptr;
		IDXGISwapChain3* swapChain3 = nullptr;

		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(InSwapChain);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			_swapChain_FSR3 = swapChain3;

			spdlog::debug("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_FSR3 swapchain captured: {0:X}", (unsigned long)swapChain3);

			void** pVTable = *reinterpret_cast<void***>(swapChain3);

			oPresent_Dx12_FSR3 = (PFN_Present)pVTable[8];
			oPresent1_Dx12_FSR3 = (PFN_Present1)pVTable[22];
			oResizeBuffers_Dx12_FSR3 = (PFN_ResizeBuffers)pVTable[13];
			oResizeBuffers1_Dx12_FSR3 = (PFN_ResizeBuffers1)pVTable[39];

			// Apply the detour
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			if (oPresent_Dx12_FSR3 != oPresent_Dx12 && oPresent_Dx12_FSR3 != oPresent_Dx12_Mod)
				DetourAttach(&(PVOID&)oPresent_Dx12_FSR3, hkPresent_Dx12_FSR3);

			if (oPresent1_Dx12_FSR3 != oPresent1_Dx12 && oPresent1_Dx12_FSR3 != oPresent1_Dx12_Mod)
				DetourAttach(&(PVOID&)oPresent1_Dx12_FSR3, hkPresent1_Dx12_FSR3);

			if (oResizeBuffers_Dx12_FSR3 != oResizeBuffers_Dx12 && oResizeBuffers_Dx12_FSR3 != oResizeBuffers_Dx12_Mod)
				DetourAttach(&(PVOID&)oResizeBuffers_Dx12_FSR3, hkResizeBuffers_Dx12_FSR3);

			if (oResizeBuffers1_Dx12_FSR3 != oResizeBuffers1_Dx12 && oResizeBuffers1_Dx12_FSR3 != oResizeBuffers1_Dx12_Mod)
				DetourAttach(&(PVOID&)oResizeBuffers1_Dx12_FSR3, hkResizeBuffers1_Dx12_FSR3);

			DetourTransactionCommit();

			spdlog::info("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_FSR3 added swapchain hooks!");

			// get result of this call
			result = offxGetDX12Swapchain_FSR3(InSwapChain);

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			DetourDetach(&(PVOID&)offxGetDX12Swapchain_FSR3, hkffxGetDX12Swapchain_FSR3);
			offxGetDX12Swapchain_FSR3 = nullptr;
			DetourTransactionCommit();

			spdlog::info("imgui_overlay_dx12::hkffxGetDX12SwapchainPtr_FSR3 disabled hkffxGetDX12SwapchainPtr_FSR3 hook!");
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();

		if (offxGetDX12Swapchain_FSR3 == nullptr)
			return result;
	}

	return offxGetDX12Swapchain_FSR3(InSwapChain);
}

static void WINAPI hkExecuteCommandLists_Dx12(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
{

	if (_changeQueue)
		_changeQueueCount++;

	if (g_pd3dCommandQueue == nullptr || (_changeQueue && (g_pd3dCommandQueue != pCommandQueue || _changeQueueCount > 10)))
	{
		if (g_pd3dCommandQueue != nullptr)
			spdlog::info("hkExecuteCommandLists_Dx12 new g_pd3dCommandQueue: {0:X} -> {0:X}", (unsigned long)g_pd3dCommandQueue, (unsigned long)pCommandQueue);
		else
			spdlog::info("hkExecuteCommandLists_Dx12 new g_pd3dCommandQueue: {0:X}", (unsigned long)pCommandQueue);

		g_pd3dCommandQueue = pCommandQueue;
		_changeQueue = false;
	}

	return oExecuteCommandLists_Dx12(pCommandQueue, NumCommandLists, ppCommandLists);
}

// Hook for DXGIFactory for native DX12
static HRESULT WINAPI hkCreateSwapChain_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChain_Dx12(pFactory, pDevice, pDesc, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForHwnd_Dx12(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForCoreWindow_Dx12(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForComposition_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForComposition_Dx12(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

// Hook for DXGIFactory for native StreamLine
static HRESULT WINAPI hkCreateSwapChain_SL(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChain_SL(pFactory, pDevice, pDesc, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_SL(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForHwnd_SL(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_SL(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForCoreWindow_SL(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

static HRESULT WINAPI hkCreateSwapChainForComposition_SL(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget();
	return oCreateSwapChainForComposition_SL(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}

static bool CreateDeviceD3D12(HWND InHWnd, ID3D12Device* InDevice)
{
	spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Handle: {0:X}", (unsigned long)InHWnd);

	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd = { };
	sd.BufferCount = NUM_BACK_BUFFERS;
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	bool isUniscaler = false;

	HRESULT result;

	// Create D3D12 device if needed
	if (!InDevice)
	{
		// Create device
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		// check for sl.interposer
		PFN_D3D12_CREATE_DEVICE slCD = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("sl.interposer.dll", "D3D12CreateDevice");

		if (slCD != nullptr)
			spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 sl.interposer.dll D3D12CreateDevice found");

		if (slCD != nullptr && Config::Instance()->NVNGX_Engine != NVSDK_NGX_ENGINE_TYPE_UNREAL && Config::Instance()->HookSLDevice.value_or(false))
			result = slCD(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice));
		else
			result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice));

		if (result != S_OK)
		{
			spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 D3D12CreateDevice: {0:X}", (unsigned long)result);
			return false;
		}
	}
	else
	{
		g_pd3dDevice = InDevice;
	}

	// Create queue
	D3D12_COMMAND_QUEUE_DESC desc = { };
	result = g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue));
	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateCommandQueue: {0:X}", (unsigned long)result);
		return false;
	}

	if (g_pd3dCommandQueue != nullptr)
	{
		void** pCommandQueueVTable = *reinterpret_cast<void***>(g_pd3dCommandQueue);

		oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

		if (oExecuteCommandLists_Dx12 != nullptr)
		{
			spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 hooking SL CommandQueue");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

			DetourTransactionCommit();
		}
	}

	// Check for FSR3-FG
	if (Config::Instance()->HookFSR3Proxy.value_or(true))
	{
		// Check for Uniscaler
		offxGetDX12Swapchain_Mod = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("Uniscaler.asi", "ffxGetSwapchainDX12");

		if (offxGetDX12Swapchain_Mod != nullptr)
		{
			spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Uniscaler's ffxGetDX12SwapchainPtr found");
			isUniscaler = true;
		}
		else
		{
			// Check for Nukem's
			offxGetDX12Swapchain_Mod = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("dlssg_to_fsr3_amd_is_better.dll", "ffxGetSwapchainDX12");

			if (offxGetDX12Swapchain_Mod)
				spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Nukem's ffxGetDX12SwapchainPtr found");
		}

		// Check for native FSR3 (if game has native FSR3-FG)
		offxGetDX12Swapchain_FSR3 = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("ffx_backend_dx12_x64.dll", "ffxGetSwapchainDX12");

		if (offxGetDX12Swapchain_FSR3)
			spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 FSR3's offxGetDX12SwapchainPtr found");

		// Hook FSR3 ffxGetDX12SwapchainPtr methods
		if (offxGetDX12Swapchain_Mod != nullptr || offxGetDX12Swapchain_FSR3 != nullptr)
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			if (offxGetDX12Swapchain_Mod != nullptr)
				DetourAttach(&(PVOID&)offxGetDX12Swapchain_Mod, hkffxGetDX12Swapchain_Mod);

			if (offxGetDX12Swapchain_FSR3 != nullptr)
				DetourAttach(&(PVOID&)offxGetDX12Swapchain_FSR3, hkffxGetDX12Swapchain_FSR3);

			DetourTransactionCommit();
		}
	}

	if (!isUniscaler)
	{
		// Try to create SL DXGIFactory
		if (Config::Instance()->HookSLProxy.value_or(true))
		{
			// Check for sl.interposer
			PFN_CreateDXGIFactory1 slFactory = (PFN_CreateDXGIFactory1)DetourFindFunction("sl.interposer.dll", "CreateDXGIFactory1");

			if (slFactory != nullptr)
				spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 sl.interposer.dll CreateDXGIFactory1 found");

			IDXGISwapChain1* swapChain1 = nullptr;

			if (slFactory != nullptr)
			{
				result = slFactory(IID_PPV_ARGS(&g_dxgiFactory));

				if (result != S_OK)
				{
					spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 SL CreateDXGIFactory1: {0:X}", (unsigned long)result);
					return false;
				}

				void** pFactoryVTable = *reinterpret_cast<void***>(g_dxgiFactory);

				oCreateSwapChain_SL = (PFN_CreateSwapChain)pFactoryVTable[10];
				oCreateSwapChainForHwnd_SL = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
				oCreateSwapChainForCoreWindow_SL = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
				oCreateSwapChainForComposition_SL = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

				if (oCreateSwapChain_SL != nullptr)
				{
					spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 hooking SL DXGIFactory");

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());

					DetourAttach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);
					DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);
					DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);
					DetourAttach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);

					DetourTransactionCommit();
				}

				// Hook DXGI Factory 
				if (g_dxgiFactory != nullptr && g_pd3dCommandQueue != nullptr)
				{
					// Create SwapChain
					result = g_dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, InHWnd, &sd, NULL, NULL, &swapChain1);
					if (result != S_OK)
					{
						spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 SL CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
						return false;
					}

					result = swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
					if (result != S_OK)
					{
						spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 SL QueryInterface: {0:X}", (unsigned long)result);
						return false;
					}

					swapChain1->Release();
					swapChain1 = nullptr;
				}

				if (g_pSwapChain != nullptr)
				{
					void** pVTable = *reinterpret_cast<void***>(g_pSwapChain);

					oPresent_SL = (PFN_Present)pVTable[8];
					oPresent1_SL = (PFN_Present1)pVTable[22];

					oResizeBuffers_SL = (PFN_ResizeBuffers)pVTable[13];
					oResizeBuffers1_SL = (PFN_ResizeBuffers1)pVTable[39];

					if (oPresent_SL != nullptr)
					{
						spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 hooking SL SwapChain");

						DetourTransactionBegin();
						DetourUpdateThread(GetCurrentThread());

						DetourAttach(&(PVOID&)oPresent_SL, hkPresent_SL);
						DetourAttach(&(PVOID&)oPresent1_SL, hkPresent1_SL);

						DetourAttach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);
						DetourAttach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);

						DetourTransactionCommit();
					}
				}
			}
		}

		// Try to create native DXGIFactory
		if (oCreateSwapChain_SL == nullptr && oPresent_SL == nullptr && Config::Instance()->HookD3D12.value_or(true))
		{
			if (g_dxgiFactory != nullptr)
			{
				g_dxgiFactory->Release();
				g_dxgiFactory = nullptr;
			}

			IDXGISwapChain1* swapChain1 = nullptr;

			spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Creating DXGIFactory for hooking");
			result = CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgiFactory));

			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateDXGIFactory1: {0:X}", (unsigned long)result);
				return false;
			}

			void** pFactoryVTable = *reinterpret_cast<void***>(g_dxgiFactory);

			oCreateSwapChain_Dx12 = (PFN_CreateSwapChain)pFactoryVTable[10];
			oCreateSwapChainForHwnd_Dx12 = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
			oCreateSwapChainForCoreWindow_Dx12 = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
			oCreateSwapChainForComposition_Dx12 = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

			if (oCreateSwapChain_Dx12 != nullptr)
			{
				spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Hooking native DXGIFactory");

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				DetourAttach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);
				DetourTransactionCommit();
			}

			// Hook DXGI Factory 
			if (g_dxgiFactory != nullptr && g_pd3dCommandQueue != nullptr)
			{
				// Create SwapChain
				result = g_dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, InHWnd, &sd, NULL, NULL, &swapChain1);
				if (result != S_OK)
				{
					spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
					return false;
				}

				if (g_pSwapChain != nullptr)
				{
					g_pSwapChain->Release();
					g_pSwapChain = nullptr;
				}

				result = swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
				if (result != S_OK)
				{
					spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 QueryInterface: {0:X}", (unsigned long)result);
					return false;
				}

				swapChain1->Release();
			}

			if (g_pSwapChain != nullptr)
			{
				void** pVTable = *reinterpret_cast<void***>(g_pSwapChain);

				oPresent_Dx12 = (PFN_Present)pVTable[8];
				oPresent1_Dx12 = (PFN_Present1)pVTable[22];

				oResizeBuffers_Dx12 = (PFN_ResizeBuffers)pVTable[13];
				oResizeBuffers1_Dx12 = (PFN_ResizeBuffers1)pVTable[39];

				if (oPresent_Dx12 != nullptr)
				{
					spdlog::info("imgui_overlay_dx12::CreateDeviceD3D12 Hooking native SwapChain");

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());

					DetourAttach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
					DetourAttach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);

					DetourAttach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
					DetourAttach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);

					DetourTransactionCommit();
				}
			}
		}
	}

	if (g_pd3dCommandQueue != nullptr)
	{
		g_pd3dCommandQueue->Release();
		g_pd3dCommandQueue = nullptr;
	}

	return true;
}

static void CreateRenderTarget(IDXGISwapChain* pSwapChain)
{
	spdlog::info("imgui_overlay_dx12::CreateRenderTarget");

	DXGI_SWAP_CHAIN_DESC desc;
	HRESULT hr = pSwapChain->GetDesc(&desc);

	if (hr != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateRenderTarget pSwapChain->GetDesc: {0:X}", (unsigned long)hr);
		return;
	}

	for (UINT i = 0; i < desc.BufferCount; ++i)
	{
		ID3D12Resource* pBackBuffer = NULL;

		auto result = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));

		if (result != S_OK)
		{
			spdlog::error("imgui_overlay_dx12::CreateRenderTarget pSwapChain->GetBuffer: {0:X}", (unsigned long)result);
			return;
		}

		if (pBackBuffer)
		{
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);

			D3D12_RENDER_TARGET_VIEW_DESC desc = { };
			desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			g_pd3dDevice->CreateRenderTargetView(pBackBuffer, &desc, g_mainRenderTargetDescriptor[i]);
			g_mainRenderTargetResource[i] = pBackBuffer;
		}
	}

	spdlog::info("imgui_overlay_dx12::CreateRenderTarget done!");
}

static void CleanupDeviceD3D12(ID3D12Device* InDevice)
{
	CleanupRenderTarget();

	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		g_pSwapChain = NULL;
	}

	for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
	{
		if (g_commandAllocators[i])
		{
			g_commandAllocators[i]->Release();
			g_commandAllocators[i] = NULL;
		}
	}

	if (g_pd3dCommandList)
	{
		g_pd3dCommandList->Release();
		g_pd3dCommandList = NULL;
	}

	if (g_pd3dRtvDescHeap)
	{
		g_pd3dRtvDescHeap->Release();
		g_pd3dRtvDescHeap = NULL;
	}

	if (g_pd3dSrvDescHeap)
	{
		g_pd3dSrvDescHeap->Release();
		g_pd3dSrvDescHeap = NULL;
	}

	if (g_pd3dDevice != InDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}

	if (g_dxgiFactory)
	{
		g_dxgiFactory->Release();
		g_dxgiFactory = NULL;
	}
}

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain)
{
	if (Config::Instance()->CurrentFeature != nullptr &&
		Config::Instance()->CurrentFeature->FrameCount() <= Config::Instance()->MenuInitDelay.value_or(90) + 10)
	{
		return;
	}

	if (!ImGuiOverlayBase::IsInited())
		ImGuiOverlayBase::Init(Util::GetProcessWindow());

	if (ImGuiOverlayBase::IsInited() && ImGuiOverlayBase::IsResetRequested())
	{
		auto hwnd = Util::GetProcessWindow();
		spdlog::info("RenderImGui_DX12 Reset request detected, resetting ImGui, new handle {0:X}", (unsigned long)hwnd);
		ImGuiOverlayDx12::ReInitDx12(hwnd);
		return;
	}

	if (!ImGuiOverlayBase::IsInited() || !ImGuiOverlayBase::IsVisible())
		return;

	if (!ImGui::GetIO().BackendRendererUserData)
	{
		spdlog::debug("imgui_overlay_dx12::RenderImGui_DX12 ImGui::GetIO().BackendRendererUserData == nullptr");

		auto result = pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice));
		if (result != S_OK)
		{
			spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 GetDevice: {0:X}", (unsigned long)result);
			return;
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = { };
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = NUM_BACK_BUFFERS;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;

			result = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dRtvDescHeap): {0:X}", (unsigned long)result);
				return;
			}

			SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

			for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
			{
				g_mainRenderTargetDescriptor[i] = rtvHandle;
				rtvHandle.ptr += rtvDescriptorSize;
			}
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = { };
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			result = g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dSrvDescHeap): {0:X}", (unsigned long)result);
				return;
			}
		}

		for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
		{
			result = g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i]));

			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 CreateCommandAllocator[{0}]: {1:X}", i, (unsigned long)result);
				return;
			}
		}

		result = g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList));
		if (result != S_OK)
		{
			spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 CreateCommandList: {0:X}", (unsigned long)result);
			return;
		}

		result = g_pd3dCommandList->Close();
		if (result != S_OK)
		{
			spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
			return;
		}

		ImGui_ImplDX12_Init(g_pd3dDevice, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
			g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
	}

	if (_isInited)
	{
		if (!g_mainRenderTargetResource[0])
		{
			CreateRenderTarget(pSwapChain);
			return;
		}

		if (ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])
		{
			_showRenderImGuiDebugOnce = true;

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGuiOverlayBase::RenderMenu();

			ImGui::Render();


			UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
			ID3D12CommandAllocator* commandAllocator = g_commandAllocators[backBufferIdx];

			auto result = commandAllocator->Reset();
			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 commandAllocator->Reset: {0:X}", (unsigned long)result);
				return;
			}

			D3D12_RESOURCE_BARRIER barrier = { };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			result = g_pd3dCommandList->Reset(commandAllocator, nullptr);
			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 g_pd3dCommandList->Reset: {0:X}", (unsigned long)result);
				return;
			}

			g_pd3dCommandList->ResourceBarrier(1, &barrier);
			g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
			g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			g_pd3dCommandList->ResourceBarrier(1, &barrier);

			result = g_pd3dCommandList->Close();
			if (result != S_OK)
			{
				spdlog::error("imgui_overlay_dx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
				return;
			}

			ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };

			if (oExecuteCommandLists_Dx12 != nullptr)
				oExecuteCommandLists_Dx12(g_pd3dCommandQueue, 1, (ID3D12CommandList*)ppCommandLists);
			else
				g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

			spdlog::trace("RenderImGui_DX12 ExecuteCommandLists done!");
		}
		else
		{
			if (_showRenderImGuiDebugOnce)
				spdlog::info("RenderImGui_DX12 !(ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])");

			_showRenderImGuiDebugOnce = false;
		}
	}
}

void DeatachAllHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (oPresent_Dx12_Mod)
	{
		DetourDetach(&(PVOID&)oPresent_Dx12_Mod, hkPresent_Dx12_Mod);
		oPresent_Dx12_Mod = nullptr;
	}

	if (oPresent1_Dx12_Mod)
	{
		DetourDetach(&(PVOID&)oPresent1_Dx12_Mod, hkPresent1_Dx12_Mod);
		oPresent1_Dx12_Mod = nullptr;
	}

	if (oResizeBuffers_Dx12_Mod)
	{
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12_Mod, hkResizeBuffers_Dx12_Mod);
		oResizeBuffers_Dx12_Mod = nullptr;
	}

	if (oResizeBuffers1_Dx12_Mod)
	{
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12_Mod, hkResizeBuffers1_Dx12_Mod);
		oResizeBuffers1_Dx12_Mod = nullptr;
	}

	if (oPresent_Dx12_FSR3)
	{
		DetourDetach(&(PVOID&)oPresent_Dx12_FSR3, hkPresent_Dx12_FSR3);
		oPresent_Dx12_FSR3 = nullptr;
	}

	if (oPresent1_Dx12_FSR3)
	{
		DetourDetach(&(PVOID&)oPresent1_Dx12_FSR3, hkPresent1_Dx12_FSR3);
		oPresent1_Dx12_FSR3 = nullptr;
	}

	if (oResizeBuffers_Dx12_FSR3)
	{
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12_FSR3, hkResizeBuffers_Dx12_FSR3);
		oResizeBuffers_Dx12_FSR3 = nullptr;
	}

	if (oResizeBuffers1_Dx12_FSR3)
	{
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12_FSR3, hkResizeBuffers1_Dx12_FSR3);
		oResizeBuffers1_Dx12_FSR3 = nullptr;
	}

	if (offxGetDX12Swapchain_Mod)
	{
		DetourDetach(&(PVOID&)offxGetDX12Swapchain_Mod, hkffxGetDX12Swapchain_Mod);
		offxGetDX12Swapchain_Mod = nullptr;
	}

	if (offxGetDX12Swapchain_FSR3)
	{
		DetourDetach(&(PVOID&)offxGetDX12Swapchain_FSR3, hkffxGetDX12Swapchain_FSR3);
		offxGetDX12Swapchain_FSR3 = nullptr;
	}

	if (oCreateSwapChain_SL)
	{
		DetourDetach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);
		oCreateSwapChain_SL = nullptr;
	}

	if (oCreateSwapChainForHwnd_SL)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);
		oCreateSwapChainForHwnd_SL = nullptr;
	}

	if (oCreateSwapChainForCoreWindow_SL)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);
		oCreateSwapChainForCoreWindow_SL = nullptr;
	}

	if (oCreateSwapChainForComposition_SL)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);
		oCreateSwapChainForComposition_SL = nullptr;
	}

	if (oExecuteCommandLists_Dx12)
	{
		DetourDetach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);
		oExecuteCommandLists_Dx12 = nullptr;
	}

	if (oPresent_SL)
	{
		DetourDetach(&(PVOID&)oPresent_SL, hkPresent_SL);
		oPresent_SL = nullptr;
	}

	if (oPresent1_SL)
	{
		DetourDetach(&(PVOID&)oPresent1_SL, hkPresent1_SL);
		oPresent1_SL = nullptr;
	}

	if (oResizeBuffers_SL)
	{
		DetourDetach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);
		oResizeBuffers_SL = nullptr;
	}

	if (oResizeBuffers1_SL)
	{
		DetourDetach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);
		oResizeBuffers1_SL = nullptr;
	}

	if (oCreateSwapChain_Dx12)
	{
		DetourDetach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
		oCreateSwapChain_Dx12 = nullptr;
	}

	if (oCreateSwapChainForHwnd_Dx12)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
		oCreateSwapChainForHwnd_Dx12 = nullptr;
	}

	if (oCreateSwapChainForCoreWindow_Dx12)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
		oCreateSwapChainForCoreWindow_Dx12 = nullptr;
	}

	if (oCreateSwapChainForComposition_Dx12)
	{
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);
		oCreateSwapChainForComposition_Dx12 = nullptr;
	}

	if (oExecuteCommandLists_Dx12)
	{
		DetourDetach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);
		oExecuteCommandLists_Dx12 = nullptr;
	}

	if (oPresent_Dx12)
	{
		DetourDetach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
		oPresent_Dx12 = nullptr;
	}

	if (oPresent1_Dx12)
	{
		DetourDetach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);
		oPresent1_Dx12 = nullptr;
	}

	if (oResizeBuffers_Dx12)
	{
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
		oResizeBuffers_Dx12 = nullptr;
	}

	if (oResizeBuffers1_Dx12)
	{
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);
		oResizeBuffers1_Dx12 = nullptr;
	}

	DetourTransactionCommit();
}

bool ImGuiOverlayDx12::IsInitedDx12()
{
	return _isInited;
}

HWND ImGuiOverlayDx12::Handle()
{
	return ImGuiOverlayBase::Handle();
}

void ImGuiOverlayDx12::InitDx12(HWND InHandle, ID3D12Device* InDevice)
{
	spdlog::info("RenderImGui_DX12 InitDx12 Handle: {0:X}", (unsigned long)InHandle);

	g_pd3dDeviceParam = InDevice;

	if (g_pd3dDevice == nullptr && CreateDeviceD3D12(InHandle, InDevice))
	{
		_isInited = true;
	}
	else
	{
		spdlog::error("imgui_overlay_dx12::InitDx12 CreateDeviceD3D12 failed!");
	}

	CleanupDeviceD3D12(InDevice);
}

void ImGuiOverlayDx12::ShutdownDx12()
{
	if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX12_Shutdown();

	ImGuiOverlayBase::Shutdown();

	if (_isInited)
	{
		CleanupDeviceD3D12(g_pd3dDeviceParam);

		DeatachAllHooks();
	}

	_isInited = false;
}

void ImGuiOverlayDx12::ReInitDx12(HWND InNewHwnd)
{
	if (!_isInited)
		return;

	spdlog::info("ImGuiOverlayDx12::ReInitDx12 hwnd: {0:X}", (unsigned long)InNewHwnd);

	if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX12_Shutdown();

	ImGuiOverlayBase::Shutdown();
	ImGuiOverlayBase::Init(InNewHwnd);

	CleanupRenderTarget();

	_changeQueue = true;
	_changeQueueCount = 0;
}

