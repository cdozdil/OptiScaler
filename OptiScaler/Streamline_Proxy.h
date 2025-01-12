#pragma once

#include "pch.h"

#include <d3d12.h>
#include "Config.h"
#include "Util.h"

#include "detours/detours.h"
#include "../external/streamline/sl.h"


typedef int(*PFN_slSetTag)(uint64_t viewport, sl::ResourceTag* tags, uint32_t numTags, uint64_t cmdBuffer);
typedef int(*PFN_slInit)(sl::Preferences* pref, uint64_t sdkVersion);
typedef void(*PFN_LogCallback)(sl::LogType type, const char* msg);

static PFN_slSetTag o_slSetTag = nullptr;
static PFN_slInit o_slInit = nullptr;

static PFN_LogCallback o_logCallback = nullptr;

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

static int hkslInit(sl::Preferences* pref, uint64_t sdkVersion) {
    LOG_FUNC();
    // o_logCallback = pref->logMessageCallback; // cyberpunk might be getting itself here
    pref->logLevel = sl::LogLevel::eCount;
    pref->logMessageCallback = &streamlineLogCallback;
    return o_slInit(pref, sdkVersion);
}

static int hkslSetTag(uint64_t viewport, sl::ResourceTag* tags, uint32_t numTags, uint64_t cmdBuffer) {
    LOG_FUNC();
    for (auto i = 0; i < numTags; i++) {
        if (Config::Instance()->gameQuirk == Cyberpunk && tags[i].type == 2 && tags[i].resource->state == (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
            tags[i].resource->state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            LOG_TRACE("Changing hudless resource state");
        }
    }
    auto result = o_slSetTag(viewport, tags, numTags, cmdBuffer);
    return result;
}

static void unhookStreamline() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_slSetTag != nullptr) {
        DetourDetach(&(PVOID&)o_slSetTag, hkslSetTag);
        o_slSetTag = nullptr;
    }

    if (o_slInit != nullptr) {
        DetourDetach(&(PVOID&)o_slInit, hkslInit);
        o_slInit = nullptr;
    }

    DetourTransactionCommit();
}

// Call it just after sl.interposer's load or if sl.interposer is already loaded
static void hookStreamline(HMODULE slInterposer) {
    LOG_FUNC();

    if (o_slSetTag != nullptr || o_slInit != nullptr)
        unhookStreamline();

    if (Config::Instance()->DLSSGMod.value_or(false)) {
        o_slSetTag = reinterpret_cast<PFN_slSetTag>(GetProcAddress(slInterposer, "slSetTag"));
        o_slInit = reinterpret_cast<PFN_slInit>(GetProcAddress(slInterposer, "slInit"));


        if (o_slSetTag != nullptr && o_slInit != nullptr) {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            // Get a handle for sl.interposer and then the path
            HMODULE hModule = nullptr;
            char dllPath[MAX_PATH];
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(o_slSetTag), &hModule);
            GetModuleFileNameA(hModule, dllPath, MAX_PATH);

            Util::version_t sl_version;
            Util::GetDLLVersion(string_to_wstring(dllPath), &sl_version);
            LOG_INFO("Streamline version: {}.{}.{}", sl_version.major, sl_version.minor, sl_version.patch);

            if (sl_version.major >= 2) {
                DetourAttach(&(PVOID&)o_slSetTag, hkslSetTag);
                // TODO: fix some weird logging loop in cyberpunk
                DetourAttach(&(PVOID&)o_slInit, hkslInit);
            }

            DetourTransactionCommit();
        }
    }
}

