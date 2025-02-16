#pragma once

#include <pch.h>

#include <hudfix/Hudfix_Dx12.h>

// Clear heap info when ResourceDiscard is called
//#define USE_RESOURCE_DISCARD

// Use CopyResource & CopyTextureRegion to capture hudless
// It's a unstable
//#define USE_COPY_RESOURCE

// Use resource barriers before and after capture operation
#define USE_RESOURCE_BARRIRER

// Uses std::thread when processing descriptor heap operations
//#define USE_THREAD_FOR_COPY_DESCS

#define USE_ARRAY_HEAP_INFO

#ifdef USE_ARRAY_HEAP_INFO
typedef struct HeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T cpuEnd = NULL;
    SIZE_T gpuStart = NULL;
    SIZE_T gpuEnd = NULL;
    UINT numDescriptors = 0;
    UINT increment = 0;
    UINT type = 0;
    std::shared_ptr<ResourceInfo[]> info;

    HeapInfo(SIZE_T cpuStart, SIZE_T cpuEnd, SIZE_T gpuStart, SIZE_T gpuEnd, UINT numResources, UINT increment, UINT type)
        : cpuStart(cpuStart), cpuEnd(cpuEnd), gpuStart(gpuStart), gpuEnd(gpuEnd), numDescriptors(numResources), increment(increment), info(new ResourceInfo[numResources]), type(type) {}

    ResourceInfo* GetByCpuHandle(SIZE_T cpuHandle) const
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return nullptr;

        auto index = (cpuHandle - cpuStart) / increment;

        return &info[index];
    }

    ResourceInfo* GetByGpuHandle(SIZE_T gpuHandle) const
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return nullptr;

        auto index = (gpuHandle - gpuStart) / increment;

        return &info[index];
    }

    void SetByCpuHandle(SIZE_T cpuHandle, ResourceInfo setInfo) const
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return;

        auto index = (cpuHandle - cpuStart) / increment;

        info[index] = setInfo;
    }

    void SetByGpuHandle(SIZE_T gpuHandle, ResourceInfo setInfo) const
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return;

        auto index = (gpuHandle - gpuStart) / increment;

        info[index] = setInfo;
    }
} heap_info; 
#else
// Vector version for lower heap usage
typedef struct HeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T cpuEnd = NULL;
    SIZE_T gpuStart = NULL;
    SIZE_T gpuEnd = NULL;
    UINT numDescriptors = 0;
    UINT increment = 0;
    UINT type = 0;
    std::vector<ResourceInfo> info;

    HeapInfo(SIZE_T cpuStart, SIZE_T cpuEnd, SIZE_T gpuStart, SIZE_T gpuEnd, UINT numResources, UINT increment, UINT type)
        : cpuStart(cpuStart), cpuEnd(cpuEnd), gpuStart(gpuStart), gpuEnd(gpuEnd), numDescriptors(numResources), increment(increment), info(numResources), type(type) {}

    ResourceInfo* GetByCpuHandle(SIZE_T cpuHandle)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return nullptr;

        auto index = (cpuHandle - cpuStart) / increment;

        return (index < info.size()) ? &info[index] : nullptr;
    }

    ResourceInfo* GetByGpuHandle(SIZE_T gpuHandle)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return nullptr;

        auto index = (gpuHandle - gpuStart) / increment;

        return (index < info.size()) ? &info[index] : nullptr;
    }

    void SetByCpuHandle(SIZE_T cpuHandle, ResourceInfo setInfo)
    {
        if (cpuStart > cpuHandle || cpuEnd < cpuHandle)
            return;

        auto index = (cpuHandle - cpuStart) / increment;

        if (index < info.size())
        {
            //setInfo.lastUsedFrame = Util::MillisecondsNow();
            info[index] = setInfo;
        }
    }

    void SetByGpuHandle(SIZE_T gpuHandle, ResourceInfo setInfo)
    {
        if (gpuStart > gpuHandle || gpuEnd < gpuHandle)
            return;

        auto index = (gpuHandle - gpuStart) / increment;

        if (index < info.size())
        {
            //setInfo.lastUsedFrame = Util::MillisecondsNow();
            info[index] = setInfo;
        }
    }
} heap_info;
#endif

typedef struct ResourceHeapInfo
{
    SIZE_T cpuStart = NULL;
    SIZE_T gpuStart = NULL;
} resource_heap_info;

class ResTrack_Dx12 {
private:

public:
    static void HookDevice(ID3D12Device* device);
    static void ResetCaptureList();
    static void ClearPossibleHudless();
};