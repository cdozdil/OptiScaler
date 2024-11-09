#include "MenuBase.h"

#include <Config.h>
#include <Logger.h>
#include <resource.h>
#include "MenuCommon.h"

#include <include/imgui/imgui.h>
#include <include/imgui/imgui_impl_win32.h>


HWND MenuBase::Handle()
{
	return MenuCommon::Handle();
}

void MenuBase::Dx11Ready()
{
	MenuCommon::Dx11Inited();
}

void MenuBase::Dx12Ready()
{
	MenuCommon::Dx12Inited();
}

void MenuBase::VulkanReady()
{
	MenuCommon::VulkanInited();
}

bool MenuBase::IsInited()
{
	return MenuCommon::IsInited();
}

bool MenuBase::IsVisible()
{
	return MenuCommon::IsVisible();
}

bool MenuBase::IsResetRequested()
{
	return MenuCommon::IsResetRequested();
}

void MenuBase::Init(HWND InHandle)
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return;

	LOG_FUNC();
	MenuCommon::Init(InHandle);
}

void MenuBase::RenderMenu()
{
	if (!Config::Instance()->OverlayMenu.value_or(true))
		return;

	MenuCommon::RenderMenu();
}

void MenuBase::Shutdown()
{
	MenuCommon::Shutdown();
}

void MenuBase::HideMenu()
{
	MenuCommon::HideMenu();
}
