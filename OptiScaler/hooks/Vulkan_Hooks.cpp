#include "Vulkan_Hooks.h"

#include "../pch.h"
#include "../Config.h"

#include <vulkan/vulkan_core.h>

#include "../detours/detours.h"

static PFN_vkGetPhysicalDeviceProperties o_vkGetPhysicalDeviceProperties = nullptr;
static PFN_vkGetPhysicalDeviceProperties2 o_vkGetPhysicalDeviceProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceProperties2KHR o_vkGetPhysicalDeviceProperties2KHR = nullptr;

static PFN_vkEnumerateInstanceExtensionProperties o_vkEnumerateInstanceExtensionProperties = nullptr;
static PFN_vkEnumerateDeviceExtensionProperties o_vkEnumerateDeviceExtensionProperties = nullptr;
static PFN_vkCreateDevice o_vkCreateDevice = nullptr;
static PFN_vkCreateInstance o_vkCreateInstance = nullptr;

typedef struct VkDummyProps
{
    VkStructureType    sType;
    void* pNext;
} VkDummyProps;

static uint32_t vkEnumerateInstanceExtensionPropertiesCount = 0;
static uint32_t vkEnumerateDeviceExtensionPropertiesCount = 0;

#pragma region Vulkan Hooks

static void hkvkGetPhysicalDeviceProperties(VkPhysicalDevice physical_device, VkPhysicalDeviceProperties* properties)
{
    o_vkGetPhysicalDeviceProperties(physical_device, properties);

    std::strcpy(properties->deviceName, "NVIDIA GeForce RTX 4090");
    properties->vendorID = 0x10de;
    properties->deviceID = 0x2684;
    properties->driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);
}

static void hkvkGetPhysicalDeviceProperties2(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2(phys_dev, properties2);

    std::strcpy(properties2->properties.deviceName, "NVIDIA GeForce RTX 4090");
    properties2->properties.vendorID = 0x10de;
    properties2->properties.deviceID = 0x2684;
    properties2->properties.driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);

    auto next = (VkDummyProps*)properties2->pNext;

    while (next != nullptr)
    {
        if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
        {
            auto ddp = (VkPhysicalDeviceDriverProperties*)(void*)next;
            ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
            std::strcpy(ddp->driverName, "NVIDIA");
            std::strcpy(ddp->driverInfo, "559.0");
        }

        next = (VkDummyProps*)next->pNext;
    }
}

static void hkvkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice phys_dev, VkPhysicalDeviceProperties2* properties2)
{
    o_vkGetPhysicalDeviceProperties2KHR(phys_dev, properties2);

    std::strcpy(properties2->properties.deviceName, "NVIDIA GeForce RTX 4090");
    properties2->properties.vendorID = 0x10de;
    properties2->properties.deviceID = 0x2684;
    properties2->properties.driverVersion = VK_MAKE_API_VERSION(559, 0, 0, 0);

    auto next = (VkDummyProps*)properties2->pNext;

    while (next != nullptr)
    {
        if (next->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES)
        {
            auto ddp = (VkPhysicalDeviceDriverProperties*)(void*)next;
            ddp->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
            std::strcpy(ddp->driverName, "NVIDIA");
            std::strcpy(ddp->driverInfo, "559.0");
        }

        next = (VkDummyProps*)next->pNext;
    }
}

