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

		LOG_INFO("{0}", wstring_to_string(dll.wstring()));
	}

	return dll;
}

std::optional<std::filesystem::path> Util::NvngxPath()
{
	// Checking _nvngx.dll / nvngx.dll location from registry based on DLSSTweaks 
	// https://github.com/emoose/DLSSTweaks/blob/7ebf418c79670daad60a079c0e7b84096c6a7037/src/ProxyNvngx.cpp#L303
	LOG_INFO("trying to load nvngx from registry path!");

	HKEY regNGXCore;
	LSTATUS ls;
	std::optional<std::filesystem::path> result;

	ls = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\nvlddmkm\\NGXCore", 0, KEY_READ, &regNGXCore);

	if (ls == ERROR_SUCCESS)
	{
		wchar_t regNGXCorePath[260];
		DWORD NGXCorePathSize = 260;

		ls = RegQueryValueExW(regNGXCore, L"NGXPath", nullptr, nullptr, (LPBYTE)regNGXCorePath, &NGXCorePathSize);

		if (ls == ERROR_SUCCESS)
		{
			auto path = std::filesystem::path(regNGXCorePath);
			LOG_INFO("nvngx registry path: {0}", path.string());
			return path;
		}
	}

	return result;
}

double Util::MillisecondsNow()
{
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	double milliseconds = 0;

	if (s_use_qpc)
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		milliseconds = double(1000.0 * now.QuadPart) / s_frequency.QuadPart;
	}
	else
	{
		milliseconds = double(GetTickCount64());
	}

	return milliseconds;
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
		LOG_DEBUG("EnumWindows returned null using GetForegroundWindow()");
		hwnd = GetForegroundWindow();
	}

	return hwnd;
}

