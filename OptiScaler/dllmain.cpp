#pragma once
#include "resource.h"

#include "Logger.h"
#include "Config.h"
#include "Working_Mode.h"

#include "hooks/Dxgi_Hooks.h"
#include "hooks/Vulkan_Hooks.h"
#include "hooks/LoadLibrary_Hooks.h"
#include "hooks/NvApi_Hooks.h"
#include "hooks/Nvngx_Hooks.h"

#include "imgui/imgui_overlay_dx12.h"

static void DetachHooks()
{
    Hooks::DetachLoadLibraryHooks();
    Hooks::DetachNvApiHooks();
    Hooks::DetachNvngxHooks();
    Hooks::DetachDxgiHooks();
    Hooks::DetachVulkanDeviceHooks();
    Hooks::DetachVulkanExtensionHooks();
}

static void AttachHooks()
{
    LOG_FUNC();

    // Dll redirect hooks
    if (!WorkingMode::IsNvngxMode())
        Hooks::AttachLoadLibraryHooks();

    // Nvapi spoofing fo 16xx
    if (WorkingMode::IsNvngxAvailable() && !Config::Instance()->OverrideNvapiDll.value_or(false))
        Hooks::AttachNvApiHooks();

    // Nvngx 16xx spoofing
    if (WorkingMode::IsNvngxAvailable())
        Hooks::AttachNvngxHooks();

    // Dxgi spoofing
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
        Hooks::AttachDxgiHooks();

    // Vulkan spoofing
    if (Config::Instance()->VulkanSpoofing.value_or(false))
        Hooks::AttachVulkanDeviceHooks();

    // Vulkan extension spoofing
    if (Config::Instance()->VulkanExtensionSpoofing.value_or(false))
        Hooks::AttachVulkanExtensionHooks();
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

            // Check for working mode
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
            spdlog::info("");
            spdlog::info("Unloading OptiScaler");
            CloseLogger();
            DetachHooks();

            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        default:
            LOG_WARN("Call reason: {0:X}", ul_reason_for_call);
            break;
    }

    return TRUE;
}
