#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define WIN32_NO_STATUS
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
#include "spdlog/spdlog.h"

// Enables logging of DLSS NV Parameters
//#define DLSS_PARAM_DUMP

#define _CRT_SECURE_NO_DEPRECATE

inline HMODULE dllModule;
inline DWORD processId;

