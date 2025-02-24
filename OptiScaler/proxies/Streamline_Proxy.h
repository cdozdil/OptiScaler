#pragma once

#include "pch.h"

#include <d3d12.h>
#include "Config.h"
#include "Util.h"

#include "detours/detours.h"
#include <sl.h>
#include <sl1.h>

static decltype(&slSetTag) o_slSetTag = nullptr;
static decltype(&slInit) o_slInit = nullptr;
static decltype(&sl1::slInit) o_slInit_sl1 = nullptr;

static sl::PFun_LogMessageCallback* o_logCallback = nullptr;
static sl1::pfunLogMessageCallback* o_logCallback_sl1 = nullptr;

static char* trimStreamlineLog(const char* msg) {
    int bracket_count = 0;

    char* result = (char*)malloc(strlen(msg) + 1);
    if (!result) return NULL;

    strcpy(result, msg);

    size_t length = strlen(result);
    if (length > 0 && result[length - 1] == '\n') {
        result[length - 1] = '\0';
    }

    return result;
}

static void streamlineLogCallback(sl::LogType type, const char* msg) {
    char* trimmed_msg = trimStreamlineLog(msg);

    switch (type) {
    case sl::LogType::eWarn:
        LOG_WARN("{}", trimmed_msg);
        break;
    case sl::LogType::eInfo:
        LOG_INFO("{}", trimmed_msg);
        break;
    case sl::LogType::eError:
        LOG_ERROR("{}", trimmed_msg);
        break;
    case sl::LogType::eCount:
        LOG_ERROR("{}", trimmed_msg);
        break;
    }

    free(trimmed_msg);

    if (o_logCallback != nullptr)
        o_logCallback(type, msg);
}

static sl::Result hkslInit(sl::Preferences* pref, uint64_t sdkVersion) {
    LOG_FUNC();
    if (pref->logMessageCallback != &streamlineLogCallback)
        o_logCallback = pref->logMessageCallback;
    pref->logLevel = sl::LogLevel::eCount;
    pref->logMessageCallback = &streamlineLogCallback;
    return o_slInit(*pref, sdkVersion);
}

static sl::Result hkslSetTag(sl::ViewportHandle& viewport, sl::ResourceTag* tags, uint32_t numTags, sl::CommandBuffer* cmdBuffer) {
    for (uint32_t i = 0; i < numTags; i++) {
        if (State::Instance().gameQuirk == Cyberpunk && tags[i].type == 2 && tags[i].resource->state == (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
            tags[i].resource->state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            LOG_TRACE("Changing hudless resource state");
        }
    }
    auto result = o_slSetTag(viewport, tags, numTags, cmdBuffer);
    return result;
}

static void streamlineLogCallback_sl1(sl1::LogType type, const char* msg) {
    char* trimmed_msg = trimStreamlineLog(msg);

    switch (type) {
    case sl1::LogType::eLogTypeWarn:
        LOG_WARN("{}", trimmed_msg);
        break;
    case sl1::LogType::eLogTypeInfo:
        LOG_INFO("{}", trimmed_msg);
        break;
    case sl1::LogType::eLogTypeError:
        LOG_ERROR("{}", trimmed_msg);
        break;
    case sl1::LogType::eLogTypeCount:
        LOG_ERROR("{}", trimmed_msg);
        break;
    }

    free(trimmed_msg);

    if (o_logCallback_sl1)
        o_logCallback_sl1(type, msg);
}

static bool hkslInit_sl1(sl1::Preferences* pref, int applicationId) {
    LOG_FUNC();
    if (pref->logMessageCallback != &streamlineLogCallback_sl1)
        o_logCallback_sl1 = pref->logMessageCallback;
    pref->logLevel = sl1::LogLevel::eLogLevelCount;
    pref->logMessageCallback = &streamlineLogCallback_sl1;
    return o_slInit_sl1(*pref, applicationId);
}

static void unhookStreamline() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_slSetTag) {
        DetourDetach(&(PVOID&)o_slSetTag, hkslSetTag);
        o_slSetTag = nullptr;
    }

    if (o_slInit) {
        DetourDetach(&(PVOID&)o_slInit, hkslInit);
        o_slInit = nullptr;
    }

    if (o_slInit_sl1) {
        DetourDetach(&(PVOID&)o_slInit_sl1, hkslInit_sl1);
        o_slInit_sl1 = nullptr;
    }

    o_logCallback_sl1 = nullptr;
    o_logCallback = nullptr;

    DetourTransactionCommit();
}

// Call it just after sl.interposer's load or if sl.interposer is already loaded
static void hookStreamline(HMODULE slInterposer) {
    LOG_FUNC();

    if (!slInterposer) {
        LOG_WARN("Streamline module in NULL");
        return;
    }

    if (o_slSetTag || o_slInit || o_slInit_sl1)
        unhookStreamline();

    if (Config::Instance()->FGType.value_or_default() == FGType::Nukems) {
        char dllPath[MAX_PATH];
        GetModuleFileNameA(slInterposer, dllPath, MAX_PATH);

        Util::version_t sl_version;
        Util::GetDLLVersion(string_to_wstring(dllPath), &sl_version);
        LOG_INFO("Streamline version: {}.{}.{}", sl_version.major, sl_version.minor, sl_version.patch);

        if (sl_version.major >= 2) {
            o_slSetTag = reinterpret_cast<decltype(&slSetTag)>(GetProcAddress(slInterposer, "slSetTag"));
            o_slInit = reinterpret_cast<decltype(&slInit)>(GetProcAddress(slInterposer, "slInit"));

            if (o_slSetTag != nullptr && o_slInit != nullptr) {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)o_slSetTag, hkslSetTag);
                DetourAttach(&(PVOID&)o_slInit, hkslInit);

                DetourTransactionCommit();
            }
        }
        else if (sl_version.major == 1) 
        {
            o_slInit_sl1 = reinterpret_cast<decltype(&sl1::slInit)>(GetProcAddress(slInterposer, "slInit"));

            if (o_slInit_sl1) {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)o_slInit_sl1, hkslInit_sl1);

                DetourTransactionCommit();
            }
        }
    }
}

