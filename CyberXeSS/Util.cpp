#pragma once
#include "pch.h"
#include "Util.h"
#include "Config.h"

namespace fs = std::filesystem;

extern HMODULE dllModule;

fs::path Util::DllPath()
{
	static fs::path dll;

	if (dll.empty())
	{
		wchar_t dllPath[MAX_PATH];
		GetModuleFileNameW(cyberDllModule, dllPath, MAX_PATH);
		dll = fs::path(dllPath);
	}

	return dll;
}

fs::path Util::ExePath()
{
	static fs::path exe;

	if (exe.empty())
	{
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		exe = fs::path(exePath);
	}

	return exe;
}

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
	const auto isMainWindow = [handle]() {
		return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle);
		};

	DWORD pID = 0;
	GetWindowThreadProcessId(handle, &pID);

	if (GetCurrentProcessId() != pID || !isMainWindow() || handle == GetConsoleWindow())
		return TRUE;

	*(HWND*)lParam = handle;

	return FALSE;
}

HWND Util::GetProcessWindow() {
	HWND hwnd = nullptr;
	EnumWindows(EnumWindowsCallback, (LPARAM)&hwnd);

	while (!hwnd) {
		EnumWindows(EnumWindowsCallback, (LPARAM)&hwnd);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	return hwnd;
}