#pragma once
#include "dllmain.h"
#include "resource.h"

#include "Logger.h"
#include "Util.h"
#include "NVNGX_Proxy.h"
#include "XeSS_Proxy.h"
#include "FfxApi_Proxy.h"
#include "FSR2_Dx12.h"
#include "FSR3_Dx12.h"

#include "hooks/HooksDx.h"
#include "hooks/HooksVk.h"

#include <vulkan/vulkan_core.h>

// Enables hooking of GetModuleHandle
// which might create issues, not tested very well
//#define HOOK_GET_MODULE

#ifdef HOOK_GET_MODULE
// Handle nvngx.dll calls on GetModule handle
//#define GET_MODULE_NVNGX

// Handle Opti dll calls on GetModule handle
#define GET_MODULE_DLL
#endif

#pragma warning (disable : 4996)

typedef BOOL(*PFN_FreeLibrary)(HMODULE lpLibrary);
typedef HMODULE(*PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(*PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(*PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(*PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef FARPROC(*PFN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

typedef HMODULE(*PFN_GetModuleHandleA)(LPCSTR lpModuleName);
typedef HMODULE(*PFN_GetModuleHandleW)(LPCWSTR lpModuleName);
typedef BOOL(*PFN_GetModuleHandleExA)(DWORD dwFlags, LPCSTR lpModuleName, HMODULE* phModule);
typedef BOOL(*PFN_GetModuleHandleExW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule);

typedef const char* (CDECL* PFN_wine_get_version)(void);

typedef struct VkDummyProps
{
    VkStructureType    sType;
    void* pNext;
} VkDummyProps;

static PFN_FreeLibrary o_FreeLibrary = nullptr;
static PFN_LoadLibraryA o_LoadLibraryA = nullptr;
static PFN_LoadLibraryW o_LoadLibraryW = nullptr;
static PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
static PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;
static PFN_GetProcAddress o_GetProcAddress = nullptr;
static PFN_GetModuleHandleA o_GetModuleHandleA = nullptr;
static PFN_GetModuleHandleW o_GetModuleHandleW = nullptr;
static PFN_GetModuleHandleExA o_GetModuleHandleExA = nullptr;
static PFN_GetModuleHandleExW o_GetModuleHandleExW = nullptr;

static PFN_vkCreateDevice o_vkCreateDevice = nullptr;
static PFN_vkCreateInstance o_vkCreateInstance = nullptr;
static PFN_vkGetPhysicalDeviceProperties o_vkGetPhysicalDeviceProperties = nullptr;
static PFN_vkGetPhysicalDeviceProperties2 o_vkGetPhysicalDeviceProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceProperties2KHR o_vkGetPhysicalDeviceProperties2KHR = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties o_vkGetPhysicalDeviceMemoryProperties = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties2 o_vkGetPhysicalDeviceMemoryProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties2KHR o_vkGetPhysicalDeviceMemoryProperties2KHR = nullptr;
static PFN_vkEnumerateDeviceExtensionProperties o_vkEnumerateDeviceExtensionProperties = nullptr;
static PFN_vkEnumerateInstanceExtensionProperties o_vkEnumerateInstanceExtensionProperties = nullptr;

static uint32_t vkEnumerateInstanceExtensionPropertiesCount = 0;
static uint32_t vkEnumerateDeviceExtensionPropertiesCount = 0;


inline std::vector<std::string> dllNames;
inline std::vector<std::wstring> dllNamesW;
inline std::vector<std::string> upscalerNames = { "nvngx.dll", "nvngx", "libxess.dll", "libxess" };
inline std::vector<std::wstring> upscalerNamesW = { L"nvngx.dll", L"nvngx", L"libxess.dll", L"libxess" };
inline std::vector<std::string> nvngxDlss = { "nvngx_dlss.dll", "nvngx_dlss", };
inline std::vector<std::wstring> nvngxDlssW = { L"nvngx_dlss.dll", L"nvngx_dlss", };
inline std::vector<std::string> nvapiNames = { "nvapi64.dll", "nvapi64", };
inline std::vector<std::wstring> nvapiNamesW = { L"nvapi64.dll", L"nvapi64", };
inline std::vector<std::string> dx11Names = { "d3d11.dll", "d3d11", };
inline std::vector<std::wstring> dx11NamesW = { L"d3d11.dll", L"d3d11", };
inline std::vector<std::string> dx12Names = { "d3d12.dll", "d3d12", };
inline std::vector<std::wstring> dx12NamesW = { L"d3d12.dll", L"d3d12", };
inline std::vector<std::string> dxgiNames = { "dxgi.dll", "dxgi", };
inline std::vector<std::wstring> dxgiNamesW = { L"dxgi.dll", L"dxgi", };
inline std::vector<std::string> vkNames = { "vulkan-1.dll", "vulkan-1", };
inline std::vector<std::wstring> vkNamesW = { L"vulkan-1.dll", L"vulkan-1", };
inline std::vector<std::string> overlayNames = { "eosovh-win32-shipping.dll", "eosovh-win32-shipping", "eosovh-win64-shipping.dll", "eosovh-win64-shipping",
                                                 "gameoverlayrenderer64", "gameoverlayrenderer64.dll", "gameoverlayrenderer", "gameoverlayrenderer.dll", };
                                                 // "socialclubd3d12renderer", "socialclubd3d12renderer.dll" }; // games are not working without this dll
inline std::vector<std::wstring> overlayNamesW = { L"eosovh-win32-shipping.dll", L"eosovh-win32-shipping", L"eosovh-win64-shipping.dll", L"eosovh-win64-shipping",
                                                   L"gameoverlayrenderer64", L"gameoverlayrenderer64.dll", L"gameoverlayrenderer", L"gameoverlayrenderer.dll", };
                                                  // L"socialclubd3d12renderer", L"socialclubd3d12renderer.dll" }; // games are not working without this dll

inline std::vector<std::string> fsr2Dx12Names = { "ffx_fsr2_api_dx12_x64.dll", "ffx_fsr2_api_dx12_x64" };
inline std::vector<std::string> fsr2Names = { "ffx_fsr2_api_x64.dll", "ffx_fsr2_api_x64" };
inline std::vector<std::wstring> fsr2Dx12NamesW = { L"ffx_fsr2_api_dx12_x64.dll", L"ffx_fsr2_api_dx12_x64" };
inline std::vector<std::wstring> fsr2NamesW = { L"ffx_fsr2_api_x64.dll", L"ffx_fsr2_api_x64" };

inline std::vector<std::string> fsr3Dx12Names = { "ffx_backend_dx12_x64.dll", "ffx_backend_dx12_x64" };
inline std::vector<std::string> fsr3Names = { "ffx_fsr3upscaler_x64.dll", "ffx_fsr3upscaler_x64" };
inline std::vector<std::wstring> fsr3Dx12NamesW = { L"ffx_backend_dx12_x64.dll", L"ffx_backend_dx12_x64" };
inline std::vector<std::wstring> fsr3NamesW = { L"ffx_fsr3upscaler_x64.dll", L"ffx_fsr3upscaler_x64" };

static int loadCount = 0;
static bool dontCount = false;
static bool skipDllLoadChecks = false;
static bool isNvngxMode = false;
static bool isWorkingWithEnabler = false;
static bool isNvngxAvailable = false;

void AttachHooks();
void DetachHooks();
HMODULE LoadNvApi();
HMODULE LoadNvgxDlss(std::wstring originalPath);
void HookForDxgiSpoofing();
void HookForVulkanSpoofing();
void HookForVulkanExtensionSpoofing();
void HookForVulkanVRAMSpoofing();

inline static bool CheckDllName(std::string* dllName, std::vector<std::string>* namesList)
{
    for (size_t i = 0; i < namesList->size(); i++)
    {
        auto name = namesList->at(i);
        auto pos = dllName->rfind(name);

        if (pos != std::string::npos && pos == (dllName->size() - name.size()))
            return true;
    }

    return false;
}

inline static bool CheckDllNameW(std::wstring* dllName, std::vector<std::wstring>* namesList)
{
    auto found = false;

    for (size_t i = 0; i < namesList->size(); i++)
    {
        auto name = namesList->at(i);
        auto pos = dllName->rfind(name);

        if (pos != std::string::npos && pos == (dllName->size() - name.size()))
            return true;
    }

    return false;
}

inline static HMODULE LoadLibraryCheck(std::string lcaseLibName)
{
    LOG_TRACE("{}", lcaseLibName);

    // If Opti is not loading as nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->upscalerDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos = lcaseLibName.rfind(wstring_to_string(exePath));

        if (CheckDllName(&lcaseLibName, &upscalerNames) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (CheckDllName(&lcaseLibName, &nvapiNames)) {
        if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
        {
            LOG_INFO("{0} call!", lcaseLibName);

            LoadNvApi();
            auto nvapi = GetModuleHandleA(lcaseLibName.c_str());

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
                nvapi = o_LoadLibraryExA(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

            if (nvapi != nullptr)
                NvApiHooks::Hook(nvapi);

            // AMD without nvapi override should fall through
        }
    }

    // nvngx_dlss
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->DLSSLibrary.has_value() && CheckDllName(&lcaseLibName, &nvngxDlss))
    {
        auto nvngxDlss = LoadNvgxDlss(string_to_wstring(lcaseLibName));

        if (nvngxDlss != nullptr)
            return nvngxDlss;
    }

    if (Config::Instance()->FGUseFGSwapChain.value_or(true))
    {
        if (Config::Instance()->FGDisableOverlays.value_or(true) && CheckDllName(&lcaseLibName, &overlayNames))
        {
            LOG_DEBUG("Trying to load overlay dll: {}", lcaseLibName);
            return (HMODULE)1;
        }
    }

    // Hooks
    if (CheckDllName(&lcaseLibName, &dx11Names) && Config::Instance()->OverlayMenu.value_or(true))
    {
        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HooksDx::HookDx11();
            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllName(&lcaseLibName, &dx12Names) && Config::Instance()->OverlayMenu.value_or(true))
    {
        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HooksDx::HookDx12();
            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllName(&lcaseLibName, &vkNames))
    {
        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HookForVulkanSpoofing();
            HookForVulkanExtensionSpoofing();
            HookForVulkanVRAMSpoofing();

            if (Config::Instance()->OverlayMenu.value_or(true))
                HooksVk::HookVk();

            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllName(&lcaseLibName, &dxgiNames))
    {
        skipDllLoadChecks = true;
        HookForDxgiSpoofing();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksDx::HookDxgi();

        skipDllLoadChecks = false;
    }

    if (CheckDllName(&lcaseLibName, &fsr2Names))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        HookFSR2Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllName(&lcaseLibName, &fsr2Dx12Names))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        HookFSR2Dx12Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllName(&lcaseLibName, &fsr3Names))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        HookFSR3Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllName(&lcaseLibName, &fsr3Dx12Names))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryA(lcaseLibName.c_str());

        HookFSR3Dx12Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (!isNvngxMode && CheckDllName(&lcaseLibName, &dllNames))
    {
        LOG_INFO("{0} call returning this dll!", lcaseLibName);

        if (!dontCount)
            loadCount++;

        return dllModule;
    }

    return nullptr;
}

inline static HMODULE LoadLibraryCheckW(std::wstring lcaseLibName)
{
    auto lcaseLibNameA = wstring_to_string(lcaseLibName);
    LOG_TRACE("{}", lcaseLibNameA);

    // If Opti is not loading as nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->upscalerDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos = lcaseLibName.rfind(exePath);

        if (CheckDllNameW(&lcaseLibName, &upscalerNamesW) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // nvngx_dlss
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->DLSSLibrary.has_value() && CheckDllNameW(&lcaseLibName, &nvngxDlssW))
    {
        auto nvngxDlss = LoadNvgxDlss(lcaseLibName);

        if (nvngxDlss != nullptr)
            return nvngxDlss;
    }

    // NvApi64.dll
    if (CheckDllNameW(&lcaseLibName, &nvapiNamesW)) {
        if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            LoadNvApi();
            auto nvapi = GetModuleHandleW(lcaseLibName.c_str());

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
                nvapi = o_LoadLibraryExW(lcaseLibName.c_str(), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

            if (nvapi != nullptr)
                NvApiHooks::Hook(nvapi);

            // AMD without nvapi override should fall through
        }
    }

    if (Config::Instance()->FGUseFGSwapChain.value_or(true))
    {
        if (Config::Instance()->FGDisableOverlays.value_or(true) && CheckDllNameW(&lcaseLibName, &overlayNamesW))
        {
            LOG_DEBUG("Trying to load overlay dll: {}", lcaseLibNameA);
            return (HMODULE)1;
        }
    }

    // Hooks
    if (CheckDllNameW(&lcaseLibName, &dx11NamesW) && Config::Instance()->OverlayMenu.value_or(true))
    {
        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HooksDx::HookDx11();
            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllNameW(&lcaseLibName, &dx12NamesW) && Config::Instance()->OverlayMenu.value_or(true))
    {
        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HooksDx::HookDx12();
            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllNameW(&lcaseLibName, &vkNamesW))
    {
        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        if (module != nullptr)
        {
            skipDllLoadChecks = true;
            HookForVulkanSpoofing();
            HookForVulkanExtensionSpoofing();
            HookForVulkanVRAMSpoofing();

            if (Config::Instance()->OverlayMenu.value_or(true))
                HooksVk::HookVk();
            skipDllLoadChecks = false;

            return module;
        }
    }

    if (CheckDllNameW(&lcaseLibName, &dxgiNamesW))
    {
        skipDllLoadChecks = true;
        HookForDxgiSpoofing();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksDx::HookDxgi();
        skipDllLoadChecks = false;
    }

    if (CheckDllNameW(&lcaseLibName, &fsr2NamesW))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        HookFSR2Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllNameW(&lcaseLibName, &fsr2Dx12NamesW))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        HookFSR2Dx12Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllNameW(&lcaseLibName, &fsr3NamesW))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        HookFSR3Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (CheckDllNameW(&lcaseLibName, &fsr3Dx12NamesW))
    {
        skipDllLoadChecks = true;

        auto module = o_LoadLibraryW(lcaseLibName.c_str());

        HookFSR3Dx12Inputs(module);

        skipDllLoadChecks = false;

        return module;
    }

    if (!isNvngxMode && CheckDllNameW(&lcaseLibName, &dllNamesW))
    {
        LOG_INFO("{0} call returning this dll!", lcaseLibNameA);

        if (!dontCount)
            loadCount++;

        return dllModule;
    }

    return nullptr;
}

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

#pragma region Load & nvngxDlss Library hooks

static FARPROC hkGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == dllModule)
        LOG_DEBUG("Trying to get process address of {0}", lpProcName);

    return o_GetProcAddress(hModule, lpProcName);
}

