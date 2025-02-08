#include "NvApiHooks.h"
#include "State.h"
#include <detours/detours.h>

NvAPI_Status __stdcall NvApiHooks::hkNvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO* pGpuArchInfo)
{
    if (!o_NvAPI_GPU_GetArchInfo)
    {
        LOG_DEBUG("nullptr");
        return NVAPI_ERROR;
    }

    const auto status = o_NvAPI_GPU_GetArchInfo(hPhysicalGpu, pGpuArchInfo);

    if (status == NVAPI_OK && pGpuArchInfo)
    {
        LOG_DEBUG("Original arch: {0:X} impl: {1:X} rev: {2:X}!", pGpuArchInfo->architecture, pGpuArchInfo->implementation, pGpuArchInfo->revision);

        // for DLSS on 16xx cards
        // Can't spoof ada for DLSSG here as that breaks DLSS/DLSSD
        if (pGpuArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && pGpuArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106)
        {
            pGpuArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;
            LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", pGpuArchInfo->architecture, pGpuArchInfo->implementation, pGpuArchInfo->revision);
        }
    }

    return status;
}

void* __stdcall NvApiHooks::hkNvAPI_QueryInterface(unsigned int InterfaceId)
{
    //LOG_DEBUG("counter: {} InterfaceId: {:X}", ++qiCounter, (uint32_t)InterfaceId);

    if (InterfaceId == GET_ID(NvAPI_D3D_SetSleepMode) ||
        InterfaceId == GET_ID(NvAPI_D3D_Sleep) ||
        InterfaceId == GET_ID(NvAPI_D3D_GetLatency) ||
        InterfaceId == GET_ID(NvAPI_D3D_SetLatencyMarker) ||
        InterfaceId == GET_ID(NvAPI_D3D12_SetAsyncFrameMarker))
    {
        //LOG_DEBUG("counter: {}, hookReflex()", qiCounter);
        ReflexHooks::hookReflex(o_NvAPI_QueryInterface);
        return ReflexHooks::getHookedReflex(InterfaceId);
    }

    ReflexHooks::hookReflex(o_NvAPI_QueryInterface);

    const auto functionPointer = o_NvAPI_QueryInterface(InterfaceId);

    if (functionPointer)
    {
        if (InterfaceId == GET_ID(NvAPI_GPU_GetArchInfo) && !State::Instance().enablerAvailable)
        {
            o_NvAPI_GPU_GetArchInfo = static_cast<decltype(&NvAPI_GPU_GetArchInfo)>(functionPointer);
            return &hkNvAPI_GPU_GetArchInfo;
        }
    }

    //LOG_DEBUG("counter: {} functionPointer: {:X}", qiCounter, (size_t)functionPointer);

    return functionPointer;
}

// Requires HMODULE to make sure nvapi is loaded before calling this function
void NvApiHooks::Hook(HMODULE nvapiModule)
{
    if (o_NvAPI_QueryInterface != nullptr)
        return;

    if (nvapiModule == nullptr) {
        LOG_ERROR("Hook called with a nullptr nvapi module");
        return;
    }

    LOG_DEBUG("Trying to hook NvApi");

    o_NvAPI_QueryInterface = (PFN_NvApi_QueryInterface)GetProcAddress(nvapiModule, "nvapi_QueryInterface");

    LOG_DEBUG("OriginalNvAPI_QueryInterface = {0:X}", (unsigned long long)o_NvAPI_QueryInterface);

    if (o_NvAPI_QueryInterface != nullptr)
    {
        LOG_INFO("NvAPI_QueryInterface found, hooking!");
        fakenvapi::Init(o_NvAPI_QueryInterface);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)o_NvAPI_QueryInterface, hkNvAPI_QueryInterface);
        DetourTransactionCommit();
    }
}

void NvApiHooks::Unhook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_NvAPI_QueryInterface != nullptr)
    {
        DetourDetach(&(PVOID&)o_NvAPI_QueryInterface, hkNvAPI_QueryInterface);
        o_NvAPI_QueryInterface = nullptr;
    }

    DetourTransactionCommit();
}