#pragma once

#include "pch.h"
#include "Util.h"
#include "Config.h"
#include "Logger.h"

#include "xess_d3d12.h"
#include "xess_d3d12_debug.h"
#include "xess_debug.h"
#include "detours/detours.h"

#pragma comment(lib, "Version.lib")

typedef xess_result_t(*PFN_xessD3D12CreateContext)(ID3D12Device* pDevice, xess_context_handle_t* phContext);
typedef xess_result_t(*PFN_xessD3D12BuildPipelines)(xess_context_handle_t hContext, ID3D12PipelineLibrary* pPipelineLibrary, bool blocking, uint32_t initFlags);
typedef xess_result_t(*PRN_xessD3D12Init)(xess_context_handle_t hContext, const xess_d3d12_init_params_t* pInitParams);
typedef xess_result_t(*PFN_xessD3D12Execute)(xess_context_handle_t hContext, ID3D12GraphicsCommandList* pCommandList, const xess_d3d12_execute_params_t* pExecParams);
typedef xess_result_t(*PFN_xessSelectNetworkModel)(xess_context_handle_t hContext, xess_network_model_t network);
typedef xess_result_t(*PFN_xessStartDump)(xess_context_handle_t hContext, const xess_dump_parameters_t* dump_parameters);
typedef xess_result_t(*PRN_xessGetVersion)(xess_version_t* pVersion);
typedef xess_result_t(*PFN_xessIsOptimalDriver)(xess_context_handle_t hContext);
typedef xess_result_t(*PFN_xessSetLoggingCallback)(xess_context_handle_t hContext, xess_logging_level_t loggingLevel, xess_app_log_callback_t loggingCallback);
typedef xess_result_t(*PFN_xessGetProperties)(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_properties_t* pBindingProperties);
typedef xess_result_t(*PFN_xessDestroyContext)(xess_context_handle_t hContext);
typedef xess_result_t(*PFN_xessSetVelocityScale)(xess_context_handle_t hContext, float x, float y);

typedef xess_result_t(*PFN_xessD3D12GetInitParams)(xess_context_handle_t hContext, xess_d3d12_init_params_t* pInitParams);
typedef xess_result_t(*PFN_xessForceLegacyScaleFactors)(xess_context_handle_t hContext, bool force);
typedef xess_result_t(*PFN_xessGetExposureMultiplier)(xess_context_handle_t hContext, float* pScale);
typedef xess_result_t(*PFN_xessGetInputResolution)(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_quality_settings_t qualitySettings, xess_2d_t* pInputResolution);
typedef xess_result_t(*PFN_xessGetIntelXeFXVersion)(xess_context_handle_t hContext, xess_version_t* pVersion);
typedef xess_result_t(*PFN_xessGetJitterScale)(xess_context_handle_t hContext, float* pX, float* pY);
typedef xess_result_t(*PFN_xessGetOptimalInputResolution)(xess_context_handle_t hContext, const xess_2d_t* pOutputResolution, xess_quality_settings_t qualitySettings,
                                                          xess_2d_t* pInputResolutionOptimal, xess_2d_t* pInputResolutionMin, xess_2d_t* pInputResolutionMax);
typedef xess_result_t(*PFN_xessSetExposureMultiplier)(xess_context_handle_t hContext, float scale);
typedef xess_result_t(*PFN_xessSetJitterScale)(xess_context_handle_t hContext, float x, float y);

typedef xess_result_t(*PFN_xessD3D12GetResourcesToDump)(xess_context_handle_t hContext, xess_resources_to_dump_t** pResourcesToDump);
typedef xess_result_t(*PFN_xessD3D12GetProfilingData)(xess_context_handle_t hContext, xess_profiling_data_t** pProfilingData);

typedef xess_result_t(*PFN_xessSetContextParameterF)();

class XeSSProxy
{
private:
    inline static HMODULE _dll = nullptr;
    inline static xess_version_t _xessVersion{};

