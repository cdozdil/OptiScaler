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

#include "wrapped_swapchain.h"
#include "wrapped_command_queue.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

static ID3D12Device* g_pd3dDeviceParam = nullptr;

static int const NUM_BACK_BUFFERS = 4;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };

typedef void(WINAPI* PFN_ExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList*);
typedef ULONG(WINAPI* PFN_Release)(IUnknown*);

typedef void* (WINAPI* PFN_ffxGetDX12SwapchainPtr)(void* ffxSwapChain);
typedef HRESULT(WINAPI* PFN_CreateCommandQueue)(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue);
typedef HRESULT(WINAPI* PFN_CreateDXGIFactory1)(REFIID riid, void** ppFactory);

// Dx12
static PFN_Present oPresent_Dx12 = nullptr;
static PFN_Present1 oPresent1_Dx12 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12 = nullptr;

// Dx12 early binding
static PFN_D3D12_CREATE_DEVICE oD3D12CreateDevice = nullptr;
static PFN_CreateCommandQueue oCreateCommandQueue = nullptr;
static PFN_CreateDXGIFactory1 oCreateDXGIFactory1 = nullptr;
static PFN_Release oCommandQueueRelease = nullptr;

// Streamline
static PFN_Present oPresent_SL = nullptr;
static PFN_Present1 oPresent1_SL = nullptr;
static PFN_ResizeBuffers oResizeBuffers_SL = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_SL = nullptr;

// Native FSR3
static PFN_Present oPresent_Dx12_FSR3 = nullptr;
static PFN_Present1 oPresent1_Dx12_FSR3 = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12_FSR3 = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12_FSR3 = nullptr;

// Mod FSR3
static PFN_Present oPresent_Dx12_Mod = nullptr;
static PFN_Present1 oPresent1_Dx12_Mod = nullptr;
static PFN_ResizeBuffers oResizeBuffers_Dx12_Mod = nullptr;
static PFN_ResizeBuffers1 oResizeBuffers1_Dx12_Mod = nullptr;

// Dx12
static PFN_ExecuteCommandLists oExecuteCommandLists_Dx12 = nullptr;

// Streamline 
static PFN_ExecuteCommandLists oExecuteCommandLists_SL = nullptr;

// Dx12 Late Binding
static PFN_CreateSwapChain oCreateSwapChain_Dx12 = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_Dx12 = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_Dx12 = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_Dx12 = nullptr;

// Dx12 early binding
static PFN_CreateSwapChain oCreateSwapChain_EB = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_EB = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_EB = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_EB = nullptr;

// Streamline
static PFN_CreateSwapChain oCreateSwapChain_SL = nullptr;
static PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_SL = nullptr;
static PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_SL = nullptr;
static PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_SL = nullptr;

// Mod FSR3
static PFN_ffxGetDX12SwapchainPtr offxGetDX12Swapchain_Mod = nullptr;

// Native FSR3
static PFN_ffxGetDX12SwapchainPtr offxGetDX12Swapchain_FSR3 = nullptr;

static bool _isInited = false;
static bool _reInit = false;
static bool _dx12BindingActive = false; // To prevent wrapping of swapchains created for hook by mod

static bool _isEarlyBind = false;	// if dx12 calls loaded early (non native nvngx mode)

static bool _usingNative = false;	// Native Dx12 present received a call
static bool _usingDLSSG = false;	// Streamline presenct received a call
static bool _usingFSR3_Native = false;	// FSR3 native present received a call
static bool _usingFSR3_Mod = false;	// FSR3 mod received present call

/*
* From what I've observed priotry order is
* 1. FSR3 Native
* 2. FSR3 Mod (only if Uniscaler is active)
* 3. Streamline (For Nukem & native SL)
* 4. Native (For non-fg situations)
*/

static bool _bindedNative = false;	// if native dx12 calls detoured
static bool _bindedDLSSG = false;	// if streamline calls detoured
static bool _bindedFSR3_Native = false;		// if fsr3 native calls detoured
static bool _bindedFSR3_Uniscaler = false;	// if Uniscaler calls detoured
static bool _bindedFSR3_Nukem = false;	// if Nukem's calls detoured
static bool _bindedFSR3_Mod = false;	// if fsr3 mod calls detoured (_bindedFSR3_Uniscaler || _bindedFSR3_Nukem)

// Optiscaler will try to capture right commandqueue for each possible swapchain
static ID3D12CommandQueue* _cqFsr3Mod = nullptr;	// Not used
static ID3D12CommandQueue* _cqFsr3Native = nullptr;	// Not used
static ID3D12CommandQueue* _cqSL = nullptr;	// Not used
static ID3D12CommandQueue* _cqDx12 = nullptr;

// for showing 
static bool _showRenderImGuiDebugOnce = true;

// FFXModHookCounter for disabling to prevent 
static int _changeFFXModHookCounter_Mod = 0;
static int _changeFFXModHookCounter_Native = 0;

enum RenderSource
{
	Unknown,
	Dx12,
	SL,
	FSR3_Mod,
	FSR3_Native
};

const std::string renderSourceNames[] = { "Unknown", "Dx12", "SL", "FSR3 Mod", "FSR3 Native" };

static RenderSource _lastActiveSource = Unknown;

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain);
static bool BindAll(HWND InHWnd, ID3D12Device* InDevice);
static void DeatachAllHooks(bool reInit);
static void CleanupRenderTarget(bool clearQueue);

static bool IsActivePath(RenderSource source, bool justQuery = false)
{
	bool result = false;

	switch (source)
	{
	case Dx12:
		result = !IsActivePath(SL, true) && !IsActivePath(FSR3_Mod, true) && !IsActivePath(FSR3_Native, true) && _usingNative && _cqDx12 != nullptr;

		if (result)
			g_pd3dCommandQueue = _cqDx12;

		break;

	case SL:
		result = !IsActivePath(FSR3_Native) && _usingDLSSG && (_cqSL != nullptr || _cqDx12 != nullptr);

		if (result)
			g_pd3dCommandQueue = _cqDx12;

		break;

	case FSR3_Mod:
		result = !IsActivePath(FSR3_Native) && _usingFSR3_Mod && _cqDx12 != nullptr;

		if (result)
			g_pd3dCommandQueue = _cqDx12;

		break;

	case FSR3_Native:
		result = _usingFSR3_Native && _cqDx12 != nullptr;

		if (result)
			g_pd3dCommandQueue = _cqDx12;

		break;

	}

	if (!justQuery && result && _lastActiveSource != source)
	{
		spdlog::info("ImGuiOverlayDx12::IsActivePath changed from {0} to {1}!", renderSourceNames[(int)_lastActiveSource], renderSourceNames[(int)source]);
		CleanupRenderTarget(true);
		_lastActiveSource = source;
	}

	return result;
}

