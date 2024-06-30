#pragma once
#include "dllmain.h"
#include "resource.h"

#include "Logger.h"
#include "Util.h"
#include "NVNGX_Proxy.h"

#include "imgui/imgui_overlay_dx12.h"

#include <vulkan/vulkan_core.h>

#pragma warning (disable : 4996)

typedef BOOL(WINAPI* PFN_FreeLibrary)(HMODULE lpLibrary);
typedef HMODULE(WINAPI* PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(WINAPI* PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef const char* (CDECL* PFN_wine_get_version)(void);

PFN_FreeLibrary o_FreeLibrary = nullptr;
PFN_LoadLibraryA o_LoadLibraryA = nullptr;
PFN_LoadLibraryW o_LoadLibraryW = nullptr;
PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;
PFN_vkGetPhysicalDeviceProperties o_vkGetPhysicalDeviceProperties = nullptr;
PFN_vkGetPhysicalDeviceProperties2 o_vkGetPhysicalDeviceProperties2 = nullptr;
PFN_vkGetPhysicalDeviceProperties2KHR o_vkGetPhysicalDeviceProperties2KHR = nullptr;

static std::string nvngxA("nvngx.dll");
static std::string nvngxExA("nvngx");
static std::wstring nvngxW(L"nvngx.dll");
static std::wstring nvngxExW(L"nvngx");

static std::string nvapiA("nvapi64.dll");
static std::string nvapiExA("nvapi64");
static std::wstring nvapiW(L"nvapi64.dll");
static std::wstring nvapiExW(L"nvapi64");

static std::string dllNameA;
static std::string dllNameExA;
static std::wstring dllNameW;
static std::wstring dllNameExW;

static int loadCount = 0;

void AttachHooks();
void DetachHooks();

static HMODULE LoadNvApi()
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

	if (nvapi == nullptr)
	{
		nvapi = o_LoadLibraryA("nvapi64.dll");

		if (nvapi != nullptr)
		{
			spdlog::info("LoadNvApi Fallback! nvapi64.dll loaded from system");
			return nvapi;
		}
	}

	return nullptr;
}

static BOOL hkFreeLibrary(HMODULE lpLibrary)
{
	if (lpLibrary == nullptr)
		return FALSE;

	if (lpLibrary == dllModule)
	{
		loadCount--;
		spdlog::info("hkFreeLibrary call for this module loadCount: {0}", loadCount);

		if (loadCount == 0)
			return o_FreeLibrary(lpLibrary);
		else
			return TRUE;
	}

	return o_FreeLibrary(lpLibrary);
}

static HMODULE hkLoadLibraryA(LPCSTR lpLibFileName)
{
	if (lpLibFileName == nullptr)
		return NULL;

	std::string libName(lpLibFileName);
	std::string lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	size_t pos;

#ifdef DEBUG
	spdlog::trace("hkLoadLibraryA call: {0}", lcaseLibName);
#endif // DEBUG


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
		loadCount++;
		return dllModule;
	}

	auto result = o_LoadLibraryA(lpLibFileName);
	return result;
}

static HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName)
{
	if (lpLibFileName == nullptr)
		return NULL;

	std::wstring libName(lpLibFileName);
	std::wstring lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);


	std::string lcaseLibNameA(lcaseLibName.length(), 0);
	std::transform(lcaseLibName.begin(), lcaseLibName.end(), lcaseLibNameA.begin(), [](wchar_t c) { return (char)c; });

#ifdef DEBUG
	spdlog::trace("hkLoadLibraryW call: {0}", lcaseLibNameA);
#endif

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
		loadCount++;
		return dllModule;
	}

	auto result = o_LoadLibraryW(lpLibFileName);
	return result;
}

static HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	if (lpLibFileName == nullptr)
		return NULL;

	std::string libName(lpLibFileName);
	std::string lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef DEBUG
	spdlog::trace("hkLoadLibraryExA call: {0}", lcaseLibName);
#endif

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
		loadCount++;
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExA);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExA.size()))
	{
		spdlog::info("hkLoadLibraryExA {0} call, returning this dll!", libName);
		loadCount++;
		return dllModule;
	}

	auto result = o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
	return result;
}

static HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	if (lpLibFileName == nullptr)
		return NULL;

	std::wstring libName(lpLibFileName);
	std::wstring lcaseLibName(libName);

	for (size_t i = 0; i < lcaseLibName.size(); i++)
		lcaseLibName[i] = std::tolower(lcaseLibName[i]);

	std::string lcaseLibNameA(lcaseLibName.length(), 0);
	std::transform(lcaseLibName.begin(), lcaseLibName.end(), lcaseLibNameA.begin(), [](wchar_t c) { return (char)c; });

