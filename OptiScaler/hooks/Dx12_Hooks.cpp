#include "Dx12_Hooks.h"

#include "../pch.h"
#include "../Config.h"
#include "../detours/detours.h"

// Menu Hooks
typedef void(*PFN_ExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);
typedef ULONG(*PFN_Release)(IUnknown*);
typedef HRESULT(*PFN_CreateCommandQueue)(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue);

static PFN_D3D12_CREATE_DEVICE o_D3D12CreateDevice = nullptr;
static PFN_CreateCommandQueue o_CreateCommandQueue = nullptr;
static PFN_Release o_CommandQueueRelease = nullptr;
static PFN_ExecuteCommandLists o_ExecuteCommandLists = nullptr;

// Sampler stuff for MipLodBias and Anisotropic Filtering
typedef void(*PFN_CreateSampler)(ID3D12Device* device, const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
static PFN_CreateSampler o_CreateSampler = nullptr;

// Root signature stuff
typedef void(*PFN_SetRootSignature)(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature);
static PFN_SetRootSignature o_SetComputeRootSignature = nullptr;
static PFN_SetRootSignature o_SetGraphicRootSignature = nullptr;

static ID3D12RootSignature* rootSigCompute = nullptr;
static ID3D12RootSignature* rootSigGraphic = nullptr;

static bool contextRendering = false;
static ULONGLONG computeTime = 0;
static ULONGLONG graphTime = 0;
static ULONGLONG lastEvalTime = 0;
static uint64_t evaluateStart;
static std::mutex sigatureMutex;

static int64_t GetTicks()
{
    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks))
        return 0;

    return ticks.QuadPart;
}

static void HookToCommandList(ID3D12GraphicsCommandList* InCmdList);
static void HookToDevice(ID3D12Device* InDevice);

static void hkSetComputeRootSignature(ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* pRootSignature)
{
    if (!contextRendering && commandList != nullptr && pRootSignature != nullptr)
    {
        sigatureMutex.lock();
        rootSigCompute = pRootSignature;
        computeTime = GetTicks();
        sigatureMutex.unlock();
    }

    return o_SetComputeRootSignature(commandList, pRootSignature);
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

    if (Config::Instance()->AnisotropyOverride.has_value() &&
        (pDesc->Filter == D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT ||
        pDesc->Filter == D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT ||
        pDesc->Filter == D3D12_FILTER_MIN_MAG_MIP_LINEAR ||
        pDesc->Filter == D3D12_FILTER_ANISOTROPIC))
    {
        LOG_INFO("Overriding Anisotrpic ({2}) filtering {0} -> {1}", pDesc->MaxAnisotropy, Config::Instance()->AnisotropyOverride.value(), (UINT)pDesc->Filter);
        newDesc.Filter = D3D12_FILTER_ANISOTROPIC;
        newDesc.MaxAnisotropy = Config::Instance()->AnisotropyOverride.value();
    }
    else
    {
        newDesc.Filter = pDesc->Filter;
        newDesc.MaxAnisotropy = pDesc->MaxAnisotropy;
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

static HRESULT WINAPI hkD3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
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

static void WINAPI hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    // TODO: Continue from here to Dx12 Overlay Menu stuff
    // 
    //if (_cqDx12 == nullptr && _dlssCommandList != nullptr)
    //{
    //    // trying to find command list for dlss, 
    //    // it should be native dx12 cq
    //    for (size_t i = 0; i < NumCommandLists; i++)
    //    {
    //        if (ppCommandLists[i] == _dlssCommandList)
    //        {
    //            LOG_INFO("new _cqDx12: {0:X}", (ULONG64)pCommandQueue);
    //            _cqDx12 = pCommandQueue;
    //            break;
    //        }
    //    }
    //}

    return o_ExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

static ULONG WINAPI hkCommandQueueRelease(IUnknown* This)
{
    auto result = o_CommandQueueRelease(This);

    // TODO: Continue from here to Dx12 Overlay Menu stuff

    //if (result == 0)
    //{
    //    if (This == _cqDx12)
    //        _cqDx12 = nullptr;
    //    else if (This == _cqFsr3Mod)
    //        _cqFsr3Mod = nullptr;
    //    else if (This == _cqFsr3Native)
    //        _cqFsr3Native = nullptr;
    //    else if (This == _cqSL)
    //        _cqSL = nullptr;
    //}

    return result;
}

static HRESULT WINAPI hkCreateCommandQueue(ID3D12Device* This, const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue)
{
    auto result = o_CreateCommandQueue(This, pDesc, riid, ppCommandQueue);

    if (result == S_OK && o_ExecuteCommandLists == nullptr && o_CommandQueueRelease == nullptr && pDesc->Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        auto commandQueue = (ID3D12CommandQueue*)*ppCommandQueue;

        PVOID* pVTable = *(PVOID**)commandQueue;
        o_CommandQueueRelease = (PFN_Release)pVTable[2];
        o_ExecuteCommandLists = (PFN_ExecuteCommandLists)pVTable[10];

        if (o_ExecuteCommandLists != nullptr || o_CommandQueueRelease != nullptr)
        {
            LOG_INFO("hooking Dx12 CommandQueue");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourAttach(&(PVOID&)o_ExecuteCommandLists, hkExecuteCommandLists);
            DetourAttach(&(PVOID&)o_CommandQueueRelease, hkCommandQueueRelease);

            DetourTransactionCommit();
        }

        commandQueue = nullptr;
    }

    return result;
}

static void HookToCommandList(ID3D12GraphicsCommandList* InCmdList)
{
    if (o_SetComputeRootSignature != nullptr || o_SetGraphicRootSignature != nullptr)
        return;

    PVOID* pVTable = *(PVOID**)InCmdList;
    o_SetComputeRootSignature = (PFN_SetRootSignature)pVTable[29];
    o_SetGraphicRootSignature = (PFN_SetRootSignature)pVTable[30];

    if (o_SetComputeRootSignature != nullptr || o_SetGraphicRootSignature != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_SetComputeRootSignature != nullptr)
            DetourAttach(&(PVOID&)o_SetComputeRootSignature, hkSetComputeRootSignature);

        if (o_SetGraphicRootSignature != nullptr)
            DetourAttach(&(PVOID&)o_SetGraphicRootSignature, hkSetGraphicRootSignature);

        DetourTransactionCommit();
    }
}

static void HookToDevice(ID3D12Device* InDevice)
{
    if (o_CreateSampler != nullptr || o_CreateCommandQueue != nullptr)
        return;

    PVOID* pVTable = *(PVOID**)InDevice;
    o_CreateSampler = (PFN_CreateSampler)pVTable[22];
    o_CreateCommandQueue = (PFN_CreateCommandQueue)pVTable[8];

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_CreateSampler != nullptr)
        DetourAttach(&(PVOID&)o_CreateSampler, hkCreateSampler);

    if (o_CreateCommandQueue != nullptr)
        DetourAttach(&(PVOID&)o_CreateCommandQueue, hkCreateCommandQueue);

    DetourTransactionCommit();
}

