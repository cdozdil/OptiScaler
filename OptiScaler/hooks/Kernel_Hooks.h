#pragma once

#include <pch.h>

#include <Util.h>
#include <State.h>
#include <Config.h>
#include <DllNames.h>

#include <proxies/Kernel32_Proxy.h>
#include <proxies/KernelBase_Proxy.h>
#include <proxies/NVNGX_Proxy.h>
#include <proxies/XeSS_Proxy.h>
#include <proxies/FfxApi_Proxy.h>
#include <proxies/Gdi32_Proxy.h>
#include <proxies/Dxgi_Proxy.h>
#include <proxies/D3D12_Proxy.h>
#include <proxies/Streamline_Proxy.h>
#include <inputs/FSR2_Dx12.h>
#include <inputs/FSR3_Dx12.h>
#include <inputs/FfxApiExe_Dx12.h>

#include <spoofing/Dxgi_Spoofing.h>
#include <spoofing/Vulkan_Spoofing.h>

#include <hooks/HooksDx.h>
#include <hooks/HooksVk.h>

// Enables hooking of GetModuleHandle
// which might create issues, not tested very well
//#define HOOK_GET_MODULE

#ifdef HOOK_GET_MODULE
// Handle nvngx.dll calls on GetModule handle
//#define GET_MODULE_NVNGX

// Handle Opti dll calls on GetModule handle
#define GET_MODULE_DLL
#endif

class KernelHooks
{
private:
    inline static Kernel32Proxy::PFN_FreeLibrary o_K32_FreeLibrary = nullptr;
    inline static Kernel32Proxy::PFN_LoadLibraryA o_K32_LoadLibraryA = nullptr;
    inline static Kernel32Proxy::PFN_LoadLibraryW o_K32_LoadLibraryW = nullptr;
    inline static Kernel32Proxy::PFN_LoadLibraryExA o_K32_LoadLibraryExA = nullptr;
    inline static Kernel32Proxy::PFN_LoadLibraryExW o_K32_LoadLibraryExW = nullptr;
    inline static Kernel32Proxy::PFN_GetProcAddress o_K32_GetProcAddress = nullptr;

    inline static KernelBaseProxy::PFN_FreeLibrary o_KB_FreeLibrary = nullptr;
    inline static KernelBaseProxy::PFN_LoadLibraryExA o_KB_LoadLibraryExA = nullptr;
    inline static KernelBaseProxy::PFN_LoadLibraryExW o_KB_LoadLibraryExW = nullptr;
    inline static KernelBaseProxy::PFN_GetProcAddress o_KB_GetProcAddress = nullptr;

    inline static bool _overlayMethodsCalled = false;

    inline static HMODULE LoadLibraryCheck(std::string lcaseLibName, LPCSTR lpLibFullPath)
    {
        LOG_TRACE("{}", lcaseLibName);

        // If Opti is not loading as nvngx.dll
        if (!State::Instance().enablerAvailable && !State::Instance().isWorkingAsNvngx)
        {
            // exe path
            auto exePath = Util::ExePath().parent_path().wstring();

            for (size_t i = 0; i < exePath.size(); i++)
                exePath[i] = std::tolower(exePath[i]);

            auto pos = lcaseLibName.rfind(wstring_to_string(exePath));

            if (Config::Instance()->DlssInputs.value_or_default() && CheckDllName(&lcaseLibName, &nvngxNames) &&
                (!Config::Instance()->HookOriginalNvngxOnly.value_or_default() || pos == std::string::npos))
            {
                LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibName);
                //loadCount++;

                return dllModule;
            }
        }

        if (!State::Instance().isWorkingAsNvngx && (!State::Instance().isDxgiMode || !State::Instance().skipDxgiLoadChecks) && CheckDllName(&lcaseLibName, &dllNames))
        {
            LOG_INFO("{0} call returning this dll!", lcaseLibName);
            //loadCount++;

            return dllModule;
        }

        // NvApi64.dll
        if (CheckDllName(&lcaseLibName, &nvapiNames)) {
            if (!State::Instance().enablerAvailable && Config::Instance()->OverrideNvapiDll.value_or_default())
            {
                LOG_INFO("{0} call!", lcaseLibName);

                auto nvapi = LoadNvApi();

                // Nvapihooks intentionally won't load nvapi so have to make sure it's loaded
                if (nvapi != nullptr) {
                    NvApiHooks::Hook(nvapi);
                    return nvapi;
                }
            }
            else
            {
                auto nvapi = GetModuleHandleA(lcaseLibName.c_str());

                // Try to load nvapi only from system32, like the original call would
                if (nvapi == nullptr)
                {
                    nvapi = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
                }

                if (nvapi != nullptr)
                    NvApiHooks::Hook(nvapi);

                // AMD without nvapi override should fall through
            }
        }

