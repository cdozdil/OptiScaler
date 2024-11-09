#pragma once

#include "Util.h"
#include "Logger.h"
#include "resource.h"
#include "WorkingMode.h"
#include "proxies/NVNGX_Proxy.h"
#include "proxies/FfxApi_Proxy.h"
#include "proxies/XeSS_Proxy.h"
#include "menu/imgui_overlay_dx.h"
#include "menu/imgui_overlay_vk.h"

#pragma warning (disable : 4996)

inline std::vector<std::string> upscalerNames =
{
    "nvngx.dll",
    "nvngx",
    "libxess.dll",
    "libxess"
};

inline std::vector<std::string> nvngxDlss =
{
    "nvngx_dlss.dll",
    "nvngx_dlss",
};

inline std::vector<std::string> nvapiNames =
{
    "nvapi64.dll",
    "nvapi64",
};

inline std::vector<std::string> dllNames;

inline std::vector<std::wstring> upscalerNamesW =
{
    L"nvngx.dll",
    L"nvngx",
    L"libxess.dll",
    L"libxess"
};

inline std::vector<std::wstring> nvngxDlssW =
{
    L"nvngx_dlss.dll",
    L"nvngx_dlss",
};

inline std::vector<std::wstring> nvapiNamesW =
{
    L"nvapi64.dll",
    L"nvapi64",
};

inline std::vector<std::wstring> dllNamesW;

static int loadCount = 0;
static bool dontCount = false;

static bool isNvngxMode = false;
static bool isWorkingWithEnabler = false;
static bool isNvngxAvailable = false;

void AttachHooks();
void DetachHooks();
HMODULE LoadNvApi();
HMODULE LoadNvgxDlss(std::wstring originalPath);

static HMODULE LoadNvApi()
{
    HMODULE nvapi = nullptr;

    if (Config::Instance()->NvapiDllPath.has_value())
    {
        nvapi = o_LoadLibraryW(Config::Instance()->NvapiDllPath->c_str());

        if (nvapi != nullptr)
        {
            LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(Config::Instance()->NvapiDllPath.value()));
            return nvapi;
        }
    }

    if (nvapi == nullptr)
    {
        auto localPath = Util::DllPath().parent_path() / L"nvapi64.dll";
        nvapi = o_LoadLibraryW(localPath.wstring().c_str());

        if (nvapi != nullptr)
        {
            LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(localPath.wstring()));
            return nvapi;
        }
    }

    if (nvapi == nullptr)
    {
        nvapi = o_LoadLibraryW(L"nvapi64.dll");

        if (nvapi != nullptr)
        {
            LOG_WARN("nvapi64.dll loaded from system!");
            return nvapi;
        }
    }

    return nullptr;
}

static HMODULE LoadNvgxDlss(std::wstring originalPath)
{
    HMODULE nvngxDlss = nullptr;

    if (Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        nvngxDlss = o_LoadLibraryW(Config::Instance()->NVNGX_DLSS_Library.value().c_str());

        if (nvngxDlss != nullptr)
        {
            LOG_INFO("nvngx_dlss.dll loaded from {0}", wstring_to_string(Config::Instance()->NVNGX_DLSS_Library.value()));
            return nvngxDlss;
        }
        else
        {
            LOG_WARN("nvngx_dlss.dll can't found at {0}", wstring_to_string(Config::Instance()->NVNGX_DLSS_Library.value()));
        }
    }

    if (nvngxDlss == nullptr)
    {
        nvngxDlss = o_LoadLibraryW(originalPath.c_str());

        if (nvngxDlss != nullptr)
        {
            LOG_INFO("nvngx_dlss.dll loaded from {0}", wstring_to_string(originalPath));
            return nvngxDlss;
        }
    }

    return nullptr;
}

static void DetachHooks()
{
    if (!isNvngxMode)
    {
        ImGuiOverlayDx::UnHookDx();
        ImGuiOverlayVk::UnHookVk();
    }

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

        if (o_GetProcAddress)
        {
            DetourDetach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);
            o_GetProcAddress = nullptr;
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

        if (o_vkEnumerateInstanceExtensionProperties)
        {
            DetourDetach(&(PVOID&)o_vkEnumerateInstanceExtensionProperties, hkvkEnumerateInstanceExtensionProperties);
            o_vkEnumerateInstanceExtensionProperties = nullptr;
        }

        if (o_vkEnumerateDeviceExtensionProperties)
        {
            DetourDetach(&(PVOID&)o_vkEnumerateDeviceExtensionProperties, hkvkEnumerateDeviceExtensionProperties);
            o_vkEnumerateDeviceExtensionProperties = nullptr;
        }

        if (o_vkCreateDevice)
        {
            DetourDetach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);
            o_vkCreateDevice = nullptr;
        }

        if (o_vkCreateInstance)
        {
            DetourDetach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);
            o_vkCreateInstance = nullptr;
        }

        //if (!Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
        //{
        //    if (dxgi.CreateDxgiFactory != nullptr)
        //    {
        //        DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory, _CreateDXGIFactory);
        //        dxgi.CreateDxgiFactory = nullptr;
        //    }

        //    if (dxgi.CreateDxgiFactory1 != nullptr)
        //    {
        //        DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory1, _CreateDXGIFactory1);
        //        dxgi.CreateDxgiFactory1 = nullptr;
        //    }

        //    if (dxgi.CreateDxgiFactory2 != nullptr)
        //    {
        //        DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory2, _CreateDXGIFactory2);
        //        dxgi.CreateDxgiFactory2 = nullptr;
        //    }
        //}

        DetourTransactionCommit();

        //FreeLibrary(shared.dll);
    }
}

