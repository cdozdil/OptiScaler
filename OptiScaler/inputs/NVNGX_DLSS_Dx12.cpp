#include "Config.h"
#include "Util.h"

#include "NVNGX_Parameter.h"
#include "proxies/NVNGX_Proxy.h"
#include "DLSSG_Mod.h" 

#include "upscalers/dlss/DLSSFeature_Dx12.h"
#include "upscalers/dlssd/DLSSDFeature_Dx12.h"
#include "upscalers/fsr2/FSR2Feature_Dx12.h"
#include "upscalers/fsr2_212/FSR2Feature_Dx12_212.h"
#include "upscalers/fsr31/FSR31Feature_Dx12.h"
#include "upscalers/xess/XeSSFeature_Dx12.h"

#include "hooks/HooksDx.h"
#include "proxies/FfxApi_Proxy.h"

#include <dxgi1_4.h>
#include <shared_mutex>
#include "detours/detours.h"
#include <ffx_framegeneration.h>
#include <ankerl/unordered_dense.h>

// Use a dedicated Queue + CommandList for copying Depth + Velocity
// Looks like it is causing issues so disabled 
//#define USE_COPY_QUEUE_FOR_FG

// Use a dedicated Queue + CommandList for FG without hudfix
// Looks like causing stutter/sync issues
//#define USE_QUEUE_FOR_FG

// Do not make copies of Depth + Velocity for FG
//#define DONT_USE_DEPTH_MV_COPIES

#ifndef USE_PRESENT_FOR_FT
static UINT64 fgLastFrameTime = 0;
#endif
static UINT64 fgLastFGFrame = 0;
static UINT fgCallbackFrameIndex = 0;

static ankerl::unordered_dense::map <unsigned int, std::unique_ptr<IFeature_Dx12>> Dx12Contexts;
static ankerl::unordered_dense::map <ID3D12GraphicsCommandList*, ID3D12RootSignature*> computeSignatures;
static ankerl::unordered_dense::map <ID3D12GraphicsCommandList*, ID3D12RootSignature*> graphicSignatures;
static ID3D12Device* D3D12Device = nullptr;
static NVSDK_NGX_Parameter* createParams = nullptr;
static int changeBackendCounter = 0;
static int evalCounter = 0;
static std::wstring appDataPath = L".";
static bool shutdown = false;
static bool inited = false;

static void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = InResource;
    barrier.Transition.StateBefore = InBeforeState;
    barrier.Transition.StateAfter = InAfterState;
    barrier.Transition.Subresource = 0;
    InCommandList->ResourceBarrier(1, &barrier);
}

static bool CreateBufferResource(LPCWSTR Name, ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource)
{
    if (InDevice == nullptr || InSource == nullptr)
        return false;

    D3D12_RESOURCE_DESC texDesc = InSource->GetDesc();

    if (*OutResource != nullptr)
    {
        auto bufDesc = (*OutResource)->GetDesc();

        if (bufDesc.Width != (UINT64)(texDesc.Width) || bufDesc.Height != (UINT)(texDesc.Height) || bufDesc.Format != texDesc.Format)
        {
            (*OutResource)->Release();
            (*OutResource) = nullptr;
        }
        else
            return true;
    }

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    HRESULT hr = InSource->GetHeapProperties(&heapProperties, &heapFlags);

    if (hr != S_OK)
    {
        LOG_ERROR("GetHeapProperties result: {0:X}", hr);
        return false;
    }

    //texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(OutResource));

    if (hr != S_OK)
    {
        LOG_ERROR("CreateCommittedResource result: {0:X}", hr);
        return false;
    }

    (*OutResource)->SetName(Name);
    return true;
}

#pragma region Hooks

typedef void(*PFN_SetComputeRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);
typedef void(*PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

static PFN_SetComputeRootSignature orgSetComputeRootSignature = nullptr;
static PFN_SetComputeRootSignature orgSetGraphicRootSignature = nullptr;

static bool contextRendering = false;
static std::shared_mutex computeSigatureMutex;
static std::shared_mutex graphSigatureMutex;

static IID streamlineRiid{};

static int64_t GetTicks()
{
    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks))
        return 0;

    return ticks.QuadPart;
}

static void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (Config::Instance()->RestoreComputeSignature.value_or_default() && !contextRendering && commandList != nullptr && pRootSignature != nullptr)
    {
        std::unique_lock<std::shared_mutex> lock(computeSigatureMutex);
        computeSignatures.insert_or_assign(commandList, pRootSignature);
    }

    orgSetComputeRootSignature(commandList, pRootSignature);
}

static void hkSetGraphicRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (Config::Instance()->RestoreGraphicSignature.value_or_default() && !contextRendering && commandList != nullptr && pRootSignature != nullptr)
    {
        std::unique_lock<std::shared_mutex> lock(graphSigatureMutex);
        graphicSignatures.insert_or_assign(commandList, pRootSignature);
    }

    orgSetGraphicRootSignature(commandList, pRootSignature);
}

static void HookToCommandList(ID3D12GraphicsCommandList* InCmdList)
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

static void UnhookAll()
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

    DetourTransactionCommit();
}

#pragma endregion

