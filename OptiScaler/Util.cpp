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

