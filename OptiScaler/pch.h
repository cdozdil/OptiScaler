#pragma once
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define WIN32_NO_STATUS
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include <Windows.h>
#include <string>
#include <stdint.h>
#include <libloaderapi.h>

#define NV_WINDOWS
#define NVSDK_NGX
#define NGX_ENABLE_DEPRECATED_GET_PARAMETERS
#define NGX_ENABLE_DEPRECATED_SHUTDOWN
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_WCHAR_FILENAMES
#include "spdlog/spdlog.h"

// Enables logging of DLSS NV Parameters
//#define DLSS_PARAM_DUMP

inline HMODULE dllModule;
inline DWORD processId;

inline static std::string wstring_to_string(const std::wstring& wstr)
{
    size_t len = std::wcstombs(nullptr, wstr.c_str(), 0);

    if (len == static_cast<size_t>(-1))
    {
        throw std::runtime_error("Conversion error");
    }

    std::vector<char> str(len + 1);
    std::wcstombs(str.data(), wstr.c_str(), len + 1);

    return std::string(str.data());
}

inline static std::wstring string_to_wstring(const std::string& str)
{
    int len;
    int slength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slength, 0, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slength, &wstr[0], len);
    return wstr;
}