        // sl.interposer.dll
        if (Config::Instance()->FGType.value_or_default() == FGType::Nukems && CheckDllName(&lcaseLibName, &streamlineNames))
        {
            auto streamlineModule = KernelBaseProxy::LoadLibraryExA_()(lpLibFullPath, NULL, 0);

            if (streamlineModule != nullptr)
            {
                hookStreamline(streamlineModule);
            }
            else
            {
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);
            }

            return streamlineModule;
        }

        // nvngx_dlss
        if (Config::Instance()->DLSSEnabled.value_or_default() && Config::Instance()->NVNGX_DLSS_Library.has_value() && CheckDllName(&lcaseLibName, &nvngxDlssNames))
        {
            auto nvngxDlss = LoadNvngxDlss(string_to_wstring(lcaseLibName));

            if (nvngxDlss == nullptr)
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return nvngxDlss;
        }

        // NGX OTA
        // Try to catch something like this: c:\programdata/nvidia/ngx/models//dlss/versions/20316673/files/160_e658700.bin
        if (lcaseLibName.ends_with(".bin"))
        {
            auto loadedBin = KernelBaseProxy::LoadLibraryExA_()(lpLibFullPath, NULL, 0);

            if (loadedBin && lcaseLibName.contains("/versions/"))
            {
                if (lcaseLibName.contains("/dlss/")) {
                    State::Instance().NGX_OTA_Dlss = lpLibFullPath;
                }

                if (lcaseLibName.contains("/dlssd/")) {
                    State::Instance().NGX_OTA_Dlssd = lpLibFullPath;
                }
            }
            return loadedBin;
        }

        if (Config::Instance()->FGDisableOverlays.value_or_default() && CheckDllName(&lcaseLibName, &blockOverlayNames))
        {
            LOG_DEBUG("Trying to load overlay dll: {}", lcaseLibName);
            return (HMODULE)1;
        }
        else if (CheckDllName(&lcaseLibName, &overlayNames))
        {
            LOG_DEBUG("Overlay dll!");

            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                if (!_overlayMethodsCalled && DxgiProxy::Module() != nullptr)
                {
                    LOG_INFO("Calling CreateDxgiFactory methods for overlay!");
                    IDXGIFactory* factory = nullptr;
                    IDXGIFactory1* factory1 = nullptr;
                    IDXGIFactory2* factory2 = nullptr;

                    if (DxgiProxy::CreateDxgiFactory_()(__uuidof(factory), &factory) == S_OK && factory != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory ok");
                        factory->Release();
                    }

                    if (DxgiProxy::CreateDxgiFactory1_()(__uuidof(factory1), &factory1) == S_OK && factory1 != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory1 ok");
                        factory1->Release();
                    }

                    if (DxgiProxy::CreateDxgiFactory2_()(0, __uuidof(factory2), &factory2) == S_OK && factory2 != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory2 ok");
                        factory2->Release();
                    }

                    _overlayMethodsCalled = true;
                }

                return module;
            }
        }

        // Hooks
        if (CheckDllName(&lcaseLibName, &dx11Names) && Config::Instance()->OverlayMenu.value_or_default())
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HooksDx::HookDx11(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &dx12Names) && Config::Instance()->OverlayMenu.value_or_default())
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                D3d12Proxy::Init(module);
                HooksDx::HookDx12();
            }
            else
            {
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);
            }

            return module;
        }

        if (CheckDllName(&lcaseLibName, &vkNames))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                if (!State::Instance().enablerAvailable)
                {
                    HookForVulkanSpoofing(module);
                    HookForVulkanExtensionSpoofing(module);
                    HookForVulkanVRAMSpoofing(module);
                }

                if (Config::Instance()->OverlayMenu.value_or_default())
                    HooksVk::HookVk(module);
            }
            else
            {
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);
            }

            return module;
        }

        if (!State::Instance().skipDxgiLoadChecks && CheckDllName(&lcaseLibName, &dxgiNames))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

            if (module != nullptr)
            {
                DxgiProxy::Init(module);

                if (!State::Instance().enablerAvailable && Config::Instance()->DxgiSpoofing.value_or_default())
                    HookDxgiForSpoofing();

                if (Config::Instance()->OverlayMenu.value_or_default())
                    HooksDx::HookDxgi();
            }
            else
            {
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);
            }

            return module;
        }

        if (CheckDllName(&lcaseLibName, &fsr2Names))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR2Inputs(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &fsr2BENames))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR2Dx12Inputs(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &fsr3Names))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR3Inputs(module);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &fsr3BENames))
        {
            auto module = KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR3Dx12Inputs(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &xessNames))
        {
            auto module = LoadLibxess(string_to_wstring(lcaseLibName)); // KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                XeSSProxy::HookXeSS(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }
                
        if (CheckDllName(&lcaseLibName, &xessDx11Names))
        {
            auto module = LoadLibxessDx11(string_to_wstring(lcaseLibName)); // KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                XeSSProxy::HookXeSSDx11(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &ffxDx12Names))
        {
            auto module = LoadFfxapiDx12(string_to_wstring(lcaseLibName));  // KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                FfxApiProxy::InitFfxDx12(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        if (CheckDllName(&lcaseLibName, &ffxVkNames))
        {
            auto module = LoadFfxapiVk(string_to_wstring(lcaseLibName)); // KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                FfxApiProxy::InitFfxVk(module);
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibName);

            return module;
        }

        return nullptr;
    }

    inline static HMODULE LoadLibraryCheckW(std::wstring lcaseLibName, LPCWSTR lpLibFullPath)
    {
        auto lcaseLibNameA = wstring_to_string(lcaseLibName);
        LOG_TRACE("{}", lcaseLibNameA);

        // If Opti is not loading as nvngx.dll
        if (!State::Instance().enablerAvailable && !State::Instance().isWorkingAsNvngx)
        {
            // exe path
            auto exePath = Util::ExePath().parent_path().wstring();

            for (size_t i = 0; i < exePath.size(); i++)
                exePath[i] = std::tolower(exePath[i]);

            auto pos = lcaseLibName.rfind(exePath);

            if (Config::Instance()->DlssInputs.value_or_default() && CheckDllNameW(&lcaseLibName, &nvngxNamesW) &&
                (!Config::Instance()->HookOriginalNvngxOnly.value_or_default() || pos == std::string::npos))
            {
                LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

                //if (!dontCount)
                //loadCount++;

                return dllModule;
            }
        }

        if (!State::Instance().isWorkingAsNvngx && (!State::Instance().isDxgiMode || !State::Instance().skipDxgiLoadChecks) && CheckDllNameW(&lcaseLibName, &dllNamesW))
        {
            LOG_INFO("{0} call returning this dll!", lcaseLibNameA);

            //if (!dontCount)
            //loadCount++;

            return dllModule;
        }

        // nvngx_dlss
        if (Config::Instance()->DLSSEnabled.value_or_default() && Config::Instance()->NVNGX_DLSS_Library.has_value() && CheckDllNameW(&lcaseLibName, &nvngxDlssNamesW))
        {
            auto nvngxDlss = LoadNvngxDlss(lcaseLibName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
            else
                LOG_ERROR("Trying to load dll: {}", lcaseLibNameA);
        }

        // NGX OTA
        // Try to catch something like this: c:\programdata/nvidia/ngx/models//dlss/versions/20316673/files/160_e658700.bin
        if (lcaseLibName.ends_with(L".bin"))
        {
            auto loadedBin = KernelBaseProxy::LoadLibraryExW_()(lpLibFullPath, NULL, 0);

            if (loadedBin && lcaseLibName.contains(L"/versions/"))
            {
                if (lcaseLibName.contains(L"/dlss/")) {
                    State::Instance().NGX_OTA_Dlss = wstring_to_string(lpLibFullPath);
                }

                if (lcaseLibName.contains(L"/dlssd/")) {
                    State::Instance().NGX_OTA_Dlssd = wstring_to_string(lpLibFullPath);
                }
            }
            return loadedBin;
        }

        // NvApi64.dll
        if (CheckDllNameW(&lcaseLibName, &nvapiNamesW)) {
            if (!State::Instance().enablerAvailable && Config::Instance()->OverrideNvapiDll.value_or_default())
            {
                LOG_INFO("{0} call!", lcaseLibNameA);

                auto nvapi = LoadNvApi();

                // Nvapihooks intentionally won't load nvapi so have to make sure it's loaded
                if (nvapi != nullptr) {
                    NvApiHooks::Hook(nvapi);
                    return nvapi;
                }
            }
            else
            {
                auto nvapi = GetModuleHandleW(lcaseLibName.c_str());

                // Try to load nvapi only from system32, like the original call would
                if (nvapi == nullptr)
                {
                    nvapi = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
                }

                if (nvapi != nullptr)
                    NvApiHooks::Hook(nvapi);

                // AMD without nvapi override should fall through
            }
        }

        // sl.interposer.dll
        if (Config::Instance()->FGType.value_or_default() == FGType::Nukems && CheckDllNameW(&lcaseLibName, &streamlineNamesW))
        {
            auto streamlineModule = KernelBaseProxy::LoadLibraryExW_()(lpLibFullPath, NULL, 0);

            if (streamlineModule != nullptr)
            {
                hookStreamline(streamlineModule);
            }
            else
            {
                LOG_ERROR("Trying to load dll: {}", lcaseLibNameA);
            }

            return streamlineModule;
        }


        if (Config::Instance()->FGDisableOverlays.value_or_default() && CheckDllNameW(&lcaseLibName, &blockOverlayNamesW))
        {
            LOG_DEBUG("Trying to load overlay dll: {}", wstring_to_string(lcaseLibName));
            return (HMODULE)1;
        }
        else if (CheckDllNameW(&lcaseLibName, &overlayNamesW))
        {
            LOG_DEBUG("Overlay dll!");

            // If we hook CreateSwapChainForHwnd & CreateSwapChainForCoreWindow here
            // Order of CreateSwapChain calls become
            // Game -> Overlay -> Opti 
            // and Overlays really does not like Opti's wrapped swapchain
            // If we skip hooking here first Steam hook CreateSwapChainForHwnd & CreateSwapChainForCoreWindow
            // Then hopefully Opti hook and call order become
            // Game -> Opti -> Overlay 
            // And Opti menu works with Overlay without issues

            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                if (!_overlayMethodsCalled && DxgiProxy::Module() != nullptr)
                {
                    LOG_INFO("Calling CreateDxgiFactory methods for overlay!");
                    IDXGIFactory* factory = nullptr;
                    IDXGIFactory1* factory1 = nullptr;
                    IDXGIFactory2* factory2 = nullptr;

                    if (DxgiProxy::CreateDxgiFactory_()(__uuidof(factory), &factory) == S_OK && factory != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory ok");
                        factory->Release();
                    }

                    if (DxgiProxy::CreateDxgiFactory1_()(__uuidof(factory1), &factory1) == S_OK && factory1 != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory1 ok");
                        factory1->Release();
                    }

                    if (DxgiProxy::CreateDxgiFactory2_()(0, __uuidof(factory2), &factory2) == S_OK && factory2 != nullptr)
                    {
                        LOG_DEBUG("CreateDxgiFactory2 ok");
                        factory2->Release();
                    }

                    _overlayMethodsCalled = true;
                }

                return module;
            }
        }

        // Hooks
        if (CheckDllNameW(&lcaseLibName, &dx11NamesW) && Config::Instance()->OverlayMenu.value_or_default())
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HooksDx::HookDx11(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &dx12NamesW) && Config::Instance()->OverlayMenu.value_or_default())
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                D3d12Proxy::Init(module);
                HooksDx::HookDx12();
            }

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &vkNamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
            {
                if (!State::Instance().enablerAvailable)
                {
                    HookForVulkanSpoofing(module);
                    HookForVulkanExtensionSpoofing(module);
                    HookForVulkanVRAMSpoofing(module);
                }

                if (Config::Instance()->OverlayMenu.value_or_default())
                    HooksVk::HookVk(module);
            }

            return module;
        }

        if (!State::Instance().skipDxgiLoadChecks && CheckDllNameW(&lcaseLibName, &dxgiNamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

            if (module != nullptr)
            {
                DxgiProxy::Init(module);

                if (!State::Instance().enablerAvailable && Config::Instance()->DxgiSpoofing.value_or_default())
                    HookDxgiForSpoofing();

                if (Config::Instance()->OverlayMenu.value_or_default())
                    HooksDx::HookDxgi();
            }
        }

        if (CheckDllNameW(&lcaseLibName, &fsr2NamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR2Inputs(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &fsr2BENamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR2Dx12Inputs(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &fsr3NamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR3Inputs(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &fsr3BENamesW))
        {
            auto module = KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                HookFSR3Dx12Inputs(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &xessNamesW))
        {
            auto module = LoadLibxess(lcaseLibName); // KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                XeSSProxy::HookXeSS(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &xessDx11NamesW))
        {
            auto module = LoadLibxessDx11(lcaseLibName); // KernelBaseProxy::LoadLibraryExA_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                XeSSProxy::HookXeSSDx11(module);
            else
                LOG_ERROR("Trying to load dll: {}", wstring_to_string(lcaseLibName));

            return module;
        }


        if (CheckDllNameW(&lcaseLibName, &ffxDx12NamesW))
        {
            auto module = LoadFfxapiDx12(lcaseLibName); // KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                FfxApiProxy::InitFfxDx12(module);

            return module;
        }

        if (CheckDllNameW(&lcaseLibName, &ffxVkNamesW))
        {
            auto module = LoadFfxapiVk(lcaseLibName); // KernelBaseProxy::LoadLibraryExW_()(lcaseLibName.c_str(), NULL, 0);

            if (module != nullptr)
                FfxApiProxy::InitFfxVk(module);

            return module;
        }

        return nullptr;
    }

    static HMODULE LoadNvApi()
    {
        HMODULE nvapi = nullptr;

        if (Config::Instance()->NvapiDllPath.has_value())
        {
            nvapi = KernelBaseProxy::LoadLibraryExW_()(Config::Instance()->NvapiDllPath->c_str(), NULL, 0);

            if (nvapi != nullptr)
            {
                LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(Config::Instance()->NvapiDllPath.value()));
                return nvapi;
            }
        }

        if (nvapi == nullptr)
        {
            auto localPath = Util::DllPath().parent_path() / L"nvapi64.dll";
            nvapi = KernelBaseProxy::LoadLibraryExW_()(localPath.wstring().c_str(), NULL, 0);

            if (nvapi != nullptr)
            {
                LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(localPath.wstring()));
                return nvapi;
            }
        }

        if (nvapi == nullptr)
        {
            nvapi = KernelBaseProxy::LoadLibraryExW_()(L"nvapi64.dll", NULL, 0);

            if (nvapi != nullptr)
            {
                LOG_WARN("nvapi64.dll loaded from system!");
                return nvapi;
            }
        }

        return nullptr;
    }

    static HMODULE LoadNvngxDlss(std::wstring originalPath)
    {
        HMODULE nvngxDlss = nullptr;

        if (Config::Instance()->NVNGX_DLSS_Library.has_value())
        {
            nvngxDlss = KernelBaseProxy::LoadLibraryExW_()(Config::Instance()->NVNGX_DLSS_Library.value().c_str(), NULL, 0);

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
            nvngxDlss = KernelBaseProxy::LoadLibraryExW_()(originalPath.c_str(), NULL, 0);

            if (nvngxDlss != nullptr)
            {
                LOG_INFO("nvngx_dlss.dll loaded from {0}", wstring_to_string(originalPath));
                return nvngxDlss;
            }
        }

        return nullptr;
    }

    static HMODULE LoadLibxess(std::wstring originalPath)
    {
        HMODULE libxess = nullptr;

        if (Config::Instance()->XeSSLibrary.has_value())
        {
            std::filesystem::path libPath(Config::Instance()->XeSSLibrary.value().c_str());

            if(libPath.has_filename())
                libxess = KernelBaseProxy::LoadLibraryExW_()(libPath.c_str(), NULL, 0);
            else
                libxess = KernelBaseProxy::LoadLibraryExW_()((libPath / L"libxess.dll").c_str(), NULL, 0);

            if (libxess != nullptr)
            {
                LOG_INFO("libxess.dll loaded from {0}", wstring_to_string(Config::Instance()->XeSSLibrary.value()));
                return libxess;
            }
            else
            {
                LOG_WARN("libxess.dll can't found at {0}", wstring_to_string(Config::Instance()->XeSSLibrary.value()));
            }
        }

        if (libxess == nullptr)
        {
            libxess = KernelBaseProxy::LoadLibraryExW_()(originalPath.c_str(), NULL, 0);

            if (libxess != nullptr)
            {
                LOG_INFO("libxess.dll loaded from {0}", wstring_to_string(originalPath));
                return libxess;
            }
        }

        return nullptr;
    }

    static HMODULE LoadLibxessDx11(std::wstring originalPath)
    {
        HMODULE libxess = nullptr;

        if (Config::Instance()->XeSSDx11Library.has_value())
        {
            std::filesystem::path libPath(Config::Instance()->XeSSDx11Library.value().c_str());

            if(libPath.has_filename())
                libxess = KernelBaseProxy::LoadLibraryExW_()(libPath.c_str(), NULL, 0);
            else
                libxess = KernelBaseProxy::LoadLibraryExW_()((libPath / L"libxess_dx11.dll").c_str(), NULL, 0);

            if (libxess != nullptr)
            {
                LOG_INFO("libxess_dx11.dll loaded from {0}", wstring_to_string(Config::Instance()->XeSSDx11Library.value()));
                return libxess;
            }
            else
            {
                LOG_WARN("libxess_dx11.dll can't found at {0}", wstring_to_string(Config::Instance()->XeSSDx11Library.value()));
            }
        }

        if (libxess == nullptr)
        {
            libxess = KernelBaseProxy::LoadLibraryExW_()(originalPath.c_str(), NULL, 0);

            if (libxess != nullptr)
            {
                LOG_INFO("libxess_dx11.dll loaded from {0}", wstring_to_string(originalPath));
                return libxess;
            }
        }

        return nullptr;
    }

    static HMODULE LoadFfxapiDx12(std::wstring originalPath)
    {
        HMODULE ffxDx12 = nullptr;

        if (Config::Instance()->FfxDx12Path.has_value())
        {
            std::filesystem::path libPath(Config::Instance()->FfxDx12Path.value().c_str());

            if (libPath.has_filename())
                ffxDx12 = KernelBaseProxy::LoadLibraryExW_()(libPath.c_str(), NULL, 0);
            else
                ffxDx12 = KernelBaseProxy::LoadLibraryExW_()((libPath / L"amd_fidelityfx_dx12.dll").c_str(), NULL, 0);

            if (ffxDx12 != nullptr)
            {
                LOG_INFO("amd_fidelityfx_dx12.dll loaded from {0}", wstring_to_string(Config::Instance()->FfxDx12Path.value()));
                return ffxDx12;
            }
            else
            {
                LOG_WARN("amd_fidelityfx_dx12.dll can't found at {0}", wstring_to_string(Config::Instance()->FfxDx12Path.value()));
            }
        }

        if (ffxDx12 == nullptr)
        {
            ffxDx12 = KernelBaseProxy::LoadLibraryExW_()(originalPath.c_str(), NULL, 0);

            if (ffxDx12 != nullptr)
            {
                LOG_INFO("amd_fidelityfx_dx12.dll loaded from {0}", wstring_to_string(originalPath));
                return ffxDx12;
            }
        }

        return nullptr;
    }
        
    static HMODULE LoadFfxapiVk(std::wstring originalPath)
    {
        HMODULE ffxVk = nullptr;

        if (Config::Instance()->FfxVkPath.has_value())
        {
            std::filesystem::path libPath(Config::Instance()->FfxVkPath.value().c_str());

            if (libPath.has_filename())
                ffxVk = KernelBaseProxy::LoadLibraryExW_()(libPath.c_str(), NULL, 0);
            else
                ffxVk = KernelBaseProxy::LoadLibraryExW_()((libPath / L"amd_fidelityfx_vk.dll").c_str(), NULL, 0);

            if (ffxVk != nullptr)
            {
                LOG_INFO("amd_fidelityfx_vk.dll loaded from {0}", wstring_to_string(Config::Instance()->FfxVkPath.value()));
                return ffxVk;
            }
            else
            {
                LOG_WARN("amd_fidelityfx_vk.dll can't found at {0}", wstring_to_string(Config::Instance()->FfxVkPath.value()));
            }
        }

        if (ffxVk == nullptr)
        {
            ffxVk = KernelBaseProxy::LoadLibraryExW_()(originalPath.c_str(), NULL, 0);

            if (ffxVk != nullptr)
            {
                LOG_INFO("amd_fidelityfx_vk.dll loaded from {0}", wstring_to_string(originalPath));
                return ffxVk;
            }
        }

        return nullptr;
    }

    static BOOL hk_K32_FreeLibrary(HMODULE lpLibrary)
    {
        if (lpLibrary == nullptr)
            return FALSE;

        if (lpLibrary == dllModule)
        {
            //if (loadCount > 0)
            //    loadCount--;

            LOG_INFO("Call for this module");

            //if (loadCount == 0)
            // return Kernel32Proxy::FreeLibrary_()(lpLibrary);
            //else
            return TRUE;
        }

        return o_K32_FreeLibrary(lpLibrary);
    }

    static BOOL hk_KB_FreeLibrary(HMODULE lpLibrary)
    {
        if (lpLibrary == nullptr)
            return FALSE;

        if (lpLibrary == dllModule)
        {
            //if (loadCount > 0)
            //    loadCount--;

            LOG_INFO("Call for this module");

            //if (loadCount == 0)
            // return Kernel32Proxy::FreeLibrary_()(lpLibrary);
            //else
            return TRUE;
        }

        return o_KB_FreeLibrary(lpLibrary);
    }

    static HMODULE hk_KB_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::string libName(lpLibFileName);
        std::string lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_KB_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
            }

            auto dllName = State::SkipDllName();
            auto pos = lcaseLibName.rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_KB_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
            }
        }

        auto moduleHandle = LoadLibraryCheck(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        auto result = o_KB_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
        return result;
    }

    static HMODULE hk_KB_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::wstring libName(lpLibFileName);
        std::wstring lcaseLibName(libName);

        LOG_DEBUG("{}", wstring_to_string(lcaseLibName));

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_KB_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
            }

            auto dllName = State::SkipDllName();
            auto pos = wstring_to_string(lcaseLibName).rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_KB_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
            }
        }

        auto moduleHandle = LoadLibraryCheckW(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        auto result = o_KB_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
        return result;
    }

    static HMODULE hk_K32_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::string libName(lpLibFileName);
        std::string lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_K32_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
            }

            auto dllName = State::SkipDllName();
            auto pos = lcaseLibName.rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_K32_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
            }
        }