#pragma region DLSS Init Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
                                                        ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (inited)
    {
        LOG_WARN("NVNGX already inited");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->UseGenericAppIdWithDlss.value_or_default())
        InApplicationId = app_id_override;

    State::Instance().NVNGX_ApplicationId = InApplicationId;
    State::Instance().NVNGX_ApplicationDataPath = std::wstring(InApplicationDataPath);
    State::Instance().NVNGX_Version = InSDKVersion;
    State::Instance().NVNGX_FeatureInfo = InFeatureInfo;

    if (InFeatureInfo != nullptr && InSDKVersion > 0x0000013)
        State::Instance().NVNGX_Logger = InFeatureInfo->LoggingInfo;

    if (Config::Instance()->DLSSEnabled.value_or_default() && !NVNGXProxy::IsDx12Inited())
    {
        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init_Ext() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D12_Init_Ext");
            auto result = NVNGXProxy::D3D12_Init_Ext()(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
            LOG_INFO("calling NVNGXProxy::D3D12_Init_Ext result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
        else
        {
            LOG_WARN("NVNGXProxy::NVNGXModule or NVNGXProxy::D3D12_Init_Ext is nullptr!");
        }
    }

    DLSSGMod::InitDLSSGMod_Dx12();
    DLSSGMod::D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    LOG_INFO("AppId: {0}", InApplicationId);
    LOG_INFO("SDK: {0:x}", (unsigned int)InSDKVersion);
    appDataPath = std::wstring(InApplicationDataPath);

    LOG_INFO("InApplicationDataPath {0}", wstring_to_string(appDataPath));

    State::Instance().NVNGX_FeatureInfo_Paths.clear();

    if (InFeatureInfo != nullptr)
    {
        for (size_t i = 0; i < InFeatureInfo->PathListInfo.Length; i++)
        {
            const wchar_t* path = InFeatureInfo->PathListInfo.Path[i];

            State::Instance().NVNGX_FeatureInfo_Paths.push_back(std::wstring(path));

            std::wstring iniPathW(path);
            LOG_DEBUG("PathListInfo[{1}] checking nvngx.ini file in: {0}", wstring_to_string(iniPathW), i);

            if (Config::Instance()->LoadFromPath(path))
                LOG_INFO("PathListInfo[{1}] nvngx.ini file reloaded from: {0}", wstring_to_string(iniPathW), i);
        }
    }

    D3D12Device = InDevice;

    State::Instance().api = DX12;

    if (!State::Instance().isWorkingAsNvngx)
    {
        // Create query heap for timestamp queries
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Count = 2; // Start and End timestamps
        queryHeapDesc.NodeMask = 0;
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        auto result = InDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&HooksDx::queryHeap));

        // Create a readback buffer to retrieve timestamp data
        D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(2 * sizeof(UINT64));
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        result = InDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&HooksDx::readbackBuffer));
    }

    // early hooking for signatures
    if (orgSetComputeRootSignature == nullptr)
    {
        ID3D12CommandAllocator* alloc = nullptr;
        ID3D12GraphicsCommandList* gcl = nullptr;

        auto result = InDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
        if (result == S_OK)
        {

            result = InDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, NULL, IID_PPV_ARGS(&gcl));
            if (result == S_OK)
            {
                HookToCommandList(gcl);
                gcl->Release();
            }

            alloc->Release();
        }
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath,
                                                    ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    LOG_FUNC();

    if (inited)
    {
        LOG_WARN("NVNGX already inited");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or_default() && !NVNGXProxy::IsDx12Inited())
    {
        if (Config::Instance()->UseGenericAppIdWithDlss.value_or_default())
            InApplicationId = app_id_override;

        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D12_Init");

            auto result = NVNGXProxy::D3D12_Init()(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);

            LOG_INFO("calling NVNGXProxy::D3D12_Init result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
    }

    DLSSGMod::InitDLSSGMod_Dx12();
    DLSSGMod::D3D12_Init(InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion);

    auto result = NVSDK_NGX_D3D12_Init_Ext(InApplicationId, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);
    LOG_DEBUG("was called NVSDK_NGX_D3D12_Init_Ext");
    return result;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType,
                                                              const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo)
{
    LOG_FUNC();

    if (inited)
    {
        LOG_WARN("NVNGX already inited");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or_default() && !NVNGXProxy::IsDx12Inited())
    {
        if (Config::Instance()->UseGenericAppIdWithDlss.value_or_default())
            InProjectId = project_id_override;

        if (NVNGXProxy::NVNGXModule() == nullptr)
            NVNGXProxy::InitNVNGX();

        if (NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_Init_ProjectID() != nullptr)
        {
            LOG_INFO("calling NVNGXProxy::D3D12_Init_ProjectID");

            auto result = NVNGXProxy::D3D12_Init_ProjectID()(InProjectId, InEngineType, InEngineVersion, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

            LOG_INFO("calling NVNGXProxy::D3D12_Init_ProjectID result: {0:X}", (UINT)result);

            if (result == NVSDK_NGX_Result_Success)
                NVNGXProxy::SetDx12Inited(true);
        }
    }

    auto result = NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    LOG_INFO("InProjectId: {0}", InProjectId);
    LOG_INFO("InEngineType: {0}", (int)InEngineType);
    LOG_INFO("InEngineVersion: {0}", InEngineVersion);

    State::Instance().NVNGX_ProjectId = std::string(InProjectId);
    State::Instance().NVNGX_Engine = InEngineType;
    State::Instance().NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

// Not sure about this one, original nvngx does not export this method
NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion,
                                                                   const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
    LOG_FUNC();

    if (inited)
    {
        LOG_WARN("NVNGX already inited");
        return NVSDK_NGX_Result_Success;
    }

    auto result = NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InSDKVersion, InFeatureInfo);

    LOG_INFO("InProjectId: {0}", InProjectId);
    LOG_INFO("InEngineType: {0}", (int)InEngineType);
    LOG_INFO("InEngineVersion: {0}", InEngineVersion);

    State::Instance().NVNGX_ProjectId = std::string(InProjectId);
    State::Instance().NVNGX_Engine = InEngineType;
    State::Instance().NVNGX_EngineVersion = std::string(InEngineVersion);

    return result;
}

#pragma endregion

#pragma region DLSS Shutdown Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown(void)
{
    shutdown = true;
    inited = false;

    for (auto const& [key, val] : Dx12Contexts) {
        if (val)
            NVSDK_NGX_D3D12_ReleaseFeature(val->Handle());
    }

    Dx12Contexts.clear();
    D3D12Device = nullptr;

    State::Instance().currentFeature = nullptr;

    // Unhooking and cleaning stuff causing issues during shutdown. 
    // Disabled for now to check if it cause any issues
    //UnhookAll();

    DLSSFeatureDx12::Shutdown(D3D12Device);

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_Shutdown() != nullptr)
    {
        auto result = NVNGXProxy::D3D12_Shutdown()();
        NVNGXProxy::SetDx12Inited(false);
    }

    // Unhooking and cleaning stuff causing issues during shutdown. 
    // Disabled for now to check if it cause any issues
    //HooksDx::UnHookDx();

    FrameGen_Dx12::StopAndDestroyFGContext(true, true);

    shutdown = false;

    DLSSGMod::D3D12_Shutdown();

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
    shutdown = true;
    inited = false;

    DLSSGMod::D3D12_Shutdown1(InDevice);

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_Shutdown1() != nullptr)
    {
        auto result = NVNGXProxy::D3D12_Shutdown1()(InDevice);
        NVNGXProxy::SetDx12Inited(false);
    }

    return NVSDK_NGX_D3D12_Shutdown();
}

#pragma endregion

