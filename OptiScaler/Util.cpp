#pragma once
#include "pch.h"
#include "Util.h"
#include "Config.h"

extern HMODULE dllModule;

std::filesystem::path Util::DllPath()
{
	static std::filesystem::path dll;

	if (dll.empty())
	{
		wchar_t dllPath[MAX_PATH];
		GetModuleFileNameW(dllModule, dllPath, MAX_PATH);
		dll = std::filesystem::path(dllPath);
	}

	return dll;
}

std::filesystem::path Util::ExePath()
{
	static std::filesystem::path exe;

	if (exe.empty())
	{
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		exe = std::filesystem::path(exePath);
	}

	return exe;
}

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
	const auto isMainWindow = [handle]() {
		return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle) && handle != GetConsoleWindow();
		};


	DWORD pID = 0;
	GetWindowThreadProcessId(handle, &pID);

	if (pID != processId || !isMainWindow() || handle == GetConsoleWindow())
		return TRUE;

	*reinterpret_cast<HWND*>(lParam) = handle;

	return FALSE;
}

HWND Util::GetProcessWindow() {
	HWND hwnd = nullptr;
	EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));

	if (hwnd == nullptr)
	{
		spdlog::debug("Util::GetProcessWindow EnumWindows returned null using GetForegroundWindow()");
		hwnd = GetForegroundWindow();
	}

	return hwnd;
}