#if _DEBUG
        LOG_DEBUG("{}", lcaseLibName);
#endif

        auto moduleHandle = LoadLibraryCheck(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        auto result = o_K32_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
        return result;
    }

    static HMODULE hk_K32_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::wstring libName(lpLibFileName);
        std::wstring lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_K32_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
            }

            auto dllName = State::SkipDllName();
            auto pos = wstring_to_string(lcaseLibName).rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_K32_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
            }
        }

#if _DEBUG
        LOG_DEBUG("{}", wstring_to_string(lcaseLibName));
#endif

        auto moduleHandle = LoadLibraryCheckW(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        auto result = o_K32_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
        return result;
    }

    static HMODULE hk_K32_LoadLibraryA(LPCSTR lpLibFileName)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::string libName(lpLibFileName);
        std::string lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_K32_LoadLibraryA(lpLibFileName);
            }

            auto dllName = State::SkipDllName();
            auto pos = lcaseLibName.rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", lcaseLibName);
                return o_K32_LoadLibraryA(lpLibFileName);
            }
        }

#if _DEBUG
        LOG_DEBUG("{}", lcaseLibName);
#endif
        auto moduleHandle = LoadLibraryCheck(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        return o_K32_LoadLibraryA(lpLibFileName);
    }

    static HMODULE hk_K32_LoadLibraryW(LPCWSTR lpLibFileName)
    {
        if (lpLibFileName == nullptr)
            return NULL;

        std::wstring libName(lpLibFileName);
        std::wstring lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        if (State::SkipDllChecks())
        {
            if (State::SkipDllName() == "")
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_K32_LoadLibraryW(lpLibFileName);
            }

            auto dllName = State::SkipDllName();
            auto pos = wstring_to_string(lcaseLibName).rfind(dllName);

            // -4 for extension `.dll`
            if (pos == (lcaseLibName.length() - dllName.length()) || pos == (lcaseLibName.length() - dllName.length() - 4))
            {
                LOG_TRACE("Skip checks for: {}", wstring_to_string(lcaseLibName));
                return o_K32_LoadLibraryW(lpLibFileName);
            }
        }