#pragma region DLSS Parameter Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_GetParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D12_GetParameters");
        auto result = NVNGXProxy::D3D12_GetParameters()(OutParameters);
        LOG_INFO("calling NVNGXProxy::D3D12_GetParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

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
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::IsDx12Inited() && NVNGXProxy::D3D12_GetCapabilityParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D12_GetCapabilityParameters");
        auto result = NVNGXProxy::D3D12_GetCapabilityParameters()(OutParameters);
        LOG_INFO("calling NVNGXProxy::D3D12_GetCapabilityParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

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
    LOG_FUNC();

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_AllocateParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D12_AllocateParameters");
        auto result = NVNGXProxy::D3D12_AllocateParameters()(OutParameters);
        LOG_INFO("calling NVNGXProxy::D3D12_AllocateParameters result: {0:X}, ptr: {1:X}", (UINT)result, (UINT64)*OutParameters);

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
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    InitNGXParameters(InParameters);

    DLSSGMod::D3D12_PopulateParameters_Impl(InParameters);

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
    LOG_FUNC();

    if (InParameters == nullptr)
        return NVSDK_NGX_Result_Fail;

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::NVNGXModule() != nullptr && NVNGXProxy::D3D12_DestroyParameters() != nullptr)
    {
        LOG_INFO("calling NVNGXProxy::D3D12_DestroyParameters");
        auto result = NVNGXProxy::D3D12_DestroyParameters()(InParameters);
        LOG_INFO("calling NVNGXProxy::D3D12_DestroyParameters result: {0:X}", (UINT)result);
        return NVSDK_NGX_Result_Success;
    }

    delete InParameters;
    return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region DLSS Feature Calls

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
    LOG_FUNC();

    if (InCmdList != nullptr)
        HookToCommandList(InCmdList);

    if (DLSSGMod::isDx12Available() && InFeatureID == NVSDK_NGX_Feature_FrameGeneration)
    {
        auto result = DLSSGMod::D3D12_CreateFeature(InCmdList, InFeatureID, InParameters, OutHandle);
        LOG_INFO("Creating new modded DLSSG feature with HandleId: {0}", (*OutHandle)->Id);
        return result;
    }
    else if (InFeatureID != NVSDK_NGX_Feature_SuperSampling && InFeatureID != NVSDK_NGX_Feature_RayReconstruction)
    {
        if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::InitDx12(D3D12Device) && NVNGXProxy::D3D12_CreateFeature() != nullptr)
        {
            LOG_INFO("calling D3D12_CreateFeature for ({0})", (int)InFeatureID);
            auto result = NVNGXProxy::D3D12_CreateFeature()(InCmdList, InFeatureID, InParameters, OutHandle);

            if (result == NVSDK_NGX_Result_Success)
            {
                LOG_INFO("D3D12_CreateFeature HandleId for ({0}): {1:X}", (int)InFeatureID, (*OutHandle)->Id);
            }
            else
            {
                LOG_INFO("D3D12_CreateFeature result for ({0}): {1:X}", (int)InFeatureID, (UINT)result);
            }

            return result;
        }
        else
        {
            LOG_ERROR("Can't create this feature ({0})!", (int)InFeatureID);
            return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
        }
    }

    // Create feature
    auto handleId = IFeature::GetNextHandleId();
    LOG_INFO("HandleId: {0}", handleId);

    // DLSS Enabler check
    int deAvail;
    if (InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
    {
        LOG_INFO("DLSSEnabler.Available: {0}", deAvail);
        State::Instance().enablerAvailable = (deAvail > 0);
    }

    // nvsdk logging - ini first
    if (!Config::Instance()->LogToNGX.has_value())
    {
        int nvsdkLogging = 0;
        InParameters->Get("DLSSEnabler.Logging", &nvsdkLogging);
        Config::Instance()->LogToNGX.set_volatile_value(nvsdkLogging > 0);
    }

    // Root signature restore
    if (Config::Instance()->RestoreComputeSignature.value_or_default() || Config::Instance()->RestoreGraphicSignature.value_or_default())
        contextRendering = true;

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
                LOG_INFO("DLSS Enabler does not set any upscaler using ini: {0}", Config::Instance()->Dx12Upscaler.value());

                if (Config::Instance()->Dx12Upscaler.value() == "xess")
                    upscalerChoice = 0;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr22")
                    upscalerChoice = 1;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr21")
                    upscalerChoice = 2;
                else if (Config::Instance()->Dx12Upscaler.value() == "dlss" && Config::Instance()->DLSSEnabled.value_or_default())
                    upscalerChoice = 3;
                else if (Config::Instance()->Dx12Upscaler.value() == "fsr31")
                    upscalerChoice = 4;
            }

            LOG_INFO("upscalerChoice: {0}", upscalerChoice);
        }
        else
        {
            LOG_INFO("DLSS Enabler upscalerChoice: {0}", upscalerChoice);
        }

        if (upscalerChoice == 3)
        {
            Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new DLSS feature, fallback to XeSS!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 0;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "dlss";
                LOG_INFO("creating new DLSS feature");
            }
        }

        if (upscalerChoice == 0)
        {
            Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new XeSS feature, Fallback to FSR2.1!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 2;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "xess";
                LOG_INFO("creating new XeSS feature");
            }
        }

        if (upscalerChoice == 4)
        {
            Dx12Contexts[handleId] = std::make_unique<FSR31FeatureDx12>(handleId, InParameters);

            if (!Dx12Contexts[handleId]->ModuleLoaded())
            {
                LOG_ERROR("can't create new FSR 3.X feature, Fallback to FSR2.1!");

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                upscalerChoice = 2;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "fsr31";
                LOG_INFO("creating new FSR 3.X feature");
            }
        }

        if (upscalerChoice == 1)
        {
            Config::Instance()->Dx12Upscaler = "fsr22";
            LOG_INFO("creating new FSR 2.2.1 feature");
            Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, InParameters);
        }
        else if (upscalerChoice == 2)
        {
            Config::Instance()->Dx12Upscaler = "fsr21";
            LOG_INFO("creating new FSR 2.1.2 feature");
            Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, InParameters);
        }

        // write back finel selected upscaler 
        InParameters->Set("DLSSEnabler.Dx12Backend", upscalerChoice);
    }
    else
    {
        LOG_INFO("creating new DLSSD feature");
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
        LOG_DEBUG("Get D3d12 device from InCmdList!");
        auto deviceResult = InCmdList->GetDevice(IID_PPV_ARGS(&D3D12Device));

        if (deviceResult != S_OK || !D3D12Device)
        {
            LOG_ERROR("Can't get Dx12Device from InCmdList!");
            return NVSDK_NGX_Result_Fail;
        }
    }