    inline static PFN_xessD3D12CreateContext _xessD3D12CreateContext = nullptr;
    inline static PFN_xessD3D12BuildPipelines _xessD3D12BuildPipelines = nullptr;
    inline static PRN_xessD3D12Init _xessD3D12Init = nullptr;
    inline static PFN_xessD3D12Execute _xessD3D12Execute = nullptr;
    inline static PFN_xessSelectNetworkModel _xessSelectNetworkModel = nullptr;
    inline static PFN_xessStartDump _xessStartDump = nullptr;
    inline static PRN_xessGetVersion _xessGetVersion = nullptr;
    inline static PFN_xessIsOptimalDriver _xessIsOptimalDriver = nullptr;
    inline static PFN_xessSetLoggingCallback _xessSetLoggingCallback = nullptr;
    inline static PFN_xessGetProperties _xessGetProperties = nullptr;
    inline static PFN_xessDestroyContext _xessDestroyContext = nullptr;
    inline static PFN_xessSetVelocityScale _xessSetVelocityScale = nullptr;

    inline static PFN_xessD3D12GetInitParams _xessD3D12GetInitParams = nullptr;
    inline static PFN_xessForceLegacyScaleFactors _xessForceLegacyScaleFactors = nullptr;
    inline static PFN_xessGetExposureMultiplier _xessGetExposureMultiplier = nullptr;
    inline static PFN_xessGetInputResolution _xessGetInputResolution = nullptr;
    inline static PFN_xessGetIntelXeFXVersion _xessGetIntelXeFXVersion = nullptr;
    inline static PFN_xessGetJitterScale _xessGetJitterScale = nullptr;
    inline static PFN_xessGetOptimalInputResolution _xessGetOptimalInputResolution = nullptr;
    inline static PFN_xessSetExposureMultiplier _xessSetExposureMultiplier = nullptr;
    inline static PFN_xessSetJitterScale _xessSetJitterScale = nullptr;

    inline static PFN_xessD3D12GetResourcesToDump _xessD3D12GetResourcesToDump = nullptr;
    inline static PFN_xessD3D12GetProfilingData _xessD3D12GetProfilingData = nullptr;

    inline static PFN_xessSetContextParameterF _xessSetContextParameterF = nullptr;

    inline static std::filesystem::path DllPath(HMODULE module)
    {
        static std::filesystem::path dll;

        if (dll.empty())
        {
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(module, dllPath, MAX_PATH);
            dll = std::filesystem::path(dllPath);
        }

        return dll;
    }

    static bool LoadXeSS()
    {
        auto dllPath = Util::DllPath();

        // we would like to prioritize file pointed at ini
        if (Config::Instance()->XeSSLibrary.has_value())
        {
            std::filesystem::path cfgPath(Config::Instance()->XeSSLibrary.value().c_str());
            LOG_INFO("Trying to load libxess.dll from ini path: {}", cfgPath.string());

            if (!cfgPath.has_filename())
                cfgPath = cfgPath / L"libxess.optidll";

            _dll = LoadLibrary(cfgPath.c_str());
        }

        if (_dll == nullptr)
        {
            std::filesystem::path libXessPath = dllPath.parent_path() / L"libxess.optidll";
            LOG_INFO("Trying to load libxess.dll from dll path: {}", libXessPath.string());
            _dll = LoadLibrary(libXessPath.c_str());
        }

        // try to load from anywhere possible
        if (_dll == nullptr) {
            LOG_INFO("Trying to load libxess.dll");
            _dll = LoadLibrary(L"libxess.optidll");
        }

        return _dll != nullptr;
    }

