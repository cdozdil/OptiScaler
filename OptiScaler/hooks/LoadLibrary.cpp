#include "LoadLibrary.h"

#include <Util.h>
#include <Config.h>
#include "Vulkan.h"
#include <WorkingMode.h>

#include <include/detours/detours.h>

typedef BOOL(*PFN_FreeLibrary)(HMODULE lpLibrary);
typedef HMODULE(*PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(*PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(*PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(*PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef FARPROC(*PFN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

static PFN_FreeLibrary o_FreeLibrary = nullptr;
static PFN_LoadLibraryA o_LoadLibraryA = nullptr;
static PFN_LoadLibraryW o_LoadLibraryW = nullptr;
static PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
static PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;
static PFN_GetProcAddress o_GetProcAddress = nullptr;

static std::vector<std::string> upscalerNames =
{
    "nvngx.dll",
    "nvngx",
    "libxess.dll",
    "libxess"
};

static std::vector<std::string> nvngxDlss =
{
    "nvngx_dlss.dll",
    "nvngx_dlss",
};

static std::vector<std::string> nvapiNames =
{
    "nvapi64.dll",
    "nvapi64",
};

static std::vector<std::wstring> upscalerNamesW =
{
    L"nvngx.dll",
    L"nvngx",
    L"libxess.dll",
    L"libxess"
};

static std::vector<std::wstring> nvngxDlssW =
{
    L"nvngx_dlss.dll",
    L"nvngx_dlss",
};

static std::vector<std::wstring> nvapiNamesW =
{
    L"nvapi64.dll",
    L"nvapi64",
};

static std::vector<std::wstring> dx11NamesW =
{
    L"d3d11.dll",
    L"d3d11",
};

static std::vector<std::string> dx11Names =
{
    "d3d11.dll",
    "d3d11",
};

static std::vector<std::wstring> dx12NamesW =
{
    L"d3d12.dll",
    L"d3d12",
};

static std::vector<std::string> dx12Names =
{
    "d3d12.dll",
    "d3d12",
};

inline std::vector<std::wstring> dxgiNamesW =
{
    L"dxgi.dll",
    L"dxgi",
};

static std::vector<std::string> dxgiNames =
{
    "dxgi.dll",
    "dxgi",
};

static std::vector<std::wstring> vkNamesW =
{
    L"vulkan-1.dll",
    L"vulkan-1",
};

static std::vector<std::string> vkNames =
{
    "vulkan-1.dll",
    "vulkan-1",
};

static bool dontCount = false;
static bool skipLoadChecks = false;
static UINT loadCount = 0;

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
    // If Opti is not loading as nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->upscalerDisableHook)
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
        if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
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

    // Hooks
    skipLoadChecks = true;

    if (CheckDllName(&lcaseLibName, &dx11Names) && Config::Instance()->OverlayMenu.value_or(true))
        HooksDx::HookDx11();

    if (CheckDllName(&lcaseLibName, &dx12Names) && Config::Instance()->OverlayMenu.value_or(true))
        HooksDx::HookDx12();

    if (CheckDllName(&lcaseLibName, &dxgiNames))
    {
        HookForDxgiSpoofing();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksDx::HookDxgi();
    }

    if (CheckDllName(&lcaseLibName, &vkNames))
    {
        VulkanHooks::Hook();
        VulkanHooks::HookExtension();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksVk::HookVk();
    }

    skipLoadChecks = false;

    if (!WorkingMode::IsNvngxMode() && CheckDllName(&lcaseLibName, &WorkingMode::DllNames()))
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

    // If Opti is not loading as nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->upscalerDisableHook)
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
        if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
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

    // Hooks
    skipLoadChecks = true;

    if (CheckDllNameW(&lcaseLibName, &dx11NamesW) && Config::Instance()->OverlayMenu.value_or(true))
        HooksDx::HookDx11();

    if (CheckDllNameW(&lcaseLibName, &dx12NamesW) && Config::Instance()->OverlayMenu.value_or(true))
        HooksDx::HookDx12();

    if (CheckDllNameW(&lcaseLibName, &dxgiNamesW))
    {
        HookForDxgiSpoofing();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksDx::HookDxgi();
    }

    if (CheckDllNameW(&lcaseLibName, &vkNamesW))
    {
        VulkanHooks::Hook();
        VulkanHooks::HookExtension();

        if (Config::Instance()->OverlayMenu.value_or(true))
            HooksVk::HookVk();
    }

    skipLoadChecks = false;

    if (!WorkingMode::IsNvngxMode() && CheckDllNameW(&lcaseLibName, &WorkingMode::DllNamesW()))
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

static FARPROC hkGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == dllModule)
        LOG_DEBUG("Trying to get process address of {0}", lpProcName);

    return o_GetProcAddress(hModule, lpProcName);
}

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
        {
            auto result = o_FreeLibrary(lpLibrary);
            LOG_DEBUG("o_FreeLibrary result: {0:X}", result);
        }
        else
        {
            return TRUE;
        }
    }

    return o_FreeLibrary(lpLibrary);
}

static HMODULE hkLoadLibraryA(LPCSTR lpLibFileName)
{
    if (lpLibFileName == nullptr)
        return NULL;

    if (skipLoadChecks)
        return o_LoadLibraryA(lpLibFileName);;

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif // DEBUG

    auto moduleHandle = LoadLibraryCheck(lcaseLibName);

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

    if (skipLoadChecks)
        return o_LoadLibraryW(lpLibFileName);;

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", wstring_to_string(lcaseLibName));
#endif // DEBUG

    auto moduleHandle = LoadLibraryCheckW(lcaseLibName);

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

    if (skipLoadChecks)
        return o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif

    auto moduleHandle = LoadLibraryCheck(lcaseLibName);

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

    if (skipLoadChecks)
        return o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef _DEBUG
    LOG_TRACE("call: {0}", wstring_to_string(lcaseLibName));
#endif

    auto moduleHandle = LoadLibraryCheckW(lcaseLibName);

    if (moduleHandle != nullptr)
        return moduleHandle;

    dontCount = true;
    auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

void LoadLibraryHooks::Hook()
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

            if (o_GetProcAddress)
                DetourAttach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);

            DetourTransactionCommit();
        }
    }
}
 