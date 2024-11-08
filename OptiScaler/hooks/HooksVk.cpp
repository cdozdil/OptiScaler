#include "HooksVk.h"

#include "../Util.h"
#include "../Config.h"

#include "../imgui/imgui_overlay_vk.h"

#include "../detours/detours.h"

typedef struct VkWin32SurfaceCreateInfoKHR {
    VkStructureType                 sType;
    const void* pNext;
    VkFlags                         flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

static bool _isInited = false;

// for menu rendering
static VkDevice _device = VK_NULL_HANDLE;
static VkInstance _instance = VK_NULL_HANDLE;
static VkPhysicalDevice _PD = VK_NULL_HANDLE;
static HWND _hwnd = nullptr;

static std::mutex _vkPresentMutex;

// hooking
typedef VkResult(*PFN_QueuePresentKHR)(VkQueue, const VkPresentInfoKHR*);
typedef VkResult(*PFN_CreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*);
typedef VkResult(*PFN_vkCreateWin32SurfaceKHR)(VkInstance, const VkWin32SurfaceCreateInfoKHR*, const VkAllocationCallbacks*, VkSurfaceKHR*);

PFN_vkCreateDevice o_vkCreateDevice = nullptr;
PFN_vkCreateInstance o_vkCreateInstance = nullptr;
PFN_vkCreateWin32SurfaceKHR o_vkCreateWin32SurfaceKHR = nullptr;
PFN_QueuePresentKHR o_QueuePresentKHR = nullptr;
PFN_CreateSwapchainKHR o_CreateSwapchainKHR = nullptr;

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
static VkResult hkvkQueuePresentKHR(VkQueue queue, VkPresentInfoKHR* pPresentInfo);
static VkResult hkvkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

static void HookDevice(VkDevice InDevice)
{
    if (o_CreateSwapchainKHR != nullptr || Config::Instance()->VulkanSkipHooks)
        return;

    LOG_FUNC();

    o_QueuePresentKHR = (PFN_QueuePresentKHR)(vkGetDeviceProcAddr(InDevice, "vkQueuePresentKHR"));
    o_CreateSwapchainKHR = (PFN_CreateSwapchainKHR)(vkGetDeviceProcAddr(InDevice, "vkCreateSwapchainKHR"));

    if (o_CreateSwapchainKHR)
    {
        LOG_DEBUG("Hooking VkDevice");

        // Hook
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)o_QueuePresentKHR, hkvkQueuePresentKHR);
        DetourAttach(&(PVOID&)o_CreateSwapchainKHR, hkvkCreateSwapchainKHR);

        DetourTransactionCommit();
    }
}

static VkResult hkvkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    LOG_FUNC();

    auto result = o_vkCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

    auto procHwnd = Util::GetProcessWindow();
    LOG_DEBUG("procHwnd: {0:X}, swapchain hwnd: {1:X}", (UINT64)procHwnd, (UINT64)pCreateInfo->hwnd);

    if (result == VK_SUCCESS && !Config::Instance()->VulkanSkipHooks) // && procHwnd == pCreateInfo->hwnd) // On linux sometimes procHwnd != pCreateInfo->hwnd
    {
        ImGuiOverlayVk::DestroyVulkanObjects(false);

        _instance = instance;
        LOG_DEBUG("_instance captured: {0:X}", (UINT64)_instance);
        _hwnd = pCreateInfo->hwnd;
        LOG_DEBUG("_hwnd captured: {0:X}", (UINT64)_hwnd);
    }

    LOG_FUNC_RESULT(result);

    return result;

}

