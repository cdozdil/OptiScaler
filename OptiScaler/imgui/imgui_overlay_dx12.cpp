#include "imgui_overlay_base.h"
#include "imgui_overlay_dx12.h"

#include "../Util.h"
#include "../Logger.h"
#include "../Config.h"

#include <d3d12.h>

#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"

// Dx12 overlay code adoptes from 
// https://github.com/bruhmoment21/UniversalHookX

static int const NUM_BACK_BUFFERS = 3;
static IDXGIFactory4* g_dxgiFactory = NULL;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static IDXGISwapChain3* g_pSwapChain = NULL;
static ID3D12CommandAllocator* g_commandAllocators[NUM_BACK_BUFFERS] = { };
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { };
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { };

typedef void(__fastcall* PFN_ExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList*);
typedef HRESULT(__fastcall* PFN_CreateDXGIFactory1)(REFIID riid, void** ppFactory);

PFN_Present oPresent_Dx12 = nullptr;
PFN_Present1 oPresent1_Dx12 = nullptr;
PFN_ResizeBuffers oResizeBuffers_Dx12 = nullptr;
PFN_ResizeBuffers1 oResizeBuffers1_Dx12 = nullptr;
PFN_ExecuteCommandLists oExecuteCommandLists_Dx12 = nullptr;
PFN_CreateSwapChain oCreateSwapChain_Dx12 = nullptr;
PFN_CreateSwapChainForHwnd oCreateSwapChainForHwnd_Dx12 = nullptr;
PFN_CreateSwapChainForComposition oCreateSwapChainForComposition_Dx12 = nullptr;
PFN_CreateSwapChainForCoreWindow oCreateSwapChainForCoreWindow_Dx12 = nullptr;

static bool _isInited = false;

static int GetCorrectDXGIFormat(int eCurrentFormat)
{
	switch (eCurrentFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return eCurrentFormat;
}

static bool CreateDeviceD3D12(HWND InHWnd)
{
	spdlog::debug("imgui_overlay_dx12::CreateDeviceD3D12 Handle: {0:X}", (unsigned long)InHWnd);

	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd = { };
	sd.BufferCount = NUM_BACK_BUFFERS;
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT result;

	// Create device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	PFN_D3D12_CREATE_DEVICE slCD = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("sl.interposer.dll", "D3D12CreateDevice");

	if (slCD != nullptr && Config::Instance()->NVNGX_Engine != NVSDK_NGX_ENGINE_TYPE_UNREAL && Config::Instance()->HookSLDevice.value_or(false))
		result = slCD(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice));
	else
		result = D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice));

	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 D3D12CreateDevice: {0:X}", (unsigned long)result);
		return false;
	}

	// Create device
	D3D12_COMMAND_QUEUE_DESC desc = { };
	result = g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue));
	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateCommandQueue: {0:X}", (unsigned long)result);
		return false;
	}

	PFN_CreateDXGIFactory1 slFactory = (PFN_CreateDXGIFactory1)DetourFindFunction("sl.interposer.dll", "CreateDXGIFactory1");

	IDXGISwapChain1* swapChain1 = NULL;

	if (slFactory != nullptr && Config::Instance()->HookSLProxy.value_or(true))
		result = slFactory(IID_PPV_ARGS(&g_dxgiFactory));
	else
		result = CreateDXGIFactory1(IID_PPV_ARGS(&g_dxgiFactory));

	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateDXGIFactory1: {0:X}", (unsigned long)result);
		return false;
	}

	result = g_dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, InHWnd, &sd, NULL, NULL, &swapChain1);
	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 CreateSwapChainForHwnd: {0:X}", (unsigned long)result);
		return false;
	}

	result = swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
	if (result != S_OK)
	{
		spdlog::error("imgui_overlay_dx12::CreateDeviceD3D12 QueryInterface: {0:X}", (unsigned long)result);
		return false;
	}

	swapChain1->Release();

	return true;
}

static void CreateRenderTarget(IDXGISwapChain* pSwapChain)
{
	spdlog::info("imgui_overlay_dx12::CreateRenderTarget");

	for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
	{
		ID3D12Resource* pBackBuffer = NULL;
		pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));

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

static void CleanupDeviceD3D12()
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

	if (g_pd3dDevice)
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
		return;

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

			result = g_pd3dCommandList->Reset(commandAllocator, NULL);
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
			g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

			spdlog::trace("RenderImGui_DX12 ExecuteCommandLists done!");
		}
		else
		{
			spdlog::debug("!(ImGui::GetCurrentContext() && g_pd3dCommandQueue && g_mainRenderTargetResource[0])");
		}
	}
}

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

