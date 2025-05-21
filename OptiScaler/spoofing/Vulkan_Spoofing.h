#pragma once

#include <pch.h>

#include <Config.h>

#include <proxies/KernelBase_Proxy.h>

#include <detours/detours.h>

#include <vulkan/vulkan_core.h>

typedef struct VkDummyProps
{
    VkStructureType sType;
    void* pNext;
} VkDummyProps;

static PFN_vkCreateDevice o_vkCreateDevice = nullptr;
static PFN_vkCreateInstance o_vkCreateInstance = nullptr;
static PFN_vkGetPhysicalDeviceProperties o_vkGetPhysicalDeviceProperties = nullptr;
static PFN_vkGetPhysicalDeviceProperties2 o_vkGetPhysicalDeviceProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceProperties2KHR o_vkGetPhysicalDeviceProperties2KHR = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties o_vkGetPhysicalDeviceMemoryProperties = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties2 o_vkGetPhysicalDeviceMemoryProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceMemoryProperties2KHR o_vkGetPhysicalDeviceMemoryProperties2KHR = nullptr;
static PFN_vkEnumerateDeviceExtensionProperties o_vkEnumerateDeviceExtensionProperties = nullptr;
static PFN_vkEnumerateInstanceExtensionProperties o_vkEnumerateInstanceExtensionProperties = nullptr;

static uint32_t vkEnumerateInstanceExtensionPropertiesCount = 0;
static uint32_t vkEnumerateDeviceExtensionPropertiesCount = 0;

inline static void hkvkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                         VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t) Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryHeaps[i].size = newMemSize;
        }
    }
}

inline static void hkvkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr ||
        pMemoryProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryProperties.memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t) Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryProperties.memoryHeaps[i].size = newMemSize;
        }
    }
}

inline static void hkvkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    o_vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

    if (pMemoryProperties == nullptr ||
        pMemoryProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR)
        return;

    for (size_t i = 0; i < pMemoryProperties->memoryProperties.memoryHeapCount; i++)
    {
        if (pMemoryProperties->memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            uint64_t newMemSize = (uint64_t) Config::Instance()->VulkanVRAM.value() * 1024 * 1024 * 1024;
            pMemoryProperties->memoryProperties.memoryHeaps[i].size = newMemSize;
        }
    }
}

inline static void hkvkGetPhysicalDeviceProperties(VkPhysicalDevice physical_device,
                                                   VkPhysicalDeviceProperties* properties)
{
    o_vkGetPhysicalDeviceProperties(physical_device, properties);

    if (properties->vendorID != 0x1414 &&
        !State::Instance().adapterDescs.contains(properties->vendorID | properties->deviceID))
    {
        std::string szName(properties->deviceName);
        std::string descStr = std::format("Adapter: {}, VendorId: {:#x}, DeviceId: {:#x}", szName, properties->vendorID,
                                          properties->deviceID);
        LOG_INFO("{}", descStr);
        State::Instance().adapterDescs.insert_or_assign(properties->vendorID | properties->deviceID, descStr);
    }

    if (!State::Instance().skipSpoofing &&
        (!Config::Instance()->TargetVendorId.has_value() ||
         Config::Instance()->TargetVendorId.value() == properties->vendorID) &&
        (!Config::Instance()->TargetDeviceId.has_value() ||
         Config::Instance()->TargetDeviceId.value() == properties->deviceID))
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or_default());
        std::strcpy(properties->deviceName, deviceName.c_str());

        properties->vendorID = Config::Instance()->SpoofedVendorId.value_or_default();
        properties->deviceID = Config::Instance()->SpoofedDeviceId.value_or_default();
        properties->driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

