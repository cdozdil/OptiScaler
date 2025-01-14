#include "../Config.h"
#include "../Logger.h"
#include "../resource.h"
#include "../Util.h"

#include "imgui_common.h"
#include "imgui_dx_base.h"
#include "imgui/imgui_impl_win32.h"

void ImguiDxBase::RenderMenu()
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return;

	ImGuiCommon::RenderMenu();
	ImGui::Render();
}

bool ImguiDxBase::IsHandleDifferent()
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return false;

	HWND frontWindow = Util::GetProcessWindow();

	if (frontWindow == _handle)
		return false;

	_handle = frontWindow;

	return true;
}

void ImguiDxBase::Dx11Ready()
{
	ImGuiCommon::Dx11Inited();
}

void ImguiDxBase::Dx12Ready()
{
	ImGuiCommon::Dx12Inited();
}

ImguiDxBase::ImguiDxBase(HWND handle) : _handle(handle)
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return;

	ImGuiCommon::Init(handle);

	_baseInit = ImGuiCommon::IsInited();
}

ImguiDxBase::~ImguiDxBase()
{
	if (!_baseInit)
		return;

	ImGuiCommon::Shutdown();
}

bool ImguiDxBase::IsVisible() const
{
	return ImGuiCommon::IsVisible();
}
