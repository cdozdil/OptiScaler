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

//#define LOG_ASYNC
//#define DETAILED_DEBUG_LOGS

inline HMODULE dllModule = nullptr;
inline HMODULE skHandle = nullptr;
inline DWORD processId;

#define LOG_TRACE(msg, ...) \
    spdlog::trace(__FUNCTION__ " " msg, ##__VA_ARGS__)

#define LOG_DEBUG(msg, ...) \
    spdlog::debug(__FUNCTION__ " " msg, ##__VA_ARGS__)

#ifdef DETAILED_DEBUG_LOGS
#define LOG_DEBUG_ONLY(msg, ...) \
    spdlog::debug(__FUNCTION__ " " msg, ##__VA_ARGS__) 
#else
#define LOG_DEBUG_ONLY(msg, ...) 
#endif

#ifdef LOG_ASYNC
#define LOG_DEBUG_ASYNC(msg, ...) \
    spdlog::debug(__FUNCTION__ " " msg, ##__VA_ARGS__) 
#else
#define LOG_DEBUG_ASYNC(msg, ...) 
#endif

#define LOG_INFO(msg, ...) \
    spdlog::info(__FUNCTION__ " " msg, ##__VA_ARGS__)

#define LOG_WARN(msg, ...) \
    spdlog::warn(__FUNCTION__ " " msg, ##__VA_ARGS__)

#define LOG_ERROR(msg, ...) \
    spdlog::error(__FUNCTION__ " " msg, ##__VA_ARGS__)

#define LOG_FUNC() \
    spdlog::trace(__FUNCTION__)

#define LOG_FUNC_RESULT(result) \
    spdlog::trace(__FUNCTION__ " result: {0:X}" , (UINT64)result)

typedef struct _feature_version
{
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
} feature_version;

inline static bool isVersionOrBetter(const feature_version& current, const feature_version& required) {
    if (current.major > required.major) {
        return true;
    }
    if (current.major == required.major) {
        if (current.minor > required.minor) {
            return true;
        }
        if (current.minor == required.minor) {
            if (current.patch >= required.patch) {
                return true;
            }
        }
    }
    return false;
}

inline static std::string wstring_to_string(const std::wstring& wide_str) 
{
    std::string str(wide_str.length(), 0);
    std::transform(wide_str.begin(), wide_str.end(), str.begin(), [](wchar_t c) { return (char)c; });
    return str;
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

