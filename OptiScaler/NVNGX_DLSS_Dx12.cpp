#pragma once
#include "Config.h"

#include "NVNGX_Parameter.h"
#include "NVNGX_Proxy.h"

#include "backends/dlss/DLSSFeature_Dx12.h"
#include "backends/dlssd/DLSSDFeature_Dx12.h"
#include "backends/fsr2/FSR2Feature_Dx12.h"
#include "backends/fsr2_212/FSR2Feature_Dx12_212.h"
#include "backends/fsr31/FSR31Feature_Dx12.h"
#include "backends/xess/XeSSFeature_Dx12.h"

#include "imgui/imgui_overlay_dx12.h"

#include "detours/detours.h"
#include <ankerl/unordered_dense.h>
#include <dxgi1_4.h>

inline ID3D12Device* D3D12Device = nullptr;
static inline ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx12>> Dx12Contexts;
static inline NVSDK_NGX_Parameter* createParams = nullptr;
static inline int changeBackendCounter = 0;
static inline int evalCounter = 0;
static inline std::wstring appDataPath = L".";

#pragma region Hooks

typedef void(__fastcall* PFN_SetComputeRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);
typedef void(__fastcall* PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

static inline PFN_SetComputeRootSignature orgSetComputeRootSignature = nullptr;
static inline PFN_SetComputeRootSignature orgSetGraphicRootSignature = nullptr;
static inline PFN_CreateSampler orgCreateSampler = nullptr;

static inline ID3D12RootSignature* rootSigCompute = nullptr;
static inline ID3D12RootSignature* rootSigGraphic = nullptr;
static inline bool contextRendering = false;
static inline ULONGLONG computeTime = 0;
static inline ULONGLONG graphTime = 0;
static inline ULONGLONG lastEvalTime = 0;
inline static std::mutex sigatureMutex;

static inline int64_t GetTicks()
{
    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks))
        return 0;

    return ticks.QuadPart;
}

static void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (!contextRendering && commandList != nullptr && pRootSignature != nullptr)
    {
        sigatureMutex.lock();
        rootSigCompute = pRootSignature;
        computeTime = GetTicks();
        sigatureMutex.unlock();
    }

    return orgSetComputeRootSignature(commandList, pRootSignature);
}

static void hkSetGraphicRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (!contextRendering && commandList != nullptr && pRootSignature != nullptr)
    {
        sigatureMutex.lock();
        rootSigGraphic = pRootSignature;
        graphTime = GetTicks();
        sigatureMutex.unlock();
    }

    return orgSetGraphicRootSignature(commandList, pRootSignature);
}

static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    if (pDesc == nullptr || device == nullptr)
        return;

    if (pDesc->MipLODBias < 0.0f && Config::Instance()->MipmapBiasOverride.has_value())
    {
        spdlog::info("hkCreateSampler Overriding mipmap bias {0} -> {1}", pDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());

        D3D12_SAMPLER_DESC newDesc{};

        newDesc.AddressU = pDesc->AddressU;
        newDesc.AddressV = pDesc->AddressV;
        newDesc.AddressW = pDesc->AddressW;
        newDesc.BorderColor[0] = pDesc->BorderColor[0];
        newDesc.BorderColor[1] = pDesc->BorderColor[1];
        newDesc.BorderColor[2] = pDesc->BorderColor[2];
        newDesc.BorderColor[3] = pDesc->BorderColor[3];
        newDesc.ComparisonFunc = pDesc->ComparisonFunc;
        newDesc.Filter = pDesc->Filter;
        newDesc.MaxAnisotropy = pDesc->MaxAnisotropy;
        newDesc.MaxLOD = pDesc->MaxLOD;
        newDesc.MinLOD = pDesc->MinLOD;
        newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();

        Config::Instance()->lastMipBias = newDesc.MipLODBias;

        return orgCreateSampler(device, &newDesc, DestDescriptor);
    }
    else if (pDesc->MipLODBias < 0.0f)
    {
        Config::Instance()->lastMipBias = pDesc->MipLODBias;
    }

    return orgCreateSampler(device, pDesc, DestDescriptor);
}

void HookToCommandList(ID3D12GraphicsCommandList* InCmdList)
{
    if (orgSetComputeRootSignature != nullptr || orgSetGraphicRootSignature != nullptr)
        return;

    PVOID* pVTable = *(PVOID**)InCmdList;

    orgSetComputeRootSignature = (PFN_SetComputeRootSignature)pVTable[29];
    orgSetGraphicRootSignature = (PFN_SetComputeRootSignature)pVTable[30];

    if (orgSetComputeRootSignature != nullptr || orgSetGraphicRootSignature != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (orgSetComputeRootSignature != nullptr)
            DetourAttach(&(PVOID&)orgSetComputeRootSignature, hkSetComputeRootSignature);

        if (orgSetGraphicRootSignature != nullptr)
            DetourAttach(&(PVOID&)orgSetGraphicRootSignature, hkSetGraphicRootSignature);

        DetourTransactionCommit();
    }
}