static VkResult hkvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LOG_DEBUG("for {0}", pCreateInfo->pApplicationInfo->pApplicationName);

    LOG_DEBUG("extensions ({0}):", pCreateInfo->enabledExtensionCount);
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);

    LOG_DEBUG("layers ({0}):", pCreateInfo->enabledLayerCount);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledLayerNames[i]);

    // Skip spoofing for Intel Arc
    Config::Instance()->dxgiSkipSpoofing = true;
    auto result = o_vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    Config::Instance()->dxgiSkipSpoofing = false;

    LOG_DEBUG("o_vkCreateInstance result: {0:X}", (INT)result);

    auto head = (VkBaseInStructure*)pCreateInfo;

    while (head->pNext != nullptr)
    {
        head = (VkBaseInStructure*)head->pNext;
        LOG_DEBUG("o_vkCreateInstance type: {0:X}", (UINT)head->sType);
    }

    return result;
}

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LOG_FUNC();

    if (!Config::Instance()->VulkanExtensionSpoofing.value_or(false))
    {
        LOG_DEBUG("extension spoofing is disabled");

        Config::Instance()->dxgiSkipSpoofing = true;
        return o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        Config::Instance()->dxgiSkipSpoofing = false;
    }

    LOG_DEBUG("layers ({0}):", pCreateInfo->enabledLayerCount);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledLayerNames[i]);

    std::vector<const char*> newExtensionList;

    auto bVK_KHR_get_memory_requirements2 = false;

    LOG_DEBUG("checking extensions and removing VK_NVX_BINARY_IMPORT & VK_NVX_IMAGE_VIEW_HANDLE from list");
    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
        if (std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_BINARY_IMPORT_EXTENSION_NAME) == 0 || std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME) == 0)
        {
            LOG_DEBUG("removing {0}", pCreateInfo->ppEnabledExtensionNames[i]);
        }
        else
        {
            LOG_DEBUG("adding {0}", pCreateInfo->ppEnabledExtensionNames[i]);
            newExtensionList.push_back(pCreateInfo->ppEnabledExtensionNames[i]);

            if (std::strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
                bVK_KHR_get_memory_requirements2 = true;
        }
    }

    if (!bVK_KHR_get_memory_requirements2)
        newExtensionList.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(newExtensionList.size());
    pCreateInfo->ppEnabledExtensionNames = newExtensionList.data();

    LOG_DEBUG("final extension count: {0}", pCreateInfo->enabledExtensionCount);
    LOG_DEBUG("final extensions:");

    for (size_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
        LOG_DEBUG("  {0}", pCreateInfo->ppEnabledExtensionNames[i]);

    // Skip spoofing for Intel Arc
    Config::Instance()->dxgiSkipSpoofing = true;
    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    Config::Instance()->dxgiSkipSpoofing = false;

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;
    auto result = o_vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
        return result;

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        *pPropertyCount += 2;
        vkEnumerateDeviceExtensionPropertiesCount = *pPropertyCount;
        return result;
    }

    if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    {
        if (count == vkEnumerateDeviceExtensionPropertiesCount)
            *pPropertyCount = count;

        VkExtensionProperties bi{ VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

        VkExtensionProperties ivh{ VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
    LOG_FUNC();

    auto count = *pPropertyCount;
    auto result = o_vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);

    if (result != VK_SUCCESS)
        return result;

    if (pLayerName == nullptr && pProperties == nullptr && count == 0)
    {
        *pPropertyCount += 2;
        vkEnumerateInstanceExtensionPropertiesCount = *pPropertyCount;
        return result;
    }

    if (pLayerName == nullptr && pProperties != nullptr && *pPropertyCount > 0)
    {
        if (vkEnumerateInstanceExtensionPropertiesCount == count)
            *pPropertyCount = count;

        VkExtensionProperties bi{ VK_NVX_BINARY_IMPORT_EXTENSION_NAME, VK_NVX_BINARY_IMPORT_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 1], &bi, sizeof(VkExtensionProperties));

        VkExtensionProperties ivh{ VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, VK_NVX_IMAGE_VIEW_HANDLE_SPEC_VERSION };
        memcpy(&pProperties[*pPropertyCount - 2], &ivh, sizeof(VkExtensionProperties));
    }

    LOG_FUNC_RESULT(result);

    return result;
}

#pragma endregion

