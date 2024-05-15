#pragma once
#include "pch.h"
#include "Logger.h"
#include "resource.h"
#include "Util.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		dllModule = hModule;
		processId = GetCurrentProcessId();

		PrepareLogger();

		spdlog::info("OptiScaler v{0}.{1}.{2} loaded", VER_MAJOR_VERSION, VER_MINOR_VERSION, VER_HOTFIX_VERSION);

		break;

	case DLL_PROCESS_DETACH:
		CloseLogger();
		break;

	default:
		break;
	}

	return TRUE;
}