static bool IsBeingUsed(IDXGISwapChain1* InSwapchain)
{
	HWND hwnd = nullptr;
	InSwapchain->GetHwnd(&hwnd);
	return hwnd == ImGuiOverlayBase::Handle();
}

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
	switch (eCurrentFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return eCurrentFormat;
}

static void CleanupRenderTarget(bool clearQueue)
{
	spdlog::debug("ImGuiOverlayDx12::CleanupRenderTarget({0})!", clearQueue);

	for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
	{
		if (g_mainRenderTargetResource[i])
		{
			g_mainRenderTargetResource[i]->Release();
			g_mainRenderTargetResource[i] = NULL;
		}
	}

	if (clearQueue)
	{
		if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
			ImGui_ImplDX12_Shutdown();

		if (g_pd3dRtvDescHeap != nullptr)
		{
			g_pd3dRtvDescHeap->Release();
			g_pd3dRtvDescHeap = nullptr;

		}

		if (g_pd3dSrvDescHeap != nullptr)
		{
			g_pd3dSrvDescHeap->Release();
			g_pd3dSrvDescHeap = nullptr;

		}

		for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
		{
			if (g_commandAllocators[i] != nullptr)
			{
				g_commandAllocators[i]->Release();
				g_commandAllocators[i] = nullptr;
			}
		}

		if (g_pd3dCommandList != nullptr)
		{
			g_pd3dCommandList->Release();
			g_pd3dCommandList = nullptr;
		}
	}

	_lastActiveSource = Unknown;

	_changeFFXModHookCounter_Mod = 0;
	_changeFFXModHookCounter_Native = 0;

	_usingNative = false;
	_usingDLSSG = false;
	_usingFSR3_Native = false;
	_usingFSR3_Mod = false;
}

#pragma region Hooks for native EB Prensent Methods

static HRESULT WINAPI hkPresent_EB(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
		return S_OK;

	if (IsActivePath(Dx12))
		RenderImGui_DX12(pSwapChain);

	_usingNative = IsBeingUsed(pSwapChain);

	return S_OK;
}

static HRESULT WINAPI hkPresent1_EB(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
		return S_OK;

	if (IsActivePath(Dx12))
		RenderImGui_DX12(pSwapChain);

	_usingNative = IsBeingUsed(pSwapChain);

	return S_OK;
}

#pragma endregion

