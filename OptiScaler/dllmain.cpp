#pragma once
#include "dllmain.h"
#include "Logger.h"
#include "resource.h"
#include "Util.h"

#include "detours/detours.h"

typedef HMODULE(WINAPI* PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(WINAPI* PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

PFN_LoadLibraryA o_LoadLibraryA = nullptr;
PFN_LoadLibraryW o_LoadLibraryW = nullptr;
PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;

std::string nvngxA("nvngx.dll");
std::string nvngxExA("nvngx");
std::wstring nvngxW(L"nvngx.dll");
std::wstring nvngxExW(L"nvngx");

std::string dllNameA;
std::string dllNameExA;
std::wstring dllNameW;
std::wstring dllNameExW;

void AttachHooks();
void DeatachHooks();

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName)
{
	std::string libName(lpLibFileName);
	std::string lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	auto pos = lcaseLibName.rfind(nvngxA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
	{
		spdlog::info("LoadLibraryA nvngx call: {0}, returning this dll!", libName);
		return dllModule;
	}
	
	pos = lcaseLibName.rfind(dllNameA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
	{
		spdlog::info("LoadLibraryA winmm call: {0}, returning this dll!", libName);
		return dllModule;
	}

	auto result = o_LoadLibraryA(lpLibFileName);
	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName)
{
	std::wstring libName(lpLibFileName);
	std::wstring lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	auto pos = lcaseLibName.rfind(nvngxW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
	{
		std::string libNameA(lcaseLibName.begin(), lcaseLibName.end());
		spdlog::info("LoadLibraryW nvngx call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
	{
		std::string libNameA(lcaseLibName.begin(), lcaseLibName.end());
		spdlog::info("LoadLibraryW winmm call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	auto result = o_LoadLibraryW(lpLibFileName);
	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	std::string libName(lpLibFileName);
	std::string lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	auto pos = lcaseLibName.rfind(nvngxA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
	{
		spdlog::info("LoadLibraryA nvngx call: {0}, returning this dll!", libName);
		return dllModule;
	}

	pos = lcaseLibName.rfind(nvngxExA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()))
	{
		spdlog::info("LoadLibraryA nvngx call: {0}, returning this dll!", libName);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
	{
		spdlog::info("LoadLibraryA winmm call: {0}, returning this dll!", libName);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExA.size()))
	{
		spdlog::info("LoadLibraryA winmm call: {0}, returning this dll!", libName);
		return dllModule;
	}

	auto result = o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	std::wstring libName(lpLibFileName);
	std::wstring lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	auto pos = lcaseLibName.rfind(nvngxW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
	{
		std::string libNameA(libName.begin(), libName.end());
		spdlog::info("LoadLibraryW nvngx call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	pos = lcaseLibName.rfind(nvngxExW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()))
	{
		std::string libNameA(libName.begin(), libName.end());
		spdlog::info("LoadLibraryW nvngx call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
	{
		std::string libNameA(libName.begin(), libName.end());
		spdlog::info("LoadLibraryW winmm call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExW.size()))
	{
		std::string libNameA(libName.begin(), libName.end());
		spdlog::info("LoadLibraryW winmm call: {0}, returning this dll!", libNameA);
		return dllModule;
	}

	auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
	return result;
}

void DeatachHooks()
{
	if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
	{
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());

		if (o_LoadLibraryA)
		{
			DetourDetach(&(PVOID&)o_LoadLibraryA, hkLoadLibraryA);
			o_LoadLibraryA = nullptr;
		}

		if (o_LoadLibraryW)
		{
			DetourDetach(&(PVOID&)o_LoadLibraryW, hkLoadLibraryW);
			o_LoadLibraryW = nullptr;
		}

		if (o_LoadLibraryExA)
		{
			DetourDetach(&(PVOID&)o_LoadLibraryExA, hkLoadLibraryExA);
			o_LoadLibraryExA = nullptr;
		}

		if (o_LoadLibraryExW)
		{
			DetourDetach(&(PVOID&)o_LoadLibraryExW, hkLoadLibraryExW);
			o_LoadLibraryExW = nullptr;
		}

		DetourTransactionCommit();

		FreeLibrary(shared.dll);
	}
}

void AttachHooks()
{
	if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr)
		return;

	// Detour the functions
	o_LoadLibraryA = reinterpret_cast<PFN_LoadLibraryA>(DetourFindFunction("kernel32.dll", "LoadLibraryA"));
	o_LoadLibraryW = reinterpret_cast<PFN_LoadLibraryW>(DetourFindFunction("kernel32.dll", "LoadLibraryW"));
	o_LoadLibraryExA = reinterpret_cast<PFN_LoadLibraryExA>(DetourFindFunction("kernel32.dll", "LoadLibraryExA"));
	o_LoadLibraryExW = reinterpret_cast<PFN_LoadLibraryExW>(DetourFindFunction("kernel32.dll", "LoadLibraryExW"));

	if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		if (o_LoadLibraryA)
			DetourAttach(&(PVOID&)o_LoadLibraryA, hkLoadLibraryA);

		if (o_LoadLibraryW)
			DetourAttach(&(PVOID&)o_LoadLibraryW, hkLoadLibraryW);

		if (o_LoadLibraryExA)
			DetourAttach(&(PVOID&)o_LoadLibraryExA, hkLoadLibraryExA);

		if (o_LoadLibraryExW)
			DetourAttach(&(PVOID&)o_LoadLibraryExW, hkLoadLibraryExW);

		DetourTransactionCommit();
	}
}

void CheckWorkingMode()
{
	std::string filename = Util::DllPath().filename().string();
	std::string lCaseFilename(filename);

	for (size_t i = 0; i < lCaseFilename.size(); i++)
		lCaseFilename[i] = std::tolower(lCaseFilename[i]);

	if (lCaseFilename == "nvngx.dll" || lCaseFilename == "_nvngx.dll" || lCaseFilename == "dlss-enabler-upscaler.dll")
	{
		spdlog::info("OptiScaler working as native upscaler: {0}", filename);
		return;
	}
	
	HMODULE dll = nullptr;

	if (lCaseFilename == "version.dll")
	{
		dll = LoadLibraryA("version-original.dll");

		if (dll == nullptr)
		{
			char dllpath[MAX_PATH];
			GetSystemDirectoryA(dllpath, MAX_PATH);
			std::string syspath(dllpath);
			syspath += "\\version.dll";
			dll = LoadLibraryA(syspath.c_str());

			spdlog::info("OptiScaler working as version.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as version.dll, version-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "version.dll";
			dllNameExA = "version";
			dllNameW = L"version.dll";
			dllNameExW = L"version";

			shared.LoadOriginalLibrary(dll);
			version.LoadOriginalLibrary(dll);
		}
	}

	if (lCaseFilename == "winmm.dll")
	{
		dll = LoadLibraryA("winmm-original.dll");

		if (dll == nullptr)
		{
			char dllpath[MAX_PATH];
			GetSystemDirectoryA(dllpath, MAX_PATH);
			std::string syspath(dllpath);
			syspath += "\\winmm.dll";
			dll = LoadLibraryA(syspath.c_str());

			spdlog::info("OptiScaler working as winmm.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as winmm.dll, winmm-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "winmm.dll";
			dllNameExA = "winmm";
			dllNameW = L"winmm.dll";
			dllNameExW = L"winmm";

			shared.LoadOriginalLibrary(dll);
			winmm.LoadOriginalLibrary(dll);
		}
	}

	if (lCaseFilename == "wininet.dll")
	{
		dll = LoadLibraryA("wininet-original.dll");

		if (dll == nullptr)
		{
			char dllpath[MAX_PATH];
			GetSystemDirectoryA(dllpath, MAX_PATH);
			std::string syspath(dllpath);
			syspath += "\\wininet.dll";
			dll = LoadLibraryA(syspath.c_str());

			spdlog::info("OptiScaler working as wininet.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as wininet.dll, wininet-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "wininet.dll";
			dllNameExA = "wininet";
			dllNameW = L"wininet.dll";
			dllNameExW = L"wininet";

			shared.LoadOriginalLibrary(dll);
			wininet.LoadOriginalLibrary(dll);
		}
	}

	if (lCaseFilename == "winhttp.dll")
	{
		dll = LoadLibraryA("winhttp-original.dll");

		if (dll == nullptr)
		{
			char dllpath[MAX_PATH];
			GetSystemDirectoryA(dllpath, MAX_PATH);
			std::string syspath(dllpath);
			syspath += "\\winhttp.dll";
			dll = LoadLibraryA(syspath.c_str());

			spdlog::info("OptiScaler working as winhttp.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as winhttp.dll, winhttp-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "winhttp.dll";
			dllNameExA = "winhttp";
			dllNameW = L"winhttp.dll";
			dllNameExW = L"winhttp";

			shared.LoadOriginalLibrary(dll);
			winhttp.LoadOriginalLibrary(dll);
		}
	}

	if (lCaseFilename == "dxgi.dll")
	{
		dll = LoadLibraryA("dxgi-original.dll");

		if (dll == nullptr)
		{
			char dllpath[MAX_PATH];
			GetSystemDirectoryA(dllpath, MAX_PATH);
			std::string syspath(dllpath);
			syspath += "\\dxgi.dll";
			dll = LoadLibraryA(syspath.c_str());

			spdlog::info("OptiScaler working as dxgi.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as dxgi.dll, winhttp-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "dxgi.dll";
			dllNameExA = "dxgi";
			dllNameW = L"dxgi.dll";
			dllNameExW = L"dxgi";

			shared.LoadOriginalLibrary(dll);
			dxgi.LoadOriginalLibrary(dll);
		}
	}

	if (dll != nullptr)
	{
		spdlog::info("Attaching LoadLibrary hooks");
		AttachHooks();
		return;
	}

	spdlog::info("Unsupported dll name: {0}", filename);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		dllModule = hModule;
		processId = GetCurrentProcessId();

		PrepareLogger();

		CheckWorkingMode();

		break;

	case DLL_PROCESS_DETACH:
		CloseLogger();
		DeatachHooks();

		break;

	default:
		break;
	}

	return TRUE;
}
