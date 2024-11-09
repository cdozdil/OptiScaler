#pragma once

#include "Util.h"
#include "Logger.h"
#include "resource.h"
#include "WorkingMode.h"
#include "proxies/NVNGX_Proxy.h"
#include "proxies/FfxApi_Proxy.h"
#include "proxies/XeSS_Proxy.h"

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
