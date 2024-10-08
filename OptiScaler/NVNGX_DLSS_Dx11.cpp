#pragma once
#include "pch.h"

#include "Config.h"
#include "Util.h"

#include "NVNGX_Parameter.h"
#include "NVNGX_Proxy.h"

#include "backends/dlss/DLSSFeature_Dx11.h"
#include "backends/dlssd/DLSSDFeature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11.h"
#include "backends/fsr2/FSR2Feature_Dx11On12.h"
#include "backends/fsr2_212/FSR2Feature_Dx11On12_212.h"
#include "backends/fsr31/FSR31Feature_Dx11.h"
#include "backends/fsr31/FSR31Feature_Dx11On12.h"
#include "backends/xess/XeSSFeature_Dx11.h"

#include <ankerl/unordered_dense.h>

#include "imgui/imgui_overlay_dx.h"


inline ID3D11Device* D3D11Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx11>> Dx11Contexts;
static inline NVSDK_NGX_Parameter* createParams;
static inline int changeBackendCounter = 0;
static inline int evalCounter = 0;
static inline bool shutdown = false;

#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx11Inited())
    {
        if (Config::Instance()->UseGenericAppIdWithDlss.value_or(false))
            InApplicationId = app_id_override;

        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_Init_Ext() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D11_Init_Ext");

            auto result = NVNGXProxy::D3D11_Init_Ext()(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

            LOG_INFO("calling NVNGXProxy::D3D11_Init_Ext result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx11Inited(true);
        }
    }

    Config::Instance()->NVNGX_ApplicationId = InApplicationId;
    Config::Instance()->NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
    Config::Instance()->NVNGX_Version = InSDKVersion;
    Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;

    if (InFeatureInfo != nullptr && InSDKVersion > 0x0000013)
        Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

    LOG_INFO("AppId: {0}", InApplicationId);
    LOG_INFO("SDK: {0:x}", (int)InSDKVersion);
    std::wstring string(InApplicationDataPath);

    LOG_DEBUG("InApplicationDataPath {0}", wstring_to_string(string));

    Config::Instance()->NVNGX_FeatureInfo_Paths.clear();

    if (InFeatureInfo != nullptr)
    {
        for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
        {
            const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];
            std::wstring iniPathW(path);

            Config::Instance()->NVNGX_FeatureInfo_Paths.push_back(iniPathW);
            LOG_DEBUG("PathListInfo[{0}]: {1}", i, wstring_to_string(iniPathW));

        }
    }

    if (InDevice)
        D3D11Device = InDevice;

    Config::Instance()->Api = NVNGX_DX11;

    // Create Disjoint Query
    D3D11_QUERY_DESC disjointQueryDesc = {};
    disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

    // Create Timestamp Queries
    D3D11_QUERY_DESC timestampQueryDesc = {};
    timestampQueryDesc.Query = D3D11_QUERY_TIMESTAMP;

    for (int i = 0; i < ImGuiOverlayDx::QUERY_BUFFER_COUNT; i++)
    {
        InDevice->CreateQuery(&disjointQueryDesc, &ImGuiOverlayDx::disjointQueries[i]);
        InDevice->CreateQuery(&timestampQueryDesc, &ImGuiOverlayDx::startQueries[i]);
        InDevice->CreateQuery(&timestampQueryDesc, &ImGuiOverlayDx::endQueries[i]);
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx11Inited())
    {
        if (Config::Instance()->UseGenericAppIdWithDlss.value_or(false))
            InApplicationId = app_id_override;

        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_Init() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D11_Init");

            auto result = NVNGXProxy::D3D11_Init()(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);

            LOG_INFO("calling NVNGXProxy::D3D11_Init result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx11Inited(true);
        }
    }

    auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
    LOG_DEBUG("was called NVSDK_NGX_D3D11_Init_Ext");
    return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx11Inited())
    {
        if (Config::Instance()->UseGenericAppIdWithDlss.value_or(false))
            InProjectId = project_id_override;

        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_Init_ProjectID() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D11_Init_ProjectID");

            auto result = NVNGXProxy::D3D11_Init_ProjectID()(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

            LOG_INFO("calling NVNGXProxy::D3D11_Init_ProjectID result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx11Inited(true);
        }
    }

    auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    LOG_INFO("InProjectId: {0}", InProjectId);
    LOG_INFO("InEngineType: {0}", (int)InEngineType);
    LOG_INFO("InEngineVersion: {0}", InEngineVersion);

    Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
    Config::Instance()->NVNGX_Engine = InEngineType;
    Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

// Not sure about this one, original nvngx does not export this method
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    auto result = NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    LOG_INFO("InProjectId: {0}", InProjectId);
    LOG_INFO("InEngineType: {0}", (int)InEngineType);
    LOG_INFO("InEngineVersion: {0}", InEngineVersion);

    Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
    Config::Instance()->NVNGX_Engine = InEngineType;
    Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown()
{
    shutdown = true;

    for (auto const& [key, val] : Dx11Contexts)
    {
        if (val)
            NVSDK_NGX_D3D11_ReleaseFeature(val->Handle());
    }

    Dx11Contexts.clear();
    D3D11Device = nullptr;
    Config::Instance()->CurrentFeature = nullptr;

    DLSSFeatureDx11::Shutdown(D3D11Device);

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsDx11Inited() && NVNGXProxy::D3D11_Shutdown() != nullptr)
    {
        auto result = NVNGXProxy::D3D11_Shutdown()();
        NVNGXProxy::SetDx11Inited(false);
    }

    // Unhooking and cleaning stuff causing issues during shutdown. 
    // Disabled for now to check if it cause any issues
    //ImGuiOverlayDx::UnHookDx();

    shutdown = false;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
    shutdown = true;

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsDx11Inited() && NVNGXProxy::D3D11_Shutdown1() != nullptr)
    {
        auto result = NVNGXProxy::D3D11_Shutdown1()(InDevice);
        NVNGXProxy::SetDx11Inited(false);
    }

    return NVSDK_NGX_D3D11_Shutdown();
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_GetParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D11_GetParameters");

        auto result = NVNGXProxy::D3D11_GetParameters()(OutParameters);

        LOG_INFO("calling NVNGXProxy::D3D11_GetParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return result;
        }
    }

    *OutParameters = GetNGXParameters("OptiDx11");
    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::IsDx11Inited() && NVNGXProxy::D3D11_GetCapabilityParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D11_GetCapabilityParameters");

        auto result = NVNGXProxy::D3D11_GetCapabilityParameters()(OutParameters);

        LOG_INFO("calling NVNGXProxy::D3D11_GetCapabilityParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return result;
        }
    }

    *OutParameters = GetNGXParameters("OptiDx11");

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_AllocateParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D11_AllocateParameters");

        auto result = NVNGXProxy::D3D11_AllocateParameters()(OutParameters);

        LOG_INFO("calling NVNGXProxy::D3D11_AllocateParameters result: {0:X}", (UINT)result);

        if (result == NVSDK_NGX_Result_Success)
            return result;
    }

    auto params = new NVNGX_Parameters();
    params->Name = "OptiDx11";
    *OutParameters = params;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    InitNGXParameters(InParameters);

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D11_DestroyParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D11_DestroyParameters");
        auto result = NVNGXProxy::D3D11_DestroyParameters()(InParameters);
        LOG_INFO("calling NVNGXProxy::D3D11_DestroyParameters result: {0:X}", (UINT)result);

        return result;
    }

    delete InParameters;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
    LOG_WARN("-> 52428800");
    *OutSizeInBytes = 52428800;
    return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Feature

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
    // FeatureId check
    if (InFeatureID != NVSDK_NGX_Feature_SuperSampling && InFeatureID != NVSDK_NGX_Feature_RayReconstruction)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::InitDx11(D3D11Device) && NVNGXProxy::D3D11_CreateFeature() != nullptr)
        {
            auto result = NVNGXProxy::D3D11_CreateFeature()(InDevCtx, InFeatureID, InParameters, OutHandle);
            LOG_INFO("D3D11_CreateFeature result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
            return result;
        }
        else
        {
            LOG_ERROR("Can't create this feature ({0})!", (int)InFeatureID);
            return NVSDK_NGX_Result_Fail;
        }
    }

    // CreateFeature
    auto handleId = IFeature::GetNextHandleId();
    LOG_INFO("HandleId: {0}", handleId);

    if (InFeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        std::string defaultUpscaler = "fsr22";

        // If original NVNGX available use DLSS as base upscaler
        if (NVNGXProxy::IsDx11Inited())
            defaultUpscaler = "dlss";

        if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "xess")
        {
            Dx11Contexts[handleId] = std::make_unique<XeSSFeatureDx11>(handleId, InParameters);


            if (!Dx11Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new XeSS with Dx12 feature, Fallback to FSR2.2!");

                Dx11Contexts[handleId].reset();
                auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx11Contexts.erase(it);

                Config::Instance()->Dx11Upscaler = "fsr22";
            }
            else
            {
                LOG_INFO("creating new XeSS with Dx12 feature");
            }
        }

        if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "dlss")
        {
            if (Config::Instance()->DLSSEnabled.value_or(true))
            {
                Dx11Contexts[handleId] = std::make_unique<DLSSFeatureDx11>(handleId, InParameters);

                if (!Dx11Contexts[handleId]->ModuleLoaded())
                {
                    LOG_ERROR("can't create new DLSS feature, Fallback to FSR2.2!");

                    Dx11Contexts[handleId].reset();
                    auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                    Dx11Contexts.erase(it);

                    Config::Instance()->Dx11Upscaler = "fsr22";
                }
                else
                {
                    LOG_INFO("creating new DLSS feature");
                }
            }
            else
            {
                Config::Instance()->Dx11Upscaler.reset();
            }
        }

        if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr31_12")
        {
            Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11on12>(handleId, InParameters);

            if (!Dx11Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new FSR 3.1 feature, Fallback to FSR2.2!");

                Dx11Contexts[handleId].reset();
                auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx11Contexts.erase(it);

                Config::Instance()->Dx11Upscaler = "fsr22";
            }
            else
            {
                LOG_INFO("creating new FSR 3.1 feature");
            }
        }

        if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr22_12")
        {
            LOG_INFO("creating new FSR 2.2.1 with Dx12 feature");
            Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12>(handleId, InParameters);
        }
        else if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr21_12")
        {
            LOG_INFO("creating new FSR 2.1.2 with Dx12 feature");
            Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12_212>(handleId, InParameters);
        }
        else if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr22")
        {
            LOG_INFO("creating new native FSR 2.2.1 feature");
            Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11>(handleId, InParameters);
        }
        else if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr31")
        {
            LOG_INFO("creating new native FSR 3.1.0 feature");
            Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11>(handleId, InParameters);
        }
        //else if (Config::Instance()->Dx11Upscaler.value_or(defaultUpscaler) == "fsr304")
        //{
        //    LOG_INFO("creating new native FSR 3.1.0 feature");
        //    Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11>(handleId, InParameters);
        //}
    }
    else
    {
        LOG_INFO("creating new DLSSD feature");
        Dx11Contexts[handleId] = std::make_unique<DLSSDFeatureDx11>(handleId, InParameters);
    }

    auto deviceContext = Dx11Contexts[handleId].get();
    *OutHandle = deviceContext->Handle();

    if (!D3D11Device)
    {
        LOG_DEBUG("Get Dx11Device from InDevCtx!");
        InDevCtx->GetDevice(&D3D11Device);
        evalCounter = 0;

        if (!D3D11Device)
        {
            LOG_ERROR("Can't get Dx11Device from InDevCtx!");
            return NVSDK_NGX_Result_Fail;
        }
    }

    if (deviceContext->ModuleLoaded() && deviceContext->Init(D3D11Device, InDevCtx, InParameters))
    {
        Config::Instance()->CurrentFeature = deviceContext;
        return NVSDK_NGX_Result_Success;
    }

    LOG_ERROR("CreateFeature failed");

    Config::Instance()->newBackend = "fsr22";
    Config::Instance()->changeBackend = true;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
    if (!InHandle)
        return NVSDK_NGX_Result_Success;

    auto handleId = InHandle->Id;
    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D11_ReleaseFeature() != nullptr)
        {
            auto result = NVNGXProxy::D3D11_ReleaseFeature()(InHandle);

            if (!shutdown)
                LOG_INFO("D3D11_ReleaseFeature result for ({0}): {1:X}", handleId, (UINT)result);

            return result;
        }
        else
        {
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    if (!shutdown)
        LOG_INFO("releasing feature with id {0}", handleId);

    if (auto deviceContext = Dx11Contexts[handleId].get(); deviceContext != nullptr)
    {
        if (!shutdown)
        {
            LOG_TRACE("sleeping for 500ms before reset()!");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        if (deviceContext == Config::Instance()->CurrentFeature)
        {
            deviceContext->Shutdown();
            Config::Instance()->CurrentFeature = nullptr;
        }

        Dx11Contexts[handleId].reset();
        auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
        Dx11Contexts.erase(it);

        if (!shutdown && Config::Instance()->Dx11DelayedInit.value_or(false))
        {
            LOG_TRACE("sleeping for 500ms after reset()!");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_FUNC();

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        if (OutSupported == nullptr)
            OutSupported = new NVSDK_NGX_FeatureRequirement();

        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;

        //Some windows 10 os version
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D11_GetFeatureRequirements() != nullptr)
    {
        LOG_DEBUG("D3D11_GetFeatureRequirements for ({0})", (int)FeatureDiscoveryInfo->FeatureID);
        auto result = NVNGXProxy::D3D11_GetFeatureRequirements()(Adapter, FeatureDiscoveryInfo, OutSupported);
        LOG_DEBUG("result for D3D11_GetFeatureRequirements ({0}): {1:X}", (int)FeatureDiscoveryInfo->FeatureID, (UINT)result);
        return result;
    }

    OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
    return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
    if (InFeatureHandle == nullptr)
    {
        LOG_DEBUG("InFeatureHandle is null");
        return NVSDK_NGX_Result_FAIL_FeatureNotFound;
    }
    else
    {
        LOG_DEBUG("Handle: {0}", InFeatureHandle->Id);
    }

    if (InDevCtx == nullptr)
    {
        LOG_ERROR("InDevCtx is null!!!");
        return NVSDK_NGX_Result_Fail;
    }

    auto handleId = InFeatureHandle->Id;
    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D11_EvaluateFeature() != nullptr)
        {
            LOG_DEBUG("D3D11_EvaluateFeature for ({0})", handleId);
            auto result = NVNGXProxy::D3D11_EvaluateFeature()(InDevCtx, InFeatureHandle, InParameters, InCallback);
            LOG_INFO("D3D11_EvaluateFeature result for ({0}): {1:X}", handleId, (UINT)result);
            return result;
        }
        else
        {
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    evalCounter++;
    if (Config::Instance()->SkipFirstFrames.has_value() && evalCounter < Config::Instance()->SkipFirstFrames.value())
        return NVSDK_NGX_Result_Success;

    // DLSS Enabler check
    int deAvail;
    if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
    {
        if (Config::Instance()->DE_Available != (deAvail > 0))
            LOG_INFO("DLSSEnabler.Available: {0}", deAvail);

        Config::Instance()->DE_Available = (deAvail > 0);
    }

    if (InCallback)
        LOG_INFO("callback exist");

    IFeature_Dx11* deviceContext = nullptr;

    if (Config::Instance()->changeBackend)
    {
        if (Config::Instance()->newBackend == "" || (!Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->newBackend == "dlss"))
            Config::Instance()->newBackend = Config::Instance()->Dx11Upscaler.value_or("fsr22");

        changeBackendCounter++;

        // first release everything
        if (changeBackendCounter == 1)
        {
            if (Dx11Contexts.contains(handleId))
            {
                LOG_INFO("changing backend to {0}", Config::Instance()->newBackend);

                auto dc = Dx11Contexts[handleId].get();

                if (Config::Instance()->newBackend != "dlssd" && Config::Instance()->newBackend != "dlss")
                    createParams = GetNGXParameters("OptiDx11");
                else
                    createParams = InParameters;

                createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
                createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
                createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
                createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
                createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
                createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

                LOG_TRACE("sleeping before reset of current feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                Dx11Contexts[handleId].reset();
                auto it = std::find_if(Dx11Contexts.begin(), Dx11Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx11Contexts.erase(it);

                Config::Instance()->CurrentFeature = nullptr;
            }
            else
            {
                LOG_ERROR("can't find handle {0} in Dx11Contexts!", handleId);

                Config::Instance()->newBackend = "";
                Config::Instance()->changeBackend = false;

                if (createParams != nullptr)
                {
                    free(createParams);
                    createParams = nullptr;
                }

                changeBackendCounter = 0;
            }

            return NVSDK_NGX_Result_Success;
        }

        if (changeBackendCounter == 2)
        {
            // next frame prepare stuff
            if (Config::Instance()->newBackend == "xess")
            {
                Config::Instance()->Dx11Upscaler = "xess";
                LOG_INFO("creating new XeSS with Dx12 feature");
                Dx11Contexts[handleId] = std::make_unique<XeSSFeatureDx11>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "dlss")
            {
                Config::Instance()->Dx11Upscaler = "dlss";
                LOG_INFO("creating new DLSS feature");
                Dx11Contexts[handleId] = std::make_unique<DLSSFeatureDx11>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "dlssd")
            {
                LOG_INFO("creating new DLSSD feature");
                Dx11Contexts[handleId] = std::make_unique<DLSSDFeatureDx11>(handleId, InParameters);
            }
            else if (Config::Instance()->newBackend == "fsr21_12")
            {
                Config::Instance()->Dx11Upscaler = "fsr21_12";
                LOG_INFO("creating new FSR 2.1.2 with Dx12 feature");
                Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12_212>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr31_12")
            {
                Config::Instance()->Dx11Upscaler = "fsr31_12";
                LOG_INFO("creating new FSR 3.1 with Dx12 feature");
                Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11on12>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr22_12")
            {
                Config::Instance()->Dx11Upscaler = "fsr22_12";
                LOG_INFO("creating new FSR 2.2.1 with Dx12 feature");
                Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11on12>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr22")
            {
                Config::Instance()->Dx11Upscaler = "fsr22";
                LOG_INFO("creating new native FSR 2.2.1 feature");
                Dx11Contexts[handleId] = std::make_unique<FSR2FeatureDx11>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr31")
            {
                Config::Instance()->Dx11Upscaler = "fsr31";
                LOG_INFO("creating new native FSR 3.1.0 feature");
                Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11>(handleId, createParams);
            }
            //else if (Config::Instance()->newBackend == "fsr304")
            //{
            //    Config::Instance()->Dx11Upscaler = "fsr31";
            //    LOG_INFO("creating new native FSR 3.1.0 feature");
            //    Dx11Contexts[handleId] = std::make_unique<FSR31FeatureDx11>(handleId, createParams);
            //}

            if (Config::Instance()->Dx11DelayedInit.value_or(false) &&
                Config::Instance()->newBackend != "fsr22" && Config::Instance()->newBackend != "dlss" && Config::Instance()->newBackend != "dlssd")
            {
                LOG_TRACE("sleeping after new creation of new feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            return NVSDK_NGX_Result_Success;
        }

        if (changeBackendCounter == 3)
        {
            // then init and continue
            auto initResult = Dx11Contexts[handleId]->Init(D3D11Device, InDevCtx, createParams);

            if (Config::Instance()->Dx11DelayedInit.value_or(false))
            {
                LOG_TRACE("sleeping after new Init of new feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            changeBackendCounter = 0;

            if (!initResult || !Dx11Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("init failed with {0} feature", Config::Instance()->newBackend);

                if (Config::Instance()->newBackend != "dlssd")
                {
                    Config::Instance()->newBackend = "fsr22";
                    Config::Instance()->changeBackend = true;
                }
                else
                {
                    Config::Instance()->newBackend = "";
                    Config::Instance()->changeBackend = false;
                    return NVSDK_NGX_Result_Fail;
                }
            }
            else
            {
                LOG_INFO("init successful for {0}, upscaler changed", Config::Instance()->newBackend);

                Config::Instance()->newBackend = "";
                Config::Instance()->changeBackend = false;
                evalCounter = 0;
            }

            // if opti nvparam release it
            int optiParam = 0;
            if (createParams->Get("OptiScaler", &optiParam) == NVSDK_NGX_Result_Success && optiParam == 1)
            {
                free(createParams);
                createParams = nullptr;
            }
        }

        // if initial feature can't be inited
        Config::Instance()->CurrentFeature = Dx11Contexts[handleId].get();

        return NVSDK_NGX_Result_Success;
    }

    deviceContext = Dx11Contexts[handleId].get();
    Config::Instance()->CurrentFeature = deviceContext;

    if (deviceContext == nullptr)
    {
        LOG_DEBUG("trying to use released handle, returning NVSDK_NGX_Result_Success");
        return NVSDK_NGX_Result_Success;
    }

    // In the render loop:
    ImGuiOverlayDx::previousFrameIndex = (ImGuiOverlayDx::currentFrameIndex + ImGuiOverlayDx::QUERY_BUFFER_COUNT - 2) % ImGuiOverlayDx::QUERY_BUFFER_COUNT;
    int nextFrameIndex = ImGuiOverlayDx::currentFrameIndex;

    // Record the queries in the current frame
    InDevCtx->Begin(ImGuiOverlayDx::disjointQueries[nextFrameIndex]);
    InDevCtx->End(ImGuiOverlayDx::startQueries[nextFrameIndex]);

    if (!deviceContext->Evaluate(InDevCtx, InParameters) && !deviceContext->IsInited() && (deviceContext->Name() == "XeSS" || deviceContext->Name() == "DLSS" || deviceContext->Name() == "FSR3 w/Dx12"))
    {
        Config::Instance()->newBackend = "fsr22";
        Config::Instance()->changeBackend = true;
    }

    InDevCtx->End(ImGuiOverlayDx::endQueries[nextFrameIndex]);
    InDevCtx->End(ImGuiOverlayDx::disjointQueries[nextFrameIndex]);

    ImGuiOverlayDx::dx11UpscaleTrig[nextFrameIndex] = true;

    return NVSDK_NGX_Result_Success;
}

#pragma endregion