#if _DEBUG
        LOG_DEBUG("{}", wstring_to_string(lcaseLibName));
#endif

        auto moduleHandle = LoadLibraryCheckW(lcaseLibName, lpLibFileName);

        // skip loading of dll
        if (moduleHandle == (HMODULE)1)
        {
            SetLastError(ERROR_ACCESS_DENIED);
            return NULL;
        }

        if (moduleHandle != nullptr)
            return moduleHandle;

        return o_K32_LoadLibraryW(lpLibFileName);
    }

    inline static UINT customD3D12SDKVersion = 615;
    inline static const char8_t* customD3D12SDKPath = u8".\\D3D12_Optiscaler\\"; //Hardcoded for now

    static FARPROC hk_K32_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
    {
        if ((size_t)lpProcName < 0x000000000000F000)
        {
            if (hModule == dllModule)
                LOG_TRACE("Ordinal call: {:X}", (size_t)lpProcName);

            return o_K32_GetProcAddress(hModule, lpProcName);
        }

        if (hModule == dllModule && lpProcName != nullptr)
            LOG_TRACE("Trying to get process address of {0}", lpProcName);

        // For Agility SDK Upgrade
        if (Config::Instance()->FsrAgilitySDKUpgrade.value_or_default())
        {
            HMODULE mod_mainExe = nullptr;
            Kernel32Proxy::GetModuleHandleExW_()(2u, 0i64, &mod_mainExe);
            if (hModule == mod_mainExe && lpProcName != nullptr)
            {
                if (strcmp(lpProcName, "D3D12SDKVersion") == 0)
                {
                    LOG_INFO("D3D12SDKVersion call, returning this version!");
                    return (FARPROC)&customD3D12SDKVersion;
                }

                if (strcmp(lpProcName, "D3D12SDKPath") == 0)
                {
                    LOG_INFO("D3D12SDKPath call, returning this path!");
                    return (FARPROC)&customD3D12SDKPath;
                }
            }
        }

        if (State::Instance().isRunningOnLinux && lpProcName != nullptr &&
            hModule == Kernel32Proxy::GetModuleHandleW_()(L"gdi32.dll") && lstrcmpA(lpProcName, "D3DKMTEnumAdapters2") == 0)
            return (FARPROC)&customD3DKMTEnumAdapters2;

        return o_K32_GetProcAddress(hModule, lpProcName);
    }

    static FARPROC hk_KB_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
    {
        if ((size_t)lpProcName < 0x000000000000F000)
        {
            if (hModule == dllModule)
                LOG_TRACE("Ordinal call: {:X}", (size_t)lpProcName);

            return o_K32_GetProcAddress(hModule, lpProcName);
        }

        if (hModule == dllModule && lpProcName != nullptr)
            LOG_TRACE("Trying to get process address of {0}", lpProcName);

        // For Agility SDK Upgrade
        if (Config::Instance()->FsrAgilitySDKUpgrade.value_or_default())
        {
            HMODULE mod_mainExe = nullptr;
            Kernel32Proxy::GetModuleHandleExW_()(2u, 0i64, &mod_mainExe);
            if (hModule == mod_mainExe && lpProcName != nullptr)
            {
                if (strcmp(lpProcName, "D3D12SDKVersion") == 0)
                {
                    LOG_INFO("D3D12SDKVersion call, returning this version!");
                    return (FARPROC)&customD3D12SDKVersion;
                }

                if (strcmp(lpProcName, "D3D12SDKPath") == 0)
                {
                    LOG_INFO("D3D12SDKPath call, returning this path!");
                    return (FARPROC)&customD3D12SDKPath;
                }
            }
        }

        if (State::Instance().isRunningOnLinux && lpProcName != nullptr &&
            hModule == Kernel32Proxy::GetModuleHandleW_()(L"gdi32.dll") && lstrcmpA(lpProcName, "D3DKMTEnumAdapters2") == 0)
            return (FARPROC)&customD3DKMTEnumAdapters2;

        return o_KB_GetProcAddress(hModule, lpProcName);
    }