#pragma region Hooks for native Dx12 Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
		return oPresent_Dx12(pSwapChain, SyncInterval, Flags);

	if (IsActivePath(Dx12))
		RenderImGui_DX12(pSwapChain);

	_usingNative = IsBeingUsed(pSwapChain);

	return oPresent_Dx12(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
		return oPresent1_Dx12(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (IsActivePath(Dx12))
		RenderImGui_DX12(pSwapChain);

	_usingNative = IsBeingUsed(pSwapChain);

	return oPresent1_Dx12(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto result = oResizeBuffers_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (!_isEarlyBind && IsActivePath(Dx12))
		CleanupRenderTarget(false);

	return result;
}

static HRESULT WINAPI hkResizeBuffers1_Dx12(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	auto result = oResizeBuffers1_Dx12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

	if (!_isEarlyBind && IsActivePath(Dx12))
		CleanupRenderTarget(false);

	return result;
}

#pragma endregion

#pragma region Hooks for native StreamLine Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
		return oPresent_SL(pSwapChain, SyncInterval, Flags);

	if (IsActivePath(SL))
		RenderImGui_DX12(pSwapChain);

	_usingDLSSG = IsBeingUsed(pSwapChain);

	return oPresent_SL(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_SL(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
		return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (IsActivePath(SL))
		RenderImGui_DX12(pSwapChain);

	_usingDLSSG = IsBeingUsed(pSwapChain);

	return oPresent1_SL(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_SL(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto result = oResizeBuffers_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (!_isEarlyBind && IsActivePath(SL))
		CleanupRenderTarget(false);

	return result;
}

static HRESULT WINAPI hkResizeBuffers1_SL(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	auto result = oResizeBuffers1_SL(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

	if (!_isEarlyBind && IsActivePath(SL))
		CleanupRenderTarget(false);

	return result;
}

#pragma endregion

#pragma region Hooks for Native FSR3 Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
		return oPresent_Dx12_FSR3(pSwapChain, SyncInterval, Flags);

	if (IsActivePath(FSR3_Native))
		RenderImGui_DX12(pSwapChain);

	_usingFSR3_Native = IsBeingUsed(pSwapChain);

	return oPresent_Dx12_FSR3(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
		return oPresent1_Dx12_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (IsActivePath(FSR3_Native))
		RenderImGui_DX12(pSwapChain);

	_usingFSR3_Native = IsBeingUsed(pSwapChain);

	return oPresent1_Dx12_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12_FSR3(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto result = oResizeBuffers_Dx12_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (!_isEarlyBind && IsActivePath(FSR3_Native))
		CleanupRenderTarget(false);

	return result;
}

static HRESULT WINAPI hkResizeBuffers1_Dx12_FSR3(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	auto result = oResizeBuffers1_Dx12_FSR3(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

	if (!_isEarlyBind && IsActivePath(FSR3_Native))
		CleanupRenderTarget(false);

	return result;
}

#pragma endregion

#pragma region Hooks for FSR3 Mods Present & Resize Buffer methods

static HRESULT WINAPI hkPresent_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if ((Flags & DXGI_PRESENT_TEST) || (Flags & DXGI_PRESENT_RESTART))
		return oPresent_Dx12_FSR3(pSwapChain, SyncInterval, Flags);

	if (IsActivePath(FSR3_Mod))
		RenderImGui_DX12(pSwapChain);

	_usingFSR3_Mod = IsBeingUsed(pSwapChain);

	return oPresent_Dx12_Mod(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI hkPresent1_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	if ((PresentFlags & DXGI_PRESENT_TEST) || (PresentFlags & DXGI_PRESENT_RESTART))
		return oPresent1_Dx12_FSR3(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);

	if (IsActivePath(FSR3_Mod))
		RenderImGui_DX12(pSwapChain);

	_usingFSR3_Mod = IsBeingUsed(pSwapChain);

	return oPresent1_Dx12_Mod(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}

static HRESULT WINAPI hkResizeBuffers_Dx12_Mod(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	auto result = oResizeBuffers_Dx12_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	if (!_isEarlyBind && IsActivePath(FSR3_Mod))
		CleanupRenderTarget(false);

	return result;
}

static HRESULT WINAPI hkResizeBuffers1_Dx12_Mod(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
	UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	auto result = oResizeBuffers1_Dx12_Mod(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags, pCreationNodeMask, ppPresentQueue);

	if (!_isEarlyBind && IsActivePath(FSR3_Mod))
		CleanupRenderTarget(false);

	return result;
}

#pragma endregion

#pragma region Hook for ffxGetDX12Swapchain

bool IsIDXGISwapChain4(void* ptr) {
	if (ptr == nullptr) {
		return false;
	}

	// Use a smart pointer to manage the queried interface
	IDXGISwapChain4* swapChain4 = nullptr;

	// Use structured exception handling to catch access violations
	__try {
		HRESULT hr = static_cast<IUnknown*>(ptr)->QueryInterface(IID_PPV_ARGS(&swapChain4));

		if (swapChain4 != nullptr)
		{
			swapChain4->Release();
			swapChain4 = nullptr;
		}

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
	ID3D12CommandQueue* queue = nullptr;

	// Use structured exception handling to catch access violations
	__try {
		HRESULT hr = static_cast<IUnknown*>(ptr)->QueryInterface(IID_PPV_ARGS(&queue));

		if (queue != nullptr)
		{
			queue->Release();
			queue = nullptr;
		}

		return SUCCEEDED(hr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

// Hook for ffxGetDX12SwapchainPtr for Mod's 
static void* WINAPI hkffxGetDX12Swapchain_Mod(void* InParam)
{
	if (_changeFFXModHookCounter_Mod > 1000 || oPresent_Dx12_Mod != nullptr)
		return offxGetDX12Swapchain_Mod(InParam);

	spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_Mod ({0})!", _changeFFXModHookCounter_Mod);

	void* result = nullptr;

	if (oPresent_Dx12_Mod == nullptr && IsIDXGISwapChain4(InParam))
	{
		IDXGISwapChain3* swapChain3 = nullptr;

		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(InParam);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_Mod swapchain captured: {0:X}", (unsigned long)swapChain3);

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

			spdlog::info("ImGuiOverlayDx12::hkffxGetDX12Swapchain_Mod added swapchain hooks!");

			// get result of this call
			result = offxGetDX12Swapchain_Mod(InParam);
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();

		if (offxGetDX12Swapchain_Mod == nullptr)
			return result;
	}

	result = offxGetDX12Swapchain_Mod(InParam);

	if (oPresent_Dx12_Mod == nullptr && result != nullptr && IsIDXGISwapChain4(result))
	{
		IDXGISwapChain3* swapChain3 = nullptr;

		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(result);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_Mod swapchain captured: {0:X}", (unsigned long)swapChain3);

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

			spdlog::info("ImGuiOverlayDx12::hkffxGetDX12Swapchain_Mod added swapchain hooks!");
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();
	}

	_changeFFXModHookCounter_Mod++;

	return result;
}

// Hook for ffxGetDX12SwapchainPtr with native FSR3 
static void* WINAPI hkffxGetDX12Swapchain_FSR3(void* InParam)
{
	if (_changeFFXModHookCounter_Native > 1000 || oPresent_Dx12_FSR3 != nullptr)
		return offxGetDX12Swapchain_FSR3(InParam);

	spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_FSR3 ({0})!", _changeFFXModHookCounter_Mod);

	void* result = nullptr;

	if (oPresent_Dx12_FSR3 == nullptr && IsIDXGISwapChain4(InParam))
	{
		IDXGISwapChain3* swapChain3 = nullptr;

		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(InParam);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_FSR3 swapchain captured: {0:X}", (unsigned long)swapChain3);

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

			spdlog::info("ImGuiOverlayDx12::hkffxGetDX12Swapchain_FSR3 added swapchain hooks!");

			// get result of this call
			result = offxGetDX12Swapchain_FSR3(InParam);
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();

		if (offxGetDX12Swapchain_FSR3 == nullptr)
			return result;
	}

	result = offxGetDX12Swapchain_FSR3(InParam);

	if (oPresent_Dx12_FSR3 == nullptr && result != nullptr && IsIDXGISwapChain4(result))
	{
		IDXGISwapChain3* swapChain3 = nullptr;
		IDXGISwapChain* swapChain4 = reinterpret_cast<IDXGISwapChain*>(result);
		swapChain4->QueryInterface(IID_PPV_ARGS(&swapChain3));

		if (swapChain3 != nullptr)
		{
			spdlog::debug("ImGuiOverlayDx12::hkffxGetDX12Swapchain_FSR3 swapchain captured: {0:X}", (unsigned long)swapChain3);

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

			spdlog::info("ImGuiOverlayDx12::hkffxGetDX12Swapchain_FSR3 added swapchain hooks!");
		}

		if (swapChain3 != nullptr)
			swapChain3->Release();
	}

	_changeFFXModHookCounter_Native++;

	return result;
}

#pragma endregion

#pragma region Hooks for ExecuteCommandLists 

static void WINAPI hkExecuteCommandLists_Dx12(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
{
	if (_cqDx12 == nullptr)
	{
		auto desc = pCommandQueue->GetDesc();

		if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT || _bindedFSR3_Uniscaler)
		{
			spdlog::info("ImGuiOverlayDx12::hkExecuteCommandLists_Dx12 new _cqDx12: {0:X}", (unsigned long)pCommandQueue);
			_cqDx12 = pCommandQueue;
		}
	}

	return oExecuteCommandLists_Dx12(pCommandQueue, NumCommandLists, ppCommandLists);
}

static void WINAPI hkExecuteCommandLists_SL(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
{
	if (_cqSL == nullptr)
	{
		auto desc = pCommandQueue->GetDesc();

		if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			spdlog::info("ImGuiOverlayDx12::hkExecuteCommandLists_SL new _cqSL: {0:X}", (unsigned long)pCommandQueue);
			_cqSL = pCommandQueue;
		}
	}

	return oExecuteCommandLists_SL(pCommandQueue, NumCommandLists, ppCommandLists);
}

#pragma endregion

#pragma region Hook for DXGIFactory for native DX12

static HRESULT WINAPI hkCreateSwapChain_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	//if (IsActivePath(Dx12))
	CleanupRenderTarget(true);

	auto result = oCreateSwapChain_Dx12(pFactory, pDevice, pDesc, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	//if (IsActivePath(Dx12))
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForHwnd_Dx12(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	//if (IsActivePath(Dx12))
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForCoreWindow_Dx12(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_Dx12(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	//if (IsActivePath(Dx12))
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForComposition_Dx12(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);

	return result;
}

#pragma endregion

#pragma region Hook for DXGIFactory for Early bindind

static void HookToSwapChain_EB(IDXGISwapChain* InSwapChain)
{
	if (InSwapChain != nullptr)
	{
		void** pVTable = *reinterpret_cast<void***>(InSwapChain);

		oPresent_Dx12 = (PFN_Present)pVTable[8];
		oPresent1_Dx12 = (PFN_Present1)pVTable[22];

		oResizeBuffers_Dx12 = (PFN_ResizeBuffers)pVTable[13];
		oResizeBuffers1_Dx12 = (PFN_ResizeBuffers1)pVTable[39];

		if (oPresent_Dx12 != nullptr)
		{
			spdlog::info("ImGuiOverlayDx12::CreateDeviceD3D12 Hooking native SwapChain");

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

static HRESULT WINAPI hkCreateSwapChain_EB(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	CleanupRenderTarget(true);

	auto result = oCreateSwapChain_EB(pFactory, pDevice, pDesc, ppSwapChain);

	if (result == S_OK && !_dx12BindingActive)
	{
		*ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
	}

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_EB(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForHwnd_EB(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

	if (result == S_OK && !_dx12BindingActive)
	{
		*ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
	}

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_EB(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForCoreWindow_EB(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

	if (result == S_OK && !_dx12BindingActive)
	{
		*ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
	}

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_EB(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	CleanupRenderTarget(true);

	auto result = oCreateSwapChainForComposition_EB(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);

	if (result == S_OK && !_dx12BindingActive)
	{
		*ppSwapChain = new WrappedIDXGISwapChain4((*ppSwapChain), hkPresent_EB, hkPresent1_EB, CleanupRenderTarget);
	}

	return result;
}

#pragma endregion

#pragma region Hook for DXGIFactory for native StreamLine

static HRESULT WINAPI hkCreateSwapChain_SL(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	if (!_isEarlyBind)
		CleanupRenderTarget(true);

	auto result = oCreateSwapChain_SL(pFactory, pDevice, pDesc, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForHwnd_SL(IDXGIFactory* pFactory, IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	if (!_isEarlyBind)
		CleanupRenderTarget(true);

	auto result = oCreateSwapChainForHwnd_SL(pFactory, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForCoreWindow_SL(IDXGIFactory* pFactory, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	if (!_isEarlyBind)
		CleanupRenderTarget(true);

	auto result = oCreateSwapChainForCoreWindow_SL(pFactory, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

	return result;
}

static HRESULT WINAPI hkCreateSwapChainForComposition_SL(IDXGIFactory* pFactory, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain)
{
	if (!_isEarlyBind)
		CleanupRenderTarget(true);

	auto result = oCreateSwapChainForComposition_SL(pFactory, pDevice, pDesc, pRestrictToOutput, ppSwapChain);;

	return result;
}

#pragma endregion

#pragma region Hooks for early binding

static ULONG WINAPI hkCommandQueueRelease(IUnknown* This)
{
	auto result = oCommandQueueRelease(This);

	if (result == 0)
	{
		if (This == _cqDx12)
			_cqDx12 = nullptr;
		else if (This == _cqFsr3Mod)
			_cqFsr3Mod = nullptr;
		else if (This == _cqFsr3Native)
			_cqFsr3Native = nullptr;
		else if (This == _cqSL)
			_cqSL = nullptr;
	}

	return result;
}

static HRESULT WINAPI hkCreateCommandQueue(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue)
{
	auto result = oCreateCommandQueue(This, pDesc, riid, ppCommandQueue);

	if (result == S_OK && oExecuteCommandLists_Dx12 == nullptr && pDesc->Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
	{
		auto cq = (ID3D12CommandQueue*)*ppCommandQueue;

		void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

		oCommandQueueRelease = (PFN_Release)pCommandQueueVTable[2];
		oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

		if (oExecuteCommandLists_Dx12 != nullptr)
		{
			spdlog::info("ImGuiOverlayDx12::hkCreateCommandQueue hooking CommandQueue");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);
			DetourAttach(&(PVOID&)oCommandQueueRelease, hkCommandQueueRelease);

			DetourTransactionCommit();
		}

		cq = nullptr;
	}

	if (result == S_OK && _cqDx12 == nullptr && pDesc->Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
	{
		_cqDx12 = (ID3D12CommandQueue*)(*ppCommandQueue);
		//*ppCommandQueue = new WrappedID3D12CommandQueue(g_pd3dCommandQueue);
	}

	return result;
}

static HRESULT WINAPI hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
	auto result = oD3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

	if (result == S_OK && oCreateCommandQueue == nullptr)
	{
		auto device = (ID3D12Device*)*ppDevice;

		void** pCommandQueueVTable = *reinterpret_cast<void***>(device);

		oCreateCommandQueue = (PFN_CreateCommandQueue)pCommandQueueVTable[8];

		if (oCreateCommandQueue != nullptr)
		{
			spdlog::info("ImGuiOverlayDx12::hkD3D12CreateDevice hooking D3D12Device");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			DetourAttach(&(PVOID&)oCreateCommandQueue, hkCreateCommandQueue);

			DetourTransactionCommit();
		}
	}

	return result;
}

static HRESULT WINAPI hkCreateDXGIFactory1(REFIID riid, void** ppFactory)
{
	auto result = oCreateDXGIFactory1(riid, ppFactory);

	if (result == S_OK && oCreateSwapChain_EB == nullptr)
	{
		auto factory = (IDXGIFactory*)*ppFactory;
		IDXGIFactory4* factory4 = nullptr;

		if (factory->QueryInterface(IID_PPV_ARGS(&factory4)) == S_OK)
		{
			void** pFactoryVTable = *reinterpret_cast<void***>(factory4);

			oCreateSwapChain_EB = (PFN_CreateSwapChain)pFactoryVTable[10];
			oCreateSwapChainForHwnd_EB = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
			oCreateSwapChainForCoreWindow_EB = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
			oCreateSwapChainForComposition_EB = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

			if (oCreateSwapChain_EB != nullptr)
			{
				spdlog::info("ImGuiOverlayDx12::hkCreateDXGIFactory1 Hooking native DXGIFactory");

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				DetourAttach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);
				DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);
				DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);
				DetourAttach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);

				DetourTransactionCommit();
			}

			factory4->Release();
			factory4 = nullptr;
		}
	}

	return result;
}

#pragma endregion

#pragma region Bindings

static bool CheckDx12(ID3D12Device* InDevice)
{
	HRESULT result;

	// Native DX12 CommandQueue Bind
	if (oExecuteCommandLists_Dx12 == nullptr)
	{
		ID3D12Device* device = InDevice;

		if (device == nullptr)
		{
			// Create device
			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

			result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::CheckDx12 D3D12CreateDevice: {0:X}", (unsigned long)result);
				return false;
			}
		}

		// Create queue
		ID3D12CommandQueue* cq = nullptr;
		D3D12_COMMAND_QUEUE_DESC desc = { };
		result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::CheckDx12 CreateCommandQueue: {0:X}", (unsigned long)result);
			return false;
		}

		if (cq != nullptr)
		{
			void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

			oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

			if (oExecuteCommandLists_Dx12 != nullptr)
			{
				spdlog::info("ImGuiOverlayDx12::CheckDx12 hooking CommandQueue");

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

				DetourTransactionCommit();
			}

			cq->Release();
			cq = nullptr;
		}

		if (device != InDevice && device != nullptr)
		{
			device->Release();
			device = nullptr;
		}
	}

	return true;
}

static bool CheckMods()
{

	// Check for FSR3-FG & Mods
	if (Config::Instance()->HookFSR3Proxy.value_or(true))
	{
		if (offxGetDX12Swapchain_Mod == nullptr)
		{
			// Check for Uniscaler
			offxGetDX12Swapchain_Mod = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("Uniscaler.asi", "ffxGetSwapchainDX12");

			if (offxGetDX12Swapchain_Mod != nullptr)
			{
				spdlog::info("ImGuiOverlayDx12::CheckMods Uniscaler's ffxGetDX12SwapchainPtr found");
				_bindedFSR3_Uniscaler = true;
				_bindedFSR3_Mod = true;
			}
			else
			{
				// Check for Nukem's
				offxGetDX12Swapchain_Mod = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("dlssg_to_fsr3_amd_is_better.dll", "ffxGetSwapchainDX12");

				if (offxGetDX12Swapchain_Mod != nullptr)
				{
					spdlog::info("ImGuiOverlayDx12::CheckMods Nukem's ffxGetDX12SwapchainPtr found");
					_bindedFSR3_Mod = true;
				}
			}
		}

		if (offxGetDX12Swapchain_FSR3 == nullptr)
		{
			// Check for native FSR3 (if game has native FSR3-FG)
			offxGetDX12Swapchain_FSR3 = (PFN_ffxGetDX12SwapchainPtr)DetourFindFunction("ffx_backend_dx12_x64.dll", "ffxGetSwapchainDX12");


			if (offxGetDX12Swapchain_FSR3)
			{
				spdlog::info("ImGuiOverlayDx12::CheckMods FSR3's offxGetDX12SwapchainPtr found");
				_bindedFSR3_Native = true;
			}
		}

		// Hook FSR3 ffxGetDX12SwapchainPtr methods
		if ((offxGetDX12Swapchain_Mod != nullptr && _bindedFSR3_Mod) || (offxGetDX12Swapchain_FSR3 != nullptr && _bindedFSR3_Native))
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			// Use SL calls for Nukem's mod
			if (offxGetDX12Swapchain_Mod != nullptr && _bindedFSR3_Uniscaler)
			{
				DetourAttach(&(PVOID&)offxGetDX12Swapchain_Mod, hkffxGetDX12Swapchain_Mod);
			}

			if (offxGetDX12Swapchain_FSR3 != nullptr && _bindedFSR3_Native)
			{
				DetourAttach(&(PVOID&)offxGetDX12Swapchain_FSR3, hkffxGetDX12Swapchain_FSR3);
			}

			DetourTransactionCommit();
		}
	}

	return true;
}

static bool BindAll(HWND InHWnd, ID3D12Device* InDevice)
{
	spdlog::info("ImGuiOverlayDx12::CreateDeviceD3D12 Handle: {0:X}", (unsigned long)InHWnd);

	HRESULT result;


	if (!CheckDx12(InDevice))
	{
		_dx12BindingActive = false;
		return false;
	}

	_dx12BindingActive = false;

	CheckMods();

	// Uniscaler captures and uses latest swapchain
	// Avoid creating and hooking swapchains to prevent crashes
	// Try to create native DXGIFactory
	if (!_bindedFSR3_Uniscaler && oPresent_Dx12 == nullptr && Config::Instance()->HookD3D12.value_or(true))
	{
		_dx12BindingActive = true;

		// Create device
		ID3D12Device* device = nullptr;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::BindAll Dx12 D3D12CreateDevice: {0:X}", (unsigned long)result);
			_dx12BindingActive = false;

			return false;
		}

		// Create queue
		ID3D12CommandQueue* cq = nullptr;
		D3D12_COMMAND_QUEUE_DESC desc = { };
		result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateCommandQueue2: {0:X}", (unsigned long)result);
			_dx12BindingActive = false;

			return false;
		}

		IDXGIFactory4* factory = nullptr;
		IDXGISwapChain1* swapChain1 = nullptr;
		IDXGISwapChain3* swapChain3 = nullptr;

		spdlog::info("ImGuiOverlayDx12::BindAll Dx12 Creating DXGIFactory for hooking");
		result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateDXGIFactory1: {0:X}", (unsigned long)result);
			_dx12BindingActive = false;

			return false;
		}

		if (oCreateSwapChain_Dx12 == nullptr)
		{
			void** pFactoryVTable = *reinterpret_cast<void***>(factory);

			oCreateSwapChain_Dx12 = (PFN_CreateSwapChain)pFactoryVTable[10];
			oCreateSwapChainForHwnd_Dx12 = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
			oCreateSwapChainForCoreWindow_Dx12 = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
			oCreateSwapChainForComposition_Dx12 = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

			if (oCreateSwapChain_Dx12 != nullptr)
			{
				spdlog::info("ImGuiOverlayDx12::BindAll Hooking native DXGIFactory");

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				DetourAttach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
				DetourAttach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);
				DetourTransactionCommit();
			}
		}

		// Hook DXGI Factory 
		if (factory != nullptr && cq != nullptr && oPresent_Dx12 == nullptr)
		{
			// Setup swap chain
			DXGI_SWAP_CHAIN_DESC1 sd = { };
			sd.BufferCount = NUM_BACK_BUFFERS;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.SampleDesc.Count = 1;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			// Create SwapChain
			result = oCreateSwapChainForHwnd_Dx12(factory, cq, InHWnd, &sd, NULL, NULL, &swapChain1);
			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::BindAll Dx12 CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
				_dx12BindingActive = false;
				return false;
			}

			result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::BindAll Dx12 QueryInterface: {0:X}", (unsigned long)result);
				_dx12BindingActive = false;
				return false;
			}
		}

		if (swapChain3 != nullptr && oPresent_Dx12 == nullptr)
		{
			void** pVTable = *reinterpret_cast<void***>(swapChain3);

			oPresent_Dx12 = (PFN_Present)pVTable[8];
			oPresent1_Dx12 = (PFN_Present1)pVTable[22];

			oResizeBuffers_Dx12 = (PFN_ResizeBuffers)pVTable[13];
			oResizeBuffers1_Dx12 = (PFN_ResizeBuffers1)pVTable[39];

			if (oPresent_Dx12 != nullptr)
			{
				spdlog::info("ImGuiOverlayDx12::BindAll Dx12 Hooking native SwapChain");

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				DetourAttach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
				DetourAttach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);

				DetourAttach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
				DetourAttach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);

				DetourTransactionCommit();
			}
		}

		if (swapChain3 != nullptr)
		{
			swapChain3->Release();
			swapChain3 = nullptr;
		}

		if (swapChain1 != nullptr)
		{
			swapChain1->Release();
			swapChain1 = nullptr;
		}

		if (factory != nullptr)
		{
			factory->Release();
			factory = nullptr;
		}

		if (cq != nullptr)
		{
			cq->Release();
			cq = nullptr;
		}

		if (device != nullptr)
		{
			device->Release();
			device = nullptr;
		}

	}	_dx12BindingActive = false;


	// If not using uniscaler, attaching to sl is needed
	// Try to create SL DXGIFactory
	if (oPresent_SL == nullptr && Config::Instance()->HookSLProxy.value_or(true))
	{
		// Create device
		ID3D12Device* device;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		// check for sl.interposer
		PFN_D3D12_CREATE_DEVICE slCD = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("sl.interposer.dll", "D3D12CreateDevice");

		if (slCD != nullptr)
		{
			spdlog::info("ImGuiOverlayDx12::BindAll sl.interposer.dll D3D12CreateDevice found");

			result = slCD(NULL, featureLevel, IID_PPV_ARGS(&device));

			if (result != S_OK)
				result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&device));

			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::BindAll SL D3D12CreateDevice: {0:X}", (unsigned long)result);
				return false;
			}

			// Create queue
			ID3D12CommandQueue* cq = nullptr;
			D3D12_COMMAND_QUEUE_DESC desc = { };
			result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cq));
			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::BindAll SL CreateCommandQueue1: {0:X}", (unsigned long)result);
				return false;
			}

			if (cq != nullptr)
			{
				void** pCommandQueueVTable = *reinterpret_cast<void***>(cq);

				oExecuteCommandLists_SL = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

				if (oExecuteCommandLists_SL != nullptr)
				{
					spdlog::info("ImGuiOverlayDx12::BindAll SL hooking CommandQueue");

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());

					DetourAttach(&(PVOID&)oExecuteCommandLists_SL, hkExecuteCommandLists_SL);

					DetourTransactionCommit();
				}
			}

			if (!_bindedFSR3_Uniscaler)
			{
				// Check for sl.interposer
				PFN_CreateDXGIFactory1 slFactory = (PFN_CreateDXGIFactory1)DetourFindFunction("sl.interposer.dll", "CreateDXGIFactory1");

				if (slFactory != nullptr)
					spdlog::info("ImGuiOverlayDx12::BindAll sl.interposer.dll CreateDXGIFactory1 found");

				IDXGIFactory4* factory = nullptr;
				IDXGISwapChain1* swapChain1 = nullptr;
				IDXGISwapChain3* swapChain3 = nullptr;

				if (slFactory != nullptr)
				{
					result = slFactory(IID_PPV_ARGS(&factory));

					if (result != S_OK)
					{
						spdlog::error("ImGuiOverlayDx12::BindAll SL CreateDXGIFactory1: {0:X}", (unsigned long)result);
						return false;
					}

					if (oCreateSwapChain_SL == nullptr)
					{
						void** pFactoryVTable = *reinterpret_cast<void***>(factory);

						oCreateSwapChain_SL = (PFN_CreateSwapChain)pFactoryVTable[10];
						oCreateSwapChainForHwnd_SL = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
						oCreateSwapChainForCoreWindow_SL = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
						oCreateSwapChainForComposition_SL = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

						if (oCreateSwapChain_SL != nullptr)
						{
							spdlog::info("ImGuiOverlayDx12::BindAll hooking SL DXGIFactory");

							DetourTransactionBegin();
							DetourUpdateThread(GetCurrentThread());

							if (oCreateSwapChain_SL != oCreateSwapChain_EB && oCreateSwapChain_SL != oCreateSwapChain_Dx12)
								DetourAttach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);

							if (oCreateSwapChainForHwnd_SL != oCreateSwapChainForHwnd_EB && oCreateSwapChainForHwnd_SL != oCreateSwapChainForHwnd_Dx12)
								DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);

							if (oCreateSwapChainForCoreWindow_SL != oCreateSwapChainForCoreWindow_EB && oCreateSwapChainForCoreWindow_SL != oCreateSwapChainForCoreWindow_Dx12)
								DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);

							if (oCreateSwapChainForComposition_SL != oCreateSwapChainForComposition_EB && oCreateSwapChainForComposition_SL != oCreateSwapChainForComposition_Dx12)
								DetourAttach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);

							DetourTransactionCommit();
						}
					}

					// Hook DXGI Factory 
					if (factory != nullptr && cq != nullptr && oPresent_SL == nullptr)
					{
						// Setup swap chain
						DXGI_SWAP_CHAIN_DESC1 sd = { };
						sd.BufferCount = NUM_BACK_BUFFERS;
						sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
						sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
						sd.SampleDesc.Count = 1;
						sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

						// Create SwapChain
						result = oCreateSwapChainForHwnd_SL(factory, cq, InHWnd, &sd, NULL, NULL, &swapChain1);
						if (result != S_OK)
						{
							spdlog::error("ImGuiOverlayDx12::BindAll SL CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
							return false;
						}

						result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
						if (result != S_OK)
						{
							spdlog::error("ImGuiOverlayDx12::BindAll SL QueryInterface: {0:X}", (unsigned long)result);
							return false;
						}
					}

					if (swapChain3 != nullptr && oPresent_SL == nullptr)
					{
						void** pVTable = *reinterpret_cast<void***>(swapChain3);

						oPresent_SL = (PFN_Present)pVTable[8];
						oPresent1_SL = (PFN_Present1)pVTable[22];

						oResizeBuffers_SL = (PFN_ResizeBuffers)pVTable[13];
						oResizeBuffers1_SL = (PFN_ResizeBuffers1)pVTable[39];

						if (oPresent_SL != nullptr)
						{
							spdlog::info("ImGuiOverlayDx12::BindAll hooking SL SwapChain");

							DetourTransactionBegin();
							DetourUpdateThread(GetCurrentThread());

							if (oPresent_SL != oPresent_Dx12)
								DetourAttach(&(PVOID&)oPresent_SL, hkPresent_SL);

							if (oPresent1_SL != oPresent1_Dx12)
								DetourAttach(&(PVOID&)oPresent1_SL, hkPresent1_SL);

							if (oResizeBuffers_SL != oResizeBuffers_Dx12)
								DetourAttach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);

							if (oResizeBuffers1_SL != oResizeBuffers1_Dx12)
								DetourAttach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);

							DetourTransactionCommit();
						}
					}

					if (swapChain3 != nullptr)
					{
						swapChain3->Release();
						swapChain3 = nullptr;
					}

					if (swapChain1 != nullptr)
					{
						swapChain1->Release();
						swapChain1 = nullptr;
					}

					if (factory != nullptr)
					{
						factory->Release();
						factory = nullptr;
					}
				}
			}

			if (cq != nullptr)
			{
				cq->Release();
				cq = nullptr;
			}

			if (device != nullptr)
			{
				device->Release();
				device = nullptr;
			}
		}
	}

	return true;
}

#pragma endregion

static void CreateRenderTarget(ID3D12Device* device, IDXGISwapChain* pSwapChain)
{
	spdlog::info("ImGuiOverlayDx12::CreateRenderTarget");

	DXGI_SWAP_CHAIN_DESC desc;
	HRESULT hr = pSwapChain->GetDesc(&desc);

	if (hr != S_OK)
	{
		spdlog::error("ImGuiOverlayDx12::CreateRenderTarget pSwapChain->GetDesc: {0:X}", (unsigned long)hr);
		return;
	}

	for (UINT i = 0; i < desc.BufferCount; ++i)
	{
		ID3D12Resource* pBackBuffer = NULL;

		auto result = pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));

		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::CreateRenderTarget pSwapChain->GetBuffer: {0:X}", (unsigned long)result);
			return;
		}

		if (pBackBuffer)
		{
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);

			D3D12_RENDER_TARGET_VIEW_DESC desc = { };
			desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			device->CreateRenderTargetView(pBackBuffer, &desc, g_mainRenderTargetDescriptor[i]);
			g_mainRenderTargetResource[i] = pBackBuffer;
		}
	}

	spdlog::info("ImGuiOverlayDx12::CreateRenderTarget done!");
}

