#include "../pch.h"

#include "d3dx12.h"

#include "imgui_dx12.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

const int NUM_BACK_BUFFERS = 2;
ID3D12Resource* g_mainRenderTargetResource[2] = { nullptr, nullptr };
ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[2] = { };

long counter = 0;
bool imguiInit = false;
bool showImgui = false;
bool show_another_window = false;
bool show_demo_window = false;

WNDPROC oWndProc = nullptr;

/*Forward declare message handler from imgui_impl_win32.cpp*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Win32 message handler
//You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
//- When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
//- When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
//Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (msg == WM_KEYDOWN) {
		if (wParam == VK_INSERT) {
			showImgui = !showImgui;

			io.WantCaptureKeyboard = showImgui;
			io.WantCaptureMouse = showImgui;

			return true;
		}
	}

	if (showImgui && (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) || io.WantCaptureKeyboard || io.WantCaptureMouse))
		return true;

	return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

int BitsPerPixel(DXGI_FORMAT fmt)
{
	switch ((int)fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
}

void CreateRenderTarget(ID3D12Device* pDevice, D3D12_RESOURCE_DESC* InDesc) {
	for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i) {

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = InDesc->Width;
		desc.Height = InDesc->Height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = InDesc->Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ID3D12Resource* renderTarget;
		pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			nullptr,
			IID_PPV_ARGS(&renderTarget)
		);

		if (InDesc) {
			D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
			rtDesc.Format = static_cast<DXGI_FORMAT>(InDesc->Format);
			rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			pDevice->CreateRenderTargetView(renderTarget, &rtDesc, g_mainRenderTargetDescriptor[i]);
			g_mainRenderTargetResource[i] = renderTarget;
		}
	}
}

void RenderImGui_DX12(HWND handle, ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* outTexture) {
	counter++;
	auto backbuf = counter % 2;

	D3D12_RESOURCE_DESC outDesc;
	outDesc = outTexture->GetDesc();

	if (!imguiInit) {
		if (pDevice) {
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = { };
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				desc.NumDescriptors = NUM_BACK_BUFFERS;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				desc.NodeMask = 1;
				if (pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
					return;

				SIZE_T rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

				for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i) {
					g_mainRenderTargetDescriptor[i] = rtvHandle;
					rtvHandle.ptr += rtvDescriptorSize;
				}
			}

			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = { };
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				if (pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
					return;
			}

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			io.MouseDrawCursor = true;

			if (g_mainRenderTargetResource[0] == nullptr)
				CreateRenderTarget(pDevice, &outDesc);

			ImGui_ImplWin32_Init(handle);

			ImGui_ImplDX12_Init(pDevice, NUM_BACK_BUFFERS,
				outDesc.Format, g_pd3dSrvDescHeap,
				g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
				g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

			//oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(currentHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
			oWndProc = (WNDPROC)SetWindowLongPtr(currentHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

			imguiInit = true;
		}
	}

	if (showImgui) {
		if (ImGui::GetCurrentContext() && outTexture) {
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int cnt = 0;
				ImGuiWindowFlags flags = 0;
				flags |= ImGuiWindowFlags_NoCollapse;
				flags |= ImGuiWindowFlags_NoSavedSettings;

				ImGui::Begin("CyberXeSS v0.4", NULL, flags);

				ImGui::SeparatorText("Upscalers");
				if (ImGui::BeginCombo(" ", "XeSS"))
				{
					ImGui::Selectable("XeSS", true);
					ImGui::Selectable("FSR 2.1", false);
					ImGui::Selectable("FSR 2.2", false);
					ImGui::EndCombo();
				}
				if (ImGui::BeginTable("up_select", 2))
				{
					ImGui::TableNextColumn(); ImGui::Button("Apply");
					ImGui::TableNextColumn(); ImGui::Button("Revert");
					ImGui::EndTable();
				}

				//ImGui::Spacing();
				ImGui::SeparatorText("???");
				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", cnt);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();
			}

			ImGui::Render();

			D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
			rtDesc.Format = outDesc.Format;
			rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			pDevice->CreateRenderTargetView(g_mainRenderTargetResource[backbuf], &rtDesc, g_mainRenderTargetDescriptor[backbuf]);

			pCmdList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backbuf], FALSE, NULL);
			pCmdList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

			D3D12_RESOURCE_BARRIER bufferBarrier = { };
			bufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			bufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			bufferBarrier.Transition.pResource = g_mainRenderTargetResource[backbuf];
			bufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			pCmdList->ResourceBarrier(1, &bufferBarrier);

			D3D12_RESOURCE_BARRIER outBarrier = { };
			outBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			outBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			outBarrier.Transition.pResource = outTexture;
			outBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			pCmdList->ResourceBarrier(1, &outBarrier);

			// Copy intermediate render target to your texture
			pCmdList->CopyResource(g_mainRenderTargetResource[backbuf], outTexture);

			bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			pCmdList->ResourceBarrier(1, &bufferBarrier);

			auto drawData = ImGui::GetDrawData();
			ImGui_ImplDX12_RenderDrawData(drawData, pCmdList);

			bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			pCmdList->ResourceBarrier(1, &bufferBarrier);

			outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			pCmdList->ResourceBarrier(1, &outBarrier);

			// Copy intermediate render target to your texture
			pCmdList->CopyResource(outTexture, g_mainRenderTargetResource[backbuf]);

			outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			pCmdList->ResourceBarrier(1, &outBarrier);

			bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			pCmdList->ResourceBarrier(1, &bufferBarrier);
		}
	}
}