public:
    static void Hook()
    {
        if (o_K32_FreeLibrary != nullptr)
            return;

        LOG_DEBUG("");

        o_K32_FreeLibrary = Kernel32Proxy::Hook_FreeLibrary(hk_K32_FreeLibrary);
        o_K32_LoadLibraryA = Kernel32Proxy::Hook_LoadLibraryA(hk_K32_LoadLibraryA);
        o_K32_LoadLibraryW = Kernel32Proxy::Hook_LoadLibraryW(hk_K32_LoadLibraryW);
        o_K32_LoadLibraryExA = Kernel32Proxy::Hook_LoadLibraryExA(hk_K32_LoadLibraryExA);
        o_K32_LoadLibraryExW = Kernel32Proxy::Hook_LoadLibraryExW(hk_K32_LoadLibraryExW);
        o_K32_GetProcAddress = Kernel32Proxy::Hook_GetProcAddress(hk_K32_GetProcAddress);

        // These hooks cause stability regressions
        //o_KB_FreeLibrary = KernelBaseProxy::Hook_FreeLibrary(hk_KB_FreeLibrary);
        //o_KB_LoadLibraryExA = KernelBaseProxy::Hook_LoadLibraryExA(hk_KB_LoadLibraryExA);
        //o_KB_LoadLibraryExW = KernelBaseProxy::Hook_LoadLibraryExW(hk_KB_LoadLibraryExW);
        o_KB_GetProcAddress = KernelBaseProxy::Hook_GetProcAddress(hk_KB_GetProcAddress);
    }
};