#ifdef HOOK_GET_MODULE

static HMODULE hkGetModuleHandleA(LPCSTR lpModuleName)
{
    if (!skipGetModuleHandle && lpModuleName != nullptr)
    {
        std::string libName(lpModuleName);
        std::string lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        LOG_DEBUG_ONLY("{}", lcaseLibName);

        int pos = 0;

#ifdef GET_MODULE_NVNGX

        // nvngx dll
        pos = lcaseLibName.rfind(nvngxA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            return dllModule;
        }

        // nvngx 
        pos = lcaseLibName.rfind(nvngxExA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            return dllModule;
        }

#endif

#ifdef GET_MODULE_DLL

        // Opti 
        if (CheckDllName(&lcaseLibName, &dllNames))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            return dllModule;
        }

#endif
    }

    return o_GetModuleHandleA(lpModuleName);
}

static HMODULE hkGetModuleHandleW(LPCWSTR lpModuleName)
{
    if (!skipGetModuleHandle && lpModuleName != nullptr)
    {
        std::wstring libName(lpModuleName);
        std::wstring lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        auto lcaseLibNameA = wstring_to_string(lcaseLibName);

        LOG_DEBUG_ONLY("{}", lcaseLibNameA);

        int pos = 0;

#ifdef GET_MODULE_NVNGX

        // nvngx dll
        pos = lcaseLibName.rfind(nvngxW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            return dllModule;
        }

        // nvngx
        pos = lcaseLibName.rfind(nvngxExW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            return dllModule;
        }
#endif

#ifdef GET_MODULE_DLL

        // Opti 
        if (CheckDllNameW(&lcaseLibName, &dllNamesW))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            return dllModule;
        }

#endif
    }

    return o_GetModuleHandleW(lpModuleName);
}

