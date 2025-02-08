#pragma once
#include "NvApiTypes.h"
#include "ReflexHooks.h"
#include "fakenvapi.h"

class NvApiHooks {
public:
    inline static PFN_NvApi_QueryInterface o_NvAPI_QueryInterface = nullptr;
    inline static decltype(&NvAPI_GPU_GetArchInfo) o_NvAPI_GPU_GetArchInfo = nullptr;
    inline static UINT64 qiCounter = 0;

    static NvAPI_Status __stdcall hkNvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO* pGpuArchInfo);
    static void* __stdcall hkNvAPI_QueryInterface(unsigned int InterfaceId);
    static void Hook(HMODULE nvapiModule);
    static void Unhook();
};