void HookToDevice(ID3D12Device* InDevice)
{
    if (!ImGuiOverlayDx12::IsEarlyBind() && orgCreateSampler != nullptr || InDevice == nullptr)
        return;

    PVOID* pVTable = *(PVOID**)InDevice;

    orgCreateSampler = (PFN_CreateSampler)pVTable[22];

    if (orgCreateSampler != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)orgCreateSampler, hkCreateSampler);

        DetourTransactionCommit();
    }
}

void UnhookAll()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (orgSetComputeRootSignature != nullptr)
    {
        DetourDetach(&(PVOID&)orgSetComputeRootSignature, hkSetComputeRootSignature);
        orgSetComputeRootSignature = nullptr;
    }

    if (orgSetGraphicRootSignature != nullptr)
    {
        DetourDetach(&(PVOID&)orgSetGraphicRootSignature, hkSetGraphicRootSignature);
        orgSetGraphicRootSignature = nullptr;
    }

    if (orgCreateSampler != nullptr)
    {
        DetourDetach(&(PVOID&)orgCreateSampler, hkCreateSampler);
        orgCreateSampler = nullptr;
    }

    DetourTransactionCommit();
}

#pragma endregion

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
                                                        ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    spdlog::debug("NVSDK_NGX_D3D12_Init_Ext");

    Config::Instance()->NVNGX_ApplicationId = InApplicationId;
    Config::Instance()->NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
    Config::Instance()->NVNGX_Version = InSDKVersion;
    Config::Instance()->NVNGX_FeatureInfo = InFeatureInfo;

    if (InFeatureInfo != nullptr && InSDKVersion > 0x0000013)
        Config::Instance()->NVNGX_Logger = InFeatureInfo->LoggingInfo;

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx12Inited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init_Ext() != nullptr)
        {
            spdlog::info("NVSDK_NGX_D3D12_Init_Ext calling NVNGXProxy::D3D12_Init_Ext");
            auto result = NVNGXProxy::D3D12_Init_Ext()(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
            spdlog::info("NVSDK_NGX_D3D12_Init_Ext calling NVNGXProxy::D3D12_Init_Ext result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
        else
        {
            spdlog::warn("NVSDK_NGX_D3D12_Init_Ext NVNGXProxy::NVNGXModule or NVNGXProxy::D3D12_Init_Ext is nullptr!");
        }
    }

    spdlog::info("NVSDK_NGX_D3D12_Init_Ext AppId: {0}", InApplicationId);
    spdlog::info("NVSDK_NGX_D3D12_Init_Ext SDK: {0:x}", (unsigned int)InSDKVersion);
    appDataPath = InApplicationDataPath;

    spdlog::info("NVSDK_NGX_D3D12_Init_Ext InApplicationDataPath {0}", wstring_to_string(appDataPath));

    Config::Instance()->NVNGX_FeatureInfo_Paths.clear();

    if (InFeatureInfo != nullptr)
    {
        for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
        {
            const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];

            Config::Instance()->NVNGX_FeatureInfo_Paths.push_back(std::wstring(path));

            std::wstring iniPathW(path);
            spdlog::debug("NVSDK_NGX_D3D12_Init_Ext PathListInfo[{1}] checking nvngx.ini file in: {0}", wstring_to_string(iniPathW), i);

            if (Config::Instance()->LoadFromPath(path))
                spdlog::info("NVSDK_NGX_D3D12_Init_Ext PathListInfo[{1}] nvngx.ini file reloaded from: {0}", wstring_to_string(iniPathW), i);
        }
    }

    D3D12Device = InDevice;

    if (D3D12Device != nullptr)
        HookToDevice(D3D12Device);

    Config::Instance()->Api = NVNGX_DX12;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
                                                    ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    spdlog::debug("NVSDK_NGX_D3D12_Init");

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx12Inited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init() != nullptr)
        {
            spdlog::info("NVSDK_NGX_D3D12_Init calling NVNGXProxy::D3D12_Init");

            auto result = NVNGXProxy::D3D12_Init()(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);

            spdlog::info("NVSDK_NGX_D3D12_Init calling NVNGXProxy::D3D12_Init result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
    }

    auto result = NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
    spdlog::debug("NVSDK_NGX_D3D12_Init was called NVSDK_NGX_D3D12_Init_Ext");
    return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
                                                              const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    spdlog::debug("NVSDK_NGX_D3D12_Init_ProjectID");

    if (Config::Instance()->DLSSEnabled.value_or(true) && !NVNGXProxy::IsDx12Inited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init_ProjectID() != nullptr)
        {
            spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID calling NVNGXProxy::D3D12_Init_ProjectID");

            auto result = NVNGXProxy::D3D12_Init_ProjectID()(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

            spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID calling NVNGXProxy::D3D12_Init_ProjectID result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
    }

    auto result = NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InProjectId: {0}", InProjectId);
    spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InEngineType: {0}", (int)InEngineType);
    spdlog::info("NVSDK_NGX_D3D12_Init_ProjectID InEngineVersion: {0}", InEngineVersion);

    Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
    Config::Instance()->NVNGX_Engine = InEngineType;
    Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

// Not sure about this one, original nvngx does not export this method
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
                                                                   const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    spdlog::debug("NVSDK_NGX_D3D12_Init_with_ProjectID");

    auto result = NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InProjectId: {0}", InProjectId);
    spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineType: {0}", (int)InEngineType);
    spdlog::info("NVSDK_NGX_D3D12_Init_with_ProjectID InEngineVersion: {0}", InEngineVersion);

    Config::Instance()->NVNGX_ProjectId = std::string(InProjectId);
    Config::Instance()->NVNGX_Engine = InEngineType;
    Config::Instance()->NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

#pragma endregion

#pragma region DLSS Shutdown Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown(void)
{
    spdlog::info("NVSDK_NGX_D3D12_Shutdown");

    for (auto const& [key, val] : Dx12Contexts)
        NVSDK_NGX_D3D12_ReleaseFeature(val->Handle());

    Dx12Contexts.clear();
    D3D12Device = nullptr;

    Config::Instance()->CurrentFeature = nullptr;

    UnhookAll();

    DLSSFeatureDx12::Shutdown(D3D12Device);

    if (Config::Instance()->OverlayMenu.value_or(true) && ImGuiOverlayDx12::IsInitedDx12())
        ImGuiOverlayDx12::ShutdownDx12();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_Shutdown() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_Shutdown D3D12_Shutdown");
        auto result = NVNGXProxy::D3D12_Shutdown()();
        NVNGXProxy::SetDx12Inited(false);
        spdlog::info("NVSDK_NGX_D3D12_Shutdown D3D12_Shutdown result: {0:X}", (UINT)result);
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
    spdlog::info("NVSDK_NGX_D3D12_Shutdown1");

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_Shutdown1() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_Shutdown1 D3D12_Shutdown1");
        auto result = NVNGXProxy::D3D12_Shutdown1()(InDevice);
        NVNGXProxy::SetDx12Inited(false);
        spdlog::info("NVSDK_NGX_D3D12_Shutdown1 D3D12_Shutdown1 result: {0:X}", (UINT)result);
    }

    return NVSDK_NGX_D3D12_Shutdown();
}

#pragma endregion

#pragma region DLSS Parameter Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
    spdlog::debug("NVSDK_NGX_D3D12_GetParameters");

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_GetParameters() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_GetParameters calling NVNGXProxy::D3D12_GetParameters");
        auto result = NVNGXProxy::D3D12_GetParameters()(OutParameters);
        spdlog::info("NVSDK_NGX_D3D12_GetParameters calling NVNGXProxy::D3D12_GetParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return NVSDK_NGX_Result_Success;
        }
    }

    *OutParameters = GetNGXParameters("OptiDx12");
    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
    spdlog::debug("NVSDK_NGX_D3D12_GetCapabilityParameters");

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_GetCapabilityParameters() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_GetCapabilityParameters calling NVNGXProxy::D3D12_GetCapabilityParameters");
        auto result = NVNGXProxy::D3D12_GetCapabilityParameters()(OutParameters);
        spdlog::info("NVSDK_NGX_D3D12_GetCapabilityParameters calling NVNGXProxy::D3D12_GetCapabilityParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

        if (result == NVSDK_NGX_Result_Success)
        {
            InitNGXParameters(*OutParameters);
            return NVSDK_NGX_Result_Success;
        }
    }

    *OutParameters = GetNGXParameters("OptiDx12");

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
    spdlog::debug("NVSDK_NGX_D3D12_AllocateParameters");

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_AllocateParameters() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_AllocateParameters calling NVNGXProxy::D3D12_AllocateParameters");
        auto result = NVNGXProxy::D3D12_AllocateParameters()(OutParameters);
        spdlog::info("NVSDK_NGX_D3D12_AllocateParameters calling NVNGXProxy::D3D12_AllocateParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

        if (result == NVSDK_NGX_Result_Success)
            return result;
    }

    auto params = new NVNGX_Parameters();
    params->Name = "OptiDx12";
    *OutParameters = params;

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_PopulateParameters_Impl(NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("NVSDK_NGX_D3D12_PopulateParameters_Impl");

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    InitNGXParameters(InParameters);

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
    spdlog::debug("NVSDK_NGX_D3D12_DestroyParameters");

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_DestroyParameters() != nullptr)
    {
        spdlog::info("NVSDK_NGX_D3D12_DestroyParameters calling NVNGXProxy::D3D12_DestroyParameters");
        auto result = NVNGXProxy::D3D12_DestroyParameters()(InParameters);
        spdlog::info("NVSDK_NGX_D3D12_DestroyParameters calling NVNGXProxy::D3D12_DestroyParameters result: {0:X}", (UINT)result);
        return NVSDK_NGX_Result_Success;
    }

    delete InParameters;
    return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Feature Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
    spdlog::debug("NVSDK_NGX_D3D12_CreateFeature");

    if (InFeatureID != NVSDK_NGX_Feature_SuperSampling && InFeatureID != NVSDK_NGX_Feature_RayReconstruction)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::InitDx12(D3D12Device) && NVNGXProxy::D3D12_CreateFeature() != nullptr)
        {
            spdlog::info("NVSDK_NGX_D3D12_CreateFeature calling D3D12_CreateFeature for ({0})", (int)InFeatureID);
            auto result = NVNGXProxy::D3D12_CreateFeature()(InCmdList, InFeatureID, InParameters, OutHandle);

            if (result == NVSDK_NGX_Result_Success)
            {
                spdlog::info("NVSDK_NGX_D3D12_CreateFeature D3D12_CreateFeature HandleId for ({0}): {1:X}", (int)InFeatureID, (*OutHandle)->Id);
            }
            else
            {

                spdlog::info("NVSDK_NGX_D3D12_CreateFeature D3D12_CreateFeature result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
            }

            return result;
        }
        else
        {
            spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't create this feature ({0})!", (int)InFeatureID);
            return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
        }
    }

    // Create feature
    auto handleId = IFeature::GetNextHandleId();
    spdlog::info("NVSDK_NGX_D3D12_CreateFeature HandleId: {0}", handleId);

    // DLSS Enabler check
    int deAvail;
    if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
    {
        spdlog::info("NVSDK_NGX_D3D12_CreateFeature DLSSEnabler.Available: {0}", deAvail);
        Config::Instance()->DE_Available = (deAvail > 0);
    }

    // nvsdk logging - ini first
    if (!Config::Instance()->LogToNGX.has_value())
    {
        int nvsdkLogging = 0;
        InParameters->Get("DLSSEnabler.Logging", &nvsdkLogging);
        Config::Instance()->LogToNGX = nvsdkLogging > 0;
    }

    if (InFeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        // backend selection
        // 0 : XeSS
        // 1 : FSR2.2
        // 2 : FSR2.1
        // 3 : DLSS
        // 4 : FSR3.1

        int upscalerChoice = 0; // Default XeSS

        // If original NVNGX available use DLSS as base upscaler
        if (NVNGXProxy::IsDx12Inited())
            upscalerChoice = 3;

        // if Enabler does not set any upscaler
        if (InParameters->Get("DLSSEnabler.Dx12Backend", &upscalerChoice) != NVSDK_NGX_Result_Success)
        {

            if (Config::Instance()->Dx12Upscaler.has_value())
            {
                spdlog::info("NVSDK_NGX_D3D12_CreateFeature DLSS Enabler does not set any upscaler using ini: {0}", Config::Instance()->Dx12Upscaler.value());

                if (Config::Instance()->Dx12Upscaler.value() == "xess")
                    upscalerChoice = 0;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr22")
                    upscalerChoice = 1;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr21")
                    upscalerChoice = 2;
                else if (Config::Instance()->Dx12Upscaler.value() == "dlss" && Config::Instance()->DLSSEnabled.value_or(true))
                    upscalerChoice = 3;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr31")
                    upscalerChoice = 4;
            }

            spdlog::info("NVSDK_NGX_D3D12_CreateFeature upscalerChoice: {0}", upscalerChoice);
        }
        else
        {
            spdlog::info("NVSDK_NGX_D3D12_CreateFeature DLSS Enabler upscalerChoice: {0}", upscalerChoice);
        }


        if (upscalerChoice == 3)
        {
            Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                spdlog::error("NVSDK_NGX_D3D12_CreateFeature can't create new DLSS feature, fallback to XeSS!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 0;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "dlss";
                spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new DLSS feature");
            }
        }

        if (upscalerChoice == 0)
        {
            Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                spdlog::error("NVSDK_NGX_D3D12_CreateFeature can't create new XeSS feature, Fallback to FSR2.1!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 2;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "xess";
                spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new XeSS feature");
            }
        }

        if (upscalerChoice == 4)
        {
            Dx12Contexts[handleId] = std::make_unique<FSR31FeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                spdlog::error("NVSDK_NGX_D3D12_CreateFeature can't create new FSR 3.1 feature, Fallback to FSR2.1!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 2;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "fsr31";
                spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new FSR 3.1 feature");
            }
        }

        if (upscalerChoice == 1)
        {
            Config::Instance()->Dx12Upscaler = "fsr22";
            spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new FSR 2.2.1 feature");
            Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, InParameters);
        }
        else if (upscalerChoice == 2)
        {
            Config::Instance()->Dx12Upscaler = "fsr21";
            spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new FSR 2.1.2 feature");
            Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, InParameters);
        }

        // write back finel selected upscaler 
        InParameters->Set("DLSSEnabler.Dx12Backend", upscalerChoice);
    }
    else
    {
        spdlog::info("NVSDK_NGX_D3D12_CreateFeature creating new DLSSD feature");
        Dx12Contexts[handleId] = std::make_unique<DLSSDFeatureDx12>(handleId, InParameters);
    }

    auto deviceContext = Dx12Contexts[handleId].get();

    if (*OutHandle == nullptr)
        *OutHandle = new NVSDK_NGX_Handle{ handleId };
    else
        (*OutHandle)->Id = handleId;