static void WINAPI hkExecuteCommandLists_Dx12(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
{
	if (!g_pd3dCommandQueue)
		g_pd3dCommandQueue = pCommandQueue;

	return oExecuteCommandLists_Dx12(pCommandQueue, NumCommandLists, ppCommandLists);
}

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

bool ImGuiOverlayDx12::IsInitedDx12()
{
	return _isInited;
}

HWND ImGuiOverlayDx12::Handle()
{
	return ImGuiOverlayBase::Handle();
}

void ImGuiOverlayDx12::InitDx12(HWND InHandle)
{
	spdlog::info("RenderImGui_DX12 InitDx12 Handle: {0:X}", (unsigned long)InHandle);

	if (g_pd3dDevice == nullptr && !CreateDeviceD3D12(InHandle))
	{
		spdlog::error("imgui_overlay_dx12::InitDx12 CreateDeviceD3D12 failed!");
		return;
	}

	if (g_pd3dDevice)
	{
		spdlog::error("imgui_overlay_dx12::InitDx12 Hooking!");

		// Hook
		void** pVTable = *reinterpret_cast<void***>(g_pSwapChain);
		void** pCommandQueueVTable = *reinterpret_cast<void***>(g_pd3dCommandQueue);
		void** pFactoryVTable = *reinterpret_cast<void***>(g_dxgiFactory);

		oCreateSwapChain_Dx12 = (PFN_CreateSwapChain)pFactoryVTable[10];
		oCreateSwapChainForHwnd_Dx12 = (PFN_CreateSwapChainForHwnd)pFactoryVTable[15];
		oCreateSwapChainForCoreWindow_Dx12 = (PFN_CreateSwapChainForCoreWindow)pFactoryVTable[16];
		oCreateSwapChainForComposition_Dx12 = (PFN_CreateSwapChainForComposition)pFactoryVTable[24];

		oPresent_Dx12 = (PFN_Present)pVTable[8];
		oPresent1_Dx12 = (PFN_Present1)pVTable[22];

		oResizeBuffers_Dx12 = (PFN_ResizeBuffers)pVTable[13];
		oResizeBuffers1_Dx12 = (PFN_ResizeBuffers1)pVTable[39];

		oExecuteCommandLists_Dx12 = (PFN_ExecuteCommandLists)pCommandQueueVTable[10];

		if (g_pd3dCommandQueue)
		{
			g_pd3dCommandQueue->Release();
			g_pd3dCommandQueue = NULL;
		}

		CleanupDeviceD3D12();

		// Apply the detour
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
		DetourAttach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
		DetourAttach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
		DetourAttach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);

		DetourAttach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
		DetourAttach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);

		DetourAttach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
		DetourAttach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);

		DetourAttach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

		DetourTransactionCommit();

		_isInited = true;
	}
}

void ImGuiOverlayDx12::ShutdownDx12()
{
	if (_isInited && ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX12_Shutdown();

	ImGuiOverlayBase::Shutdown();

	if (_isInited)
	{
		CleanupDeviceD3D12();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID&)oCreateSwapChain_Dx12, hkCreateSwapChain_Dx12);
		DetourDetach(&(PVOID&)oCreateSwapChainForHwnd_Dx12, hkCreateSwapChainForHwnd_Dx12);
		DetourDetach(&(PVOID&)oCreateSwapChainForCoreWindow_Dx12, hkCreateSwapChainForCoreWindow_Dx12);
		DetourDetach(&(PVOID&)oCreateSwapChainForComposition_Dx12, hkCreateSwapChainForComposition_Dx12);

		DetourDetach(&(PVOID&)oPresent_Dx12, hkPresent_Dx12);
		DetourDetach(&(PVOID&)oPresent1_Dx12, hkPresent1_Dx12);

		DetourDetach(&(PVOID&)oResizeBuffers_Dx12, hkResizeBuffers_Dx12);
		DetourDetach(&(PVOID&)oResizeBuffers1_Dx12, hkResizeBuffers1_Dx12);

		DetourDetach(&(PVOID&)oExecuteCommandLists_Dx12, hkExecuteCommandLists_Dx12);

		DetourTransactionCommit();
	}

	_isInited = false;
}

void ImGuiOverlayDx12::ReInitDx12(HWND InNewHwnd)
{
	if (!_isInited)
		return;

	spdlog::debug("ImGuiOverlayDx12::ReInitDx12 hwnd: {0:X}", (unsigned long)InNewHwnd);

	if (ImGuiOverlayBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
		ImGui_ImplDX12_Shutdown();


	ImGuiOverlayBase::Shutdown();
	ImGuiOverlayBase::Init(InNewHwnd);

	CleanupRenderTarget();
}