inline static void hkvkGetPhysicalDeviceProperties2(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2(phys_dev, properties2);

    if (properties2->properties.vendorID != 0x1414 &&
        !State::Instance().adapterDescs.contains(properties2->properties.vendorID | properties2->properties.deviceID))
    {
        std::string szName(properties2->properties.deviceName);
        std::string descStr = std::format("Adapter: {}, VendorId: {:#x}, DeviceId: {:#x}", szName,
                                          properties2->properties.vendorID, properties2->properties.deviceID);
        LOG_INFO("{}", descStr);
        State::Instance().adapterDescs.insert_or_assign(
            properties2->properties.vendorID | properties2->properties.deviceID, descStr);
    }

    if (!State::Instance().skipSpoofing &&
        (!Config::Instance()->TargetVendorId.has_value() ||
         Config::Instance()->TargetVendorId.value() == properties2->properties.vendorID) &&
        (!Config::Instance()->TargetDeviceId.has_value() ||
         Config::Instance()->TargetDeviceId.value() == properties2->properties.deviceID))
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or_default());
        std::strcpy(properties2->properties.deviceName, deviceName.c_str());
        properties2->properties.vendorID = Config::Instance()->SpoofedVendorId.value_or_default();
        properties2->properties.deviceID = Config::Instance()->SpoofedDeviceId.value_or_default();
        properties2->properties.driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);

        if (Config::Instance()->SpoofedVendorId.value_or_default() == 0x10de)
        {
            auto next = (VkDummyProps*) properties2->pNext;

            while (next != nullptr)
            {
                if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
                {
                    auto ddp = (VkPhysicalDeviceDriverProperties*) (void*) next;
                    ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    std::strcpy(ddp->driverName, "NVIDIA");
                    std::strcpy(ddp->driverInfo, "999.99");
                }

                next = (VkDummyProps*) next->pNext;
            }
        }
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

inline static void hkvkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice phys_dev,
                                                       VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2KHR(phys_dev, properties2);

    if (properties2->properties.vendorID != 0x1414 &&
        !State::Instance().adapterDescs.contains(properties2->properties.vendorID | properties2->properties.deviceID))
    {
        std::string szName(properties2->properties.deviceName);
        std::string descStr = std::format("Adapter: {}, VendorId: {:#x}, DeviceId: {:#x}", szName,
                                          properties2->properties.vendorID, properties2->properties.deviceID);
        LOG_INFO("{}", descStr);
        State::Instance().adapterDescs.insert_or_assign(
            properties2->properties.vendorID | properties2->properties.deviceID, descStr);
    }

    if (!State::Instance().skipSpoofing &&
        (!Config::Instance()->TargetVendorId.has_value() ||
         Config::Instance()->TargetVendorId.value() == properties2->properties.vendorID) &&
        (!Config::Instance()->TargetDeviceId.has_value() ||
         Config::Instance()->TargetDeviceId.value() == properties2->properties.deviceID))
    {
        auto deviceName = wstring_to_string(Config::Instance()->SpoofedGPUName.value_or_default());
        std::strcpy(properties2->properties.deviceName, deviceName.c_str());
        properties2->properties.vendorID = Config::Instance()->SpoofedVendorId.value_or_default();
        properties2->properties.deviceID = Config::Instance()->SpoofedDeviceId.value_or_default();
        properties2->properties.driverVersion = VK_MAKE_API_VERSION(999, 99, 0, 0);

        if (Config::Instance()->SpoofedVendorId.value_or_default() == 0x10de)
        {
            auto next = (VkDummyProps*) properties2->pNext;

            while (next != nullptr)
            {
                if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
                {
                    auto ddp = (VkPhysicalDeviceDriverProperties*) (void*) next;
                    ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    std::strcpy(ddp->driverName, "NVIDIA");
                    std::strcpy(ddp->driverInfo, "999.99");
                }

                next = (VkDummyProps*) next->pNext;
            }
        }
    }
    else
    {
        LOG_DEBUG("Skipping spoofing");
    }
}

inline static VkResult hkvkCreateInstance(VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                          VkInstance* pInstance)
{
    if (pCreateInfo->pApplicationInfo->pApplicationName != nullptr)
        LOG_DEBUG("for {0}", pCreateInfo->pApplicationInfo->pApplicationName);

    std::vector<const char*> newExtensionList;

    LOG_DEBUG("extensions ({0}):", pCreateInfo->enabledExtensionCount);
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);
        newExtensionList.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
    }

    if (State::Instance().isRunningOnNvidia)
    {
        LOG_INFO("Adding NVNGX Vulkan extensions");
        newExtensionList.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        newExtensionList.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        newExtensionList.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    }

    LOG_INFO("Adding FFX Vulkan extensions");
    newExtensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    LOG_DEBUG("layers ({0}):", pCreateInfo->enabledLayerCount);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledLayerNames[i]);

    pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(newExtensionList.size());
    pCreateInfo->ppEnabledExtensionNames = newExtensionList.data();

    // Skip spoofing for Intel Arc
    State::Instance().skipSpoofing = true;
    auto result = o_vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    State::Instance().skipSpoofing = false;

    LOG_DEBUG("o_vkCreateInstance result: {0:X}", (INT) result);

    if (result == VK_SUCCESS)
        State::Instance().VulkanInstance = *pInstance;

    auto head = (VkBaseInStructure*) pCreateInfo;
    while (head->pNext != nullptr)
    {
        head = (VkBaseInStructure*) head->pNext;
        LOG_DEBUG("o_vkCreateInstance type: {0:X}", (UINT) head->sType);
    }

    return result;
}