static BOOL hkGetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE* phModule)
{
    if (!skipGetModuleHandle && lpModuleName != nullptr)
    {
        std::string libName(lpModuleName);
        std::string lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        LOG_DEBUG_ONLY("{}", lcaseLibName);

        int pos = 0;

#ifdef GET_MODULE_NVNGX

        // nvngx dll
        pos = lcaseLibName.rfind(nvngxA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            *phModule = dllModule;
            return TRUE;
        }

        // nvngx
        pos = lcaseLibName.rfind(nvngxExA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            *phModule = dllModule;
            return TRUE;
        }

#endif

#ifdef GET_MODULE_DLL

        // Opti 
        if (CheckDllName(&lcaseLibName, &dllNames))
        {
            LOG_INFO("{0} call, returning this dll!", libName);
            *phModule = dllModule;
            return TRUE;
        }


#endif

    }

    return o_GetModuleHandleExA(dwFlags, lpModuleName, phModule);
}

static BOOL hkGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule)
{
    if (!skipGetModuleHandle && lpModuleName != nullptr)
    {
        std::wstring libName(lpModuleName);
        std::wstring lcaseLibName(libName);

        for (size_t i = 0; i < lcaseLibName.size(); i++)
            lcaseLibName[i] = std::tolower(lcaseLibName[i]);

        auto lcaseLibNameA = wstring_to_string(lcaseLibName);

        LOG_DEBUG_ONLY("{}", lcaseLibNameA);

        int pos = 0;

#ifdef GET_MODULE_NVNGX

        // nvngx dll
        pos = lcaseLibName.rfind(nvngxW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            *phModule = dllModule;
            return TRUE;
        }

        // nvngx
        pos = lcaseLibName.rfind(nvngxExW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            *phModule = dllModule;
            return TRUE;
        }

#endif

#ifdef GET_MODULE_DLL

        // Opti 
        if (CheckDllNameW(&lcaseLibName, &dllNamesW))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);
            *phModule = dllModule;
            return TRUE;
        }