static VkResult hkvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LOG_FUNC();

    auto result = o_vkCreateInstance(pCreateInfo, pAllocator, pInstance);

    if (result == VK_SUCCESS && !Config::Instance()->VulkanSkipHooks)
    {
        ImGuiOverlayVk::DestroyVulkanObjects(false);

        _instance = *pInstance;
        LOG_DEBUG("_instance captured: {0:X}", (UINT64)_instance);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LOG_FUNC();

    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (result == VK_SUCCESS && !Config::Instance()->VulkanSkipHooks)
    {
        ImGuiOverlayVk::DestroyVulkanObjects(false);

        _PD = physicalDevice;
        LOG_DEBUG("_PD captured: {0:X}", (UINT64)_PD);
        _device = *pDevice;
        LOG_DEBUG("_device captured: {0:X}", (UINT64)_device);
        HookDevice(_device);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkQueuePresentKHR(VkQueue queue, VkPresentInfoKHR* pPresentInfo)
{
    LOG_FUNC();

    // get upscaler time
    if (HooksVk::vkUpscaleTrig && HooksVk::queryPool != VK_NULL_HANDLE)
    {
        // Retrieve timestamps
        uint64_t timestamps[2];
        vkGetQueryPoolResults(_device, HooksVk::queryPool, 0, 2, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

        // Calculate elapsed time in milliseconds
        double elapsedTimeMs = (timestamps[1] - timestamps[0]) * HooksVk::timeStampPeriod / 1e6;

        if (elapsedTimeMs > 0.0 && elapsedTimeMs < 5000.0)
        {
            Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
            Config::Instance()->upscaleTimes.pop_front();
        }

        HooksVk::vkUpscaleTrig = false;
    }

    // render menu if needed
    if(!ImGuiOverlayVk::QueuePresent(queue, pPresentInfo))
        return VK_ERROR_OUT_OF_DATE_KHR;
    
    // original call
    Config::Instance()->VulkanCreatingSC = true;
    auto result = o_QueuePresentKHR(queue, pPresentInfo);
    Config::Instance()->VulkanCreatingSC = false;

    LOG_FUNC_RESULT(result);
    return result;
}

static VkResult hkvkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    LOG_FUNC();

    Config::Instance()->VulkanCreatingSC = true;
    auto result = o_CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    Config::Instance()->VulkanCreatingSC = false;

    if (result == VK_SUCCESS && device != VK_NULL_HANDLE && pCreateInfo != nullptr && *pSwapchain != VK_NULL_HANDLE && !Config::Instance()->VulkanSkipHooks)
    {
        Config::Instance()->ScreenWidth = pCreateInfo->imageExtent.width;
        Config::Instance()->ScreenHeight = pCreateInfo->imageExtent.height;

        LOG_DEBUG("if (result == VK_SUCCESS && device != VK_NULL_HANDLE && pCreateInfo != nullptr && pSwapchain != VK_NULL_HANDLE)");

        _device = device;
        LOG_DEBUG("_device captured: {0:X}", (UINT64)_device);

        ImGuiOverlayVk::CreateSwapchain(device, _PD, _instance, _hwnd, pCreateInfo, pAllocator, pSwapchain);
    }

    LOG_FUNC_RESULT(result);
    return result;
}

void HooksVk::HookVk()
{
    if (o_vkCreateDevice != nullptr)
        return;

    o_vkCreateDevice = (PFN_vkCreateDevice)DetourFindFunction("vulkan-1.dll", "vkCreateDevice");
    o_vkCreateInstance = (PFN_vkCreateInstance)DetourFindFunction("vulkan-1.dll", "vkCreateInstance");
    o_vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)DetourFindFunction("vulkan-1.dll", "vkCreateWin32SurfaceKHR");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_vkCreateDevice != nullptr)
        DetourAttach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);

    if (o_vkCreateInstance != nullptr)
        DetourAttach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);

    if (o_vkCreateWin32SurfaceKHR != nullptr)
        DetourAttach(&(PVOID&)o_vkCreateWin32SurfaceKHR, hkvkCreateWin32SurfaceKHR);

    DetourTransactionCommit();
}

void HooksVk::UnHookVk()
{
    if (_isInited)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (o_QueuePresentKHR != nullptr)
            DetourDetach(&(PVOID&)o_QueuePresentKHR, hkvkQueuePresentKHR);

        if (o_CreateSwapchainKHR != nullptr)
            DetourDetach(&(PVOID&)o_CreateSwapchainKHR, hkvkCreateSwapchainKHR);

        if (o_vkCreateDevice != nullptr)
            DetourDetach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);

        if (o_vkCreateInstance != nullptr)
            DetourDetach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);

        if (o_vkCreateWin32SurfaceKHR != nullptr)
            DetourDetach(&(PVOID&)o_vkCreateWin32SurfaceKHR, hkvkCreateWin32SurfaceKHR);

        DetourTransactionCommit();
    }

    _isInited = false;
}