static void AttachHooks()
{
    LOG_FUNC();

    if (o_LoadLibraryA == nullptr || o_LoadLibraryW == nullptr)
    {
        // Detour the functions
        o_FreeLibrary = reinterpret_cast<PFN_FreeLibrary>(DetourFindFunction("kernel32.dll", "FreeLibrary"));
        o_LoadLibraryA = reinterpret_cast<PFN_LoadLibraryA>(DetourFindFunction("kernel32.dll", "LoadLibraryA"));
        o_LoadLibraryW = reinterpret_cast<PFN_LoadLibraryW>(DetourFindFunction("kernel32.dll", "LoadLibraryW"));
        o_LoadLibraryExA = reinterpret_cast<PFN_LoadLibraryExA>(DetourFindFunction("kernel32.dll", "LoadLibraryExA"));
        o_LoadLibraryExW = reinterpret_cast<PFN_LoadLibraryExW>(DetourFindFunction("kernel32.dll", "LoadLibraryExW"));
#ifdef _DEBUG
        o_GetProcAddress = reinterpret_cast<PFN_GetProcAddress>(DetourFindFunction("kernel32.dll", "GetProcAddress"));
#endif // DEBUG

        if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
        {
            LOG_INFO("Attaching LoadLibrary hooks");

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

            if (o_GetProcAddress)
                DetourAttach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);

            DetourTransactionCommit();
        }
    }



}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            if (loadCount > 1)
            {
                LOG_INFO("DLL_PROCESS_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
                return TRUE;
            }


            dllModule = hModule;
            processId = GetCurrentProcessId();

            DisableThreadLibraryCalls(hModule);

            loadCount++;

#ifdef VER_PRE_RELEASE
            // Enable file logging for pre builds
            Config::Instance()->LogToFile = true;

            // Set log level to debug
            if (Config::Instance()->LogLevel.value_or(2) > 1)
                Config::Instance()->LogLevel = 1;
#endif

            PrepareLogger();
            spdlog::warn("{0} loaded", VER_PRODUCT_NAME);
            spdlog::warn("---------------------------------");
            spdlog::warn("OptiScaler is freely downloadable from");
            spdlog::warn("GitHub : https://github.com/cdozdil/OptiScaler/releases");
            spdlog::warn("Nexus  : https://www.nexusmods.com/site/mods/986");
            spdlog::warn("If you paid for these files, you've been scammed!");
            spdlog::warn("DO NOT USE IN MULTIPLAYER GAMES");
            spdlog::warn("");
            spdlog::warn("LogLevel: {0}", Config::Instance()->LogLevel.value_or(2));

            // Check if real DLSS available
            if (Config::Instance()->DLSSEnabled.value_or(true))
            {
                spdlog::info("");
                NVNGXProxy::InitNVNGX();

                if (NVNGXProxy::NVNGXModule() == nullptr)
                {
                    spdlog::info("Can't load nvngx.dll, disabling DLSS");
                    Config::Instance()->DLSSEnabled = false;
                }
                else
                {
                    spdlog::info("nvngx.dll loaded, setting DLSS as default upscaler and disabling spoofing options set to auto");

                    Config::Instance()->DLSSEnabled = true;

                    if (!Config::Instance()->DxgiSpoofing.has_value())
                        Config::Instance()->DxgiSpoofing = false;

                    if (!Config::Instance()->VulkanSpoofing.has_value())
                        Config::Instance()->VulkanSpoofing = false;

                    if (!Config::Instance()->VulkanExtensionSpoofing.has_value())
                        Config::Instance()->VulkanExtensionSpoofing = false;

                    isNvngxAvailable = true;
                }
            }

            // Init XeSS proxy
            if (!XeSSProxy::InitXeSS())
                spdlog::warn("Can't init XeSS!");

            // Init FfxApi Dx12 proxy
            if (!FfxApiProxy::InitFfxDx12())
                spdlog::warn("Can't init Dx12 FfxApi!");

            // Init FfxApi Vulkan proxy
            if (!FfxApiProxy::InitFfxVk())
                spdlog::warn("Can't init Vulkan FfxApi!");

            // Check for working mode and attach hooks
            spdlog::info("");

            if (WorkingMode::Check())
                AttachHooks();

            spdlog::info("");

            for (size_t i = 0; i < 300; i++)
            {
                Config::Instance()->frameTimes.push_back(0.0);
                Config::Instance()->upscaleTimes.push_back(0.0);
            }

            break;

        case DLL_PROCESS_DETACH:
            // Unhooking and cleaning stuff causing issues during shutdown. 
            // Disabled for now to check if it cause any issues
            //DetachHooks();

            if (skHandle != nullptr)
                FreeLibrary(skHandle);

            spdlog::info("");
            spdlog::info("Unloading OptiScaler");
            CloseLogger();

            break;

        case DLL_THREAD_ATTACH:
#ifdef _DEBUG
            // LOG_TRACE("DllMain DLL_THREAD_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
#endif
            break;

        case DLL_THREAD_DETACH:
#ifdef _DEBUG
            // LOG_TRACE("DLL_THREAD_DETACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
#endif
            break;

        default:
            LOG_WARN("Call reason: {0:X}", ul_reason_for_call);
            break;
    }

    return TRUE;
}
