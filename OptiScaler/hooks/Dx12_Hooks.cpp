#include "Dx12_Hooks.h"

#include <Config.h>
#include <detours.h>

// Device hook and variable
static PFN_D3D12_CREATE_DEVICE o_D3D12CreateDevice = nullptr;
ID3D12Device* lastDevice = nullptr;

// Sampler hook for MipLodBias and Anisotropic Filtering
typedef void(*PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

static PFN_CreateSampler o_CreateSampler = nullptr;

// Root signature hooks and variables
typedef HRESULT(*PFN_CreateCommandList)(ID3D12Device* This, UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList);
typedef void(*PFN_SetRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);

static PFN_CreateCommandList o_CreateCommandList = nullptr;
static PFN_SetRootSignature o_SetComputeRootSignature = nullptr;
static PFN_SetRootSignature o_SetGraphicRootSignature = nullptr;

static ID3D12RootSignature* rootSigCompute = nullptr;
static ID3D12RootSignature* rootSigGraphic = nullptr;

static uint64_t computeRSTime = 0;
static uint64_t graphRSTime = 0;
static uint64_t lastEvalTime = 0;
static uint64_t evalStartTime = 0;
static std::mutex sigatureMutex;

static void HookToDevice(ID3D12Device* InDevice);
static void HookToCommandList(ID3D12CommandList* InCmdList);

static int64_t GetTicks()
{
    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks))
        return 0;

    return ticks.QuadPart;
}

static void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (Config::Instance()->RestoreComputeSignature.value_or(false) &&
        commandList != nullptr && pRootSignature != nullptr && (lastEvalTime == 0 || lastEvalTime < evalStartTime))
    {
        sigatureMutex.lock();
        rootSigCompute = pRootSignature;
        computeRSTime = GetTicks();
        sigatureMutex.unlock();
    }

    return o_SetComputeRootSignature(commandList, pRootSignature);
}

static void hkSetGraphicRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (Config::Instance()->RestoreGraphicSignature.value_or(false) &&
        commandList != nullptr && pRootSignature != nullptr && (lastEvalTime == 0 || lastEvalTime < evalStartTime))
    {
        sigatureMutex.lock();
        rootSigGraphic = pRootSignature;
        graphRSTime = GetTicks();
        sigatureMutex.unlock();
    }

    return o_SetGraphicRootSignature(commandList, pRootSignature);
}

static void hkCreateSampler(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    if (pDesc == nullptr || device == nullptr)
        return;

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

    if (Config::Instance()->AnisotropyOverride.has_value() &&
        (pDesc->Filter == D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT || pDesc->Filter == D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT || pDesc->Filter == D3D12_FILTER_MIN_MAG_MIP_LINEAR || pDesc->Filter == D3D12_FILTER_ANISOTROPIC))
    {
        LOG_INFO("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pDesc->Filter);
        newDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        newDesc.MaxAnisotropy = Config::Instance()->AnisotropyOverride.value();
    }

    newDesc.MaxLOD = pDesc->MaxLOD;
    newDesc.MinLOD = pDesc->MinLOD;
    newDesc.MipLODBias = pDesc->MipLODBias;

    if (newDesc.MipLODBias < 0.0f)
    {
        if (Config::Instance()->MipmapBiasOverride.has_value())
        {
            LOG_INFO("Overriding mipmap bias {0} -> {1}", pDesc->MipLODBias, Config::Instance()->MipmapBiasOverride.value());
            newDesc.MipLODBias = Config::Instance()->MipmapBiasOverride.value();
        }

        Config::Instance()->lastMipBias = newDesc.MipLODBias;
    }

    return o_CreateSampler(device, &newDesc, DestDescriptor);
}

static HRESULT hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    auto result = o_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (result == S_OK)
    {
        auto device = (ID3D12Device*)*ppDevice;

        LOG_INFO("hooking D3D12Device");
        HookToDevice(device);
    }

    return result;
}

static HRESULT hkCreateCommandList(ID3D12Device* This, UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList)
{
    auto result = o_CreateCommandList(This, nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);

    if (result == S_OK)
    {
        auto commandList = *reinterpret_cast<ID3D12CommandList**>(ppCommandList);
        HookToCommandList(commandList);
    }

    return result;
}

static void HookToCommandList(ID3D12CommandList* InCmdList)
{
    if (o_SetComputeRootSignature != nullptr || o_SetGraphicRootSignature != nullptr)
        return;

    LOG_FUNC();

    ID3D12GraphicsCommandList* graphCommandList = nullptr;

    if (InCmdList->QueryInterface(IID_PPV_ARGS(&graphCommandList)) != S_OK || graphCommandList == nullptr)
        return;

    PVOID* pVTable = *reinterpret_cast<PVOID**>(graphCommandList);
    o_SetComputeRootSignature = reinterpret_cast<PFN_SetRootSignature>(pVTable[29]);
    o_SetGraphicRootSignature = reinterpret_cast<PFN_SetRootSignature>(pVTable[30]);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_SetComputeRootSignature != nullptr)
        DetourAttach(&(PVOID&)o_SetComputeRootSignature, hkSetComputeRootSignature);

    if (o_SetGraphicRootSignature != nullptr)
        DetourAttach(&(PVOID&)o_SetGraphicRootSignature, hkSetGraphicRootSignature);

    DetourTransactionCommit();

    graphCommandList->Release();
}

