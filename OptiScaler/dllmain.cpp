#pragma once
#include "dllmain.h"
#include "Logger.h"
#include "resource.h"
#include "Util.h"
#include "Config.h"

#include "imgui/imgui_overlay_dx12.h"

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

std::string nvapiA("nvapi64.dll");
std::string nvapiExA("nvapi64");
std::wstring nvapiW(L"nvapi64.dll");
std::wstring nvapiExW(L"nvapi64");

std::string dllNameA;
std::string dllNameExA;
std::wstring dllNameW;
std::wstring dllNameExW;

void AttachHooks();
void DeatachHooks();

HMODULE LoadNvApi()
{
	HMODULE nvapi = nullptr;

	if (Config::Instance()->NvapiDllPath.has_value())
	{
		nvapi = o_LoadLibraryA(Config::Instance()->NvapiDllPath->c_str());

		if (nvapi != nullptr)
		{
			spdlog::info("LoadNvApi nvapi64.dll loaded from {0}", Config::Instance()->NvapiDllPath.value());
			return nvapi;
		}
	}

	if (nvapi == nullptr)
	{
		auto localPath = Util::DllPath().parent_path() / "nvapi64.dll";
		nvapi = o_LoadLibraryA(localPath.string().c_str());

		if (nvapi != nullptr)
		{
			spdlog::info("LoadNvApi nvapi64.dll loaded from {0}", localPath.string());
			return nvapi;
		}
	}

	return nullptr;
}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName)
{
	std::string libName(lpLibFileName);
	std::string lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	size_t pos;

	if (!Config::Instance()->dlssDisableHook)
	{
		pos = lcaseLibName.rfind(nvngxA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
		{
			spdlog::info("hkLoadLibraryA nvngx call: {0}, returning this dll!", libName);
			return dllModule;
		}
	}

	if (Config::Instance()->OverrideNvapiDll.value_or(false))
	{
		pos = lcaseLibName.rfind(nvapiA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
		{
			spdlog::info("hkLoadLibraryA {0} call!", libName);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}
	}

	pos = lcaseLibName.rfind(dllNameA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
	{
		spdlog::info("hkLoadLibraryA {0} call returning this dll!", libName);
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

	std::string lcaseLibNameA(lcaseLibName.begin(), lcaseLibName.end());

	size_t pos;

	if (!Config::Instance()->dlssDisableHook)
	{
		pos = lcaseLibName.rfind(nvngxW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
		{
			spdlog::info("hkLoadLibraryW nvngx call: {0}, returning this dll!", lcaseLibNameA);
			return dllModule;
		}
	}

	if (Config::Instance()->OverrideNvapiDll.value_or(false))
	{
		pos = lcaseLibName.rfind(nvapiW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
		{
			spdlog::info("hkLoadLibraryW {0} call!", lcaseLibNameA);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}
	}

	pos = lcaseLibName.rfind(dllNameW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
	{
		spdlog::info("hkLoadLibraryW {0} call, returning this dll!", lcaseLibNameA);
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

	size_t pos;

	if (!Config::Instance()->dlssDisableHook)
	{
		pos = lcaseLibName.rfind(nvngxA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
		{
			spdlog::info("hkLoadLibraryExA nvngx call: {0}, returning this dll!", libName);
			return dllModule;
		}

		pos = lcaseLibName.rfind(nvngxExA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()))
		{
			spdlog::info("hkLoadLibraryExA nvngx call: {0}, returning this dll!", libName);
			return dllModule;
		}
	}

	if (Config::Instance()->OverrideNvapiDll.value_or(false))
	{
		pos = lcaseLibName.rfind(nvapiExA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExA.size()))
		{
			spdlog::info("hkLoadLibraryExA {0} call!", libName);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}

		pos = lcaseLibName.rfind(nvapiA);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
		{
			spdlog::info("hkLoadLibraryExA {0} call!", libName);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}
	}

	pos = lcaseLibName.rfind(dllNameA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
	{
		spdlog::info("hkLoadLibraryExA {0} call, returning this dll!", libName);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExA.size()))
	{
		spdlog::info("hkLoadLibraryExA {0} call, returning this dll!", libName);
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

	std::string lcaseLibNameA(lcaseLibName.begin(), lcaseLibName.end());

	size_t pos;

	if (!Config::Instance()->dlssDisableHook)
	{
		pos = lcaseLibName.rfind(nvngxW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
		{
			spdlog::info("hkLoadLibraryExW nvngx call: {0}, returning this dll!", lcaseLibNameA);
			return dllModule;
		}

		pos = lcaseLibName.rfind(nvngxExW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()))
		{
			spdlog::info("hkLoadLibraryExW nvngx call: {0}, returning this dll!", lcaseLibNameA);
			return dllModule;
		}
	}

	if (Config::Instance()->OverrideNvapiDll.value_or(false))
	{
		pos = lcaseLibName.rfind(nvapiExW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExW.size()))
		{
			spdlog::info("hkLoadLibraryExW {0} call!", lcaseLibNameA);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}

		pos = lcaseLibName.rfind(nvapiW);

		if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
		{
			spdlog::info("hkLoadLibraryExW {0} call!", lcaseLibNameA);

			auto nvapi = LoadNvApi();

			if (nvapi != nullptr)
				return nvapi;
		}
	}

	pos = lcaseLibName.rfind(dllNameW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
	{
		spdlog::info("hkLoadLibraryExW {0} call, returning this dll!", lcaseLibNameA);
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExW.size()))
	{
		spdlog::info("hkLoadLibraryExW {0} call, returning this dll!", lcaseLibNameA);
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
	wchar_t sysFolder[MAX_PATH];
	GetSystemDirectory(sysFolder, MAX_PATH);
	std::filesystem::path sysPath(sysFolder);


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
		dll = LoadLibrary(L"version-original.dll");

		if (dll == nullptr)
		{
			auto sysFilePath = sysPath / L"version.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

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
		dll = LoadLibrary(L"winmm-original.dll");

		if (dll == nullptr)
		{
			auto sysFilePath = sysPath / L"winmm.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

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
		dll = LoadLibrary(L"wininet-original.dll");

		if (dll == nullptr)
		{
			auto sysFilePath = sysPath / L"wininet.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

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

	if (lCaseFilename == "optiscaler.asi")
	{
		spdlog::info("OptiScaler working as OptiScaler.asi");

		// quick hack for testing
		dll = dllModule;

		dllNameA = "optiscaler.asi";
		dllNameExA = "optiscaler";
		dllNameW = L"optiscaler.asi";
		dllNameExW = L"optiscaler";
	}

	if (lCaseFilename == "winhttp.dll")
	{
		dll = LoadLibrary(L"winhttp-original.dll");

		if (dll == nullptr)
		{
			auto sysFilePath = sysPath / L"winhttp.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

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
		dll = LoadLibrary(L"dxgi-original.dll");

		if (dll == nullptr)
		{
			auto sysFilePath = sysPath / L"dxgi.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			spdlog::info("OptiScaler working as dxgi.dll, system dll loaded");
		}
		else
		{
			spdlog::info("OptiScaler working as dxgi.dll, dxgi-original.dll loaded");
		}

		if (dll != nullptr)
		{
			dllNameA = "dxgi.dll";
			dllNameExA = "dxgi";
			dllNameW = L"dxgi.dll";
			dllNameExW = L"dxgi";
		}

		shared.LoadOriginalLibrary(dll);
		dxgi.LoadOriginalLibrary(dll);
	}

	if (dll != nullptr)
	{
		spdlog::info("Attaching LoadLibrary hooks");

		AttachHooks();

		if (!Config::Instance()->DisableEarlyHooking.value_or(false))
		{
			spdlog::info("Trying to early bind of Dx12 ImGui hooks");
			ImGuiOverlayDx12::EarlyBind();
		}

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
		spdlog::info("{0} loaded", VER_PRODUCT_NAME);

		CheckWorkingMode();

		if (!Config::Instance()->DisableEarlyHooking.value_or(false))
		{
			ImGuiOverlayDx12::BindMods();
		}

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
