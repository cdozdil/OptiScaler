#pragma once

#include "pch.h"
#include "Util.h"
#include "Config.h"
#include "Logger.h"

#include "xess_d3d12.h"
#include "xess_d3d12_debug.h"
#include "xess_debug.h"
#include "detours/detours.h"

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

public:
    static bool InitXeSS()
    {
        // if dll already loaded
        if (_dll != nullptr && _xessD3D12CreateContext != nullptr)
            return true;

        LOG_FUNC();

        Config::Instance()->upscalerDisableHook = true;
        Config::Instance()->dxgiSkipSpoofing = true;

        //PRN_xessGetVersion ptrDllGetVersion = nullptr;
        //PRN_xessGetVersion ptrMemoryGetVersion = nullptr;

        //xess_version_t memoryVersion{ 0,0,0,0 };
        //xess_version_t dllVersion{ 0,0,0,0 };

        //if (Config::Instance()->XeSSLibrary.has_value())
        //{
        //    std::filesystem::path cfgPath(Config::Instance()->XeSSLibrary.value().c_str());

        //    if (cfgPath.has_filename())
        //    {
        //        LOG_INFO("Trying to load libxess.dll from ini path: {}", cfgPath.string());
        //        _dll = LoadLibrary(cfgPath.c_str());
        //    }
        //    else
        //    {
        //        cfgPath = cfgPath / L"libxess.dll";
        //        LOG_INFO("Trying to load libxess.dll from ini path: {}", cfgPath.string());
        //        _dll = LoadLibrary(cfgPath.c_str());
        //    }
        //}

        //if (_dll == nullptr)
        //{
        //    auto path = Util::DllPath().parent_path() / L"libxess.dll";
        //    LOG_INFO("Trying to load libxess.dll from OptiScaler path: {}", path.string());
        //    _dll = LoadLibraryW(path.c_str());
        //}

        //if (_dll != nullptr)
        //{
        //    ptrDllGetVersion = (PRN_xessGetVersion)GetProcAddress(_dll, "xessGetVersion");

        //    // query dll version
        //    //if (ptrDllGetVersion)
        //    //{
        //    //    ptrDllGetVersion(&_xessVersion);
        //    //    LOG_DEBUG("Loaded libxess.dll v{0}.{1}.{2} file.", _xessVersion.major, _xessVersion.minor, _xessVersion.patch);
        //    //}

        //    // we would like to prioritize file pointed at ini, use methods from loaded dll
        //    _xessD3D12CreateContext = (PFN_xessD3D12CreateContext)GetProcAddress(_dll, "xessD3D12CreateContext");
        //    _xessD3D12BuildPipelines = (PFN_xessD3D12BuildPipelines)GetProcAddress(_dll, "xessD3D12BuildPipelines");
        //    _xessD3D12Init = (PRN_xessD3D12Init)GetProcAddress(_dll, "xessD3D12Init");
        //    _xessD3D12Execute = (PFN_xessD3D12Execute)GetProcAddress(_dll, "xessD3D12Execute");
        //    _xessSelectNetworkModel = (PFN_xessSelectNetworkModel)GetProcAddress(_dll, "xessSelectNetworkModel");
        //    _xessStartDump = (PFN_xessStartDump)GetProcAddress(_dll, "xessStartDump");
        //    _xessGetVersion = (PRN_xessGetVersion)GetProcAddress(_dll, "xessGetVersion");
        //    _xessIsOptimalDriver = (PFN_xessIsOptimalDriver)GetProcAddress(_dll, "xessIsOptimalDriver");
        //    _xessSetLoggingCallback = (PFN_xessSetLoggingCallback)GetProcAddress(_dll, "xessSetLoggingCallback");
        //    _xessGetProperties = (PFN_xessGetProperties)GetProcAddress(_dll, "xessGetProperties");
        //    _xessDestroyContext = (PFN_xessDestroyContext)GetProcAddress(_dll, "xessDestroyContext");
        //    _xessSetVelocityScale = (PFN_xessSetVelocityScale)GetProcAddress(_dll, "xessSetVelocityScale");

        //    _xessD3D12GetInitParams = (PFN_xessD3D12GetInitParams)GetProcAddress(_dll, "xessD3D12GetInitParams");
        //    _xessForceLegacyScaleFactors = (PFN_xessForceLegacyScaleFactors)GetProcAddress(_dll, "xessForceLegacyScaleFactors");
        //    _xessGetExposureMultiplier = (PFN_xessGetExposureMultiplier)GetProcAddress(_dll, "xessGetExposureMultiplier");
        //    _xessGetInputResolution = (PFN_xessGetInputResolution)GetProcAddress(_dll, "xessGetInputResolution");
        //    _xessGetIntelXeFXVersion = (PFN_xessGetIntelXeFXVersion)GetProcAddress(_dll, "xessGetIntelXeFXVersion");
        //    _xessGetJitterScale = (PFN_xessGetJitterScale)GetProcAddress(_dll, "xessGetJitterScale");
        //    _xessGetOptimalInputResolution = (PFN_xessGetOptimalInputResolution)GetProcAddress(_dll, "xessGetOptimalInputResolution");
        //    _xessSetExposureMultiplier = (PFN_xessSetExposureMultiplier)GetProcAddress(_dll, "xessSetExposureMultiplier");
        //    _xessSetJitterScale = (PFN_xessSetJitterScale)GetProcAddress(_dll, "xessSetJitterScale");

        //}

        // if libxess not loaded 
        if (_xessD3D12CreateContext == nullptr)
        {
            LOG_INFO("Trying to load libxess.dll with Detours");
            //ptrMemoryGetVersion = (PRN_xessGetVersion)DetourFindFunction("libxess.dll", "xessGetVersion");
            //if (ptrMemoryGetVersion)
            //{
                // get it's version to compare with dll
                //ptrMemoryGetVersion(&_xessVersion);
                //LOG_DEBUG("libxess.dll v{0}.{1}.{2} already loaded.", _xessVersion.major, _xessVersion.minor, _xessVersion.patch);

            _xessD3D12CreateContext = (PFN_xessD3D12CreateContext)DetourFindFunction("libxess.dll", "xessD3D12CreateContext");
            _xessD3D12BuildPipelines = (PFN_xessD3D12BuildPipelines)DetourFindFunction("libxess.dll", "xessD3D12BuildPipelines");
            _xessD3D12Init = (PRN_xessD3D12Init)DetourFindFunction("libxess.dll", "xessD3D12Init");
            _xessGetVersion = (PRN_xessGetVersion)DetourFindFunction("libxess.dll", "xessGetVersion");
            _xessD3D12Execute = (PFN_xessD3D12Execute)DetourFindFunction("libxess.dll", "xessD3D12Execute");
            _xessSelectNetworkModel = (PFN_xessSelectNetworkModel)DetourFindFunction("libxess.dll", "xessSelectNetworkModel");
            _xessStartDump = (PFN_xessStartDump)DetourFindFunction("libxess.dll", "xessStartDump");
            _xessIsOptimalDriver = (PFN_xessIsOptimalDriver)DetourFindFunction("libxess.dll", "xessIsOptimalDriver");
            _xessSetLoggingCallback = (PFN_xessSetLoggingCallback)DetourFindFunction("libxess.dll", "xessSetLoggingCallback");
            _xessGetProperties = (PFN_xessGetProperties)DetourFindFunction("libxess.dll", "xessGetProperties");
            _xessDestroyContext = (PFN_xessDestroyContext)DetourFindFunction("libxess.dll", "xessDestroyContext");
            _xessSetVelocityScale = (PFN_xessSetVelocityScale)DetourFindFunction("libxess.dll", "xessSetVelocityScale");
            _xessD3D12GetInitParams = (PFN_xessD3D12GetInitParams)DetourFindFunction("libxess.dll", "xessD3D12GetInitParams");
            _xessForceLegacyScaleFactors = (PFN_xessForceLegacyScaleFactors)DetourFindFunction("libxess.dll", "xessForceLegacyScaleFactors");
            _xessGetExposureMultiplier = (PFN_xessGetExposureMultiplier)DetourFindFunction("libxess.dll", "xessGetExposureMultiplier");
            _xessGetInputResolution = (PFN_xessGetInputResolution)DetourFindFunction("libxess.dll", "xessGetInputResolution");
            _xessGetIntelXeFXVersion = (PFN_xessGetIntelXeFXVersion)DetourFindFunction("libxess.dll", "xessGetIntelXeFXVersion");
            _xessGetJitterScale = (PFN_xessGetJitterScale)DetourFindFunction("libxess.dll", "xessGetJitterScale");
            _xessGetOptimalInputResolution = (PFN_xessGetOptimalInputResolution)DetourFindFunction("libxess.dll", "xessGetOptimalInputResolution");
            _xessSetExposureMultiplier = (PFN_xessSetExposureMultiplier)DetourFindFunction("libxess.dll", "xessSetExposureMultiplier");
            _xessSetJitterScale = (PFN_xessSetJitterScale)DetourFindFunction("libxess.dll", "xessSetJitterScale");
            _xessD3D12GetResourcesToDump = (PFN_xessD3D12GetResourcesToDump)DetourFindFunction("libxess.dll", "xessD3D12GetResourcesToDump");
            _xessD3D12GetProfilingData = (PFN_xessD3D12GetProfilingData)DetourFindFunction("libxess.dll", "xessD3D12GetProfilingData");
            _xessSetContextParameterF = (PFN_xessSetContextParameterF)DetourFindFunction("libxess.dll", "xessSetContextParameterF");
            //}
        }

        if (_xessD3D12CreateContext != nullptr)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (_xessD3D12CreateContext != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12CreateContext, xessD3D12CreateContext);

            if (_xessD3D12BuildPipelines != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12BuildPipelines, xessD3D12BuildPipelines);

            if (_xessD3D12Init != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12Init, xessD3D12Init);

            if (_xessGetVersion != nullptr)
                DetourAttach(&(PVOID&)_xessGetVersion, xessGetVersion);

            if (_xessD3D12Execute != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12Execute, xessD3D12Execute);

            if (_xessSelectNetworkModel != nullptr)
                DetourAttach(&(PVOID&)_xessSelectNetworkModel, xessSelectNetworkModel);

            if (_xessStartDump != nullptr)
                DetourAttach(&(PVOID&)_xessStartDump, xessStartDump);

            if (_xessIsOptimalDriver != nullptr)
                DetourAttach(&(PVOID&)_xessIsOptimalDriver, xessIsOptimalDriver);

            if (_xessSetLoggingCallback != nullptr)
                DetourAttach(&(PVOID&)_xessSetLoggingCallback, xessSetLoggingCallback);

            if (_xessGetProperties != nullptr)
                DetourAttach(&(PVOID&)_xessGetProperties, xessGetProperties);

            if (_xessDestroyContext != nullptr)
                DetourAttach(&(PVOID&)_xessDestroyContext, xessDestroyContext);

            if (_xessSetVelocityScale != nullptr)
                DetourAttach(&(PVOID&)_xessSetVelocityScale, xessSetVelocityScale);

            if (_xessD3D12GetInitParams != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12GetInitParams, xessD3D12GetInitParams);

            if (_xessForceLegacyScaleFactors != nullptr)
                DetourAttach(&(PVOID&)_xessForceLegacyScaleFactors, xessForceLegacyScaleFactors);

            if (_xessGetExposureMultiplier != nullptr)
                DetourAttach(&(PVOID&)_xessGetExposureMultiplier, xessGetExposureMultiplier);

            if (_xessGetInputResolution != nullptr)
                DetourAttach(&(PVOID&)_xessGetInputResolution, xessGetInputResolution);

            if (_xessGetIntelXeFXVersion != nullptr)
                DetourAttach(&(PVOID&)_xessGetIntelXeFXVersion, xessGetIntelXeFXVersion);

            if (_xessGetJitterScale != nullptr)
                DetourAttach(&(PVOID&)_xessGetJitterScale, xessGetJitterScale);

            if (_xessGetOptimalInputResolution != nullptr)
                DetourAttach(&(PVOID&)_xessGetOptimalInputResolution, xessGetOptimalInputResolution);

            if (_xessSetExposureMultiplier != nullptr)
                DetourAttach(&(PVOID&)_xessSetExposureMultiplier, xessSetExposureMultiplier);

            if (_xessSetJitterScale != nullptr)
                DetourAttach(&(PVOID&)_xessSetJitterScale, xessSetJitterScale);

            if (_xessD3D12GetResourcesToDump != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12GetResourcesToDump, xessD3D12GetResourcesToDump);

            if (_xessD3D12GetProfilingData != nullptr)
                DetourAttach(&(PVOID&)_xessD3D12GetProfilingData, xessD3D12GetProfilingData);

            if (_xessSetContextParameterF != nullptr)
                DetourAttach(&(PVOID&)_xessSetContextParameterF, xessSetContextParameterF);
            
            DetourTransactionCommit();

        }

        Config::Instance()->dxgiSkipSpoofing = false;
        Config::Instance()->upscalerDisableHook = false;

        return _xessD3D12CreateContext != nullptr;
    }

    static xess_version_t Version()
    {
        if (_xessVersion.major == 0)
        {
            _xessGetVersion(&_xessVersion);
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