static void HookToDevice(ID3D12Device* InDevice)
{
    lastDevice = InDevice;
    LOG_DEBUG("new D3D12Device: {0:X}", (UINT64)InDevice);

    if (o_CreateSampler != nullptr || o_CreateCommandList != nullptr)
        return;

    LOG_FUNC();

    PVOID* pVTable = *reinterpret_cast<PVOID**>(InDevice);
    o_CreateSampler = reinterpret_cast<PFN_CreateSampler>(pVTable[22]);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_CreateSampler != nullptr)
        DetourAttach(&(PVOID&)o_CreateSampler, hkCreateSampler);

    if (o_CreateCommandList != nullptr)
        DetourAttach(&(PVOID&)o_CreateCommandList, hkCreateCommandList);

    DetourTransactionCommit();
}

void Hooks::AttachDx12Hooks()
{
    if (o_D3D12CreateDevice != nullptr)
        return;

    o_D3D12CreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(DetourFindFunction("d3d12.dll", "D3D12CreateDevice"));

    if (o_D3D12CreateDevice != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_D3D12CreateDevice, hkD3D12CreateDevice);

        DetourTransactionCommit();
    }
}

void Hooks::DetachDx12Hooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_D3D12CreateDevice != nullptr)
    {
        DetourDetach(&(PVOID&)o_D3D12CreateDevice, hkD3D12CreateDevice);
        o_D3D12CreateDevice = nullptr;
    }

    if (o_SetComputeRootSignature != nullptr)
    {
        DetourDetach(&(PVOID&)o_SetComputeRootSignature, hkSetComputeRootSignature);
        o_SetComputeRootSignature = nullptr;
    }

    if (o_SetGraphicRootSignature != nullptr)
    {
        DetourDetach(&(PVOID&)o_SetGraphicRootSignature, hkSetGraphicRootSignature);
        o_SetGraphicRootSignature = nullptr;
    }

    if (o_CreateSampler != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateSampler, hkCreateSampler);
        o_CreateSampler = nullptr;
    }

    if (o_CreateCommandList != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateCommandList, hkCreateCommandList);
        o_CreateCommandList = nullptr;
    }

    DetourTransactionCommit();
}

void Hooks::ContextEvaluateStart()
{
    evalStartTime = GetTicks();
}

void Hooks::ContextEvaluateStop(ID3D12GraphicsCommandList* InCmdList, bool InRestore)
{
    if (InRestore && (Config::Instance()->RestoreComputeSignature.value_or(false) || Config::Instance()->RestoreGraphicSignature.value_or(false)))
    {
        sigatureMutex.lock();

        if (Config::Instance()->RestoreComputeSignature.value_or(false) && computeRSTime != 0 &&
            computeRSTime > lastEvalTime && computeRSTime <= evalStartTime && rootSigCompute != nullptr)
        {
            LOG_TRACE("restore orgComputeRootSig: {0:X}", (UINT64)rootSigCompute);
            o_SetComputeRootSignature(InCmdList, rootSigCompute);
        }
        else if (Config::Instance()->RestoreComputeSignature.value_or(false))

        {
            LOG_TRACE("orgComputeRootSig lastEvalTime: {0}", lastEvalTime);
            LOG_TRACE("orgComputeRootSig computeRSTime: {0}", computeRSTime);
            LOG_TRACE("orgComputeRootSig evalStartTime: {0}", evalStartTime);
        }

        if (Config::Instance()->RestoreGraphicSignature.value_or(false) &&
            graphRSTime != 0 && graphRSTime > lastEvalTime && graphRSTime <= evalStartTime && rootSigGraphic != nullptr)
        {
            LOG_TRACE("restore orgGraphicRootSig: {0:X}", (UINT64)rootSigGraphic);
            o_SetGraphicRootSignature(InCmdList, rootSigGraphic);
        }
        else if (Config::Instance()->RestoreGraphicSignature.value_or(false))
        {

            LOG_TRACE("orgGraphicRootSig lastEvalTime: {0}", lastEvalTime);
            LOG_TRACE("orgGraphicRootSig computeRSTime: {0}", graphRSTime);
            LOG_TRACE("orgGraphicRootSig evalStartTime: {0}", evalStartTime);
        }

        sigatureMutex.unlock();
    }

    lastEvalTime = evalStartTime;

    rootSigCompute = nullptr;
    rootSigGraphic = nullptr;
}

ID3D12Device* Hooks::Dx12Device()
{
    return lastDevice;
}


