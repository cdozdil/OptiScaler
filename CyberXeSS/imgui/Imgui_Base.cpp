#include "Imgui_Base.h"

WNDPROC _oWndProc = nullptr;
bool _isVisible = false;
HWND _handle;

/*Forward declare message handler from imgui_impl_win32.cpp*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO const& io = ImGui::GetIO(); (void)io;

	// CTRL + HOME
	if (msg == WM_KEYDOWN && wParam == VK_HOME && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		_isVisible = !_isVisible;
		return true;
	}

	// Imgui
	if (_isVisible && (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) || io.WantCaptureKeyboard || io.WantCaptureMouse))
		return true;

	return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
}

void Imgui_Base::RenderMenu()
{
	ImGuiIO const& io = ImGui::GetIO(); (void)io;

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


		ImGui::SeparatorText("???");
		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			cnt++;

		ImGui::SameLine();
		ImGui::Text("counter = %d", cnt);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
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
	io.MouseDrawCursor = true;

	_handle = handle;

	_baseInit = ImGui_ImplWin32_Init(_handle);

	//Capture WndProc
	_oWndProc = (WNDPROC)SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

Imgui_Base::~Imgui_Base()
{
	if (!_baseInit)
		return;

	SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)_oWndProc);

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