    static bool HookXeSS(HMODULE xessModule)
    {
        LOG_INFO("Trying to hook XeSS");

        if (!xessModule)
            return false;

        _xessD3D12CreateContext = (PFN_xessD3D12CreateContext)GetProcAddress(xessModule, "xessD3D12CreateContext");
        _xessD3D12BuildPipelines = (PFN_xessD3D12BuildPipelines)GetProcAddress(xessModule, "xessD3D12BuildPipelines");
        _xessD3D12Init = (PRN_xessD3D12Init)GetProcAddress(xessModule, "xessD3D12Init");
        _xessD3D12Execute = (PFN_xessD3D12Execute)GetProcAddress(xessModule, "xessD3D12Execute");
        _xessSelectNetworkModel = (PFN_xessSelectNetworkModel)GetProcAddress(xessModule, "xessSelectNetworkModel");
        _xessStartDump = (PFN_xessStartDump)GetProcAddress(xessModule, "xessStartDump");
        _xessGetVersion = (PRN_xessGetVersion)GetProcAddress(xessModule, "xessGetVersion");
        _xessIsOptimalDriver = (PFN_xessIsOptimalDriver)GetProcAddress(xessModule, "xessIsOptimalDriver");
        _xessSetLoggingCallback = (PFN_xessSetLoggingCallback)GetProcAddress(xessModule, "xessSetLoggingCallback");
        _xessGetProperties = (PFN_xessGetProperties)GetProcAddress(xessModule, "xessGetProperties");
        _xessDestroyContext = (PFN_xessDestroyContext)GetProcAddress(xessModule, "xessDestroyContext");
        _xessSetVelocityScale = (PFN_xessSetVelocityScale)GetProcAddress(xessModule, "xessSetVelocityScale");

        _xessD3D12GetInitParams = (PFN_xessD3D12GetInitParams)GetProcAddress(xessModule, "xessD3D12GetInitParams");
        _xessForceLegacyScaleFactors = (PFN_xessForceLegacyScaleFactors)GetProcAddress(xessModule, "xessForceLegacyScaleFactors");
        _xessGetExposureMultiplier = (PFN_xessGetExposureMultiplier)GetProcAddress(xessModule, "xessGetExposureMultiplier");
        _xessGetInputResolution = (PFN_xessGetInputResolution)GetProcAddress(xessModule, "xessGetInputResolution");
        _xessGetIntelXeFXVersion = (PFN_xessGetIntelXeFXVersion)GetProcAddress(xessModule, "xessGetIntelXeFXVersion");
        _xessGetJitterScale = (PFN_xessGetJitterScale)GetProcAddress(xessModule, "xessGetJitterScale");
        _xessGetOptimalInputResolution = (PFN_xessGetOptimalInputResolution)GetProcAddress(xessModule, "xessGetOptimalInputResolution");
        _xessSetExposureMultiplier = (PFN_xessSetExposureMultiplier)GetProcAddress(xessModule, "xessSetExposureMultiplier");
        _xessSetJitterScale = (PFN_xessSetJitterScale)GetProcAddress(xessModule, "xessSetJitterScale");

        _xessGetOptimalInputResolution = (PFN_xessGetOptimalInputResolution)GetProcAddress(xessModule, "xessGetOptimalInputResolution");
        _xessSetExposureMultiplier = (PFN_xessSetExposureMultiplier)GetProcAddress(xessModule, "xessSetExposureMultiplier");
        _xessSetJitterScale = (PFN_xessSetJitterScale)GetProcAddress(xessModule, "xessSetJitterScale");
        _xessD3D12GetResourcesToDump = (PFN_xessD3D12GetResourcesToDump)GetProcAddress(xessModule, "xessD3D12GetResourcesToDump");
        _xessD3D12GetProfilingData = (PFN_xessD3D12GetProfilingData)GetProcAddress(xessModule, "xessD3D12GetProfilingData");
        _xessSetContextParameterF = (PFN_xessSetContextParameterF)GetProcAddress(xessModule, "xessSetContextParameterF");

        if (_xessD3D12CreateContext == nullptr)
            return false;

        // read version from file because 
        // xessGetVersion cause access violation errors
        if (_dll)
        {
            auto path = DllPath(_dll);
            if (!Util::GetDLLVersion(path.wstring(), &_xessVersion))
                _xessGetVersion(&_xessVersion);
        }
        else
        {
            _xessGetVersion(&_xessVersion);
        }

        LOG_INFO("Version: {}.{}.{}", _xessVersion.major, _xessVersion.minor, _xessVersion.patch);

        if (Config::Instance()->XeSSInputs.value_or_default())
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

#define ATTACH_IF_NOT_NULL(name) \
if (_##name != nullptr) \
DetourAttach(&(PVOID&)_##name, name)

            ATTACH_IF_NOT_NULL(xessD3D12CreateContext);
            ATTACH_IF_NOT_NULL(xessD3D12BuildPipelines);
            ATTACH_IF_NOT_NULL(xessD3D12Init);
            ATTACH_IF_NOT_NULL(xessGetVersion);
            ATTACH_IF_NOT_NULL(xessD3D12Execute);
            ATTACH_IF_NOT_NULL(xessSelectNetworkModel);
            ATTACH_IF_NOT_NULL(xessStartDump);
            ATTACH_IF_NOT_NULL(xessIsOptimalDriver);
            ATTACH_IF_NOT_NULL(xessSetLoggingCallback);
            ATTACH_IF_NOT_NULL(xessGetProperties);
            ATTACH_IF_NOT_NULL(xessDestroyContext);
            ATTACH_IF_NOT_NULL(xessSetVelocityScale);
            ATTACH_IF_NOT_NULL(xessD3D12GetInitParams);
            ATTACH_IF_NOT_NULL(xessForceLegacyScaleFactors);
            ATTACH_IF_NOT_NULL(xessGetExposureMultiplier);
            ATTACH_IF_NOT_NULL(xessGetInputResolution);
            ATTACH_IF_NOT_NULL(xessGetIntelXeFXVersion);
            ATTACH_IF_NOT_NULL(xessGetJitterScale);
            ATTACH_IF_NOT_NULL(xessGetOptimalInputResolution);
            ATTACH_IF_NOT_NULL(xessSetExposureMultiplier);
            ATTACH_IF_NOT_NULL(xessSetJitterScale);
            ATTACH_IF_NOT_NULL(xessD3D12GetResourcesToDump);
            ATTACH_IF_NOT_NULL(xessD3D12GetProfilingData);
            ATTACH_IF_NOT_NULL(xessSetContextParameterF);

            DetourTransactionCommit();
        }

        return true;
    }

public:
    static bool InitXeSS(HMODULE xessModule = nullptr)
    {
        // if dll already loaded
        if (_dll != nullptr || _xessD3D12CreateContext != nullptr) {
            LOG_INFO("Already loaded");
            return true;
        }

        spdlog::info("");

        if (xessModule != nullptr)
            _dll = xessModule;
        else
            LoadXeSS();

        HookXeSS(_dll);

        bool loadResult = _xessD3D12CreateContext != nullptr;
        LOG_INFO("LoadResult: {}", loadResult);
        return loadResult;
    }

    static xess_version_t Version()
    {
        // If dll version cant be read disable 1.3.x specific stuff
        if (_xessVersion.major == 0)
        {
            _xessVersion.major = 1;
            _xessVersion.minor = 2;
            _xessVersion.patch = 0;
        }

        return _xessVersion;
    }

    static PFN_xessD3D12CreateContext D3D12CreateContext() { return _xessD3D12CreateContext; }
    static PFN_xessD3D12BuildPipelines D3D12BuildPipelines() { return _xessD3D12BuildPipelines; }
    static PRN_xessD3D12Init D3D12Init() { return _xessD3D12Init; }
    static PFN_xessD3D12Execute D3D12Execute() { return _xessD3D12Execute; }
    static PFN_xessSelectNetworkModel SelectNetworkModel() { return _xessSelectNetworkModel; }
    static PFN_xessStartDump StartDump() { return _xessStartDump; }
    static PRN_xessGetVersion GetVersion() { return _xessGetVersion; }
    static PFN_xessIsOptimalDriver IsOptimalDriver() { return _xessIsOptimalDriver; }
    static PFN_xessSetLoggingCallback SetLoggingCallback() { return _xessSetLoggingCallback; }
    static PFN_xessGetProperties GetProperties() { return _xessGetProperties; }
    static PFN_xessDestroyContext DestroyContext() { return _xessDestroyContext; }
    static PFN_xessSetVelocityScale SetVelocityScale() { return _xessSetVelocityScale; }

    static PFN_xessD3D12GetInitParams D3D12GetInitParams() { return _xessD3D12GetInitParams; }
    static PFN_xessForceLegacyScaleFactors ForceLegacyScaleFactors() { return _xessForceLegacyScaleFactors; }
    static PFN_xessGetExposureMultiplier GetExposureMultiplier() { return _xessGetExposureMultiplier; }
    static PFN_xessGetInputResolution GetInputResolution() { return _xessGetInputResolution; }
    static PFN_xessGetIntelXeFXVersion GetIntelXeFXVersion() { return _xessGetIntelXeFXVersion; }
    static PFN_xessGetJitterScale GetJitterScale() { return _xessGetJitterScale; }
    static PFN_xessGetOptimalInputResolution GetOptimalInputResolution() { return _xessGetOptimalInputResolution; }
    static PFN_xessSetExposureMultiplier SetExposureMultiplier() { return _xessSetExposureMultiplier; }
    static PFN_xessSetJitterScale SetJitterScale() { return _xessSetJitterScale; }

    static PFN_xessD3D12GetResourcesToDump xessD3D12GetResourcesToDump() { return _xessD3D12GetResourcesToDump; }
    static PFN_xessD3D12GetProfilingData xessD3D12GetProfilingData() { return _xessD3D12GetProfilingData; }
    static PFN_xessSetContextParameterF xessSetContextParameterF() { return _xessSetContextParameterF; }
};