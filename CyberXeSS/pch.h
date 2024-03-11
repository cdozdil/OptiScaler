#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define WIN32_NO_STATUS
#include <windows.h>
#include <string>
#include <stdint.h>
#include <libloaderapi.h>

#define NV_WINDOWS
#define NVSDK_NGX
#define NGX_ENABLE_DEPRECATED_GET_PARAMETERS
#define NGX_ENABLE_DEPRECATED_SHUTDOWN

#define SPDLOG_USE_STD_FORMAT
#include "spdlog/spdlog.h"
