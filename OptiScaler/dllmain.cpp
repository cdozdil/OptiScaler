#pragma once

#include "Util.h"
#include "Logger.h"
#include "resource.h"
#include "WorkingMode.h"
#include "hooks/LoadLibrary.h"
#include "proxies/NVNGX_Proxy.h"
#include "proxies/FfxApi_Proxy.h"
#include "proxies/XeSS_Proxy.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            if (LoadLibraryHooks::LoadCount() > 1)
            {
                LOG_INFO("DLL_PROCESS_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, LoadLibraryHooks::LoadCount());
                return TRUE;
            }

            dllModule = hModule;
            processId = GetCurrentProcessId();
            DisableThreadLibraryCalls(hModule);

            LoadLibraryHooks::AddLoad();

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
                NVNGXProxy::InitNVNGX();

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
                LoadLibraryHooks::Hook();

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
            //LoadLibraryHooks::Unhook();

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
