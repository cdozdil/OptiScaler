#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define WIN32_NO_STATUS

#include <windows.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <memory>
#include <vector>
#include <mutex>
#include <limits>
#include <string>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <atomic>
#include <iostream>

#include <vulkan/vulkan.hpp>

#define NV_WINDOWS
#define NVSDK_NGX
#define NGX_ENABLE_DEPRECATED_GET_PARAMETERS
#define NGX_ENABLE_DEPRECATED_SHUTDOWN
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_defs.h>
#include <nvsdk_ngx_vk.h>

#include <ankerl/unordered_dense.h>
#include <SimpleIni.h>

#define SPDLOG_USE_STD_FORMAT
#include "spdlog/spdlog.h"