#ifdef DEBUG
	spdlog::trace("hkLoadLibraryExW call: {0}", lcaseLibNameA);
#endif

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
		loadCount++;
		return dllModule;
	}

	pos = lcaseLibName.rfind(dllNameExW);

	if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExW.size()))
	{
		spdlog::info("hkLoadLibraryExW {0} call, returning this dll!", lcaseLibNameA);
		loadCount++;
		return dllModule;
	}

	auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
	return result;
}

static void WINAPI hkvkGetPhysicalDeviceProperties(VkPhysicalDevice physical_device, VkPhysicalDeviceProperties* properties)
{
	o_vkGetPhysicalDeviceProperties(physical_device, properties);

	std::strcpy(properties->deviceName, "NVIDIA GeForce RTX 4090");
	properties->vendorID = 0x10de;
	properties->deviceID = 0x2684;
	//properties->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	properties->driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);
}

static void WINAPI hkvkGetPhysicalDeviceProperties2(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
	o_vkGetPhysicalDeviceProperties2(phys_dev, properties2);

	std::strcpy(properties2->properties.deviceName, "NVIDIA GeForce RTX 4090");
	properties2->properties.vendorID = 0x10de;
	properties2->properties.deviceID = 0x2684;
	//properties2->properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	properties2->properties.driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);

}

static void WINAPI hkvkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
	o_vkGetPhysicalDeviceProperties2KHR(phys_dev, properties2);

	std::strcpy(properties2->properties.deviceName, "NVIDIA GeForce RTX 4090");
	properties2->properties.vendorID = 0x10de;
	properties2->properties.deviceID = 0x2684;
	//properties2->properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	properties2->properties.driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);
}

static void DetachHooks()
{
	if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
	{
		DetourTransactionBegin();

		DetourUpdateThread(GetCurrentThread());

		if (o_FreeLibrary)
		{
			DetourDetach(&(PVOID&)o_FreeLibrary, hkFreeLibrary);
			o_FreeLibrary = nullptr;
		}

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

		if (o_vkGetPhysicalDeviceProperties)
		{
			DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties, hkvkGetPhysicalDeviceProperties);
			o_vkGetPhysicalDeviceProperties = nullptr;
		}

		if (o_vkGetPhysicalDeviceProperties2)
		{
			DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties2, hkvkGetPhysicalDeviceProperties2);
			o_vkGetPhysicalDeviceProperties2 = nullptr;
		}

		if (o_vkGetPhysicalDeviceProperties2KHR)
		{
			DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties2KHR, hkvkGetPhysicalDeviceProperties2KHR);
			o_vkGetPhysicalDeviceProperties2KHR = nullptr;
		}

		DetourTransactionCommit();

		FreeLibrary(shared.dll);
	}
}

static void AttachHooks()
{
	if (o_LoadLibraryA == nullptr || o_LoadLibraryW == nullptr)
	{
		// Detour the functions
		o_FreeLibrary = reinterpret_cast<PFN_FreeLibrary>(DetourFindFunction("kernel32.dll", "FreeLibrary"));
		o_LoadLibraryA = reinterpret_cast<PFN_LoadLibraryA>(DetourFindFunction("kernel32.dll", "LoadLibraryA"));
		o_LoadLibraryW = reinterpret_cast<PFN_LoadLibraryW>(DetourFindFunction("kernel32.dll", "LoadLibraryW"));
		o_LoadLibraryExA = reinterpret_cast<PFN_LoadLibraryExA>(DetourFindFunction("kernel32.dll", "LoadLibraryExA"));
		o_LoadLibraryExW = reinterpret_cast<PFN_LoadLibraryExW>(DetourFindFunction("kernel32.dll", "LoadLibraryExW"));

		if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			if (o_FreeLibrary)
				DetourAttach(&(PVOID&)o_FreeLibrary, hkFreeLibrary);

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

	if (Config::Instance()->VulkanSpoofing.value_or(false) && o_vkGetPhysicalDeviceProperties == nullptr)
	{
		o_vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties"));
		o_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2"));
		o_vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2KHR"));

		if (o_vkGetPhysicalDeviceProperties != nullptr)
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());

			if (o_vkGetPhysicalDeviceProperties)
				DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties, hkvkGetPhysicalDeviceProperties);

			if (o_vkGetPhysicalDeviceProperties2)
				DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties2, hkvkGetPhysicalDeviceProperties2);

			if (o_vkGetPhysicalDeviceProperties2KHR)
				DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties2KHR, hkvkGetPhysicalDeviceProperties2KHR);

			DetourTransactionCommit();
		}
	}
}