#endif
    }

    return o_GetModuleHandleExW(dwFlags, lpModuleName, phModule);
}

#endif

static BOOL hkFreeLibrary(HMODULE lpLibrary)
{
    if (lpLibrary == nullptr)
        return FALSE;

    if (lpLibrary == dllModule)
    {
        if (loadCount > 0)
            loadCount--;

        LOG_INFO("Call for this module loadCount: {0}", loadCount);

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

    if (skipDllLoadChecks)
        return o_LoadLibraryA(lpLibFileName);;

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif // DEBUG

    auto moduleHandle = LoadLibraryCheck(lcaseLibName);

    // skip loading of dll
    if (moduleHandle == (HMODULE)1)
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    if (moduleHandle != nullptr)
        return moduleHandle;

    dontCount = true;
    auto result = o_LoadLibraryA(lpLibFileName);
    dontCount = false;

    return result;
}

static HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName)
{
    if (lpLibFileName == nullptr)
        return NULL;

    if (skipDllLoadChecks)
        return o_LoadLibraryW(lpLibFileName);;

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", wstring_to_string(lcaseLibName));
#endif // DEBUG

    auto moduleHandle = LoadLibraryCheckW(lcaseLibName);

    // skip loading of dll
    if (moduleHandle == (HMODULE)1)
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    if (moduleHandle != nullptr)
        return moduleHandle;

    dontCount = true;
    auto result = o_LoadLibraryW(lpLibFileName);
    dontCount = false;

    return result;
}

static HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    if (lpLibFileName == nullptr)
        return NULL;

    if (skipDllLoadChecks)
        return o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif

    auto moduleHandle = LoadLibraryCheck(lcaseLibName);

    // skip loading of dll
    if (moduleHandle == (HMODULE)1)
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    if (moduleHandle != nullptr)
        return moduleHandle;

    dontCount = true;
    auto result = o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

static HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    if (lpLibFileName == nullptr)
        return NULL;

    if (skipDllLoadChecks)
        return o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", wstring_to_string(lcaseLibName));
#endif

    auto moduleHandle = LoadLibraryCheckW(lcaseLibName);

    // skip loading of dll
    if (moduleHandle == (HMODULE)1)
    {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    if (moduleHandle != nullptr)
        return moduleHandle;

    dontCount = true;
    auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

#pragma endregion

#pragma region Vulkan Hooks

static void hkvkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t)Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryHeaps[i].size = newMemSize;
        }
    }
}

static void hkvkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr || pMemoryProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryProperties.memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t)Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryProperties.memoryHeaps[i].size = newMemSize;
        }
    }
}

static void hkvkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr || pMemoryProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryProperties.memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t)Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryProperties.memoryHeaps[i].size = newMemSize;
        }
    }
}

static void hkvkGetPhysicalDeviceProperties(VkPhysicalDevice physical_device, VkPhysicalDeviceProperties* properties)
{
    o_vkGetPhysicalDeviceProperties(physical_device, properties);

    if (!Config::Instance()->skipSpoofing)
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090"));
        std::strcpy(properties->deviceName, deviceName.c_str());
        properties->vendorID = 0x10de;
        properties->deviceID = 0x2684;
        properties->driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

static void hkvkGetPhysicalDeviceProperties2(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2(phys_dev, properties2);

    if (!Config::Instance()->skipSpoofing)
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090"));
        std::strcpy(properties2->properties.deviceName, deviceName.c_str());
        properties2->properties.vendorID = 0x10de;
        properties2->properties.deviceID = 0x2684;
        properties2->properties.driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);

        auto next = (VkDummyProps*)properties2->pNext;

        while (next != nullptr)
        {
            if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
            {
                auto ddp = (VkPhysicalDeviceDriverProperties*)(void*)next;
                ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                std::strcpy(ddp->driverName, "NVIDIA");
                std::strcpy(ddp->driverInfo, "999.99");
            }

            next = (VkDummyProps*)next->pNext;
        }
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

static void hkvkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2KHR(phys_dev, properties2);

    if (!Config::Instance()->skipSpoofing)
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or(L"NVIDIA GeForce RTX 4090"));
        std::strcpy(properties2->properties.deviceName, deviceName.c_str());
        properties2->properties.vendorID = 0x10de;
        properties2->properties.deviceID = 0x2684;
        properties2->properties.driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);

        auto next = (VkDummyProps*)properties2->pNext;

        while (next != nullptr)
        {
            if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
            {
                auto ddp = (VkPhysicalDeviceDriverProperties*)(void*)next;
                ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                std::strcpy(ddp->driverName, "NVIDIA");
                std::strcpy(ddp->driverInfo, "999.99");
            }

            next = (VkDummyProps*)next->pNext;
        }
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

