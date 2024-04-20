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
		GetModuleFileNameW(dllModule, dllPath, MAX_PATH);
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

BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
	DWORD proc_id = 0;
	GetWindowThreadProcessId(hWnd, &proc_id);

	if (processId == proc_id)
	{
		auto main = GetWindow(hWnd, GW_OWNER);

		if (main && IsWindowVisible(main))
		{
			*(HWND*)lParam = main;
			return FALSE;
		}
	}

	return TRUE;
}

HWND Util::GetProcessWindow() {
	HWND hwnd = nullptr;
	EnumWindows(EnumWindowsCallback, (LPARAM)&hwnd);

	//while (!hwnd) {
	//	EnumWindows(EnumWindowsCallback, (LPARAM)&hwnd);
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}

	return hwnd;
}