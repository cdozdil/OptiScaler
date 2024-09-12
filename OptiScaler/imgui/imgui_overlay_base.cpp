#include "imgui_overlay_base.h"
#include "imgui_common.h"

#include <Config.h>
#include <Logger.h>
#include <resource.h>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>


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

bool ImGuiOverlayBase::IsResetRequested()
{
	return ImGuiCommon::IsResetRequested();
}

void ImGuiOverlayBase::Init(HWND InHandle)
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return;

	LOG_FUNC();
	ImGuiCommon::Init(InHandle);
}

void ImGuiOverlayBase::RenderMenu()
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return;

	ImGuiCommon::RenderMenu();
}

void ImGuiOverlayBase::Shutdown()
{
	LOG_FUNC();
	ImGuiCommon::Shutdown();
}

void ImGuiOverlayBase::HideMenu()
{
	ImGuiCommon::HideMenu();
}