static VkResult hkvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    if (pCreateInfo->pApplicationInfo->pApplicationName != nullptr)
        LOG_DEBUG("for {0}", pCreateInfo->pApplicationInfo->pApplicationName);

    LOG_DEBUG("extensions ({0}):", pCreateInfo->enabledExtensionCount);
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);

    LOG_DEBUG("layers ({0}):", pCreateInfo->enabledLayerCount);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledLayerNames[i]);

    // Skip spoofing for Intel Arc
    Config::Instance()->skipSpoofing = true;
    auto result = o_vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    Config::Instance()->skipSpoofing = false;

    LOG_DEBUG("o_vkCreateInstance result: {0:X}", (INT)result);

    auto head = (VkBaseInStructure*)pCreateInfo;

    while (head->pNext != nullptr)
    {
        head = (VkBaseInStructure*)head->pNext;
        LOG_DEBUG("o_vkCreateInstance type: {0:X}", (UINT)head->sType);
    }

    return result;
}

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LOG_FUNC();

    if (!Config::Instance()->VulkanExtensionSpoofing.value_or(false))
    {
        LOG_DEBUG("extension spoofing is disabled");

        Config::Instance()->skipSpoofing = true;
        return o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        Config::Instance()->skipSpoofing = false;
    }

    LOG_DEBUG("layers ({0}):", pCreateInfo->enabledLayerCount);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledLayerNames[i]);

    std::vector<const char*> newExtensionList;

    auto bVK_KHR_get_memory_requirements2 = false;

    LOG_DEBUG("checking extensions and removing VK_NVX_BINARY_IMPORT & VK_NVX_IMAGE_VIEW_HANDLE from list");
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        if (std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_BINARY_IMPORT_EXTENSION_NAME) == 0 ||
            std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME) == 0 ||
            std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME) == 0)
        {
            LOG_DEBUG("removing {0}", pCreateInfo->ppEnabledExtensionNames[i]);
        }
        else
        {
            LOG_DEBUG("adding {0}", pCreateInfo->ppEnabledExtensionNames[i]);
            newExtensionList.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    if (FfxApiProxy::VULKAN_CreateContext() != nullptr)
        newExtensionList.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(newExtensionList.size());
    pCreateInfo->ppEnabledExtensionNames = newExtensionList.data();

    LOG_DEBUG("final extension count: {0}", pCreateInfo->enabledExtensionCount);
    LOG_DEBUG("final extensions:");

    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);

    // Skip spoofing for Intel Arc
    Config::Instance()->skipSpoofing = true;
    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    Config::Instance()->skipSpoofing = false;

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;

    if (pProperties == nullptr)
        count = 0;

    auto result = o_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("o_vkEnumerateDeviceExtensionProperties({0}, {1}) result: {2:X}", pLayerName, count, (UINT)result);
        return result;
    }

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        *pPropertyCount += 3;
        vkEnumerateDeviceExtensionPropertiesCount = *pPropertyCount;
        LOG_TRACE("hkvkEnumerateDeviceExtensionProperties({0}) count: {1}", pLayerName, vkEnumerateDeviceExtensionPropertiesCount);
        return result;
    }

    if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    {
        if (count == vkEnumerateDeviceExtensionPropertiesCount)
            *pPropertyCount = count;

        VkExtensionProperties bi{ VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

        VkExtensionProperties ivh{ VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));

        VkExtensionProperties mpva{ VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 3], &mpva, sizeof(VkExtensionProperties));

        LOG_DEBUG("Extensions returned:");
        for (size_t i = 0; i < *pPropertyCount; i++)
        {
            LOG_DEBUG("  {}", pProperties[i].extensionName);
        }
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;

    if (pProperties == nullptr)
        count = 0;

    auto result = o_vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("o_vkEnumerateInstanceExtensionProperties({0}, {1}) result: {2:X}", pLayerName, count, (UINT)result);
        return result;
    }

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        //*pPropertyCount += 2;
        //vkEnumerateInstanceExtensionPropertiesCount = *pPropertyCount;
        LOG_TRACE("hkvkEnumerateDeviceExtensionProperties({0}) count: {1}", pLayerName, vkEnumerateDeviceExtensionPropertiesCount);
        return result;
    }

    //if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    //{
    //    if (vkEnumerateInstanceExtensionPropertiesCount == count)
    //        *pPropertyCount = count;

    //    VkExtensionProperties bi{ VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION };
    //    memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

    //    VkExtensionProperties ivh{ VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION };
    //    memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));

    //    LOG_DEBUG("Extensions returned:");
    //    for (size_t i = 0; i < *pPropertyCount; i++)
    //    {
    //        LOG_DEBUG("  {}", pProperties[i].extensionName);
    //    }
    //}

    LOG_DEBUG("Extensions returned:");
    for (size_t i = 0; i < *pPropertyCount; i++)
    {
        LOG_DEBUG("  {}", pProperties[i].extensionName);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

#pragma endregion

inline static void HookForDxgiSpoofing()
{
    // hook dxgi when not working as dxgi.dll
    if (dxgi.CreateDxgiFactory == nullptr && !isWorkingWithEnabler && !Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        LOG_INFO("DxgiSpoofing is enabled loading dxgi.dll");

        dxgi.CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
        dxgi.CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY_1)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
        dxgi.CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

        if (dxgi.CreateDxgiFactory != nullptr || dxgi.CreateDxgiFactory1 != nullptr || dxgi.CreateDxgiFactory2 != nullptr)
        {
            LOG_INFO("dxgi.dll found, hooking CreateDxgiFactory methods");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (dxgi.CreateDxgiFactory != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory, _CreateDXGIFactory);

            if (dxgi.CreateDxgiFactory1 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory1, _CreateDXGIFactory1);

            if (dxgi.CreateDxgiFactory2 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory2, _CreateDXGIFactory2);

            DetourTransactionCommit();
        }
    }
}