#pragma endregion

    if (deviceContext->Init(D3D12Device, InCmdList, InParameters))
    {
        State::Instance().currentFeature = deviceContext;
        evalCounter = 0;

        FrameGen_Dx12::fgTarget = 10;
    }
    else
    {
        LOG_ERROR("CreateFeature failed, returning to FSR 2.1.2 upscaler");
        State::Instance().newBackend = "fsr21";
        State::Instance().changeBackend = true;
    }

    if (Config::Instance()->RestoreComputeSignature.value_or_default() || Config::Instance()->RestoreGraphicSignature.value_or_default())
    {
        if (Config::Instance()->RestoreComputeSignature.value_or_default() && computeSignatures.contains(InCmdList))
        {
            auto signature = computeSignatures[InCmdList];
            LOG_TRACE("restore ComputeRootSig: {0:X}", (UINT64)signature);
            orgSetComputeRootSignature(InCmdList, signature);
        }
        else if (Config::Instance()->RestoreComputeSignature.value_or_default())
        {
            LOG_TRACE("can't restore ComputeRootSig");
        }

        if (Config::Instance()->RestoreGraphicSignature.value_or_default() && graphicSignatures.contains(InCmdList))
        {
            auto signature = graphicSignatures[InCmdList];
            LOG_TRACE("restore GraphicRootSig: {0:X}", (UINT64)signature);
            orgSetGraphicRootSignature(InCmdList, signature);
        }
        else if (Config::Instance()->RestoreGraphicSignature.value_or_default())
        {
            LOG_TRACE("can't restore GraphicRootSig");
        }

        contextRendering = false;
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
    LOG_FUNC();

    if (!InHandle)
        return NVSDK_NGX_Result_Success;

    auto handleId = InHandle->Id;

    State::Instance().FGchanged = true;
    FrameGen_Dx12::StopAndDestroyFGContext(true, false);

    if (!shutdown)
        LOG_INFO("releasing feature with id {0}", handleId);

    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::D3D12_ReleaseFeature() != nullptr)
        {
            if (!shutdown)
                LOG_INFO("calling D3D12_ReleaseFeature for ({0})", handleId);

            auto result = NVNGXProxy::D3D12_ReleaseFeature()(InHandle);

            if (!shutdown)
                LOG_INFO("D3D12_ReleaseFeature result for ({0}): {1:X}", handleId, (UINT)result);

            return result;
        }
        else
        {
            if (!shutdown)
                LOG_INFO("D3D12_ReleaseFeature not available for ({0})", handleId);

            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }
    else if (handleId >= DLSSG_MOD_ID_OFFSET)
    {
        LOG_INFO("D3D12_ReleaseFeature modded DLSSG with HandleId: {0}", handleId);
        return DLSSGMod::D3D12_ReleaseFeature(InHandle);
    }

    if (auto deviceContext = Dx12Contexts[handleId].get(); deviceContext != nullptr)
    {
        if (deviceContext == State::Instance().currentFeature)
        {
            State::Instance().currentFeature = nullptr;
            deviceContext->Shutdown();
        }

        Dx12Contexts[handleId].reset();
        auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
        Dx12Contexts.erase(it);
    }
    else
    {
        if (!shutdown)
            LOG_ERROR("can't release feature with id {0}!", handleId);
    }

    return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
    LOG_DEBUG("for ({0})", (int)FeatureDiscoveryInfo->FeatureID);

    DLSSGMod::InitDLSSGMod_Dx12();

    if (FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_SuperSampling || (DLSSGMod::isDx12Available() && FeatureDiscoveryInfo->FeatureID == NVSDK_NGX_Feature_FrameGeneration))
    {
        if (OutSupported == nullptr)
            OutSupported = new NVSDK_NGX_FeatureRequirement();

        OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
        OutSupported->MinHWArchitecture = 0;

        //Some old windows 10 os version
        strcpy_s(OutSupported->MinOSVersion, "10.0.10240.16384");
        return NVSDK_NGX_Result_Success;
    }

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::NVNGXModule() == nullptr)
        NVNGXProxy::InitNVNGX();

    if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::D3D12_GetFeatureRequirements() != nullptr)
    {
        LOG_DEBUG("D3D12_GetFeatureRequirements for ({0})", (int)FeatureDiscoveryInfo->FeatureID);
        auto result = NVNGXProxy::D3D12_GetFeatureRequirements()(Adapter, FeatureDiscoveryInfo, OutSupported);
        LOG_DEBUG("D3D12_GetFeatureRequirements result for ({0}): {1:X}", (int)FeatureDiscoveryInfo->FeatureID, (UINT)result);
        return result;
    }
    else
    {
        LOG_DEBUG("D3D12_GetFeatureRequirements not available for ({0})", (int)FeatureDiscoveryInfo->FeatureID);
    }

    OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_AdapterUnsupported;
    return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
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

    auto handleId = InFeatureHandle->Id;
    if (Dx12Contexts[handleId] == nullptr) // prevent source api name flicker when dlssg is active
        State::Instance().setInputApiName = State::Instance().currentInputApiName;

    if (State::Instance().setInputApiName.length() == 0)
        State::Instance().currentInputApiName = "DLSS";
    else
        State::Instance().currentInputApiName = State::Instance().setInputApiName;

    State::Instance().setInputApiName.clear();

    if (!InCmdList)
    {
        LOG_ERROR("InCmdList is null!!!");
        return NVSDK_NGX_Result_Fail;
    }

    if (handleId < 1000000)
    {
        if (Config::Instance()->DLSSEnabled.value_or_default() && NVNGXProxy::D3D12_EvaluateFeature() != nullptr)
        {
            LOG_DEBUG("D3D12_EvaluateFeature for ({0})", handleId);
            auto result = NVNGXProxy::D3D12_EvaluateFeature()(InCmdList, InFeatureHandle, InParameters, InCallback);
            LOG_DEBUG("D3D12_EvaluateFeature result for ({0}): {1:X}", handleId, (UINT)result);
            return result;
        }
        else
        {
            LOG_DEBUG("D3D12_EvaluateFeature not avaliable for ({0})", handleId);
            return NVSDK_NGX_Result_FAIL_FeatureNotFound;
        }
    }
    else if (handleId >= DLSSG_MOD_ID_OFFSET)
    {
        // Workaround mostly for final fantasy xvi
        uint32_t depthInverted = 0;
        float cameraNear = 0;
        float cameraFar = 0;
        InParameters->Get("DLSSG.DepthInverted", &depthInverted);
        InParameters->Get("DLSSG.CameraNear", &cameraNear);
        InParameters->Get("DLSSG.CameraFar", &cameraFar);

        if (cameraNear == 0) {
            if (depthInverted)
                cameraNear = 100000.0f;
            else
                cameraNear = 0.1f;

            InParameters->Set("DLSSG.CameraNear", cameraNear);
        }

        if (cameraFar == 0) {
            if (depthInverted)
                cameraFar = 0.1f;
            else
                cameraFar = 100000.0f;

            InParameters->Set("DLSSG.CameraFar", cameraFar);
        } 
        else if (cameraFar == INFINITY) {
            cameraFar = 10000;
            InParameters->Set("DLSSG.CameraFar", cameraFar);
        }

        // Workaround for a bug in Nukem's mod
        if (uint32_t LowresMvec = 0; InParameters->Get("DLSSG.run_lowres_mvec_pass", &LowresMvec) == NVSDK_NGX_Result_Success && LowresMvec == 1) {
            InParameters->Set("DLSSG.MVecsSubrectWidth", 0U);
            InParameters->Set("DLSSG.MVecsSubrectHeight", 0U);
        }

        // Make a copy of the depth going to the frame generator
        // Fixes an issue with the depth being corrupted on AMD under Windows
        ID3D12Resource* dlssgDepth;
        InParameters->Get("DLSSG.Depth", &dlssgDepth);

        if (dlssgDepth) {
            D3D12_RESOURCE_DESC desc = dlssgDepth->GetDesc();

            D3D12_HEAP_PROPERTIES heapProperties;
            D3D12_HEAP_FLAGS heapFlags;

            static ID3D12Resource* copiedDlssgDepth = nullptr;
            if (copiedDlssgDepth != nullptr) {
                copiedDlssgDepth->Release();
                copiedDlssgDepth = nullptr;
            }

            if (dlssgDepth->GetHeapProperties(&heapProperties, &heapFlags) == S_OK) {
                auto result = D3D12Device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &desc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&copiedDlssgDepth)
                );
                if (result == S_OK) {
                    InCmdList->CopyResource(copiedDlssgDepth, dlssgDepth);
                    InParameters->Set("DLSSG.Depth", (void*)copiedDlssgDepth); // cast to make sure it's void*, otherwise dlssg cries
                }
                else
                {
                    LOG_ERROR("Making a new resource for DLSSG Depth has failed");
                }
            }
            else
            {
                LOG_ERROR("Getting heap properties has failed");
            }
        }

        return DLSSGMod::D3D12_EvaluateFeature(InCmdList, InFeatureHandle, InParameters, InCallback);
    }

    evalCounter++;
    if (Config::Instance()->SkipFirstFrames.has_value() && evalCounter < Config::Instance()->SkipFirstFrames.value())
        return NVSDK_NGX_Result_Success;

    if (InCallback)
        LOG_INFO("callback exist");

    // DLSS Enabler
    {
        // DLSS Enabler check
        int deAvail = 0;
        if (!State::Instance().enablerAvailable && InParameters->Get("DLSSEnabler.Available", &deAvail) == NVSDK_NGX_Result_Success)
        {
            if (State::Instance().enablerAvailable != (deAvail > 0))
                LOG_INFO("DLSSEnabler.Available: {0}", deAvail);

            State::Instance().enablerAvailable = (deAvail > 0);
        }

        if (State::Instance().enablerAvailable)
        {
            int limit = 0;
            if (InParameters->Get("FramerateLimit", &limit) == NVSDK_NGX_Result_Success)
            {
                if (Config::Instance()->DE_FramerateLimit.has_value())
                {
                    if (Config::Instance()->DE_FramerateLimit.value() != limit)
                    {
                        LOG_DEBUG("DLSS Enabler FramerateLimit new value: {0}", Config::Instance()->DE_FramerateLimit.value());
                        InParameters->Set("FramerateLimit", Config::Instance()->DE_FramerateLimit.value());
                    }
                }
                else
                {
                    LOG_INFO("DLSS Enabler FramerateLimit initial value: {0}", limit);
                    Config::Instance()->DE_FramerateLimit = limit;
                }
            }
            else if (Config::Instance()->DE_FramerateLimit.has_value())
            {
                InParameters->Set("FramerateLimit", Config::Instance()->DE_FramerateLimit.value());
            }

            int dfgAvail = 0;
            if (!Config::Instance()->DE_DynamicLimitAvailable && InParameters->Get("DFG.Available", &dfgAvail) == NVSDK_NGX_Result_Success)
                Config::Instance()->DE_DynamicLimitAvailable = dfgAvail;

            int dfgEnabled = 0;
            if (InParameters->Get("DFG.Enabled", &dfgEnabled) == NVSDK_NGX_Result_Success)
            {
                if (Config::Instance()->DE_DynamicLimitEnabled.has_value())
                {
                    if (Config::Instance()->DE_DynamicLimitEnabled.value() != dfgEnabled)
                    {
                        LOG_DEBUG("DLSS Enabler DFG {0}", Config::Instance()->DE_DynamicLimitEnabled.value() == 0 ? "disabled" : "enabled");
                        InParameters->Set("DFG.Enabled", Config::Instance()->DE_DynamicLimitEnabled.value());
                    }
                }
                else
                {
                    LOG_INFO("DLSS Enabler DFG initial value: {0} ({1})", dfgEnabled == 0 ? "disabled" : "enabled", dfgEnabled);
                    Config::Instance()->DE_DynamicLimitEnabled = dfgEnabled;
                }
            }
        }
    }

    IFeature_Dx12* deviceContext = nullptr;

    // Change backend
    if (State::Instance().changeBackend)
    {
        if (State::Instance().newBackend == "" || (!Config::Instance()->DLSSEnabled.value_or_default() && State::Instance().newBackend == "dlss"))
            State::Instance().newBackend = Config::Instance()->Dx12Upscaler.value_or_default();

        changeBackendCounter++;

        LOG_INFO("changeBackend is true, counter: {0}", changeBackendCounter);

        // first release everything
        if (changeBackendCounter == 1)
        {
            if (FrameGen_Dx12::fgIsActive)
            {
                FrameGen_Dx12::StopAndDestroyFGContext(false, false);
                State::Instance().FGchanged = true;
            }

            if (Dx12Contexts.contains(handleId))
            {
                LOG_INFO("changing backend to {0}", State::Instance().newBackend);

                auto dc = Dx12Contexts[handleId].get();

                if (State::Instance().newBackend != "dlssd" && State::Instance().newBackend != "dlss")
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

                LOG_DEBUG("sleeping before reset of current feature for 1000ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                Dx12Contexts[handleId].reset();
                auto it = std::find_if(Dx12Contexts.begin(), Dx12Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
                Dx12Contexts.erase(it);

                State::Instance().currentFeature = nullptr;

                contextRendering = false;
            }
            else
            {
                LOG_ERROR("can't find handle {0} in Dx12Contexts!", handleId);

                State::Instance().newBackend = "";
                State::Instance().changeBackend = false;

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
            int upscalerChoice = -1;

            // prepare new upscaler
            if (State::Instance().newBackend == "fsr22")
            {
                Config::Instance()->Dx12Upscaler = "fsr22";
                LOG_INFO("creating new FSR 2.2.1 feature");
                Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12>(handleId, createParams);
                upscalerChoice = 1;
            }
            else if (State::Instance().newBackend == "fsr21")
            {
                Config::Instance()->Dx12Upscaler = "fsr21";
                LOG_INFO("creating new FSR 2.1.2 feature");
                Dx12Contexts[handleId] = std::make_unique<FSR2FeatureDx12_212>(handleId, createParams);
                upscalerChoice = 2;
            }
            else if (State::Instance().newBackend == "dlss")
            {
                Config::Instance()->Dx12Upscaler = "dlss";
                LOG_INFO("creating new DLSS feature");
                Dx12Contexts[handleId] = std::make_unique<DLSSFeatureDx12>(handleId, createParams);
                upscalerChoice = 3;
            }
            else if (State::Instance().newBackend == "dlssd")
            {
                LOG_INFO("creating new DLSSD feature");
                Dx12Contexts[handleId] = std::make_unique<DLSSDFeatureDx12>(handleId, createParams);
            }
            else if (State::Instance().newBackend == "fsr31")
            {
                Config::Instance()->Dx12Upscaler = "fsr31";
                LOG_INFO("creating new FSR 3.X feature");
                Dx12Contexts[handleId] = std::make_unique<FSR31FeatureDx12>(handleId, createParams);
                upscalerChoice = 4;
            }
            else
            {
                Config::Instance()->Dx12Upscaler = "xess";
                LOG_INFO("creating new XeSS feature");
                Dx12Contexts[handleId] = std::make_unique<XeSSFeatureDx12>(handleId, createParams);
                upscalerChoice = 0;
            }

            if (upscalerChoice >= 0)
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
                LOG_ERROR("init failed with {0} feature", State::Instance().newBackend);

                if (State::Instance().newBackend != "dlssd")
                {
                    if (Config::Instance()->Dx12Upscaler == "dlss")
                    {
                        State::Instance().newBackend = "xess";
                        InParameters->Set("DLSSEnabler.Dx12Backend", 0);
                    }
                    else
                    {
                        State::Instance().newBackend = "fsr21";
                        InParameters->Set("DLSSEnabler.Dx12Backend", 2);
                    }

                }
                else
                {
                    // Retry DLSSD
                    State::Instance().newBackend = "dlssd";
                }

                State::Instance().changeBackend = true;
                return NVSDK_NGX_Result_Success;
            }
            else
            {
                LOG_INFO("init successful for {0}, upscaler changed", State::Instance().newBackend);

                State::Instance().newBackend = "";
                State::Instance().changeBackend = false;
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
        State::Instance().currentFeature = Dx12Contexts[handleId].get();
        FrameGen_Dx12::fgTarget = 20;

        return NVSDK_NGX_Result_Success;
    }

    deviceContext = Dx12Contexts[handleId].get();

    if (deviceContext == nullptr)
    {
        LOG_DEBUG("trying to use released handle, returning NVSDK_NGX_Result_Success");
        return NVSDK_NGX_Result_Success;
    }

    if (!deviceContext->IsInited() && Config::Instance()->Dx12Upscaler.value_or_default() != "fsr21")
    {
        LOG_WARN("InCmdList {0} is not inited, falling back to FSR 2.1.2", deviceContext->Name());
        State::Instance().newBackend = "fsr21";
        State::Instance().changeBackend = true;
        return NVSDK_NGX_Result_Success;
    }

    State::Instance().currentFeature = deviceContext;

    // Root signature restore
    if (deviceContext->Name() != "DLSSD" && (Config::Instance()->RestoreComputeSignature.value_or_default() || Config::Instance()->RestoreGraphicSignature.value_or_default()))
        contextRendering = true;

    // FG Init || Disable    
    if (Config::Instance()->FGUseFGSwapChain.value_or_default() && Config::Instance()->OverlayMenu.value_or_default())
    {
        if (!State::Instance().FGchanged && FrameGen_Dx12::fgTarget < deviceContext->FrameCount() && Config::Instance()->FGEnabled.value_or_default() &&
            FfxApiProxy::InitFfxDx12() && !FrameGen_Dx12::fgIsActive && HooksDx::currentSwapchain != nullptr &&
            HooksDx::CurrentSwapchainFormat() != DXGI_FORMAT_UNKNOWN)
        {
            FrameGen_Dx12::CreateFGObjects(D3D12Device);
            FrameGen_Dx12::CreateFGContext(D3D12Device, deviceContext);
            FrameGen_Dx12::fgTarget = deviceContext->FrameCount() + 3;
        }
        else if ((!Config::Instance()->FGEnabled.value_or_default() || State::Instance().FGchanged) && FrameGen_Dx12::fgIsActive)
        {
            FrameGen_Dx12::StopAndDestroyFGContext(State::Instance().SCchanged, false);
        }

        if (State::Instance().FGchanged)
        {
            LOG_DEBUG("(FG) Frame generation disabled for 20 frames");
            FrameGen_Dx12::fgTarget = deviceContext->FrameCount() + 20;
            FrameGen_Dx12::ResetIndexes();
            State::Instance().FGchanged = false;
        }
    }

    State::Instance().SCchanged = false;

    // FSR Camera values
    float cameraNear = 0.0f;
    float cameraFar = 0.0f;
    float cameraVFov = 0.0f;
    double ftDelta = 0.0f;
    float meterFactor = 0.0f;

    {
        if (!Config::Instance()->FsrUseFsrInputValues.value_or_default() || InParameters->Get("FSR.cameraNear", &cameraNear) != NVSDK_NGX_Result_Success)
        {
            if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
                cameraFar = Config::Instance()->FsrCameraNear.value_or_default();
            else
                cameraNear = Config::Instance()->FsrCameraNear.value_or_default();
        }

        if (!Config::Instance()->FsrUseFsrInputValues.value_or_default() || InParameters->Get("FSR.cameraFar", &cameraFar) != NVSDK_NGX_Result_Success)
        {
            if (deviceContext->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted)
                cameraNear = Config::Instance()->FsrCameraFar.value_or_default();
            else
                cameraFar = Config::Instance()->FsrCameraFar.value_or_default();
        }

        if (!Config::Instance()->FsrUseFsrInputValues.value_or_default() || InParameters->Get("FSR.cameraFovAngleVertical", &cameraVFov) != NVSDK_NGX_Result_Success)
        {
            if (Config::Instance()->FsrVerticalFov.has_value())
                cameraVFov = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
            else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
                cameraVFov = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)deviceContext->TargetHeight() * (float)deviceContext->TargetWidth());
            else
                cameraVFov = 1.0471975511966f;
        }

        //if (!Config::Instance()->FsrUseFsrInputValues.value_or_default() || InParameters->Get("FSR.frameTimeDelta", &ftDelta) != NVSDK_NGX_Result_Success)
        //{
        //    if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &ftDelta) != NVSDK_NGX_Result_Success || ftDelta < 1.0f)
        //        ftDelta = FrameGen_Dx12::GetFrameTime();
        //}

        ftDelta = FrameGen_Dx12::GetFrameTime();

        LOG_DEBUG("FrameTimeDeltaInMsec: {0}", FrameGen_Dx12::ftDelta);

        if (!Config::Instance()->FsrUseFsrInputValues.value_or_default() || InParameters->Get("FSR.viewSpaceToMetersFactor", &FrameGen_Dx12::meterFactor) != NVSDK_NGX_Result_Success)
            FrameGen_Dx12::meterFactor = 0.0f;

        State::Instance().lastFsrCameraFar = cameraFar;
        State::Instance().lastFsrCameraNear = cameraNear;

        if (InParameters->Get(NVSDK_NGX_Parameter_Reset, &FrameGen_Dx12::reset) != NVSDK_NGX_Result_Success)
            FrameGen_Dx12::reset = 0;
    }

    // FG Prepare
    ID3D12Resource* output;
    InParameters->Get(NVSDK_NGX_Parameter_Output, &output);
    UINT frameIndex;

    if (FrameGen_Dx12::fgIsActive && Config::Instance()->FGUseFGSwapChain.value_or_default() && Config::Instance()->OverlayMenu.value_or_default() &&
        Config::Instance()->FGEnabled.value_or_default() && FrameGen_Dx12::fgTarget <= deviceContext->FrameCount() &&
        FrameGen_Dx12::fgContext != nullptr && HooksDx::currentSwapchain != nullptr)
    {
        frameIndex = FrameGen_Dx12::NewFrame();

        bool allocatorReset = false;

#ifdef USE_QUEUE_FOR_FG
        auto allocator = FrameGen_Dx12::fgCommandAllocators[frameIndex];
        auto result = allocator->Reset();
        result = FrameGen_Dx12::fgCommandList[frameIndex]->Reset(allocator, nullptr);
        allocatorReset = true;
#endif

        if (!allocatorReset && Config::Instance()->FGHUDFix.value_or_default())
        {
            auto allocator = FrameGen_Dx12::fgCommandAllocators[frameIndex];
            auto result = allocator->Reset();
            result = FrameGen_Dx12::fgCommandList[frameIndex]->Reset(allocator, nullptr);
        }

        ID3D12GraphicsCommandList* commandList = nullptr;

#ifdef USE_COPY_QUEUE_FOR_FG
        FrameGen_Dx12::fgCopyCommandAllocator->Reset();
        FrameGen_Dx12::fgCopyCommandList->Reset(FrameGen_Dx12::fgCopyCommandAllocator, nullptr);

        commandList = FrameGen_Dx12::fgCopyCommandList;
#else
        commandList = InCmdList;
#endif

        LOG_DEBUG("(FG) copy buffers for fgUpscaledImage[{}], frame: {}", frameIndex, deviceContext->FrameCount());

        InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &FrameGen_Dx12::jitterX);
        InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &FrameGen_Dx12::jitterY);

        ID3D12Resource* paramVelocity;
        if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

        ResourceBarrier(commandList, paramVelocity, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

        if (CreateBufferResource(L"fgVelocity", D3D12Device, paramVelocity, D3D12_RESOURCE_STATE_COPY_DEST, &FrameGen_Dx12::paramVelocity[frameIndex]))
            commandList->CopyResource(FrameGen_Dx12::paramVelocity[frameIndex], paramVelocity);

        ResourceBarrier(commandList, paramVelocity, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

        ID3D12Resource* paramDepth;
        if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
            InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

        ResourceBarrier(commandList, paramDepth, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

        if (CreateBufferResource(L"fgDepth", D3D12Device, paramDepth, D3D12_RESOURCE_STATE_COPY_DEST, &FrameGen_Dx12::paramDepth[frameIndex]))
            commandList->CopyResource(FrameGen_Dx12::paramDepth[frameIndex], paramDepth);

        ResourceBarrier(commandList, paramDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

#ifdef USE_COPY_QUEUE_FOR_FG
        ID3D12CommandList* cl[1] = { nullptr };
        result = FrameGen_Dx12::fgCopyCommandList->Close();
        cl[0] = FrameGen_Dx12::fgCopyCommandList;
        FrameGen_Dx12::fgCopyCommandQueue->ExecuteCommandLists(1, cl);
#endif

        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &FrameGen_Dx12::mvScaleX);
        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &FrameGen_Dx12::mvScaleY);

        FrameGen_Dx12::cameraFar = cameraFar;
        FrameGen_Dx12::cameraNear = cameraNear;
        FrameGen_Dx12::cameraVFov = cameraVFov;
        FrameGen_Dx12::ftDelta = ftDelta;
        FrameGen_Dx12::meterFactor = meterFactor;

        LOG_DEBUG("(FG) copy buffers done, frame: {0}", deviceContext->FrameCount());
    }

    // Record the first timestamp
    if (!State::Instance().isWorkingAsNvngx)
        InCmdList->EndQuery(HooksDx::queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);

    // Run upscaler
    auto evalResult = deviceContext->Evaluate(InCmdList, InParameters);

    // Record the second timestamp 
    if (!State::Instance().isWorkingAsNvngx)
    {
        InCmdList->EndQuery(HooksDx::queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);

        // Resolve the queries to the readback buffer
        InCmdList->ResolveQueryData(HooksDx::queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, HooksDx::readbackBuffer, 0);
    }

    NVSDK_NGX_Result methodResult = NVSDK_NGX_Result_Fail;

    // FG Dispatch
    if (evalResult)
    {
        HooksDx::dx12UpscaleTrig = true;

        // FG Dispatch
        if (FrameGen_Dx12::fgIsActive && Config::Instance()->FGUseFGSwapChain.value_or_default() && Config::Instance()->OverlayMenu.value_or_default() &&
            Config::Instance()->FGEnabled.value_or_default() && FrameGen_Dx12::fgTarget < deviceContext->FrameCount() &&
            FrameGen_Dx12::fgContext != nullptr && HooksDx::currentSwapchain != nullptr)
        {
            FrameGen_Dx12::upscaleRan = true;
            fgCallbackFrameIndex = frameIndex;

#ifndef USE_PRESENT_FOR_FT
            float msDelta = 0.0;
            auto now = Util::MillisecondsNow();

            if (fgLastFrameTime != 0)
            {
                msDelta = now - fgLastFrameTime;
                LOG_DEBUG("(FG) msDelta: {0}", msDelta);
            }

            fgLastFrameTime = now;
            FrameGen_Dx12::fgFrameTime = msDelta;
#endif

            if (Config::Instance()->FGHUDFix.value_or_default())
            {
                FrameGen_Dx12::CheckUpscaledFrame(InCmdList, output);
            }
            else
            {
                LOG_DEBUG("(FG) running, frame: {0}", deviceContext->FrameCount());

                // Update frame generation config
                auto desc = output->GetDesc();

                ffxConfigureDescFrameGeneration m_FrameGenerationConfig = {};

                if (desc.Format == HooksDx::CurrentSwapchainFormat())
                {
                    LOG_DEBUG("(FG) desc.Format == HooksDx::swapchainFormat, using for hudless!");
                    m_FrameGenerationConfig.HUDLessColor = ffxApiGetResourceDX12(output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS, 0);
                }
                else
                {
                    m_FrameGenerationConfig.HUDLessColor = FfxApiResource({});
                }

                m_FrameGenerationConfig.header.type = FFX_API_CONFIGURE_DESC_TYPE_FRAMEGENERATION;
                m_FrameGenerationConfig.frameGenerationEnabled = true;
                m_FrameGenerationConfig.flags = 0;

                if (Config::Instance()->FGDebugView.value_or_default())
                    m_FrameGenerationConfig.flags |= FFX_FRAMEGENERATION_FLAG_DRAW_DEBUG_VIEW;

                m_FrameGenerationConfig.allowAsyncWorkloads = Config::Instance()->FGAsync.value_or_default();
                m_FrameGenerationConfig.generationRect.left = Config::Instance()->FGRectLeft.value_or(0);
                m_FrameGenerationConfig.generationRect.top = Config::Instance()->FGRectTop.value_or(0);

                // use swapchain buffer info 
                DXGI_SWAP_CHAIN_DESC scDesc{};
                if (HooksDx::currentSwapchain->GetDesc(&scDesc) == S_OK)
                {
                    m_FrameGenerationConfig.generationRect.width = Config::Instance()->FGRectWidth.value_or(scDesc.BufferDesc.Width);
                    m_FrameGenerationConfig.generationRect.height = Config::Instance()->FGRectHeight.value_or(scDesc.BufferDesc.Height);
                }
                else
                {
                    m_FrameGenerationConfig.generationRect.width = Config::Instance()->FGRectWidth.value_or(deviceContext->DisplayWidth());
                    m_FrameGenerationConfig.generationRect.height = Config::Instance()->FGRectHeight.value_or(deviceContext->DisplayHeight());
                }

                m_FrameGenerationConfig.frameGenerationCallbackUserContext = &FrameGen_Dx12::fgContext;
                m_FrameGenerationConfig.frameGenerationCallback = [](ffxDispatchDescFrameGeneration* params, void* pUserCtx) -> ffxReturnCode_t
                    {
                        auto fIndex = fgCallbackFrameIndex;

                        params->reset = (FrameGen_Dx12::reset != 0);

                        // check for status
                        if (!Config::Instance()->FGEnabled.value_or_default() ||
                            FrameGen_Dx12::fgContext == nullptr || State::Instance().SCchanged
#ifdef USE_QUEUE_FOR_FG
                            || FrameGen_Dx12::fgCommandList[fIndex] == nullptr || FrameGen_Dx12::fgCommandQueue == nullptr
#endif
                            )
                        {
                            LOG_WARN("(FG) Cancel async dispatch fIndex: {}", fIndex);
                            FrameGen_Dx12::fgSkipHudlessChecks = false;
                            return FFX_API_RETURN_OK;
                        }

                        // If fg is active but upscaling paused
                        if (State::Instance().currentFeature == nullptr || !FrameGen_Dx12::fgIsActive ||
                            State::Instance().FGchanged || fgLastFGFrame == State::Instance().currentFeature->FrameCount() ||
                            State::Instance().currentFeature->FrameCount() == 0)
                        {
                            LOG_WARN("(FG) Callback without active FG! fIndex:{}", fIndex);

#ifdef USE_QUEUE_FOR_FG
                            auto allocator = FrameGen_Dx12::fgCommandAllocators[fIndex];
                            auto result = allocator->Reset();
                            result = FrameGen_Dx12::fgCommandList[fIndex]->Reset(allocator, nullptr);
#endif

                            params->frameID = fgLastFGFrame;
                            params->numGeneratedFrames = 0;
                        }

                        if (State::Instance().currentFeature != nullptr)
                            fgLastFGFrame = State::Instance().currentFeature->FrameCount();

                        auto dispatchResult = FfxApiProxy::D3D12_Dispatch()(reinterpret_cast<ffxContext*>(pUserCtx), &params->header);
                        LOG_DEBUG("(FG) D3D12_Dispatch result: {}, fIndex: {}", (UINT)dispatchResult, fIndex);

                        if (dispatchResult == FFX_API_RETURN_OK)
                        {
#ifdef USE_QUEUE_FOR_FG
                            ID3D12CommandList* cl[1] = { nullptr };
                            auto result = FrameGen_Dx12::fgCommandList[fIndex]->Close();
                            cl[0] = FrameGen_Dx12::fgCommandList[fIndex];
                            FrameGen_Dx12::fgCommandQueue->ExecuteCommandLists(1, cl);

                            if (result != S_OK)
                            {
                                LOG_ERROR("(FG) Close result: {}", (UINT)result);
                            }

#endif

#ifdef DONT_USE_DEPTH_MV_COPIES
                            ResourceBarrier(InCmdList, paramVelocity, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
                            ResourceBarrier(InCmdList, paramDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
#endif
                        }

                        return dispatchResult;
                        };

                m_FrameGenerationConfig.onlyPresentGenerated = State::Instance().FGonlyGenerated; // check here
                m_FrameGenerationConfig.frameID = deviceContext->FrameCount();
                m_FrameGenerationConfig.swapChain = HooksDx::currentSwapchain;

                //State::Instance().skipSpoofing = true;
                ffxReturnCode_t retCode = FfxApiProxy::D3D12_Configure()(&FrameGen_Dx12::fgContext, &m_FrameGenerationConfig.header);
                //State::Instance().skipSpoofing = false;

                if (retCode != FFX_API_RETURN_OK)
                    LOG_ERROR("(FG) D3D12_Configure error: {}({})", retCode, FfxApiProxy::ReturnCodeToString(retCode));

                if (retCode == FFX_API_RETURN_OK)
                {
                    ffxCreateBackendDX12Desc backendDesc{};
                    backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
                    backendDesc.device = D3D12Device;

                    ffxDispatchDescFrameGenerationPrepare dfgPrepare{};
                    dfgPrepare.header.type = FFX_API_DISPATCH_DESC_TYPE_FRAMEGENERATION_PREPARE;
                    dfgPrepare.header.pNext = &backendDesc.header;

#ifdef USE_QUEUE_FOR_FG
                    dfgPrepare.commandList = FrameGen_Dx12::fgCommandList[fIndex];
#else
                    dfgPrepare.commandList = InCmdList;
#endif
                    dfgPrepare.frameID = deviceContext->FrameCount();
                    dfgPrepare.flags = m_FrameGenerationConfig.flags;
                    dfgPrepare.renderSize = { deviceContext->RenderWidth(), deviceContext->RenderHeight() };

                    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &dfgPrepare.jitterOffset.x);
                    InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &dfgPrepare.jitterOffset.y);

#ifndef DONT_USE_DEPTH_MV_COPIES
                    dfgPrepare.motionVectors = ffxApiGetResourceDX12(FrameGen_Dx12::paramVelocity[frameIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
                    dfgPrepare.depth = ffxApiGetResourceDX12(FrameGen_Dx12::paramDepth[frameIndex], FFX_API_RESOURCE_STATE_COPY_DEST);
#else
                    ID3D12Resource* paramVelocity;
                    if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
                        InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

                    ResourceBarrier(InCmdList, paramVelocity, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

                    dfgPrepare.motionVectors = ffxApiGetResourceDX12(paramVelocity, FFX_API_RESOURCE_STATE_COMPUTE_READ);

                    ID3D12Resource* paramDepth;
                    if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
                        InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

                    ResourceBarrier(InCmdList, paramDepth, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value_or(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), D3D12_RESOURCE_STATE_COPY_SOURCE);

                    dfgPrepare.depth = ffxApiGetResourceDX12(paramDepth, FFX_API_RESOURCE_STATE_COMPUTE_READ);

#endif

                    float MVScaleX = 1.0f;
                    float MVScaleY = 1.0f;

                    if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
                        InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
                    {
                        dfgPrepare.motionVectorScale.x = MVScaleX;
                        dfgPrepare.motionVectorScale.y = MVScaleY;
                    }
                    else
                    {
                        LOG_WARN("(FG) Can't get motion vector scales!");

                        dfgPrepare.motionVectorScale.x = MVScaleX;
                        dfgPrepare.motionVectorScale.y = MVScaleY;
                    }

                    dfgPrepare.cameraFar = cameraFar;
                    dfgPrepare.cameraNear = cameraNear;
                    dfgPrepare.cameraFovAngleVertical = cameraVFov;
                    dfgPrepare.frameTimeDelta = ftDelta;
                    dfgPrepare.viewSpaceToMetersFactor = meterFactor;

#ifdef USE_MUTEX_FOR_FFX
                    {
                        LOG_TRACE("Waiting mutex");
                        std::unique_lock<std::shared_mutex> lock(FrameGen_Dx12::ffxMutex);
#endif

                        retCode = FfxApiProxy::D3D12_Dispatch()(&FrameGen_Dx12::fgContext, &dfgPrepare.header);

#ifdef USE_MUTEX_FOR_FFX
                    }
#endif

                    if (retCode != FFX_API_RETURN_OK)
                        LOG_ERROR("(FG) D3D12_Dispatch result: {}({})", retCode, FfxApiProxy::ReturnCodeToString(retCode));
                    else
                        LOG_DEBUG("(FG) Dispatch ok.");
                }
                    }
            }

        methodResult = NVSDK_NGX_Result_Success;
        }

    // Root signature restore
    if (deviceContext->Name() != "DLSSD" && (Config::Instance()->RestoreComputeSignature.value_or_default() || Config::Instance()->RestoreGraphicSignature.value_or_default()))
    {
        if (Config::Instance()->RestoreComputeSignature.value_or_default() && computeSignatures[InCmdList])
        {
            auto signature = computeSignatures[InCmdList];
            LOG_TRACE("restore orgComputeRootSig: {0:X}", (UINT64)signature);
            orgSetComputeRootSignature(InCmdList, signature);
        }
        else if (Config::Instance()->RestoreComputeSignature.value_or_default())
        {
            LOG_WARN("Can't restore ComputeRootSig!");
        }

        if (Config::Instance()->RestoreGraphicSignature.value_or_default() && /*graphTime != 0 && graphTime > lastEvalTime && graphTime <= evaluateStart &&*/ graphicSignatures[InCmdList])
        {
            auto signature = graphicSignatures[InCmdList];
            LOG_TRACE("restore orgGraphicRootSig: {0:X}", (UINT64)signature);
            orgSetGraphicRootSignature(InCmdList, signature);
        }
        else if (Config::Instance()->RestoreGraphicSignature.value_or_default())
        {
            LOG_WARN("Can't restore GraphicRootSig!");
        }

        contextRendering = false;
    }

    LOG_DEBUG("Upscaling done: {}", evalResult);

    return methodResult;
    }

#pragma endregion

#pragma region DLSS Buffer Size Call

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
    if (DLSSGMod::isDx12Available() && InFeatureId == NVSDK_NGX_Feature_FrameGeneration) {
        return DLSSGMod::D3D12_GetScratchBufferSize(InFeatureId, InParameters, OutSizeInBytes);
    }

    LOG_WARN("-> 52428800");
    *OutSizeInBytes = 52428800;
    return NVSDK_NGX_Result_Success;
}

#pragma endregion

