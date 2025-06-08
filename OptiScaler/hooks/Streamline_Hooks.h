#pragma once

#include "pch.h"

#include <d3d12.h>
#include <sl.h>
#include <sl1.h>

class StreamlineHooks
{
  public:
    typedef void* (*PFN_slGetPluginFunction)(const char* functionName);
    typedef bool (*PFN_slOnPluginLoad)(void* params, const char* loaderJSON, const char** pluginJSON);

    static void unhookInterposer();
    static void hookInterposer(HMODULE slInterposer);

    static void unhookDlss();
    static void hookDlss(HMODULE slDlss);

    static void unhookDlssg();
    static void hookDlssg(HMODULE slDlssg);

  private:
    // interposer
    static decltype(&slInit) o_slInit;
    static decltype(&slSetTag) o_slSetTag;
    static decltype(&sl1::slInit) o_slInit_sl1;

    static sl::PFun_LogMessageCallback* o_logCallback;
    static sl1::pfunLogMessageCallback* o_logCallback_sl1;

    // dlss / dlssg
    static PFN_slGetPluginFunction o_dlss_slGetPluginFunction;
    static PFN_slOnPluginLoad o_dlss_slOnPluginLoad;
    static PFN_slGetPluginFunction o_dlssg_slGetPluginFunction;
    static PFN_slOnPluginLoad o_dlssg_slOnPluginLoad;

    static char* trimStreamlineLog(const char* msg);
    static void streamlineLogCallback(sl::LogType type, const char* msg);
    static sl::Result hkslInit(sl::Preferences* pref, uint64_t sdkVersion);
    static sl::Result hkslSetTag(sl::ViewportHandle& viewport, sl::ResourceTag* tags, uint32_t numTags,
                                 sl::CommandBuffer* cmdBuffer);
    static void streamlineLogCallback_sl1(sl1::LogType type, const char* msg);
    static bool hkslInit_sl1(sl1::Preferences* pref, int applicationId);
    static bool hkslOnPluginLoad(PFN_slOnPluginLoad o_slOnPluginLoad, std::string& config, void* params,
                                 const char* loaderJSON, const char** pluginJSON);
    static bool hkdlss_slOnPluginLoad(void* params, const char* loaderJSON, const char** pluginJSON);
    static bool hkdlssg_slOnPluginLoad(void* params, const char* loaderJSON, const char** pluginJSON);
    static void* hkdlss_slGetPluginFunction(const char* functionName);
    static void* hkdlssg_slGetPluginFunction(const char* functionName);
};
