#include "LoadLibrary_Hooks.h"

#include <pch.h>
#include <Config.h>
#include <Util.h>
#include <Working_Mode.h>

#include <detours.h>

typedef BOOL(WINAPI* PFN_FreeLibrary)(HMODULE lpLibrary);
typedef HMODULE(WINAPI* PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(WINAPI* PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef FARPROC(WINAPI* PFN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

static PFN_FreeLibrary o_FreeLibrary = nullptr;
static PFN_LoadLibraryA o_LoadLibraryA = nullptr;
static PFN_LoadLibraryW o_LoadLibraryW = nullptr;
static PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
static PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;
static PFN_GetProcAddress o_GetProcAddress = nullptr;

static int loadCount = 1;
static bool dontCount = false;

static std::string nvngxA("nvngx.dll");
static std::string nvngxExA("nvngx");
static std::wstring nvngxW(L"nvngx.dll");
static std::wstring nvngxExW(L"nvngx");

static std::string nvapiA("nvapi64.dll");
static std::string nvapiExA("nvapi64");
static std::wstring nvapiW(L"nvapi64.dll");
static std::wstring nvapiExW(L"nvapi64");

static std::string nvngxDlssA("nvngx_dlss.dll");
static std::string nvngxDlssExA("nvngx_dlss");
static std::wstring nvngxDlssW(L"nvngx_dlss.dll");
static std::wstring nvngxDlssExW(L"nvngx_dlss");

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
        LOG_INFO("Trying to get process address of {0}", lpProcName);

    return o_GetProcAddress(hModule, lpProcName);
}

static BOOL hkFreeLibrary(HMODULE lpLibrary)
{
    if (lpLibrary == nullptr)
        return FALSE;

    if (lpLibrary == dllModule)
    {
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

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

    size_t pos;

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif // DEBUG

    // If Opti is not loading nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos2 = lcaseLibName.rfind(wstring_to_string(exePath));
        pos = lcaseLibName.rfind(nvngxA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!WorkingMode::IsNvngxMode())
    {
        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameA());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameA().size()))
        {
            LOG_INFO("{0} call returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryA(lpLibFileName);
    dontCount = false;
    return result;
}

static HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName)
{
    LOG_FUNC();

    if (lpLibFileName == nullptr)
        return NULL;

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

    auto lcaseLibNameA = wstring_to_string(lcaseLibName);

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibNameA);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePathW = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePathW.size(); i++)
            exePathW[i] = std::tolower(exePathW[i]);

        auto pos2 = lcaseLibName.rfind(exePathW);
        pos = lcaseLibName.rfind(nvngxW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!WorkingMode::IsNvngxMode())
    {
        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameW());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameW().size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryW(lpLibFileName);
    dontCount = false
        ;
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
    LOG_TRACE("call: {0}", lcaseLibName);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos2 = lcaseLibName.rfind(wstring_to_string(exePath));
        pos = lcaseLibName.rfind(nvngxA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        pos = lcaseLibName.rfind(nvngxExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }

        pos = lcaseLibName.rfind(nvapiA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssExA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }

        pos = lcaseLibName.rfind(nvngxDlssA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!WorkingMode::IsNvngxMode())
    {
        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameA());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameA().size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameExA());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameExA().size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    dontCount = false;

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

    auto lcaseLibNameA = wstring_to_string(lcaseLibName);

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibNameA);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!WorkingMode::IsWorkingWithEnabler() && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePathW = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePathW.size(); i++)
            exePathW[i] = std::tolower(exePathW[i]);

        auto pos2 = lcaseLibName.rfind(exePathW);
        pos = lcaseLibName.rfind(nvngxW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        pos = lcaseLibName.rfind(nvngxExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!WorkingMode::IsWorkingWithEnabler() && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }

        pos = lcaseLibName.rfind(nvapiW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssExW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }

        pos = lcaseLibName.rfind(nvngxDlssW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!WorkingMode::IsNvngxMode())
    {
        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameW());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameW().size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        // Opti dll
        pos = lcaseLibName.rfind(WorkingMode::DllNameExW());
        if (pos != std::string::npos && pos == (lcaseLibName.size() - WorkingMode::DllNameExW().size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

#pragma endregion

void Hooks::AttachLoadLibraryHooks()
{
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

void Hooks::DetachLoadLibraryHooks()
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

    DetourTransactionCommit();
}
