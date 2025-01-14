#include "imgui_overlay_base.h"
#include "imgui_common.h"

#include "../Config.h"
#include "../Logger.h"
#include "../resource.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"


HWND ImGuiOverlayBase::Handle()
{
	return ImGuiCommon::Handle();
}

void ImGuiOverlayBase::Dx11Ready()
{
	ImGuiCommon::Dx11Inited();
}

void ImGuiOverlayBase::Dx12Ready()
{
	ImGuiCommon::Dx12Inited();
}

void ImGuiOverlayBase::VulkanReady()
{
	ImGuiCommon::VulkanInited();
}

bool ImGuiOverlayBase::IsInited()
{
	return ImGuiCommon::IsInited();
}

bool ImGuiOverlayBase::IsVisible()
{
	return ImGuiCommon::IsVisible();
}


void ImGuiOverlayBase::Init(HWND InHandle)
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return;

	LOG_FUNC();
	ImGuiCommon::Init(InHandle);
}

bool ImGuiOverlayBase::RenderMenu()
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return false;

	return ImGuiCommon::RenderMenu();
}

void ImGuiOverlayBase::Shutdown()
{
	ImGuiCommon::Shutdown();
}

void ImGuiOverlayBase::HideMenu()
{
	ImGuiCommon::HideMenu();
}