static void CleanupDeviceD3D12(ID3D12Device* InDevice)
{
	CleanupRenderTarget(true);

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
}

static void RenderImGui_DX12(IDXGISwapChain3* pSwapChain)
{
	// Wait until upscaler feature is ready
	if (Config::Instance()->CurrentFeature != nullptr &&
		Config::Instance()->CurrentFeature->FrameCount() <= Config::Instance()->MenuInitDelay.value_or(90) + 10)
	{
		return;
	}

	// Init base
	if (!ImGuiOverlayBase::IsInited())
		ImGuiOverlayBase::Init(Util::GetProcessWindow());

	// If handle is changed
	if (ImGuiOverlayBase::IsInited() && ImGuiOverlayBase::IsResetRequested())
	{
		auto hwnd = Util::GetProcessWindow();
		spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 Reset request detected, resetting ImGui, new handle {0:X}", (unsigned long)hwnd);
		ImGuiOverlayDx12::ReInitDx12(hwnd);
	}

	if (!ImGuiOverlayBase::IsInited())
		return;

	// Draw only when menu activated
	if (!ImGuiOverlayBase::IsVisible())
		return;

	// Get device from swapchain
	ID3D12Device* device = nullptr;
	auto result = pSwapChain->GetDevice(IID_PPV_ARGS(&device));

	if (result != S_OK)
	{
		spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 GetDevice: {0:X}", (unsigned long)result);
		return;
	}

	// Generate ImGui resources
	if (!ImGui::GetIO().BackendRendererUserData && g_pd3dCommandQueue != nullptr)
	{
		spdlog::debug("ImGuiOverlayDx12::RenderImGui_DX12 ImGui::GetIO().BackendRendererUserData == nullptr");

		HRESULT result;

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = { };
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = NUM_BACK_BUFFERS;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;

			result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap));
			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dRtvDescHeap): {0:X}", (unsigned long)result);
				return;
			}

			SIZE_T rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

			result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap));
			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateDescriptorHeap(g_pd3dSrvDescHeap): {0:X}", (unsigned long)result);
				return;
			}
		}

		for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
		{
			result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i]));

			if (result != S_OK)
			{
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateCommandAllocator[{0}]: {1:X}", i, (unsigned long)result);
				return;
			}
		}

		result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList));
		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 CreateCommandList: {0:X}", (unsigned long)result);
			return;
		}

		result = g_pd3dCommandList->Close();
		if (result != S_OK)
		{
			spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
			return;
		}

		ImGui_ImplDX12_Init(device, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
			g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(), g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

		return;
	}

	if (ImGuiOverlayDx12::IsInitedDx12())
	{
		// Generate render targets
		if (!g_mainRenderTargetResource[0])
		{
			CreateRenderTarget(device, pSwapChain);
			return;
		}

		// If everything is ready render the frame
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
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 commandAllocator->Reset: {0:X}", (unsigned long)result);
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
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Reset: {0:X}", (unsigned long)result);
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
				spdlog::error("ImGuiOverlayDx12::RenderImGui_DX12 g_pd3dCommandList->Close: {0:X}", (unsigned long)result);
				return;
			}

			ID3D12CommandList* ppCommandLists[] = { g_pd3dCommandList };

			if (oExecuteCommandLists_Dx12 != nullptr)
				oExecuteCommandLists_Dx12(g_pd3dCommandQueue, 1, (ID3D12CommandList*)ppCommandLists);
			else
				g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

			spdlog::trace("ImGuiOverlayDx12::RenderImGui_DX12 ExecuteCommandLists done!");
		}
		else
		{
			if (_showRenderImGuiDebugOnce)
				spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 !(ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])");

			_showRenderImGuiDebugOnce = false;
		}
	}

	if (device != nullptr)
	{
		device->Release();
		device = nullptr;
	}
}

