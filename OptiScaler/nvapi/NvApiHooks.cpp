#include "NvApiHooks.h"
#include <NvApiDriverSettings.h>

#include "State.h"
#include <Config.h>

#include <proxies/KernelBase_Proxy.h>

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

NvAPI_Status __stdcall NvApiHooks::hkNvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting)
{
    if (!o_NvAPI_DRS_GetSetting)
        return NVAPI_ERROR;

    auto result = o_NvAPI_DRS_GetSetting(hSession, hProfile, settingId, pSetting);
    if (pSetting && result == NVAPI_OK)
    {
        if (settingId == NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION_ID)
        {
            if (Config::Instance()->RenderPresetOverride.value_or_default())
                pSetting->u32CurrentValue = NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION_OFF;
            else
                State::Instance().dlssPresetsOverriddenExternally = pSetting->u32CurrentValue != NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION_OFF;
        }

        if (settingId == NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION_ID)
        {
            if (Config::Instance()->RenderPresetOverride.value_or_default())
                pSetting->u32CurrentValue = NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION_OFF;
            else
                State::Instance().dlssdPresetsOverriddenExternally = pSetting->u32CurrentValue != NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION_OFF;
        }
    }

    return result;
}

void* __stdcall NvApiHooks::hkNvAPI_QueryInterface(unsigned int InterfaceId)
{
    if (!o_NvAPI_QueryInterface)
        return nullptr;

    //LOG_DEBUG("counter: {} InterfaceId: {:X}", ++qiCounter, (uint32_t)InterfaceId);

    // Disable flip metering
    if (InterfaceId == 0xF3148C42 && Config::Instance()->DisableFlipMetering.value_or(!State::Instance().isRunningOnNvidia))
    {
        LOG_INFO("FlipMetering is disabled!");
        return nullptr;
    }

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
            o_NvAPI_GPU_GetArchInfo = reinterpret_cast<decltype(&NvAPI_GPU_GetArchInfo)>(functionPointer);
            return &hkNvAPI_GPU_GetArchInfo;
        }
        if (InterfaceId == GET_ID(NvAPI_DRS_GetSetting))
        {
            o_NvAPI_DRS_GetSetting = reinterpret_cast<decltype(&NvAPI_DRS_GetSetting)>(functionPointer);
            return &hkNvAPI_DRS_GetSetting;
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

    o_NvAPI_QueryInterface = (PFN_NvApi_QueryInterface)KernelBaseProxy::GetProcAddress_()(nvapiModule, "nvapi_QueryInterface");

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