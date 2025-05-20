#include "pch.h"

#include "Util.h"
#include "Config.h"

#include <shlobj.h>

extern HMODULE dllModule;

typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
typedef DWORD (*PFN_GetFileVersionInfoSizeW)(LPCWSTR lptstrFilename, LPDWORD lpdwHandle);
typedef BOOL (*PFN_GetFileVersionInfoW)(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef BOOL (*PFN_VerQueryValueW)(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen);

std::wstring Util::GetWindowTitle(HWND hwnd)
{
    const int maxLength = 512;
    wchar_t buffer[maxLength] = {0};

    // First, check if the window is valid and visible
    if (!IsWindow(hwnd) || !IsWindowVisible(hwnd))
        return L"";

    // Try to get the text using SendMessageTimeout to avoid hanging
    LRESULT result = 0;
    if (SendMessageTimeoutW(hwnd, WM_GETTEXT, (WPARAM) maxLength, (LPARAM) buffer, SMTO_ABORTIFHUNG | SMTO_BLOCK, 2,
                            (PDWORD_PTR) &result) != 0)
    {
        return std::wstring(buffer);
    }

    return L"";
}

bool Util::GetRealWindowsVersion(OSVERSIONINFOW& osInfo)
{
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr)
        {
            osInfo.dwOSVersionInfoSize = sizeof(osInfo);

            if (fxPtr(&osInfo) == 0) // STATUS_SUCCESS == 0
                return true;
        }
    }

    return false;
}

std::string Util::GetWindowsName(const OSVERSIONINFOW& os)
{
    DWORD major = os.dwMajorVersion;
    DWORD minor = os.dwMinorVersion;
    DWORD build = os.dwBuildNumber;

    if (major == 10 && build >= 22000)
        return "Windows 11";
    if (major == 10)
        return "Windows 10";
    if (major == 6 && minor == 3)
        return "Windows 8.1";
    if (major == 6 && minor == 2)
        return "Windows 8";
    if (major == 6 && minor == 1)
        return "Windows 7";
    if (major == 6 && minor == 0)
        return "Windows Vista";
    if (major == 5 && minor == 1)
        return "Windows XP";

    return "Unknown Windows Version";
}

std::wstring Util::GetExeProductName()
{
    // In case of working ag version.dll
    // Loading original dll from system

    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);

    auto dll = LoadLibraryExW((sysPath / L"version.dll").c_str(), NULL, 0);

    if (dll == nullptr)
        return L"";

    auto o_GetFileVersionInfoSizeW = (PFN_GetFileVersionInfoSizeW) GetProcAddress(dll, "GetFileVersionInfoSizeW");
    auto o_GetFileVersionInfoW = (PFN_GetFileVersionInfoW) GetProcAddress(dll, "GetFileVersionInfoW");
    auto o_VerQueryValueW = (PFN_VerQueryValueW) GetProcAddress(dll, "VerQueryValueW");

    if (o_GetFileVersionInfoSizeW == nullptr || o_GetFileVersionInfoW == nullptr || o_VerQueryValueW == nullptr)
        return L"";

    DWORD handle = 0;
    DWORD versionSize = o_GetFileVersionInfoSizeW(Util::ExePath().c_str(), &handle);
    if (versionSize == 0)
        return L"";

    std::vector<BYTE> versionData(versionSize);
    if (!o_GetFileVersionInfoW(Util::ExePath().c_str(), handle, versionSize, versionData.data()))
        return L"";

    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    }* lpTranslate;

    UINT cbTranslate = 0;
    if (!o_VerQueryValueW(versionData.data(), L"\\VarFileInfo\\Translation", (LPVOID*) &lpTranslate, &cbTranslate))
        return L"";

    std::wstring query = L"\\StringFileInfo\\" +
                         std::format(L"{:04x}{:04x}", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage) +
                         L"\\ProductName";
    LPWSTR productName = nullptr;
    UINT size = 0;
    if (o_VerQueryValueW(versionData.data(), query.c_str(), (LPVOID*) &productName, &size) && productName)
        return productName;

    return L"";
}

std::filesystem::path Util::DllPath()
{
    static std::filesystem::path dll;

    if (dll.empty())
    {
        wchar_t dllPath[MAX_PATH];
        GetModuleFileNameW(dllModule, dllPath, MAX_PATH);
        dll = std::filesystem::path(dllPath);

        LOG_INFO("{0}", wstring_to_string(dll.wstring()));
    }

    return dll;
}

std::optional<std::filesystem::path> Util::NvngxPath()
{
    // Checking _nvngx.dll / nvngx.dll location from registry based on DLSSTweaks
    // https://github.com/emoose/DLSSTweaks/blob/7ebf418c79670daad60a079c0e7b84096c6a7037/src/ProxyNvngx.cpp#L303
    LOG_INFO("trying to load nvngx from registry path!");

    HKEY regNGXCore;
    LSTATUS ls;
    std::optional<std::filesystem::path> result;

    ls = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\nvlddmkm\\NGXCore", 0, KEY_READ,
                       &regNGXCore);

    if (ls == ERROR_SUCCESS)
    {
        wchar_t regNGXCorePath[260];
        DWORD NGXCorePathSize = 260;

        ls = RegQueryValueExW(regNGXCore, L"NGXPath", nullptr, nullptr, (LPBYTE) regNGXCorePath, &NGXCorePathSize);

        if (ls == ERROR_SUCCESS)
        {
            auto path = std::filesystem::path(regNGXCorePath);
            LOG_INFO("nvngx registry path: {0}", path.string());
            return path;
        }
    }

    return result;
}