void Hooks::AttachDx12Hooks()
{
    if (o_D3D12CreateDevice != nullptr)
        return;

    o_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)DetourFindFunction("d3d12.dll", "D3D12CreateDevice");

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

    if (o_CreateCommandQueue != nullptr)
    {
        DetourDetach(&(PVOID&)o_CreateCommandQueue, hkCreateCommandQueue);
        o_CreateCommandQueue = nullptr;
    }

    if (o_CommandQueueRelease != nullptr)
    {
        DetourDetach(&(PVOID&)o_CommandQueueRelease, hkCommandQueueRelease);
        o_CommandQueueRelease = nullptr;
    }

    if (o_SetGraphicRootSignature != nullptr)
    {
        DetourDetach(&(PVOID&)o_ExecuteCommandLists, hkExecuteCommandLists);
        o_ExecuteCommandLists = nullptr;
    }

    DetourTransactionCommit();
}

void Hooks::ContextEvaluateStart()
{
    evaluateStart = GetTicks();
}

void Hooks::ContextEvaluateStop(ID3D12GraphicsCommandList* InCmdList, bool InRestore)
{
    if (InRestore && (Config::Instance()->RestoreComputeSignature.value_or(false) || Config::Instance()->RestoreGraphicSignature.value_or(false)))
    {
        sigatureMutex.lock();

        if (Config::Instance()->RestoreComputeSignature.value_or(false) && computeTime != 0 && computeTime > lastEvalTime && computeTime <= evaluateStart && rootSigCompute != nullptr)
        {
            LOG_TRACE("restore orgComputeRootSig: {0:X}", (UINT64)rootSigCompute);
            o_SetComputeRootSignature(InCmdList, rootSigCompute);
        }
        else
        {
            if (Config::Instance()->RestoreComputeSignature.value_or(false))
            {
                LOG_TRACE("orgComputeRootSig lastEvalTime: {0}", lastEvalTime);
                LOG_TRACE("orgComputeRootSig computeTime: {0}", computeTime);
                LOG_TRACE("orgComputeRootSig evaluateStart: {0}", evaluateStart);
            }
        }

        if (Config::Instance()->RestoreGraphicSignature.value_or(false) && graphTime != 0 && graphTime > lastEvalTime && graphTime <= evaluateStart && rootSigGraphic != nullptr)
        {
            LOG_TRACE("restore orgGraphicRootSig: {0:X}", (UINT64)rootSigGraphic);
            o_SetGraphicRootSignature(InCmdList, rootSigGraphic);
        }
        else
        {
            if (Config::Instance()->RestoreGraphicSignature.value_or(false))
            {
                LOG_TRACE("orgGraphicRootSig lastEvalTime: {0}", lastEvalTime);
                LOG_TRACE("orgGraphicRootSig computeTime: {0}", graphTime);
                LOG_TRACE("orgGraphicRootSig evaluateStart: {0}", evaluateStart);
            }
        }

        contextRendering = false;
        lastEvalTime = evaluateStart;

        rootSigCompute = nullptr;
        rootSigGraphic = nullptr;

        sigatureMutex.unlock();
    }
}

void Hooks::ContextRenderingStart()
{
    contextRendering = true;
}

void Hooks::ContextRenderingStop()
{
    contextRendering = false;
}