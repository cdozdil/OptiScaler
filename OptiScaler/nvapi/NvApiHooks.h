#pragma once

#include <pch.h>
#include "NvApiTypes.h"
#include "ReflexHooks.h"
#include "fakenvapi.h"

class NvApiHooks {
public:
    // NvAPI_GPU_GetArchInfo hooking based on Nukem's spoofing code here
    // https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/wrapper_generic/nvapi.cpp#L50

    inline static NvApiTypes::PFN_NvApi_QueryInterface o_NvAPI_QueryInterface = nullptr;
    inline static decltype(&NvAPI_GPU_GetArchInfo) o_NvAPI_GPU_GetArchInfo = nullptr;

    inline static NvAPI_Status __stdcall hNvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO* pGpuArchInfo)
    {
        if (o_NvAPI_GPU_GetArchInfo)
        {
            const auto status = o_NvAPI_GPU_GetArchInfo(hPhysicalGpu, pGpuArchInfo);

            if (status == 0 && pGpuArchInfo)
            {
                LOG_DEBUG("From api arch: {0:X} impl: {1:X} rev: {2:X}!", pGpuArchInfo->architecture, pGpuArchInfo->implementation, pGpuArchInfo->revision);

                // for DLSSG on pre-20xx cards
                // Needs more controlled way of spoofing, was breaking DLSS-D 
                // Leaving DLSS-G spoofing to Nukem's mod for now
                //if (Config::Instance()->FGType.value_or_default() == FGType::Nukems && ArchInfo->architecture >= NV_GPU_ARCHITECTURE_GM200 && ArchInfo->architecture < NV_GPU_ARCHITECTURE_AD100) { // only GTX9xx+ supports latest reflex
                //    if (ArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && ArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106) {
                //        ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106; // let nukem's mod change the arch for those cards, breaks dlss otherwise for some reason
                //    }
                //    else {
                //        ArchInfo->architecture = NV_GPU_ARCHITECTURE_AD100;
                //        ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_AD102;
                //    }

                //    LOG_INFO("Spoofed arch for dlssg: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);
                //}

                // for DLSS on 16xx cards
                if (pGpuArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && pGpuArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106)
                {
                    pGpuArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;

                    LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", pGpuArchInfo->architecture, pGpuArchInfo->implementation, pGpuArchInfo->revision);
                }
            }

            return status;
        }

        LOG_DEBUG("return 0xFFFFFFFF");
        return NVAPI_ERROR;
    }

    inline static UINT64 qiCounter = 0;

    inline static void* __stdcall hNvAPI_QueryInterface(unsigned int InterfaceId)
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
        else
        {
            ReflexHooks::hookReflex(o_NvAPI_QueryInterface);
        }

        const auto functionPointer = o_NvAPI_QueryInterface(InterfaceId);

        if (functionPointer)
        {
            if (InterfaceId == GET_ID(NvAPI_GPU_GetArchInfo) && !State::Instance().enablerAvailable)
            {
                o_NvAPI_GPU_GetArchInfo = static_cast<decltype(&NvAPI_GPU_GetArchInfo)>(functionPointer);
                return &hNvAPI_GPU_GetArchInfo;
            }
        }

        //LOG_DEBUG("counter: {} functionPointer: {:X}", qiCounter, (size_t)functionPointer);

        return functionPointer;
    }

    // Requires HMODULE to make sure nvapi is loaded before calling this function
    inline static void Hook(HMODULE nvapiModule)
    {
        if (o_NvAPI_QueryInterface != nullptr)
            return;

        if (nvapiModule == nullptr) {
            LOG_ERROR("Hook called with a nullptr nvapi module");
            return;
        }

        LOG_DEBUG("Trying to hook NvApi");

        o_NvAPI_QueryInterface = (NvApiTypes::PFN_NvApi_QueryInterface)GetProcAddress(nvapiModule, "nvapi_QueryInterface");

        LOG_DEBUG("OriginalNvAPI_QueryInterface = {0:X}", (unsigned long long)o_NvAPI_QueryInterface);

        if (o_NvAPI_QueryInterface != nullptr)
        {
            LOG_INFO("NvAPI_QueryInterface found, hooking!");
            fakenvapi::Init((fakenvapi::PFN_Fake_QueryInterface&)o_NvAPI_QueryInterface);

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)o_NvAPI_QueryInterface, hNvAPI_QueryInterface);
            DetourTransactionCommit();
        }
    }

    inline static void Unhook() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_NvAPI_QueryInterface != nullptr)
        {
            DetourDetach(&(PVOID&)o_NvAPI_QueryInterface, hNvAPI_QueryInterface);
            o_NvAPI_QueryInterface = nullptr;
        }

        DetourTransactionCommit();
    }
};