inline static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LOG_FUNC();

    std::vector<const char*> newExtensionList;

    LOG_DEBUG("Checking extensions and removing VK_NVX_BINARY_IMPORT & VK_NVX_IMAGE_VIEW_HANDLE from list");
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        if (Config::Instance()->VulkanExtensionSpoofing.value_or_default() && !State::Instance().isRunningOnNvidia &&
            (std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_BINARY_IMPORT_EXTENSION_NAME) == 0 ||
             std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME) == 0 ||
             std::strcmp(pCreateInfo->ppEnabledExtensionNames[i],
                         VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME) == 0 ||
             std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NV_LOW_LATENCY_EXTENSION_NAME) == 0))
        {
            LOG_DEBUG("removing {0}", pCreateInfo->ppEnabledExtensionNames[i]);
        }
        else
        {
            LOG_DEBUG("adding {0}", pCreateInfo->ppEnabledExtensionNames[i]);
            newExtensionList.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    if (State::Instance().isRunningOnNvidia)
    {
        LOG_INFO("Adding NVNGX Vulkan extensions");
        newExtensionList.push_back(VK_NVX_BINARY_IMPORT_EXTENSION_NAME);
        newExtensionList.push_back(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME);
        newExtensionList.push_back(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        newExtensionList.push_back(VK_NV_LOW_LATENCY_EXTENSION_NAME);
        newExtensionList.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        newExtensionList.push_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    }

    LOG_INFO("Adding FFX Vulkan extensions");
    newExtensionList.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    LOG_INFO("Adding XeSS Vulkan extensions");
    newExtensionList.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    newExtensionList.push_back(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME);
    newExtensionList.push_back(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);

    pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(newExtensionList.size());
    pCreateInfo->ppEnabledExtensionNames = newExtensionList.data();

    LOG_DEBUG("final extension count: {0}", pCreateInfo->enabledExtensionCount);
    LOG_DEBUG("final extensions:");

    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);

    // Skip spoofing for Intel Arc
    State::Instance().skipSpoofing = true;
    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    State::Instance().skipSpoofing = false;

    LOG_FUNC_RESULT(result);

    return result;
}

inline static VkResult hkvkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                              uint32_t* pPropertyCount,
                                                              VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;

    if (pProperties == nullptr)
        count = 0;

    auto result = o_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("o_vkEnumerateDeviceExtensionProperties({0}, {1}) result: {2:X}", pLayerName, count, (UINT) result);
        return result;
    }

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        *pPropertyCount += 4;
        vkEnumerateDeviceExtensionPropertiesCount = *pPropertyCount;
        LOG_TRACE("hkvkEnumerateDeviceExtensionProperties({0}) count: {1}", pLayerName,
                  vkEnumerateDeviceExtensionPropertiesCount);
        return result;
    }

    if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    {
        if (count == vkEnumerateDeviceExtensionPropertiesCount)
            *pPropertyCount = count;

        VkExtensionProperties bi{VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION};
        memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

        VkExtensionProperties ivh{VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION};
        memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));

        VkExtensionProperties mpva{VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME,
                                   VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_SPEC_VERSION};
        memcpy(&pProperties[*pPropertyCount - 3], &mpva, sizeof(VkExtensionProperties));

        VkExtensionProperties ll{VK_NV_LOW_LATENCY_EXTENSION_NAME, VK_NV_LOW_LATENCY_SPEC_VERSION};
        memcpy(&pProperties[*pPropertyCount - 4], &ll, sizeof(VkExtensionProperties));

        LOG_DEBUG("Extensions returned:");
        for (size_t i = 0; i < *pPropertyCount; i++)
        {
            LOG_DEBUG("  {}", pProperties[i].extensionName);
        }
    }

    LOG_FUNC_RESULT(result);

    return result;
}