static bool IsRunningOnWine()
{
	HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

	if (!ntdll)
	{
		spdlog::warn("IsRunningOnWine Not running on NT!?!");
		return true;
	}

	auto pWineGetVersion = (PFN_wine_get_version)GetProcAddress(ntdll, "wine_get_version");

	if (pWineGetVersion)
	{
		spdlog::info("IsRunningOnWine Running on Wine {0}!", pWineGetVersion());
		return true;
	}

	spdlog::warn("IsRunningOnWine Wine not detected");
	return false;
}

static void CheckWorkingMode()
{
	std::string filename = Util::DllPath().filename().string();
	std::string lCaseFilename(filename);
	wchar_t sysFolder[MAX_PATH];
	GetSystemDirectory(sysFolder, MAX_PATH);
	std::filesystem::path sysPath(sysFolder);
	std::filesystem::path pluginPath(Config::Instance()->PluginPath.value_or(".\plugins"));


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
		do
		{
			auto pluginFilePath = pluginPath / L"version.dll";
			dll = LoadLibrary(pluginFilePath.wstring().c_str());

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as version.dll, original dll loaded from plugin folder");
				break;
			}

			dll = LoadLibrary(L"version-original.dll");

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as version.dll, version-original.dll loaded");
				break;
			}

			auto sysFilePath = sysPath / L"version.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			if (dll != nullptr)
				spdlog::info("OptiScaler working as version.dll, system dll loaded");

		} while (false);

		if (dll != nullptr)
		{
			dllNameA = "version.dll";
			dllNameExA = "version";
			dllNameW = L"version.dll";
			dllNameExW = L"version";

			shared.LoadOriginalLibrary(dll);
			version.LoadOriginalLibrary(dll);
		}
		else
		{
			spdlog::error("OptiScaler can't find original version.dll!");
		}
	}

	if (lCaseFilename == "winmm.dll")
	{
		do
		{
			auto pluginFilePath = pluginPath / L"winmm.dll";
			dll = LoadLibrary(pluginFilePath.wstring().c_str());

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as winmm.dll, original dll loaded from plugin folder");
				break;
			}

			dll = LoadLibrary(L"winmm-original.dll");

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as winmm.dll, winmm-original.dll loaded");
				break;
			}

			auto sysFilePath = sysPath / L"winmm.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			if (dll != nullptr)
				spdlog::info("OptiScaler working as winmm.dll, system dll loaded");

		} while (false);

		if (dll != nullptr)
		{
			dllNameA = "winmm.dll";
			dllNameExA = "winmm";
			dllNameW = L"winmm.dll";
			dllNameExW = L"winmm";

			shared.LoadOriginalLibrary(dll);
			winmm.LoadOriginalLibrary(dll);
		}
		else
		{
			spdlog::error("OptiScaler can't find original winmm.dll!");
		}
	}

	if (lCaseFilename == "wininet.dll")
	{
		do
		{
			auto pluginFilePath = pluginPath / L"wininet.dll";
			dll = LoadLibrary(pluginFilePath.wstring().c_str());

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as wininet.dll, original dll loaded from plugin folder");
				break;
			}

			dll = LoadLibrary(L"wininet-original.dll");

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as wininet.dll, wininet-original.dll loaded");
				break;
			}

			auto sysFilePath = sysPath / L"wininet.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			if (dll != nullptr)
				spdlog::info("OptiScaler working as wininet.dll, system dll loaded");

		} while (false);

		if (dll != nullptr)
		{
			dllNameA = "wininet.dll";
			dllNameExA = "wininet";
			dllNameW = L"wininet.dll";
			dllNameExW = L"wininet";

			shared.LoadOriginalLibrary(dll);
			wininet.LoadOriginalLibrary(dll);
		}
		else
		{
			spdlog::error("OptiScaler can't find original wininet.dll!");
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
		do
		{
			auto pluginFilePath = pluginPath / L"winhttp.dll";
			dll = LoadLibrary(pluginFilePath.wstring().c_str());

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as winhttp.dll, original dll loaded from plugin folder");
				break;
			}

			dll = LoadLibrary(L"winhttp-original.dll");

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as winhttp.dll, winhttp-original.dll loaded");
				break;
			}

			auto sysFilePath = sysPath / L"winhttp.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			if (dll != nullptr)
				spdlog::info("OptiScaler working as winhttp.dll, system dll loaded");

		} while (false);

		if (dll != nullptr)
		{
			dllNameA = "winhttp.dll";
			dllNameExA = "winhttp";
			dllNameW = L"winhttp.dll";
			dllNameExW = L"winhttp";

			shared.LoadOriginalLibrary(dll);
			winhttp.LoadOriginalLibrary(dll);
		}
		else
		{
			spdlog::error("OptiScaler can't find original winhttp.dll!");
		}
	}

	if (lCaseFilename == "dxgi.dll")
	{
		do
		{
			auto pluginFilePath = pluginPath / L"dxgi.dll";
			dll = LoadLibrary(pluginFilePath.wstring().c_str());

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as dxgi.dll, original dll loaded from plugin folder");
				break;
			}

			dll = LoadLibrary(L"dxgi-original.dll");

			if (dll != nullptr)
			{
				spdlog::info("OptiScaler working as dxgi.dll, dxgi-original.dll loaded");
				break;
			}

			auto sysFilePath = sysPath / L"dxgi.dll";
			dll = LoadLibrary(sysFilePath.wstring().c_str());

			if (dll != nullptr)
				spdlog::info("OptiScaler working as dxgi.dll, system dll loaded");

		} while (false);

		if (dll != nullptr)
		{
			dllNameA = "dxgi.dll";
			dllNameExA = "dxgi";
			dllNameW = L"dxgi.dll";
			dllNameExW = L"dxgi";

			shared.LoadOriginalLibrary(dll);
			dxgi.LoadOriginalLibrary(dll);

			Config::Instance()->IsRunningOnLinux = IsRunningOnWine();
			Config::Instance()->IsDxgiMode = true;
		}
		else
		{
			spdlog::error("OptiScaler can't find original dxgi.dll!");
		}
	}

	if (dll != nullptr)
	{
		spdlog::info("Attaching LoadLibrary hooks");

		if (Config::Instance()->HookOriginalNvngxOnly.value_or(false))
		{
			auto regPath = Util::NvngxPath();

			if (regPath.has_value())
			{
				nvngxA = (regPath.value() / "nvngx.dll").string();

				for (size_t i = 0; i < nvngxA.size(); i++)
					nvngxA[i] = std::tolower(nvngxA[i]);

				nvngxExA = (regPath.value() / "nvngx").string();

				for (size_t i = 0; i < nvngxExA.size(); i++)
					nvngxExA[i] = std::tolower(nvngxExA[i]);

				nvngxW = (regPath.value() / L"nvngx.dll").wstring();

				for (size_t i = 0; i < nvngxW.size(); i++)
					nvngxW[i] = std::tolower(nvngxW[i]);

				nvngxExW = (regPath.value() / L"nvngx").wstring();

				for (size_t i = 0; i < nvngxExW.size(); i++)
					nvngxExW[i] = std::tolower(nvngxExW[i]);
			}
			else
			{
				spdlog::warn("Can't read nvngx.dll address from registry, returning to default behavior!");
			}
		}

		AttachHooks();

		if (!Config::Instance()->DisableEarlyHooking.value_or(false) && Config::Instance()->OverlayMenu.value_or(true))
		{
			ImGuiOverlayDx12::Dx12Bind();
			ImGuiOverlayDx12::FSR3Bind();
		}

		return;
	}

	spdlog::error("Unsupported dll name: {0}", filename);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		loadCount++;

		DisableThreadLibraryCalls(hModule);

		dllModule = hModule;
		processId = GetCurrentProcessId();

