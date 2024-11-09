#include "OldMenuBase.h"

#include <Util.h>
#include <Config.h>
#include <Logger.h>
#include <resource.h>
#include <menu/MenuCommon.h>

#include <include/imgui/imgui_impl_win32.h>

void OldMenuBase::RenderMenu()
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return;

	MenuCommon::RenderMenu();
	ImGui::Render();
}

bool OldMenuBase::IsHandleDifferent()
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return false;

	HWND frontWindow = Util::GetProcessWindow();

	if (frontWindow == _handle)
		return false;

	_handle = frontWindow;

	return true;
}

void OldMenuBase::Dx11Ready()
{
	MenuCommon::Dx11Inited();
}

void OldMenuBase::Dx12Ready()
{
	MenuCommon::Dx12Inited();
}

OldMenuBase::OldMenuBase(HWND handle) : _handle(handle)
{
	if (Config::Instance()->OverlayMenu.value_or(true))
		return;

	MenuCommon::Init(handle);

	_baseInit = MenuCommon::IsInited();
}

OldMenuBase::~OldMenuBase()
{
	if (!_baseInit)
		return;

	MenuCommon::Shutdown();
}

bool OldMenuBase::IsVisible() const
{
	return MenuCommon::IsVisible();
}