void Hooks::AttachVulkanDeviceHooks()
{
    if (o_vkGetPhysicalDeviceProperties == nullptr)
    {
        o_vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties"));
        o_vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2"));
        o_vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(DetourFindFunction("vulkan-1.dll", "vkGetPhysicalDeviceProperties2KHR"));

        if (o_vkGetPhysicalDeviceProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan device spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkGetPhysicalDeviceProperties)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties, hkvkGetPhysicalDeviceProperties);

            if (o_vkGetPhysicalDeviceProperties2)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties2, hkvkGetPhysicalDeviceProperties2);

            if (o_vkGetPhysicalDeviceProperties2KHR)
                DetourAttach(&(PVOID&)o_vkGetPhysicalDeviceProperties2KHR, hkvkGetPhysicalDeviceProperties2KHR);

            DetourTransactionCommit();
        }
    }
}

void Hooks::DetachVulkanDeviceHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_vkGetPhysicalDeviceProperties)
    {
        DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties, hkvkGetPhysicalDeviceProperties);
        o_vkGetPhysicalDeviceProperties = nullptr;
    }

    if (o_vkGetPhysicalDeviceProperties2)
    {
        DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties2, hkvkGetPhysicalDeviceProperties2);
        o_vkGetPhysicalDeviceProperties2 = nullptr;
    }

    if (o_vkGetPhysicalDeviceProperties2KHR)
    {
        DetourDetach(&(PVOID&)o_vkGetPhysicalDeviceProperties2KHR, hkvkGetPhysicalDeviceProperties2KHR);
        o_vkGetPhysicalDeviceProperties2KHR = nullptr;
    }

    DetourTransactionCommit();
}

void Hooks::AttachVulkanExtensionHooks()
{
    if (o_vkEnumerateInstanceExtensionProperties == nullptr)
    {
        o_vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(DetourFindFunction("vulkan-1.dll", "vkCreateDevice"));
        o_vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(DetourFindFunction("vulkan-1.dll", "vkCreateInstance"));
        o_vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(DetourFindFunction("vulkan-1.dll", "vkEnumerateInstanceExtensionProperties"));
        o_vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(DetourFindFunction("vulkan-1.dll", "vkEnumerateDeviceExtensionProperties"));

        if (o_vkEnumerateInstanceExtensionProperties != nullptr || o_vkEnumerateDeviceExtensionProperties != nullptr)
        {
            LOG_INFO("Attaching Vulkan extensions spoofing hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_vkEnumerateInstanceExtensionProperties)
                DetourAttach(&(PVOID&)o_vkEnumerateInstanceExtensionProperties, hkvkEnumerateInstanceExtensionProperties);

            if (o_vkEnumerateDeviceExtensionProperties)
                DetourAttach(&(PVOID&)o_vkEnumerateDeviceExtensionProperties, hkvkEnumerateDeviceExtensionProperties);

            if (o_vkCreateDevice)
                DetourAttach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);

            if (o_vkCreateInstance)
                DetourAttach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);

            DetourTransactionCommit();
        }
    }
}

void Hooks::DetachVulkanExtensionHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_vkEnumerateInstanceExtensionProperties)
    {
        DetourDetach(&(PVOID&)o_vkEnumerateInstanceExtensionProperties, hkvkEnumerateInstanceExtensionProperties);
        o_vkEnumerateInstanceExtensionProperties = nullptr;
    }

    if (o_vkEnumerateDeviceExtensionProperties)
    {
        DetourDetach(&(PVOID&)o_vkEnumerateDeviceExtensionProperties, hkvkEnumerateDeviceExtensionProperties);
        o_vkEnumerateDeviceExtensionProperties = nullptr;
    }

    if (o_vkCreateDevice)
    {
        DetourDetach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);
        o_vkCreateDevice = nullptr;
    }

    if (o_vkCreateInstance)
    {
        DetourDetach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);
        o_vkCreateInstance = nullptr;
    }

    DetourTransactionCommit();
}