#ifdef VER_PRE_RELEASE
		// Enable file logging for pre builds
		Config::Instance()->LogToFile = true;

		// Set log level to debug
		if (Config::Instance()->LogLevel.value_or(2) > 1)
			Config::Instance()->LogLevel = 1;
#endif

		PrepareLogger();

		spdlog::info("{0} loaded", VER_PRODUCT_NAME);
		spdlog::info("============================");
		spdlog::warn("");
		spdlog::warn("OptiScaler is freely downloadable from https://github.com/cdozdil/OptiScaler/releases or https://www.nexusmods.com//mods/1/&game_id=6571");
		spdlog::warn("");
		spdlog::warn("If you paid for these files, you've been scammed");
		spdlog::warn("");
		spdlog::warn("DO NOT USE IN MULTIPLAYER GAMES");
		spdlog::warn("");

		CheckWorkingMode();

		// Check if real DLSS available
		if (Config::Instance()->DLSSEnabled.value_or(true))
		{
			NVNGXProxy::InitNVNGX();

			if (NVNGXProxy::NVNGXModule() == nullptr)
			{
				spdlog::info("Can't load nvngx.dll, disabling DLSS");
			}
			else
			{
				spdlog::info("nvngx.dll loaded, setting DLSS as default upscaler and disabling spoofing.");

				if (Config::Instance()->IsDxgiMode)
					Config::Instance()->DxgiSpoofing = false;

				Config::Instance()->VulkanSpoofing = false;
			}
		}

		break;

	case DLL_PROCESS_DETACH:
		CloseLogger();
		DetachHooks();
		break;

	default:
		break;
	}

	return TRUE;
}
