#pragma once

#include <pch.h>
#include "NvApiTypes.h"
#include "ReflexHooks.h"
#include "fakenvapi.h"

class NvApiHooks {
public:
    // NvAPI_GPU_GetArchInfo hooking based on Nukem's spoofing code here
    // https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/wrapper_generic/nvapi.cpp#L50

    inline static NvApiTypes::PFN_NvApi_QueryInterface OriginalNvAPI_QueryInterface = nullptr;
    inline static NvApiTypes::PfnNvAPI_GPU_GetArchInfo OriginalNvAPI_GPU_GetArchInfo = nullptr;

    inline static uint32_t __stdcall HookedNvAPI_GPU_GetArchInfo(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo)
    {
        if (OriginalNvAPI_GPU_GetArchInfo)
        {
            const auto status = OriginalNvAPI_GPU_GetArchInfo(GPUHandle, ArchInfo);

            if (status == 0 && ArchInfo)
            {
                LOG_DEBUG("From api arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);

                // for DLSSG on pre-20xx cards
                // Needs more controlled way of spoofing, was breaking DLSS-D 
                // Leaving DLSS-G spoofing to Nukem's mod for now
                //if (Config::Instance()->DLSSGMod.value_or_default() && ArchInfo->architecture >= NV_GPU_ARCHITECTURE_GM200 && ArchInfo->architecture < NV_GPU_ARCHITECTURE_AD100) { // only GTX9xx+ supports latest reflex
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
                if (ArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && ArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106)
                {
                    ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;

                    LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);
                }
            }

            return status;
        }

        LOG_DEBUG("return 0xFFFFFFFF");
        return 0xFFFFFFFF;
    }

    inline static UINT64 qiCounter = 0;

    inline static void* __stdcall HookedNvAPI_QueryInterface(NvApiTypes::NV_INTERFACE InterfaceId)
    {
        //LOG_DEBUG("counter: {} InterfaceId: {:X}", ++qiCounter, (uint32_t)InterfaceId);

        switch (InterfaceId) 
        {
            case NvApiTypes::NV_INTERFACE::D3D_SetSleepMode:
            case NvApiTypes::NV_INTERFACE::D3D_Sleep:
            case NvApiTypes::NV_INTERFACE::D3D_GetLatency:
            case NvApiTypes::NV_INTERFACE::D3D_SetLatencyMarker:
            case NvApiTypes::NV_INTERFACE::D3D12_SetAsyncFrameMarker:
                //LOG_DEBUG("counter: {}, hookReflex()", qiCounter);
                ReflexHooks::hookReflex(OriginalNvAPI_QueryInterface);
                return ReflexHooks::getHookedReflex(InterfaceId);

            default:
                ReflexHooks::hookReflex(OriginalNvAPI_QueryInterface);
        }

        const auto functionPointer = OriginalNvAPI_QueryInterface(InterfaceId);

        if (functionPointer)
        {
            switch (InterfaceId) 
            {
                case NvApiTypes::NV_INTERFACE::GPU_GetArchInfo:
                    //LOG_DEBUG("GPU_GetArchInfo");

                    if (!State::Instance().enablerAvailable) {
                        OriginalNvAPI_GPU_GetArchInfo = static_cast<NvApiTypes::PfnNvAPI_GPU_GetArchInfo>(functionPointer);
                        return &HookedNvAPI_GPU_GetArchInfo;
                    }

                    break;
            }
        }

        //LOG_DEBUG("counter: {} functionPointer: {:X}", qiCounter, (size_t)functionPointer);

        return functionPointer;
    }

    // Requires HMODULE to make sure nvapi is loaded before calling this function
    inline static void Hook(HMODULE nvapiModule)
    {
        if (OriginalNvAPI_QueryInterface != nullptr)
            return;

        if (nvapiModule == nullptr) {
            LOG_ERROR("Hook called with a nullptr nvapi module");
            return;
        }

        LOG_DEBUG("Trying to hook NvApi");

        OriginalNvAPI_QueryInterface = (NvApiTypes::PFN_NvApi_QueryInterface)GetProcAddress(nvapiModule, "nvapi_QueryInterface");

        LOG_DEBUG("OriginalNvAPI_QueryInterface = {0:X}", (unsigned long long)OriginalNvAPI_QueryInterface);

        if (OriginalNvAPI_QueryInterface != nullptr)
        {
            LOG_INFO("NvAPI_QueryInterface found, hooking!");
            fakenvapi::Init((fakenvapi::PFN_Fake_QueryInterface&)OriginalNvAPI_QueryInterface);

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
            DetourTransactionCommit();
        }
    }

    inline static void Unhook() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (OriginalNvAPI_QueryInterface != nullptr)
        {
            DetourDetach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
            OriginalNvAPI_QueryInterface = nullptr;
        }

        DetourTransactionCommit();
    }
};