inline static void HookForVulkanSpoofing()
{
    if (!isNvngxMode && Config::Instance()->VulkanSpoofing.value_or(false) && o_vkGetPhysicalDeviceProperties == nullptr)
    {
        o_vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties"));
        o_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2"));
        o_vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2KHR"));

        if (o_vkGetPhysicalDeviceProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan device spoofing hooks");

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

inline static void HookForVulkanExtensionSpoofing()
{
    if (!isNvngxMode && Config::Instance()->VulkanExtensionSpoofing.value_or(false) && o_vkEnumerateInstanceExtensionProperties == nullptr)
    {
        o_vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(DetourFindFunction("vulkan-1.dll", "vkCreateDevice"));
        o_vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(DetourFindFunction("vulkan-1.dll", "vkCreateInstance"));
        o_vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(DetourFindFunction("vulkan-1.dll", "vkEnumerateInstanceExtensionProperties"));
        o_vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(DetourFindFunction("vulkan-1.dll", "vkEnumerateDeviceExtensionProperties"));

        if (o_vkEnumerateInstanceExtensionProperties != nullptr || o_vkEnumerateDeviceExtensionProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan extensions spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkEnumerateInstanceExtensionProperties)
                DetourAttach(&(PVOID&)o_vkEnumerateInstanceExtensionProperties, hkvkEnumerateInstanceExtensionProperties);

            if (o_vkEnumerateDeviceExtensionProperties)
                DetourAttach(&(PVOID&)o_vkEnumerateDeviceExtensionProperties, hkvkEnumerateDeviceExtensionProperties);

            if (o_vkCreateDevice)
                DetourAttach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);

            if (o_vkCreateInstance)
                DetourAttach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);

            DetourTransactionCommit();
        }
    }
}

inline static void HookForVulkanVRAMSpoofing()
{
    if (!isNvngxMode && Config::Instance()->VulkanVRAM.has_value() && o_vkGetPhysicalDeviceMemoryProperties == nullptr)
    {
        o_vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceMemoryProperties"));
        o_vkGetPhysicalDeviceMemoryProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceMemoryProperties2"));
        o_vkGetPhysicalDeviceMemoryProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceMemoryProperties2KHR"));

        if (o_vkGetPhysicalDeviceMemoryProperties != nullptr || o_vkGetPhysicalDeviceMemoryProperties2 != nullptr || o_vkGetPhysicalDeviceMemoryProperties2KHR != nullptr)
        {
            LOG_INFO("Attaching Vulkan VRAM spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkGetPhysicalDeviceMemoryProperties)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceMemoryProperties, hkvkGetPhysicalDeviceMemoryProperties);

            if (o_vkGetPhysicalDeviceMemoryProperties2)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceMemoryProperties2, hkvkGetPhysicalDeviceMemoryProperties2);

            if (o_vkGetPhysicalDeviceMemoryProperties2KHR)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceMemoryProperties2KHR, hkvkGetPhysicalDeviceMemoryProperties2KHR);

            DetourTransactionCommit();
        }
    }
}

static void DetachHooks()
{
    if (!isNvngxMode)
    {
        HooksDx::UnHookDx();
        HooksVk::UnHookVk();
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

        if (!Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
        {
            if (dxgi.CreateDxgiFactory != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory, _CreateDXGIFactory);
                dxgi.CreateDxgiFactory = nullptr;
            }

            if (dxgi.CreateDxgiFactory1 != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory1, _CreateDXGIFactory1);
                dxgi.CreateDxgiFactory1 = nullptr;
            }

            if (dxgi.CreateDxgiFactory2 != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory2, _CreateDXGIFactory2);
                dxgi.CreateDxgiFactory2 = nullptr;
            }
        }

        DetourTransactionCommit();

        FreeLibrary(shared.dll);
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

#ifdef HOOK_GET_MODULE
        o_GetModuleHandleA = reinterpret_cast<PFN_GetModuleHandleA>(DetourFindFunction("kernel32.dll", "GetModuleHandleA"));
        o_GetModuleHandleW = reinterpret_cast<PFN_GetModuleHandleW>(DetourFindFunction("kernel32.dll", "GetModuleHandleW"));
        o_GetModuleHandleExA = reinterpret_cast<PFN_GetModuleHandleExA>(DetourFindFunction("kernel32.dll", "GetModuleHandleExA"));
        o_GetModuleHandleExW = reinterpret_cast<PFN_GetModuleHandleExW>(DetourFindFunction("kernel32.dll", "GetModuleHandleExW"));
#endif
#ifdef _DEBUG
        //o_GetProcAddress = reinterpret_cast<PFN_GetProcAddress>(DetourFindFunction("kernel32.dll", "GetProcAddress"));
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

#ifdef  HOOK_GET_MODULE

            if (o_GetModuleHandleA)
                DetourAttach(&(PVOID&)o_GetModuleHandleA, hkGetModuleHandleA);

            if (o_GetModuleHandleW)
                DetourAttach(&(PVOID&)o_GetModuleHandleW, hkGetModuleHandleW);

            if (o_GetModuleHandleExA)
                DetourAttach(&(PVOID&)o_GetModuleHandleExA, hkGetModuleHandleExA);

            if (o_GetModuleHandleExW)
                DetourAttach(&(PVOID&)o_GetModuleHandleExW, hkGetModuleHandleExW);

#endif //  HOOK_GET_MODULE

            if (o_GetProcAddress)
                DetourAttach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);

            DetourTransactionCommit();
        }
    }
}

static bool IsRunningOnWine()
{
    LOG_FUNC();

    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

    if (!ntdll)
    {
        LOG_WARN("Not running on NT!?!");
        return true;
    }

    auto pWineGetVersion = (PFN_wine_get_version)GetProcAddress(ntdll, "wine_get_version");

    if (pWineGetVersion)
    {
        LOG_INFO("Running on Wine {0}!", pWineGetVersion());
        return true;
    }

    LOG_WARN("Wine not detected");
    return false;
}