double Util::MillisecondsNow()
{
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    double milliseconds = 0;

    if (s_use_qpc)
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        milliseconds = double(1000.0 * now.QuadPart) / s_frequency.QuadPart;
    }
    else
    {
        milliseconds = double(GetTickCount64());
    }

    return milliseconds;
}

std::filesystem::path Util::ExePath()
{
    static std::filesystem::path exe;

    if (exe.empty())
    {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        exe = std::filesystem::path(exePath);
    }

    return exe;
}

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    const auto isMainWindow = [handle]()
    { return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle) && handle != GetConsoleWindow(); };

    DWORD pID = 0;
    GetWindowThreadProcessId(handle, &pID);

    if (pID != processId || !isMainWindow() || handle == GetConsoleWindow())
        return TRUE;

    *reinterpret_cast<HWND*>(lParam) = handle;

    return FALSE;
}

HWND Util::GetProcessWindow()
{
    HWND hwnd = nullptr;
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));

    if (hwnd == nullptr)
    {
        LOG_DEBUG("EnumWindows returned null using GetForegroundWindow()");
        hwnd = GetForegroundWindow();
    }

    return hwnd;
}

inline std::string LogLastError()
{
    DWORD errorCode = GetLastError();
    LPWSTR errorBuffer = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                  errorCode, 0, (LPWSTR) &errorBuffer, 0, NULL);

    std::string result;

    if (errorBuffer)
    {
        std::wstring errMsg(errorBuffer);
        result =
            std::format("{} ({})", errorCode,
                        wstring_to_string(errMsg).erase(wstring_to_string(errMsg).find_last_not_of("\t\n\v\f\r ") + 1));
        LocalFree(errorBuffer);
    }
    else
    {
        result = std::format("Unknown error ({}).", errorCode);
    }

    return result;
}

bool Util::GetDLLVersion(std::wstring dllPath, version_t* versionOut)
{
    // Step 1: Get the size of the version information
    DWORD handle = 0;
    DWORD versionSize = GetFileVersionInfoSizeW(dllPath.c_str(), &handle);

    if (versionSize == 0)
    {
        // LOG_ERROR("Failed to get version info size: {0:X}", LogLastError());
        return false;
    }

    // Step 2: Allocate buffer and get the version information
    std::vector<BYTE> versionInfo(versionSize);
    if (!GetFileVersionInfoW(dllPath.c_str(), handle, versionSize, versionInfo.data()))
    {
        // LOG_ERROR("Failed to get version info: {0:X}", LogLastError());
        return false;
    }

    // Step 3: Extract the version information
    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT size = 0;
    if (!VerQueryValueW(versionInfo.data(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &size))
    {
        // LOG_ERROR("Failed to query version value: {0:X}", LogLastError());
        return false;
    }

    if (fileInfo != nullptr && versionOut != nullptr)
    {
        // Extract major, minor, build, and revision numbers from version information
        DWORD fileVersionMS = fileInfo->dwFileVersionMS;
        DWORD fileVersionLS = fileInfo->dwFileVersionLS;

        versionOut->major = (fileVersionMS >> 16) & 0xffff;
        versionOut->minor = (fileVersionMS >> 0) & 0xffff;
        versionOut->patch = (fileVersionLS >> 16) & 0xffff;
        versionOut->reserved = (fileVersionLS >> 0) & 0xffff;
    }
    else
    {
        LOG_ERROR("No version information found!");
        return false;
    }
    return true;
}

bool Util::GetDLLVersion(std::wstring dllPath, xess_version_t* xessVersionOut)
{
    version_t tempVersion;
    auto result = Util::GetDLLVersion(dllPath, &tempVersion);

    // Don't assume that the structs are identical
    if (result)
    {
        xessVersionOut->major = tempVersion.major;
        xessVersionOut->minor = tempVersion.minor;
        xessVersionOut->patch = tempVersion.patch;
        xessVersionOut->reserved = tempVersion.reserved;
    }

    return result;
}

std::optional<std::filesystem::path> Util::FindFilePath(const std::filesystem::path& startDir,
                                                        const std::filesystem::path fileName)
{
    // 1) Direct check in startDir
    std::filesystem::path candidate = startDir / fileName;
    if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate))
    {
        LOG_INFO("{} found at {}", fileName.string(), candidate.parent_path().string());
        return candidate;
    }

    // 2) Recursive search under startDir
    for (auto& entry : std::filesystem::recursive_directory_iterator(
             startDir, std::filesystem::directory_options::skip_permission_denied))
    {
        if (!entry.is_directory() && entry.path().filename() == fileName)
        {
            LOG_INFO("{} found at {}", fileName.string(), entry.path().parent_path().string());
            return entry.path();
        }
    }

    // 3) Unreal-Engine/WinGDK fallback: check for Win64 or WinGDK in parent
    std::filesystem::path parent = startDir.parent_path().parent_path();
    for (const char* folder : {"Win64", "WinGDK"})
    {
        if (std::filesystem::exists(parent / folder) && std::filesystem::is_directory(parent / folder))
        {
            // Move up two more levels from 'parent' to reach UE project root
            std::filesystem::path ueRoot = parent.parent_path().parent_path();
            for (auto& entry : std::filesystem::recursive_directory_iterator(
                     ueRoot, std::filesystem::directory_options::skip_permission_denied))
            {
                if (!entry.is_directory() && entry.path().filename() == fileName)
                {
                    LOG_INFO("{} found at {}", fileName.string(), entry.path().parent_path().string());
                    return entry.path().parent_path();
                }
            }

            // If not found under this folder, break to avoid double-search
            break;
        }
    }

    // Not found anywhere
    return std::nullopt;
}