#pragma region Check for Dx12Device Device

    if (!D3D12Device)
    {
        spdlog::debug("NVSDK_NGX_D3D12_CreateFeature Get D3d12 device from InCmdList!");
        auto deviceResult = InCmdList->GetDevice(IID_PPV_ARGS(&D3D12Device));

        if (deviceResult != S_OK || !D3D12Device)
        {
            spdlog::error("NVSDK_NGX_D3D12_CreateFeature Can't get Dx12Device from InCmdList!");
            return NVSDK_NGX_Result_Fail;
        }

        HookToDevice(D3D12Device);
    }

#pragma endregion

    if (deviceContext->Init(D3D12Device, InCmdList, InParameters))
    {
        Config::Instance()->CurrentFeature = deviceContext;
        HookToCommandList(InCmdList);
        evalCounter = 0;

        return NVSDK_NGX_Result_Success;
    }

    spdlog::error("NVSDK_NGX_D3D12_CreateFeature: CreateFeature failed, returning to FSR 2.1.2 upscaler");
    Config::Instance()->newBackend = "fsr21";
    Config::Instance()->changeBackend = true;
    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
    spdlog::debug("NVSDK_NGX_D3D12_ReleaseFeature");

    if (!InHandle)
        return NVSDK_NGX_Result_Success;

    auto handleId = InHandle->Id;

    spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature releasing feature with id {0}", handleId);

    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D12_ReleaseFeature() != nullptr)
        {
            spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature calling D3D12_ReleaseFeature for ({0})", handleId);
            auto result = NVNGXProxy::D3D12_ReleaseFeature()(InHandle);
            spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature D3D12_ReleaseFeature result for ({0}): {1:X}", handleId, (UINT)result);
            return result;
        }
        else
        {
            spdlog::info("NVSDK_NGX_D3D12_ReleaseFeature D3D12_ReleaseFeature not available for ({0})", handleId);
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    if (auto deviceContext = Dx12Contexts[handleId].get(); deviceContext != nullptr)
    {
        if (deviceContext == Config::Instance()->CurrentFeature)
        {
            Config::Instance()->CurrentFeature = nullptr;
            deviceContext->Shutdown();
        }

        Dx12Contexts[handleId].reset();
        auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
        Dx12Contexts.erase(it);
    }
    else
    {
        spdlog::error("NVSDK_NGX_D3D12_ReleaseFeature can't release feature with id {0}!", handleId);
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    spdlog::debug("NVSDK_NGX_D3D12_GetFeatureRequirements for ({0})", (int)FeatureDiscoveryInfo->FeatureID);

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling)
    {
        if (OutSupported == nullptr)
            OutSupported = new NVSDK_NGX_FeatureRequirement();

        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;

        //Some old windows 10 os version
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D12_GetFeatureRequirements() != nullptr)
    {
        spdlog::debug("NVSDK_NGX_D3D12_GetFeatureRequirements D3D12_GetFeatureRequirements for ({0})", (int)FeatureDiscoveryInfo->FeatureID);
        auto result = NVNGXProxy::D3D12_GetFeatureRequirements()(Adapter, FeatureDiscoveryInfo, OutSupported);
        spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature D3D12_GetFeatureRequirements result for ({0}): {1:X}", (int)FeatureDiscoveryInfo->FeatureID, (UINT)result);
        return result;
    }
    else
    {
        spdlog::debug("NVSDK_NGX_D3D12_GetFeatureRequirements D3D12_GetFeatureRequirements not available for ({0})", (int)FeatureDiscoveryInfo->FeatureID);
    }

    OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
    return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
    if (InFeatureHandle == nullptr)
    {
        spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature InFeatureHandle is null");
        return NVSDK_NGX_Result_Fail;
        // returning success to prevent breaking flow of the app
        // return NVSDK_NGX_Result_Success;
    }
    else
    {
        spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature Handle: {0}", InFeatureHandle->Id);
    }

    auto evaluateStart = GetTicks();

    auto handleId = InFeatureHandle->Id;
    spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature FeatureId: {0}", handleId);

    if (!InCmdList)
    {
        spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature InCmdList is null!!!");
        return NVSDK_NGX_Result_Fail;
    }

    if (Config::Instance()->OverlayMenu.value_or(true) &&
        Config::Instance()->CurrentFeature != nullptr && Config::Instance()->CurrentFeature->FrameCount() > Config::Instance()->MenuInitDelay.value_or(90) &&
        !ImGuiOverlayDx12::IsInitedDx12())
    {
        contextRendering = true;

        auto hwnd = Util::GetProcessWindow();

        HWND consoleWindow = GetConsoleWindow();
        bool consoleAllocated = false;

        if (consoleWindow == NULL)
        {
            AllocConsole();
            consoleWindow = GetConsoleWindow();
            consoleAllocated = true;

            ShowWindow(consoleWindow, SW_HIDE);
        }

        SetForegroundWindow(hwnd);
        SetFocus(hwnd);

        ImGuiOverlayDx12::InitDx12(consoleWindow, D3D12Device);

        if (consoleAllocated)
            FreeConsole();

        contextRendering = false;
    }

    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or(true) && NVNGXProxy::D3D12_EvaluateFeature() != nullptr)
        {
            spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature D3D12_EvaluateFeature for ({0})", handleId);
            auto result = NVNGXProxy::D3D12_EvaluateFeature()(InCmdList, InFeatureHandle, InParameters, InCallback);
            spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature D3D12_EvaluateFeature result for ({0}): {1:X}", handleId, (UINT)result);

            return result;
        }
        else
        {
            spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature D3D12_EvaluateFeature not avaliable for ({0})", handleId);
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }

    evalCounter++;
    if (Config::Instance()->SkipFirstFrames.has_value() && evalCounter < Config::Instance()->SkipFirstFrames.value())
        return NVSDK_NGX_Result_Success;

    // Check window recreation
    if (Config::Instance()->OverlayMenu.value_or(true) && ImGuiOverlayDx12::IsInitedDx12())
    {
        contextRendering = true;

        HWND currentHandle = Util::GetProcessWindow();
        if (ImGuiOverlayDx12::Handle() != currentHandle)
            ImGuiOverlayDx12::ReInitDx12(currentHandle);

        contextRendering = false;
    }

    if (InCallback)
        spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature callback exist");

    // DLSS Enabler
    {
        // DLSS Enabler check
        int deAvail;
        if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
        {
            if (Config::Instance()->DE_Available != (deAvail > 0))
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature DLSSEnabler.Available: {0}", deAvail);

            Config::Instance()->DE_Available = (deAvail > 0);
        }

        int limit = 0;
        if (InParameters->Get("FramerateLimit", &limit) == NVSDK_NGX_Result_Success)
        {
            if (Config::Instance()->DE_FramerateLimit.has_value())
            {
                if (Config::Instance()->DE_FramerateLimit.value() != limit)
                {
                    spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler FramerateLimit new value: {0}", Config::Instance()->DE_FramerateLimit.value());
                    InParameters->Set("FramerateLimit", Config::Instance()->DE_FramerateLimit.value());
                }
            }
            else
            {
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler FramerateLimit initial value: {0}", limit);
                Config::Instance()->DE_FramerateLimit = limit;
            }
        }
        else if (Config::Instance()->DE_FramerateLimit.has_value())
        {
            InParameters->Set("FramerateLimit", Config::Instance()->DE_FramerateLimit.value());
        }

        int dfgAvail = 0;
        if (InParameters->Get("DFG.Available", &dfgAvail) == NVSDK_NGX_Result_Success)
            Config::Instance()->DE_DynamicLimitAvailable = dfgAvail;

        int dfgEnabled = 0;
        if (InParameters->Get("DFG.Enabled", &dfgEnabled) == NVSDK_NGX_Result_Success)
        {
            if (Config::Instance()->DE_DynamicLimitEnabled.has_value())
            {
                if (Config::Instance()->DE_DynamicLimitEnabled.value() != dfgEnabled)
                {
                    spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler DFG {0}", Config::Instance()->DE_DynamicLimitEnabled.value() == 0 ? "disabled" : "enabled");
                    InParameters->Set("DFG.Enabled", Config::Instance()->DE_DynamicLimitEnabled.value());
                }
            }
            else
            {
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature DLSS Enabler DFG initial value: {0} ({1})", dfgEnabled == 0 ? "disabled" : "enabled", dfgEnabled);
                Config::Instance()->DE_DynamicLimitEnabled = dfgEnabled;
            }
        }
    }

    IFeature_Dx12* deviceContext = nullptr;

    if (Config::Instance()->changeBackend)
    {
        if (Config::Instance()->newBackend == "" || (!Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->newBackend == "dlss"))
            Config::Instance()->newBackend = Config::Instance()->Dx12Upscaler.value_or("xess");

        changeBackendCounter++;

        spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature changeBackend is true, counter: {0}", changeBackendCounter);

        // first release everything
        if (changeBackendCounter == 1)
        {
            if (Dx12Contexts.contains(handleId))
            {
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature changing backend to {0}", Config::Instance()->newBackend);

                auto dc = Dx12Contexts[handleId].get();

                if (Config::Instance()->newBackend != "dlssd" && Config::Instance()->newBackend != "dlss")
                    createParams = GetNGXParameters("OptiDx12");
                else
                    createParams = InParameters;

                createParams->Set(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, dc->GetFeatureFlags());
                createParams->Set(NVSDK_NGX_Parameter_Width, dc->RenderWidth());
                createParams->Set(NVSDK_NGX_Parameter_Height, dc->RenderHeight());
                createParams->Set(NVSDK_NGX_Parameter_OutWidth, dc->DisplayWidth());
                createParams->Set(NVSDK_NGX_Parameter_OutHeight, dc->DisplayHeight());
                createParams->Set(NVSDK_NGX_Parameter_PerfQualityValue, dc->PerfQualityValue());

                dc = nullptr;

                spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature sleeping before reset of current feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                Config::Instance()->CurrentFeature = nullptr;

                contextRendering = false;
                lastEvalTime = evaluateStart;

                rootSigCompute = nullptr;
                rootSigGraphic = nullptr;
            }
            else
            {
                spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature can't find handle {0} in Dx12Contexts!", handleId);

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

        // create new feature
        if (changeBackendCounter == 2)
        {
            // backend selection
            // 0 : XeSS
            // 1 : FSR2.2
            // 2 : FSR2.1
            // 3 : DLSS
            // 4 : FSR3.1
            int upscalerChoice = 0;

            // prepare new upscaler
            if (Config::Instance()->newBackend == "fsr22")
            {
                Config::Instance()->Dx12Upscaler = "fsr22";
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.2.1 feature");
                Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, createParams);
                upscalerChoice = 1;
            }
            else if (Config::Instance()->newBackend == "fsr21")
            {
                Config::Instance()->Dx12Upscaler = "fsr21";
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 2.1.2 feature");
                Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, createParams);
                upscalerChoice = 2;
            }
            else if (Config::Instance()->newBackend == "dlss")
            {
                Config::Instance()->Dx12Upscaler = "dlss";
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new DLSS feature");
                Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, createParams);
                upscalerChoice = 3;
            }
            else if (Config::Instance()->newBackend == "dlssd")
            {
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new DLSSD feature");
                Dx12Contexts[handleId] = std::make_unique<DLSSDFeatureDx12>(handleId, createParams);
            }
            else if (Config::Instance()->newBackend == "fsr31")
            {
                Config::Instance()->Dx12Upscaler = "fsr31";
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new FSR 3.1 feature");
                Dx12Contexts[handleId] = std::make_unique<FSR31FeatureDx12>(handleId, createParams);
                upscalerChoice = 4;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "xess";
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature creating new XeSS feature");
                Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, createParams);
                upscalerChoice = 0;
            }

            InParameters->Set("DLSSEnabler.Dx12Backend", upscalerChoice);

            return NVSDK_NGX_Result_Success;
        }

        // init feature
        if (changeBackendCounter == 3)
        {
            auto initResult = Dx12Contexts[handleId]->Init(D3D12Device, InCmdList, createParams);

            changeBackendCounter = 0;

            if (!initResult)
            {
                spdlog::error("NVSDK_NGX_D3D12_EvaluateFeature init failed with {0} feature", Config::Instance()->newBackend);

                if (Config::Instance()->newBackend != "dlssd")
                {
                    if (Config::Instance()->Dx12Upscaler == "dlss")
                    {
                        Config::Instance()->newBackend = "xess";
                        InParameters->Set("DLSSEnabler.Dx12Backend", 0);
                    }
                    else
                    {
                        Config::Instance()->newBackend = "fsr21";
                        InParameters->Set("DLSSEnabler.Dx12Backend", 2);
                    }

                }
                else
                {
                    // Retry DLSSD
                    Config::Instance()->newBackend = "dlssd";
                }

                Config::Instance()->changeBackend = true;
                return NVSDK_NGX_Result_Success;
            }
            else
            {
                spdlog::info("NVSDK_NGX_D3D12_EvaluateFeature init successful for {0}, upscaler changed", Config::Instance()->newBackend);

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
        Config::Instance()->CurrentFeature = Dx12Contexts[handleId].get();

        return NVSDK_NGX_Result_Success;
    }

    deviceContext = Dx12Contexts[handleId].get();

    if (deviceContext == nullptr)
    {
        spdlog::debug("NVSDK_NGX_D3D12_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success");
        return NVSDK_NGX_Result_Success;
    }

    if (!deviceContext->IsInited() && Config::Instance()->Dx12Upscaler.value_or("xess") != "fsr21")
    {
        spdlog::warn("NVSDK_NGX_D3D12_EvaluateFeature InCmdList {0} is not inited, falling back to FSR 2.1.2", deviceContext->Name());
        Config::Instance()->newBackend = "fsr21";
        Config::Instance()->changeBackend = true;
        return NVSDK_NGX_Result_Success;
    }

    Config::Instance()->CurrentFeature = deviceContext;

    ID3D12RootSignature* orgComputeRootSig = nullptr;
    ID3D12RootSignature* orgGraphicRootSig = nullptr;

    if (deviceContext->Name() != "DLSSD")
    {
        sigatureMutex.lock();

        orgComputeRootSig = rootSigCompute;
        orgGraphicRootSig = rootSigGraphic;

        spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgComputeRootSig: {0:X}, orgGraphicRootSig: {1:X}", (UINT64)orgComputeRootSig, (UINT64)orgGraphicRootSig);

        contextRendering = true;

        sigatureMutex.unlock();
    }

    bool evalResult = deviceContext->Evaluate(InCmdList, InParameters);

    if (deviceContext->Name() != "DLSSD" && (Config::Instance()->RestoreComputeSignature.value_or(false) || Config::Instance()->RestoreGraphicSignature.value_or(false)))
    {
        sigatureMutex.lock();

        if (Config::Instance()->RestoreComputeSignature.value_or(false) && computeTime != 0 && computeTime > lastEvalTime && computeTime <= evaluateStart && orgComputeRootSig != nullptr)
        {
            spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature restore orgComputeRootSig: {0:X}", (UINT64)orgComputeRootSig);
            orgSetComputeRootSignature(InCmdList, orgComputeRootSig);
        }
        else
        {
            if (Config::Instance()->RestoreComputeSignature.value_or(false))
            {
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgComputeRootSig lastEvalTime: {0}", lastEvalTime);
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgComputeRootSig computeTime: {0}", computeTime);
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgComputeRootSig evaluateStart: {0}", evaluateStart);
            }
        }

        if (Config::Instance()->RestoreGraphicSignature.value_or(false) && graphTime != 0 && graphTime > lastEvalTime && graphTime <= evaluateStart && orgGraphicRootSig != nullptr)
        {
            spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature restore orgGraphicRootSig: {0:X}", (UINT64)orgGraphicRootSig);
            orgSetGraphicRootSignature(InCmdList, orgGraphicRootSig);
        }
        else
        {
            if (Config::Instance()->RestoreGraphicSignature.value_or(false))
            {
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgGraphicRootSig lastEvalTime: {0}", lastEvalTime);
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgGraphicRootSig computeTime: {0}", graphTime);
                spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature orgGraphicRootSig evaluateStart: {0}", evaluateStart);
            }
        }

        contextRendering = false;
        lastEvalTime = evaluateStart;

        rootSigCompute = nullptr;
        rootSigGraphic = nullptr;

        sigatureMutex.unlock();
    }

    if (Config::Instance()->OverlayMenu.value_or(true))
        ImGuiOverlayDx12::CaptureQueue(InCmdList);

    spdlog::trace("NVSDK_NGX_D3D12_EvaluateFeature done: {0:X}", (UINT)evalResult);

    if (evalResult)
        return NVSDK_NGX_Result_Success;
    else
        return NVSDK_NGX_Result_Fail;
}

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
    spdlog::warn("NVSDK_NGX_D3D12_GetScratchBufferSize -> 52428800");
    *OutSizeInBytes = 52428800;
    return NVSDK_NGX_Result_Success;
}

#pragma endregion