static void CheckWorkingMode()
{
    LOG_FUNC();

    bool modeFound = false;
    std::string filename = Util::DllPath().filename().string();
    std::string lCaseFilename(filename);
    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);
    std::filesystem::path pluginPath(Config::Instance()->PluginPath.value_or((Util::DllPath().parent_path() / L"plugins").wstring()));

    for (size_t i = 0; i < lCaseFilename.size(); i++)
        lCaseFilename[i] = std::tolower(lCaseFilename[i]);

    HMODULE dll = nullptr;

    do
    {
        if (lCaseFilename == "nvngx.dll" || lCaseFilename == "_nvngx.dll" || lCaseFilename == "libxess.dll" || lCaseFilename == "dlss-enabler-upscaler.dll")
        {
            LOG_INFO("OptiScaler working as native upscaler: {0}", filename);

            dllNames.push_back("OptiScaler_DontLoad.dll");
            dllNames.push_back("OptiScaler_DontLoad");
            dllNamesW.push_back(L"OptiScaler_DontLoad.dll");
            dllNamesW.push_back(L"OptiScaler_DontLoad");

            isNvngxMode = true;
            isWorkingWithEnabler = lCaseFilename == "dlss-enabler-upscaler.dll";

            if (isWorkingWithEnabler)
                Config::Instance()->LogToNGX = true;

            modeFound = true;
            break;
        }

        // version.dll
        if (lCaseFilename == "version.dll")
        {
            do
            {
                skipGetModuleHandle = true;
                auto pluginFilePath = pluginPath / L"version.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"version-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, version-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"version.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as version.dll, system dll loaded");

                skipGetModuleHandle = false;

            } while (false);

            if (dll != nullptr)
            {
                dllNames.push_back("version.dll");
                dllNames.push_back("version");
                dllNamesW.push_back(L"version.dll");
                dllNamesW.push_back(L"version");

                shared.LoadOriginalLibrary(dll);
                version.LoadOriginalLibrary(dll);

                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original version.dll!");
            }

            break;
        }

        // winmm.dll
        if (lCaseFilename == "winmm.dll")
        {
            do
            {
                skipGetModuleHandle = true;
                auto pluginFilePath = pluginPath / L"winmm.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winmm-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, winmm-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winmm.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winmm.dll, system dll loaded");
                skipGetModuleHandle = false;

            } while (false);

            if (dll != nullptr)
            {
                dllNames.push_back("winmm.dll");
                dllNames.push_back("winmm");
                dllNamesW.push_back(L"winmm.dll");
                dllNamesW.push_back(L"winmm");

                shared.LoadOriginalLibrary(dll);
                winmm.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winmm.dll!");
            }

            break;
        }

        // wininet.dll
        if (lCaseFilename == "wininet.dll")
        {
            do
            {
                skipGetModuleHandle = true;
                auto pluginFilePath = pluginPath / L"wininet.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"wininet-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, wininet-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"wininet.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as wininet.dll, system dll loaded");
                skipGetModuleHandle = false;

            } while (false);

            if (dll != nullptr)
            {
                dllNames.push_back("wininet.dll");
                dllNames.push_back("wininet");
                dllNamesW.push_back(L"wininet.dll");
                dllNamesW.push_back(L"wininet");

                shared.LoadOriginalLibrary(dll);
                wininet.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original wininet.dll!");
            }

            break;
        }

        // optiscaler.dll
        if (lCaseFilename == "optiscaler.asi")
        {
            LOG_INFO("OptiScaler working as OptiScaler.asi");

            // quick hack for testing
            dll = dllModule;

            dllNames.push_back("optiscaler.asi");
            dllNames.push_back("optiscaler");
            dllNamesW.push_back(L"optiscaler.asi");
            dllNamesW.push_back(L"optiscaler");

            modeFound = true;
            break;
        }

        // winhttp.dll
        if (lCaseFilename == "winhttp.dll")
        {
            do
            {
                skipGetModuleHandle = true;
                auto pluginFilePath = pluginPath / L"winhttp.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winhttp-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, winhttp-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winhttp.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winhttp.dll, system dll loaded");

                skipGetModuleHandle = false;

            } while (false);

            if (dll != nullptr)
            {
                dllNames.push_back("winhttp.dll");
                dllNames.push_back("winhttp");
                dllNamesW.push_back(L"winhttp.dll");
                dllNamesW.push_back(L"winhttp");

                shared.LoadOriginalLibrary(dll);
                winhttp.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winhttp.dll!");
            }

            break;
        }

        // dxgi.dll
        if (lCaseFilename == "dxgi.dll")
        {
            do
            {
                skipGetModuleHandle = true;
                auto pluginFilePath = pluginPath / L"dxgi.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"dxgi-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, dxgi-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"dxgi.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as dxgi.dll, system dll loaded");

                skipGetModuleHandle = false;
            } while (false);

            if (dll != nullptr)
            {
                dllNames.push_back("dxgi.dll");
                dllNames.push_back("dxgi");
                dllNamesW.push_back(L"dxgi.dll");
                dllNamesW.push_back(L"dxgi");

                dxgi.LoadOriginalLibrary(dll);

                Config::Instance()->IsDxgiMode = true;
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original dxgi.dll!");
            }

            break;
        }

    } while (false);

    if (modeFound)
    {
        Config::Instance()->WorkingAsNvngx = isNvngxMode && !isWorkingWithEnabler;
        Config::Instance()->OverlayMenu = (!isNvngxMode || isWorkingWithEnabler) && Config::Instance()->OverlayMenu.value_or(true);
        Config::Instance()->CheckUpscalerFiles();

        if (!isNvngxMode)
        {
            HMODULE dxgiModule = nullptr;
            dxgiModule = GetModuleHandle(L"dxgi.dll");
            if (dxgiModule != nullptr)
            {
                LOG_DEBUG("dxgi.dll already in memory");
                HookForDxgiSpoofing();
            }

            HMODULE vulkanModule = nullptr;
            vulkanModule = GetModuleHandle(L"vulkan-1.dll");
            if (vulkanModule != nullptr)
            {
                LOG_DEBUG("vulkan-1.dll already in memory");
                HookForVulkanSpoofing();
                HookForVulkanExtensionSpoofing();
                HookForVulkanVRAMSpoofing();
            }

            HMODULE nvapi64 = nullptr;
            nvapi64 = GetModuleHandle(L"nvapi64.dll");
            if (nvapi64 != nullptr)
            {
                LOG_DEBUG("nvapi64.dll already in memory");
                NvApiHooks::Hook(nvapi64);
            }

            // dx menu hooks
            HMODULE d3d11Module = nullptr;
            d3d11Module = GetModuleHandle(L"d3d11.dll");
            if (Config::Instance()->OverlayMenu.value() && d3d11Module != nullptr)
            {
                LOG_DEBUG("d3d11.dll already in memory");
                HooksDx::HookDx11();
            }

            HMODULE d3d12Module = nullptr;
            d3d12Module = GetModuleHandle(L"d3d12.dll");
            if (Config::Instance()->OverlayMenu.value() && d3d12Module != nullptr)
            {
                LOG_DEBUG("d3d12.dll already in memory");
                HooksDx::HookDx12();
            }

            if (Config::Instance()->OverlayMenu.value() && dxgiModule != nullptr)
                HooksDx::HookDxgi();

            if ((!Config::Instance()->FGUseFGSwapChain.value_or(true) || !Config::Instance()->OverlayMenu.value_or(true)) &&
                skHandle == nullptr && Config::Instance()->LoadSpecialK.value_or(false))
            {
                auto skFile = Util::DllPath().parent_path() / L"SpecialK64.dll";
                SetEnvironmentVariableW(L"RESHADE_DISABLE_GRAPHICS_HOOK", L"1");
                skHandle = LoadLibrary(skFile.c_str());
                LOG_INFO("Loading SpecialK64.dll, result: {0:X}", (UINT64)skHandle);
            }

            if (reshadeHandle == nullptr && Config::Instance()->LoadReShade.value_or(false))
            {
                auto rsFile = Util::DllPath().parent_path() / L"ReShade64.dll";
                SetEnvironmentVariableW(L"RESHADE_DISABLE_LOADING_CHECK", L"1");

                if (skHandle != nullptr)
                    SetEnvironmentVariableW(L"RESHADE_DISABLE_GRAPHICS_HOOK", L"1");

                reshadeHandle = LoadLibrary(rsFile.c_str());
                LOG_INFO("Loading ReShade64.dll, result: {0:X}", (size_t)reshadeHandle);
            }

            // vk menu hooks
            if (Config::Instance()->OverlayMenu.value() && (vulkanModule != nullptr || Config::Instance()->IsRunningOnLinux))
                HooksVk::HookVk();

            AttachHooks();
        }

        return;
    }

    LOG_ERROR("Unsupported dll name: {0}", filename);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    HMODULE handle = nullptr;

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

            if (Config::Instance()->FGUseFGSwapChain.value_or(true) && Config::Instance()->FGDisableOverlays.value_or(true))
                SetEnvironmentVariable(L"SteamNoOverlayUIDrawing", L"1");

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

            // Check for Wine
            skipGetModuleHandle = true;
            spdlog::info("");
            Config::Instance()->IsRunningOnLinux = IsRunningOnWine();
            skipGetModuleHandle = false;

            // Check if real DLSS available
            if (Config::Instance()->DLSSEnabled.value_or(true))
            {
                spdlog::info("");
                skipGetModuleHandle = true;
                NVNGXProxy::InitNVNGX();
                skipGetModuleHandle = false;

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

            // Check for working mode and attach hooks
            spdlog::info("");
            CheckWorkingMode();
            spdlog::info("");

            // Init XeSS proxy
            skipDllLoadChecks = true;

            if (!XeSSProxy::InitXeSS())
                spdlog::warn("Can't init XeSS!");

            // Init FfxApi proxy
            if (!FfxApiProxy::InitFfxDx12())
                spdlog::warn("Can't init Dx12 FfxApi!");

            if (!FfxApiProxy::InitFfxVk())
                spdlog::warn("Can't init Vulkan FfxApi!");

            spdlog::info("");
            HookFSR2ExeInputs();

            handle = GetModuleHandle(fsr2NamesW[0].c_str());
            if (handle != nullptr)
                HookFSR2Inputs(handle);

            handle = GetModuleHandle(fsr2Dx12NamesW[0].c_str());
            if (handle != nullptr)
                HookFSR2Dx12Inputs(handle);

            spdlog::info("");
            HookFSR3ExeInputs();

            handle = GetModuleHandle(fsr3NamesW[0].c_str());
            if (handle != nullptr)
                HookFSR3Inputs(handle);

            handle = GetModuleHandle(fsr3Dx12NamesW[0].c_str());
            if (handle != nullptr)
                HookFSR3Dx12Inputs(handle);

            skipDllLoadChecks = false;

            for (size_t i = 0; i < 300; i++)
            {
                Config::Instance()->frameTimes.push_back(0.0);
                Config::Instance()->upscaleTimes.push_back(0.0);
            }

            spdlog::info("");
            spdlog::info("Init done");
            spdlog::info("---------------------------------------------");
            spdlog::info("");

            break;

        case DLL_PROCESS_DETACH:
            // Unhooking and cleaning stuff causing issues during shutdown. 
            // Disabled for now to check if it cause any issues
            //DetachHooks();

            if (skHandle != nullptr)
                FreeLibrary(skHandle);

            if (reshadeHandle != nullptr)
                FreeLibrary(reshadeHandle);

            spdlog::info("");
            spdlog::info("DLL_PROCESS_DETACH");
            spdlog::info("Unloading OptiScaler");
            CloseLogger();

            break;

        case DLL_THREAD_ATTACH:
            LOG_DEBUG_ONLY("DLL_THREAD_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
            break;

        case DLL_THREAD_DETACH:
            LOG_DEBUG_ONLY("DLL_THREAD_DETACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
            break;

        default:
            LOG_WARN("Call reason: {0:X}", ul_reason_for_call);
            break;
    }

    return TRUE;
}
