#pragma once
#include "dllmain.h"
#include "resource.h"

#include "Logger.h"
#include "Config.h"
#include "Working_Mode.h"

#include "hooks/Vulkan_Hooks.h"
#include "hooks/LoadLibrary_Hooks.h"

#include "imgui/imgui_overlay_dx12.h"

//#pragma warning (disable : 4996)

void AttachHooks();
void DetachHooks();

static void DetachHooks()
{
    DetachLoadLibraryHooks();
    WorkingMode::DetachHooks();
    DetachVulkanDeviceHooks();
    DetachVulkanExtensionHooks();
}

static void AttachHooks()
{
    LOG_FUNC();

    AttachLoadLibraryHooks();

    if (Config::Instance()->VulkanSpoofing.value_or(false))
        AttachVulkanDeviceHooks();

    if (Config::Instance()->VulkanExtensionSpoofing.value_or(false))
        AttachVulkanExtensionHooks();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
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
            spdlog::warn("{0} loaded", VER_PRODUCT_NAME);
            spdlog::warn("---------------------------------");
            spdlog::warn("OptiScaler is freely downloadable from");
            spdlog::warn("GitHub : https://github.com/cdozdil/OptiScaler/releases");
            spdlog::warn("Nexus  : https://www.nexusmods.com/site/mods/986");
            spdlog::warn("If you paid for these files, you've been scammed!");
            spdlog::warn("DO NOT USE IN MULTIPLAYER GAMES");
            spdlog::warn("");
            spdlog::warn("LogLevel: {0}", Config::Instance()->LogLevel.value_or(2));

            // Check for working mode and attach hooks
            spdlog::info("");
            WorkingMode::Check();
            spdlog::info("");

            if (WorkingMode::IsModeFound())
            {
                AttachHooks();

                if (!WorkingMode::IsNvngxMode() && !Config::Instance()->DisableEarlyHooking.value_or(false) && Config::Instance()->OverlayMenu.value_or(true))
                {
                    ImGuiOverlayDx12::Dx12Bind();
                    ImGuiOverlayDx12::FSR3Bind();
                }
            }

            break;

        case DLL_PROCESS_DETACH:
#ifdef _DEBUG
            LOG_TRACE("DLL_PROCESS_DETACH from module: {0:X}", (UINT64)hModule);
#endif
            spdlog::info("");
            spdlog::info("Unloading OptiScaler");
            CloseLogger();
            DetachHooks();

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