void DeatachAllHooks(bool reInit = false)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (oPresent_Dx12 != nullptr && !reInit)
		DetourDetach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);

	if (oPresent1_Dx12 != nullptr && !reInit)
		DetourDetach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);

	if (oResizeBuffers_Dx12 != nullptr && !reInit)
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);

	if (oResizeBuffers1_Dx12 != nullptr && !reInit)
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);

	if (oD3D12CreateDevice != nullptr)
		DetourDetach(&(PVOID&)oD3D12CreateDevice, hkD3D12CreateDevice);

	if (oCreateCommandQueue != nullptr)
		DetourDetach(&(PVOID&)oCreateCommandQueue, hkCreateCommandQueue);

	if (oCreateDXGIFactory1 != nullptr)
		DetourDetach(&(PVOID&)oCreateDXGIFactory1, hkCreateDXGIFactory1);

	if (oPresent_SL != nullptr && !reInit)
		DetourDetach(&(PVOID&)oPresent_SL, hkPresent_SL);

	if (oPresent1_SL != nullptr && !reInit)
		DetourDetach(&(PVOID&)oPresent1_SL, hkPresent1_SL);

	if (oResizeBuffers_SL != nullptr && !reInit)
		DetourDetach(&(PVOID&)oResizeBuffers_SL, hkResizeBuffers_SL);

	if (oResizeBuffers1_SL != nullptr && !reInit)
		DetourDetach(&(PVOID&)oResizeBuffers1_SL, hkResizeBuffers1_SL);

	if (oPresent_Dx12_FSR3 != nullptr)
		DetourDetach(&(PVOID&)oPresent_Dx12_FSR3, hkPresent_Dx12_FSR3);

	if (oPresent1_Dx12_FSR3 != nullptr)
		DetourDetach(&(PVOID&)oPresent1_Dx12_FSR3, hkPresent1_Dx12_FSR3);

	if (oResizeBuffers_Dx12_FSR3 != nullptr)
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12_FSR3, hkResizeBuffers_Dx12_FSR3);

	if (oResizeBuffers1_Dx12_FSR3 != nullptr)
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12_FSR3, hkResizeBuffers1_Dx12_FSR3);

	if (oPresent_Dx12_Mod != nullptr)
		DetourDetach(&(PVOID&)oPresent_Dx12_Mod, hkPresent_Dx12_Mod);

	if (oPresent1_Dx12_Mod != nullptr)
		DetourDetach(&(PVOID&)oPresent1_Dx12_Mod, hkPresent1_Dx12_Mod);

	if (oResizeBuffers_Dx12_Mod != nullptr)
		DetourDetach(&(PVOID&)oResizeBuffers_Dx12_Mod, hkResizeBuffers_Dx12_Mod);

	if (oResizeBuffers1_Dx12_Mod != nullptr)
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12_Mod, hkResizeBuffers1_Dx12_Mod);

	if (oExecuteCommandLists_Dx12 != nullptr && !reInit)
		DetourDetach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

	if (oCreateSwapChain_Dx12 != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);

	if (oCreateSwapChainForHwnd_Dx12 != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);

	if (oCreateSwapChainForComposition_Dx12 != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);

	if (oCreateSwapChainForCoreWindow_Dx12 != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);

	if (oCreateSwapChain_EB != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChain_EB, hkCreateSwapChain_EB);

	if (oCreateSwapChainForHwnd_EB != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_EB, hkCreateSwapChainForHwnd_EB);

	if (oCreateSwapChainForComposition_EB != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_EB, hkCreateSwapChainForComposition_EB);

	if (oCreateSwapChainForCoreWindow_EB != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_EB, hkCreateSwapChainForCoreWindow_EB);

	if (oCreateSwapChain_SL != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChain_SL, hkCreateSwapChain_SL);

	if (oCreateSwapChainForHwnd_SL != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_SL, hkCreateSwapChainForHwnd_SL);

	if (oCreateSwapChainForComposition_SL != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_SL, hkCreateSwapChainForComposition_SL);

	if (oCreateSwapChainForCoreWindow_SL != nullptr)
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_SL, hkCreateSwapChainForCoreWindow_SL);

	if (offxGetDX12Swapchain_Mod != nullptr)
		DetourDetach(&(PVOID&)offxGetDX12Swapchain_Mod, hkffxGetDX12Swapchain_Mod);

	if (offxGetDX12Swapchain_FSR3 != nullptr)
		DetourDetach(&(PVOID&)offxGetDX12Swapchain_FSR3, hkffxGetDX12Swapchain_FSR3);

	DetourTransactionCommit();

	offxGetDX12Swapchain_Mod = nullptr;
	offxGetDX12Swapchain_FSR3 = nullptr;
	oD3D12CreateDevice = nullptr;
	oCreateCommandQueue = nullptr;
	oCreateDXGIFactory1 = nullptr;
	oPresent_Dx12_FSR3 = nullptr;
	oPresent1_Dx12_FSR3 = nullptr;
	oResizeBuffers_Dx12_FSR3 = nullptr;
	oResizeBuffers1_Dx12_FSR3 = nullptr;
	oPresent_Dx12_Mod = nullptr;
	oPresent1_Dx12_Mod = nullptr;
	oResizeBuffers_Dx12_Mod = nullptr;
	oResizeBuffers1_Dx12_Mod = nullptr;

	oCreateSwapChain_Dx12 = nullptr;
	oCreateSwapChainForHwnd_Dx12 = nullptr;
	oCreateSwapChainForComposition_Dx12 = nullptr;
	oCreateSwapChainForCoreWindow_Dx12 = nullptr;
	oCreateSwapChain_SL = nullptr;
	oCreateSwapChain_EB = nullptr;
	oCreateSwapChainForHwnd_EB = nullptr;
	oCreateSwapChainForComposition_EB = nullptr;
	oCreateSwapChainForCoreWindow_EB = nullptr;
	oCreateSwapChainForHwnd_SL = nullptr;
	oCreateSwapChainForComposition_SL = nullptr;
	oCreateSwapChainForCoreWindow_SL = nullptr;

	if (!reInit)
	{
		oPresent_Dx12 = nullptr;
		oPresent1_Dx12 = nullptr;
		oResizeBuffers_Dx12 = nullptr;
		oResizeBuffers1_Dx12 = nullptr;

		oPresent_SL = nullptr;
		oPresent1_SL = nullptr;
		oResizeBuffers_SL = nullptr;
		oResizeBuffers1_SL = nullptr;

		oExecuteCommandLists_Dx12 = nullptr;
	}
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
	spdlog::info("ImGuiOverlayDx12::RenderImGui_DX12 InitDx12 Handle: {0:X}", (unsigned long)InHandle);

	g_pd3dDeviceParam = InDevice;

	if (InDevice == nullptr || !_isInited)
	{
		if (!BindAll(InHandle, InDevice))
			spdlog::error("ImGuiOverlayDx12::InitDx12 BindAll failed!");

		_isInited = true;
		CleanupDeviceD3D12(InDevice);
	}
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

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	spdlog::info("ImGuiOverlayDx12::ReInitDx12 hwnd: {0:X}", (unsigned long)InNewHwnd);

	if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX12_Shutdown();

	ImGuiOverlayBase::Shutdown();
	ImGuiOverlayBase::Init(InNewHwnd);

	CleanupRenderTarget(true);
}

void ImGuiOverlayDx12::EarlyBind()
{
	if (_isInited)
		return;

	auto d3d12Module = LoadLibraryW(L"d3d12.dll");

	if (d3d12Module == nullptr)
		return;

	oD3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12Module, "D3D12CreateDevice");

	if (oD3D12CreateDevice != nullptr)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)oD3D12CreateDevice, hkD3D12CreateDevice);

		DetourTransactionCommit();
	}

	auto dxgiModule = LoadLibraryW(L"dxgi.dll");

	if (dxgiModule == nullptr)
		return;

	oCreateDXGIFactory1 = (PFN_CreateDXGIFactory1)GetProcAddress(dxgiModule, "CreateDXGIFactory1");

	if (oCreateDXGIFactory1 != nullptr)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		if (oCreateDXGIFactory1 != nullptr)
			DetourAttach(&(PVOID&)oCreateDXGIFactory1, hkCreateDXGIFactory1);

		DetourTransactionCommit();
	}

	_isEarlyBind = !(oD3D12CreateDevice == nullptr || oCreateDXGIFactory1 == nullptr);
}

void ImGuiOverlayDx12::BindMods()
{
	CheckMods();
}