inline static VkResult hkvkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;

    if (pProperties == nullptr)
        count = 0;

    auto result = o_vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("o_vkEnumerateInstanceExtensionProperties({0}, {1}) result: {2:X}", pLayerName, count, (UINT) result);
        return result;
    }

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        //*pPropertyCount += 2;
        // vkEnumerateInstanceExtensionPropertiesCount = *pPropertyCount;
        LOG_TRACE("hkvkEnumerateDeviceExtensionProperties({0}) count: {1}", pLayerName,
                  vkEnumerateDeviceExtensionPropertiesCount);
        return result;
    }

    // if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    //{
    //     if (vkEnumerateInstanceExtensionPropertiesCount == count)
    //         *pPropertyCount = count;

    //    VkExtensionProperties bi{ VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION };
    //    memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

    //    VkExtensionProperties ivh{ VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION };
    //    memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));

    //    LOG_DEBUG("Extensions returned:");
    //    for (size_t i = 0; i < *pPropertyCount; i++)
    //    {
    //        LOG_DEBUG("  {}", pProperties[i].extensionName);
    //    }
    //}

    LOG_DEBUG("Extensions returned:");
    for (size_t i = 0; i < *pPropertyCount; i++)
    {
        LOG_DEBUG("  {}", pProperties[i].extensionName);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

inline void HookForVulkanSpoofing(HMODULE vulkanModule)
{
    if (!State::Instance().isWorkingAsNvngx && Config::Instance()->VulkanSpoofing.value_or_default() &&
        o_vkGetPhysicalDeviceProperties == nullptr)
    {
        o_vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceProperties"));
        o_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceProperties2"));
        o_vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceProperties2KHR"));

        if (o_vkGetPhysicalDeviceProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan device spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkGetPhysicalDeviceProperties)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceProperties, hkvkGetPhysicalDeviceProperties);

            if (o_vkGetPhysicalDeviceProperties2)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceProperties2, hkvkGetPhysicalDeviceProperties2);

            if (o_vkGetPhysicalDeviceProperties2KHR)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceProperties2KHR, hkvkGetPhysicalDeviceProperties2KHR);

            DetourTransactionCommit();
        }
    }
}

inline void HookForVulkanExtensionSpoofing(HMODULE vulkanModule)
{
    if (!State::Instance().isWorkingAsNvngx && Config::Instance()->VulkanExtensionSpoofing.value_or_default() &&
        o_vkEnumerateInstanceExtensionProperties == nullptr)
    {
        o_vkCreateDevice =
            reinterpret_cast<PFN_vkCreateDevice>(KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkCreateDevice"));
        o_vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkCreateInstance"));
        o_vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkEnumerateInstanceExtensionProperties"));
        o_vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkEnumerateDeviceExtensionProperties"));

        if (o_vkEnumerateInstanceExtensionProperties != nullptr || o_vkEnumerateDeviceExtensionProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan extensions spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkEnumerateInstanceExtensionProperties)
                DetourAttach(&(PVOID&) o_vkEnumerateInstanceExtensionProperties,
                             hkvkEnumerateInstanceExtensionProperties);

            if (o_vkEnumerateDeviceExtensionProperties)
                DetourAttach(&(PVOID&) o_vkEnumerateDeviceExtensionProperties, hkvkEnumerateDeviceExtensionProperties);

            if (o_vkCreateDevice)
                DetourAttach(&(PVOID&) o_vkCreateDevice, hkvkCreateDevice);

            if (o_vkCreateInstance)
                DetourAttach(&(PVOID&) o_vkCreateInstance, hkvkCreateInstance);

            DetourTransactionCommit();
        }
    }
}

inline void HookForVulkanVRAMSpoofing(HMODULE vulkanModule)
{
    if (!State::Instance().isWorkingAsNvngx && Config::Instance()->VulkanVRAM.has_value() &&
        o_vkGetPhysicalDeviceMemoryProperties == nullptr)
    {
        o_vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceMemoryProperties"));
        o_vkGetPhysicalDeviceMemoryProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceMemoryProperties2"));
        o_vkGetPhysicalDeviceMemoryProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(
            KernelBaseProxy::GetProcAddress_()(vulkanModule, "vkGetPhysicalDeviceMemoryProperties2KHR"));

        if (o_vkGetPhysicalDeviceMemoryProperties != nullptr || o_vkGetPhysicalDeviceMemoryProperties2 != nullptr ||
            o_vkGetPhysicalDeviceMemoryProperties2KHR != nullptr)
        {
            LOG_INFO("Attaching Vulkan VRAM spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkGetPhysicalDeviceMemoryProperties)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceMemoryProperties, hkvkGetPhysicalDeviceMemoryProperties);

            if (o_vkGetPhysicalDeviceMemoryProperties2)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceMemoryProperties2, hkvkGetPhysicalDeviceMemoryProperties2);

            if (o_vkGetPhysicalDeviceMemoryProperties2KHR)
                DetourAttach(&(PVOID&) o_vkGetPhysicalDeviceMemoryProperties2KHR,
                             hkvkGetPhysicalDeviceMemoryProperties2KHR);

            DetourTransactionCommit();
        }
    }
}
