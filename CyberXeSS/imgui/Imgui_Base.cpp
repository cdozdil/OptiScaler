#include "Imgui_Base.h"
#include "../Config.h"

WNDPROC _oWndProc = nullptr;
bool _isVisible = false;
HWND _handle;

/*Forward declare message handler from imgui_impl_win32.cpp*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// CTRL + HOME
	if (msg == WM_KEYDOWN && wParam == VK_HOME && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		_isVisible = !_isVisible;
		io.MouseDrawCursor = _isVisible;
		return true;
	}

	// Imgui
	if (_isVisible && ((ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) || io.WantCaptureKeyboard || io.WantCaptureMouse)))
		return true;

	return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
}

void Imgui_Base::RenderMenu()
{
	ImGuiIO const& io = ImGui::GetIO(); (void)io;

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		static float f = 0.0f;
		static int cnt = 0;
		ImGuiWindowFlags flags = 0;
		flags |= ImGuiWindowFlags_NoSavedSettings;
		flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::SetNextWindowPos(ImVec2{ 250.0f, 250.0f }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("CyberXeSS v0.4", NULL, flags))
		{
			ImGui::TableNextColumn();
			ImGui::SeparatorText("Upscalers");

			std::string currentUpscaler = "";

			switch (Config::Instance()->Api)
			{
			case NVNGX_DX11:
				ImGui::Text("Active API is DirextX 11");

				if (ImGui::BeginCombo("Select", "XeSS"))
				{
					ImGui::Selectable("FSR 2.2 Native", Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22");
					ImGui::Selectable("XeSS with Dx12", false, Config::Instance()->Dx11Upscaler.value_or("fsr22") == "xess");
					ImGui::Selectable("FSR 2.1 with Dx12", Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr21_12");
					ImGui::Selectable("FSR 2.2 with Dx12", Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22_12");
					ImGui::EndCombo();
				}

				break;

			case NVNGX_DX12:
				ImGui::Text("DirextX 12");

				if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr21"))
					currentUpscaler = "FSR 2.1";
				else if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr22"))
					currentUpscaler = "FSR 2.2";
				else
					currentUpscaler = "XeSS";

				if (ImGui::BeginCombo("Select", currentUpscaler.c_str()))
				{
					if (ImGui::Selectable("XeSS", false, Config::Instance()->Dx12Upscaler.value_or("xess") == "xess"))
						Config::Instance()->newBackend = "xess";

					if (ImGui::Selectable("FSR 2.1", Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr21"))
						Config::Instance()->newBackend = "fsr21";

					if (ImGui::Selectable("FSR 2.2", Config::Instance()->Dx12Upscaler.value_or("xess") == "fsr22"))
						Config::Instance()->newBackend = "fsr22";

					ImGui::EndCombo();
				}

				break;

			default:
				ImGui::Text("Active API is Vulkan");

				if (ImGui::BeginCombo("Select", "XeSS"))
				{
					ImGui::Selectable("FSR 2.1", Config::Instance()->VulkanUpscaler.value_or("xess") == "fsr21");
					ImGui::Selectable("FSR 2.2", Config::Instance()->VulkanUpscaler.value_or("xess") == "fsr22");
					ImGui::EndCombo();
				}

			}

			if (ImGui::BeginTable("up_select", 2))
			{
				ImGui::TableNextColumn();

				if (ImGui::Button("Apply") && Config::Instance()->newBackend != "" && Config::Instance()->newBackend != Config::Instance()->Dx12Upscaler.value_or("xess"))
					Config::Instance()->changeBackend = true;

				ImGui::TableNextColumn();

				if (ImGui::Button("Revert"))
					Config::Instance()->newBackend = "";

				ImGui::EndTable();
			}

			ImGui::TableNextColumn();
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			auto pos = ImGui::GetWindowPos();
			_top = pos.y;
			_left = pos.x;

			auto size = ImGui::GetWindowSize();
			_width = size.x;
			_height = size.y;

			ImGui::End();
		}


	}

	ImGui::Render();
}

Imgui_Base::Imgui_Base(HWND handle)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.WantCaptureMouse = true;
	SetForegroundWindow(handle);

	_handle = handle;

	_baseInit = ImGui_ImplWin32_Init(_handle);

	//Capture WndProc
	_oWndProc = (WNDPROC)SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

Imgui_Base::~Imgui_Base()
{
	if (!_baseInit)
		return;

	_isVisible = false;

	SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)_oWndProc);
	_oWndProc = nullptr;

	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool Imgui_Base::IsVisible() const
{
	return _isVisible;
}

void Imgui_Base::SetVisible(bool visible) const
{
	_isVisible = visible;
}