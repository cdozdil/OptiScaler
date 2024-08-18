#include "NvApi_Hooks.h"

#include "../pch.h"
#include "../Config.h"

#include "../nvapi/nvapi.h"
#include "../detours/detours.h"

// NvAPI_GPU_GetArchInfo hooking based on Nukem's spoofing code here
// https://github.com/Nukem9/dlssg-to-fsr3/blob/89ddc8c1cce4593fb420e633a06605c3c4b9c3cf/source/wrapper_generic/nvapi.cpp#L50

enum class NV_INTERFACE : uint32_t
{
    GPU_GetArchInfo = 0xD8265D24,
    EnumPhysicalGPUs = 0xE5AC921F,
    D3D12_SetRawScgPriority = 0x5DB3048A,
};

typedef void* (*PFN_NvApi_QueryInterface)(NV_INTERFACE InterfaceId);
typedef uint32_t(*PFN_NvAPI_GPU_GetArchInfo)(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo);
typedef uint32_t(*PFN_NvAPI_EnumPhysicalGPUs)(void* GPUHandle, uint32_t* Count);

static PFN_NvApi_QueryInterface OriginalNvAPI_QueryInterface = nullptr;
static PFN_NvAPI_GPU_GetArchInfo OriginalNvAPI_GPU_GetArchInfo = nullptr;

bool isArchSupportsDLSS = false;

static void Hooks::NvApiCheckDLSSSupport()
{
    if (isArchSupportsDLSS)
        return;

    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);
    auto nvapi = sysPath / L"nvapi64.dll";

    auto sysNvApi = LoadLibrary(nvapi.c_str());
    if (sysNvApi == nullptr)
    {
        LOG_ERROR("Can't load nvapi64.dll from system path!");
        return;
    }

    auto queryInterface = (PFN_NvApi_QueryInterface)GetProcAddress(sysNvApi, "nvapi_QueryInterface");
    if(queryInterface == nullptr)
    {
        LOG_ERROR("Can't get adderess of nvapi_QueryInterface!");
        return;
    }

    auto enumGPUs = (PFN_NvAPI_EnumPhysicalGPUs)queryInterface(NV_INTERFACE::EnumPhysicalGPUs);
    auto getArch = (PFN_NvAPI_GPU_GetArchInfo)queryInterface(NV_INTERFACE::GPU_GetArchInfo);

    if (enumGPUs == nullptr || getArch == nullptr)
    {
        LOG_ERROR("Can't get adderess of EnumPhysicalGPUs ({0:X}) or GPU_GetArchInfo ({1:X})!", (UINT64)enumGPUs, (UINT64)getArch);
        return;
    }

    void* nvapiGpuHandles[8];
    uint32_t gpuCount;

    if (enumGPUs(nvapiGpuHandles, &gpuCount) == NVAPI_OK)
    {
        LOG_DEBUG("Found {0} NVIDIA GPUs", gpuCount);

        for (uint32_t i = 0; i < gpuCount; i++)
        {
            NV_GPU_ARCH_INFO archInfo{};
            archInfo.version = NV_GPU_ARCH_INFO_VER;

            auto result = getArch(nvapiGpuHandles[i], &archInfo);
            if (result == NVAPI_OK)
            {
                LOG_INFO("Found GPU {0}, Arch: 0x{1:X}", i, archInfo.architecture);

                if (archInfo.architecture >= NV_GPU_ARCHITECTURE_TU100)
                    isArchSupportsDLSS = true;
            }
            else
            {
                LOG_ERROR("GPU_GetArchInfo error: {0:X}", result);
            }
        }
    }
}

static uint32_t __stdcall HookedNvAPI_GPU_GetArchInfo(void* GPUHandle, NV_GPU_ARCH_INFO* ArchInfo)
{
    if (OriginalNvAPI_GPU_GetArchInfo)
    {
        const auto status = OriginalNvAPI_GPU_GetArchInfo(GPUHandle, ArchInfo);

        if (status == 0 && ArchInfo)
        {
            LOG_DEBUG("From api arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);

            // for 16xx cards
            if (ArchInfo->architecture == NV_GPU_ARCHITECTURE_TU100 && ArchInfo->implementation > NV_GPU_ARCH_IMPLEMENTATION_TU106)
            {
                ArchInfo->implementation = NV_GPU_ARCH_IMPLEMENTATION_TU106;
                ArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_TU106;

                LOG_INFO("Spoofed arch: {0:X} impl: {1:X} rev: {2:X}!", ArchInfo->architecture, ArchInfo->implementation, ArchInfo->revision);
            }
        }

        return status;
    }

    return 0xFFFFFFFF;
}

static void* __stdcall HookedNvAPI_QueryInterface(NV_INTERFACE InterfaceId)
{
    const auto result = OriginalNvAPI_QueryInterface(InterfaceId);

    if (result)
    {
        if (InterfaceId == NV_INTERFACE::GPU_GetArchInfo)
        {
            OriginalNvAPI_GPU_GetArchInfo = reinterpret_cast<PFN_NvAPI_GPU_GetArchInfo>(result);
            return &HookedNvAPI_GPU_GetArchInfo;
        }
    }

    return result;
}

void Hooks::AttachNVAPISpoofingHooks()
{
    Hooks::NvApiCheckDLSSSupport();

    if (OriginalNvAPI_QueryInterface != nullptr)
        return;

    LOG_DEBUG("Trying to hook NvApi");
    OriginalNvAPI_QueryInterface = reinterpret_cast<PFN_NvApi_QueryInterface>(DetourFindFunction("nvapi64.dll", "nvapi_QueryInterface"));
    LOG_DEBUG("OriginalNvAPI_QueryInterface = {0:X}", (UINT64)OriginalNvAPI_QueryInterface);

    if (OriginalNvAPI_QueryInterface != nullptr)
    {
        LOG_INFO("NvAPI_QueryInterface found, hooking!");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
        DetourTransactionCommit();
    }
}

void Hooks::DetachNVAPISpoofingHooks()
{
    if (OriginalNvAPI_QueryInterface != nullptr)
    {
        LOG_INFO("NvAPI_QueryInterface unhooking!");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)OriginalNvAPI_QueryInterface, HookedNvAPI_QueryInterface);
        DetourTransactionCommit();
    }
}