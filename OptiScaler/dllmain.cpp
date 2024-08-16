#pragma once
#include "dllmain.h"
#include "resource.h"

#include "Logger.h"
#include "Util.h"
#include "NVNGX_Proxy.h"

#include "imgui/imgui_overlay_dx12.h"
#include "imgui/imgui_overlay_vk.h"

#include <vulkan/vulkan_core.h>

#pragma warning (disable : 4996)

typedef BOOL(WINAPI* PFN_FreeLibrary)(HMODULE lpLibrary);
typedef HMODULE(WINAPI* PFN_LoadLibraryA)(LPCSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
typedef HMODULE(WINAPI* PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE(WINAPI* PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef FARPROC(WINAPI* PFN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

typedef const char* (CDECL* PFN_wine_get_version)(void);

typedef struct VkDummyProps
{
    VkStructureType    sType;
    void* pNext;
} VkDummyProps;

static PFN_FreeLibrary o_FreeLibrary = nullptr;
static PFN_LoadLibraryA o_LoadLibraryA = nullptr;
static PFN_LoadLibraryW o_LoadLibraryW = nullptr;
static PFN_LoadLibraryExA o_LoadLibraryExA = nullptr;
static PFN_LoadLibraryExW o_LoadLibraryExW = nullptr;
static PFN_GetProcAddress o_GetProcAddress = nullptr;
static PFN_vkGetPhysicalDeviceProperties o_vkGetPhysicalDeviceProperties = nullptr;
static PFN_vkGetPhysicalDeviceProperties2 o_vkGetPhysicalDeviceProperties2 = nullptr;
static PFN_vkGetPhysicalDeviceProperties2KHR o_vkGetPhysicalDeviceProperties2KHR = nullptr;

static PFN_vkEnumerateInstanceExtensionProperties o_vkEnumerateInstanceExtensionProperties = nullptr;
static PFN_vkEnumerateDeviceExtensionProperties o_vkEnumerateDeviceExtensionProperties = nullptr;
static PFN_vkCreateDevice o_vkCreateDevice = nullptr;
static PFN_vkCreateInstance o_vkCreateInstance = nullptr;

static uint32_t vkEnumerateInstanceExtensionPropertiesCount = 0;
static uint32_t vkEnumerateDeviceExtensionPropertiesCount = 0;

static std::string nvngxA("nvngx.dll");
static std::string nvngxExA("nvngx");
static std::wstring nvngxW(L"nvngx.dll");
static std::wstring nvngxExW(L"nvngx");

static std::string nvapiA("nvapi64.dll");
static std::string nvapiExA("nvapi64");
static std::wstring nvapiW(L"nvapi64.dll");
static std::wstring nvapiExW(L"nvapi64");

static std::string nvngxDlssA("nvngx_dlss.dll");
static std::string nvngxDlssExA("nvngx_dlss");
static std::wstring nvngxDlssW(L"nvngx_dlss.dll");
static std::wstring nvngxDlssExW(L"nvngx_dlss");

static std::string dllNameA;
static std::string dllNameExA;
static std::wstring dllNameW;
static std::wstring dllNameExW;

static int loadCount = 0;
static bool dontCount = false;

static bool isNvngxMode = false;
static bool isWorkingWithEnabler = false;
static bool isNvngxAvailable = false;

void AttachHooks();
void DetachHooks();

static HMODULE LoadNvApi()
{
    HMODULE nvapi = nullptr;

    if (Config::Instance()->NvapiDllPath.has_value())
    {
        nvapi = o_LoadLibraryW(Config::Instance()->NvapiDllPath->c_str());

        if (nvapi != nullptr)
        {
            LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(Config::Instance()->NvapiDllPath.value()));
            return nvapi;
        }
    }

    if (nvapi == nullptr)
    {
        auto localPath = Util::DllPath().parent_path() / L"nvapi64.dll";
        nvapi = o_LoadLibraryW(localPath.wstring().c_str());

        if (nvapi != nullptr)
        {
            LOG_INFO("nvapi64.dll loaded from {0}", wstring_to_string(localPath.wstring()));
            return nvapi;
        }
    }

    if (nvapi == nullptr)
    {
        nvapi = o_LoadLibraryW(L"nvapi64.dll");

        if (nvapi != nullptr)
        {
            LOG_WARN("nvapi64.dll loaded from system!");
            return nvapi;
        }
    }

    return nullptr;
}

static HMODULE LoadNvgxDlss(std::wstring originalPath)
{
    HMODULE nvngxDlss = nullptr;

    if (Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        nvngxDlss = o_LoadLibraryW(Config::Instance()->NVNGX_DLSS_Library.value().c_str());

        if (nvngxDlss != nullptr)
        {
            LOG_INFO("nvngx_dlss.dll loaded from {0}", wstring_to_string(Config::Instance()->NVNGX_DLSS_Library.value()));
            return nvngxDlss;
        }
        else
        {
            LOG_WARN("nvngx_dlss.dll can't found at {0}", wstring_to_string(Config::Instance()->NVNGX_DLSS_Library.value()));
        }
    }

    if (nvngxDlss == nullptr)
    {
        nvngxDlss = o_LoadLibraryW(originalPath.c_str());

        if (nvngxDlss != nullptr)
        {
            LOG_INFO("nvngx_dlss.dll loaded from {0}", wstring_to_string(originalPath));
            return nvngxDlss;
        }
    }

    return nullptr;
}

#pragma region Load & nvngxDlss Library hooks

static FARPROC hkGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    if (hModule == dllModule)
        LOG_INFO("Trying to get process address of {0}", lpProcName);

    return o_GetProcAddress(hModule, lpProcName);
}

static BOOL hkFreeLibrary(HMODULE lpLibrary)
{
    if (lpLibrary == nullptr)
        return FALSE;

    if (lpLibrary == dllModule)
    {
        loadCount--;
        LOG_INFO("Call for this module loadCount: {0}", loadCount);

        if (loadCount == 0)
            return o_FreeLibrary(lpLibrary);
        else
            return TRUE;
    }

    return o_FreeLibrary(lpLibrary);
}

static HMODULE hkLoadLibraryA(LPCSTR lpLibFileName)
{
    if (lpLibFileName == nullptr)
        return NULL;

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

    size_t pos;

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif // DEBUG

    // If Opti is not loading nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos2 = lcaseLibName.rfind(wstring_to_string(exePath));
        pos = lcaseLibName.rfind(nvngxA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!isNvngxMode)
    {
        // Opti dll
        pos = lcaseLibName.rfind(dllNameA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
        {
            LOG_INFO("{0} call returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryA(lpLibFileName);
    dontCount = false;
    return result;
}

static HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName)
{
    LOG_FUNC();

    if (lpLibFileName == nullptr)
        return NULL;

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

    auto lcaseLibNameA = wstring_to_string(lcaseLibName);

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibNameA);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePathW = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePathW.size(); i++)
            exePathW[i] = std::tolower(exePathW[i]);

        auto pos2 = lcaseLibName.rfind(exePathW);
        pos = lcaseLibName.rfind(nvngxW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!isNvngxMode)
    {
        // Opti dll
        pos = lcaseLibName.rfind(dllNameW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryW(lpLibFileName);
    dontCount = false
        ;
    return result;
}

static HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    if (lpLibFileName == nullptr)
        return NULL;

    std::string libName(lpLibFileName);
    std::string lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibName);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePath = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePath.size(); i++)
            exePath[i] = std::tolower(exePath[i]);

        auto pos2 = lcaseLibName.rfind(wstring_to_string(exePath));
        pos = lcaseLibName.rfind(nvngxA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        pos = lcaseLibName.rfind(nvngxExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExA.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }

        pos = lcaseLibName.rfind(nvapiA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssExA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssExA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }

        pos = lcaseLibName.rfind(nvngxDlssA);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssA.size()))
        {
            LOG_INFO("{0} call!", libName);

            auto nvngxDlss = LoadNvgxDlss(string_to_wstring(libName));

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!isNvngxMode)
    {
        // Opti dll
        pos = lcaseLibName.rfind(dllNameA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        // Opti dll
        pos = lcaseLibName.rfind(dllNameExA);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExA.size()))
        {
            LOG_INFO("{0} call, returning this dll!", libName);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

static HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    if (lpLibFileName == nullptr)
        return NULL;

    std::wstring libName(lpLibFileName);
    std::wstring lcaseLibName(libName);

    for (size_t i = 0; i < lcaseLibName.size(); i++)
        lcaseLibName[i] = std::tolower(lcaseLibName[i]);

    auto lcaseLibNameA = wstring_to_string(lcaseLibName);

#ifdef DEBUG
    LOG_TRACE("call: {0}", lcaseLibNameA);
#endif

    size_t pos;

    // If Opti is not loading nvngx.dll
    if (!isWorkingWithEnabler && !Config::Instance()->dlssDisableHook)
    {
        // exe path
        auto exePathW = Util::ExePath().parent_path().wstring();

        for (size_t i = 0; i < exePathW.size(); i++)
            exePathW[i] = std::tolower(exePathW[i]);

        auto pos2 = lcaseLibName.rfind(exePathW);
        pos = lcaseLibName.rfind(nvngxW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        pos = lcaseLibName.rfind(nvngxExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxExW.size()) &&
            (!Config::Instance()->HookOriginalNvngxOnly.value_or(false) || pos2 == std::string::npos))
        {
            LOG_INFO("nvngx call: {0}, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    // NvApi64.dll
    if (!isWorkingWithEnabler && Config::Instance()->OverrideNvapiDll.value_or(false))
    {
        pos = lcaseLibName.rfind(nvapiExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiExW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }

        pos = lcaseLibName.rfind(nvapiW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvapiW.size()))
        {
            LOG_INFO("{0} call!", lcaseLibNameA);

            auto nvapi = LoadNvApi();

            if (nvapi != nullptr)
                return nvapi;
        }
    }

    // nvngx_dlss.dll
    if (Config::Instance()->DLSSEnabled.value_or(true) && Config::Instance()->NVNGX_DLSS_Library.has_value())
    {
        pos = lcaseLibName.rfind(nvngxDlssExW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssExW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }

        pos = lcaseLibName.rfind(nvngxDlssW);

        if (pos != std::string::npos && pos == (lcaseLibName.size() - nvngxDlssW.size()))
        {
            LOG_INFO("{0} call!", wstring_to_string(libName));

            auto nvngxDlss = LoadNvgxDlss(libName);

            if (nvngxDlss != nullptr)
                return nvngxDlss;
        }
    }

    if (!isNvngxMode)
    {
        // Opti dll
        pos = lcaseLibName.rfind(dllNameW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }

        // Opti dll
        pos = lcaseLibName.rfind(dllNameExW);
        if (pos != std::string::npos && pos == (lcaseLibName.size() - dllNameExW.size()))
        {
            LOG_INFO("{0} call, returning this dll!", lcaseLibNameA);

            if (!dontCount)
                loadCount++;

            return dllModule;
        }
    }

    dontCount = true;
    auto result = o_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    dontCount = false;

    return result;
}

#pragma endregion

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

static void DetachHooks()
{
    if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
    {
        DetourTransactionBegin();

        DetourUpdateThread(GetCurrentThread());

        if (o_FreeLibrary)
        {
            DetourDetach(&(PVOID&)o_FreeLibrary, hkFreeLibrary);
            o_FreeLibrary = nullptr;
        }

        if (o_LoadLibraryA)
        {
            DetourDetach(&(PVOID&)o_LoadLibraryA, hkLoadLibraryA);
            o_LoadLibraryA = nullptr;
        }

        if (o_LoadLibraryW)
        {
            DetourDetach(&(PVOID&)o_LoadLibraryW, hkLoadLibraryW);
            o_LoadLibraryW = nullptr;
        }

        if (o_LoadLibraryExA)
        {
            DetourDetach(&(PVOID&)o_LoadLibraryExA, hkLoadLibraryExA);
            o_LoadLibraryExA = nullptr;
        }

        if (o_LoadLibraryExW)
        {
            DetourDetach(&(PVOID&)o_LoadLibraryExW, hkLoadLibraryExW);
            o_LoadLibraryExW = nullptr;
        }

        if (o_GetProcAddress)
        {
            DetourDetach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);
            o_GetProcAddress = nullptr;
        }

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

        if (!Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
        {
            if (dxgi.CreateDxgiFactory != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory, _CreateDXGIFactory);
                dxgi.CreateDxgiFactory = nullptr;
            }

            if (dxgi.CreateDxgiFactory1 != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory1, _CreateDXGIFactory1);
                dxgi.CreateDxgiFactory1 = nullptr;
            }

            if (dxgi.CreateDxgiFactory2 != nullptr)
            {
                DetourDetach(&(PVOID&)dxgi.CreateDxgiFactory2, _CreateDXGIFactory2);
                dxgi.CreateDxgiFactory2 = nullptr;
            }
        }

        DetourTransactionCommit();

        FreeLibrary(shared.dll);
    }
}

static void AttachHooks()
{
    LOG_FUNC();

    if (o_LoadLibraryA == nullptr || o_LoadLibraryW == nullptr)
    {
        // Detour the functions
        o_FreeLibrary = reinterpret_cast<PFN_FreeLibrary>(DetourFindFunction("kernel32.dll", "FreeLibrary"));
        o_LoadLibraryA = reinterpret_cast<PFN_LoadLibraryA>(DetourFindFunction("kernel32.dll", "LoadLibraryA"));
        o_LoadLibraryW = reinterpret_cast<PFN_LoadLibraryW>(DetourFindFunction("kernel32.dll", "LoadLibraryW"));
        o_LoadLibraryExA = reinterpret_cast<PFN_LoadLibraryExA>(DetourFindFunction("kernel32.dll", "LoadLibraryExA"));
        o_LoadLibraryExW = reinterpret_cast<PFN_LoadLibraryExW>(DetourFindFunction("kernel32.dll", "LoadLibraryExW"));
#ifdef _DEBUG
        o_GetProcAddress = reinterpret_cast<PFN_GetProcAddress>(DetourFindFunction("kernel32.dll", "GetProcAddress"));
#endif // DEBUG

        if (o_LoadLibraryA != nullptr || o_LoadLibraryW != nullptr || o_LoadLibraryExA != nullptr || o_LoadLibraryExW != nullptr)
        {
            LOG_INFO("Attaching LoadLibrary hooks");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (o_FreeLibrary)
                DetourAttach(&(PVOID&)o_FreeLibrary, hkFreeLibrary);

            if (o_LoadLibraryA)
                DetourAttach(&(PVOID&)o_LoadLibraryA, hkLoadLibraryA);

            if (o_LoadLibraryW)
                DetourAttach(&(PVOID&)o_LoadLibraryW, hkLoadLibraryW);

            if (o_LoadLibraryExA)
                DetourAttach(&(PVOID&)o_LoadLibraryExA, hkLoadLibraryExA);

            if (o_LoadLibraryExW)
                DetourAttach(&(PVOID&)o_LoadLibraryExW, hkLoadLibraryExW);

            if (o_GetProcAddress)
                DetourAttach(&(PVOID&)o_GetProcAddress, hkGetProcAddress);

            DetourTransactionCommit();
        }
    }

    if (!isNvngxMode && Config::Instance()->VulkanSpoofing.value_or(false) && o_vkGetPhysicalDeviceProperties == nullptr)
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

    if (!isNvngxMode && Config::Instance()->VulkanExtensionSpoofing.value_or(false) && o_vkEnumerateInstanceExtensionProperties == nullptr)
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

static bool IsRunningOnWine()
{
    LOG_FUNC();

    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

    if (!ntdll)
    {
        spdlog::warn("IsRunningOnWine Not running on NT!?!");
        return true;
    }

    auto pWineGetVersion = (PFN_wine_get_version)GetProcAddress(ntdll, "wine_get_version");

    if (pWineGetVersion)
    {
        LOG_INFO("IsRunningOnWine Running on Wine {0}!", pWineGetVersion());
        return true;
    }

    spdlog::warn("IsRunningOnWine Wine not detected");
    return false;
}

static void CheckWorkingMode()
{
    LOG_FUNC();

    bool modeFound = false;
    std::string filename = Util::DllPath().filename().string();
    std::string lCaseFilename(filename);
    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);
    std::filesystem::path pluginPath(Config::Instance()->PluginPath.value_or((Util::DllPath().parent_path() / L"plugins").wstring()));

    for (size_t i = 0; i < lCaseFilename.size(); i++)
        lCaseFilename[i] = std::tolower(lCaseFilename[i]);

    HMODULE dll = nullptr;

    do
    {
        if (lCaseFilename == "nvngx.dll" || lCaseFilename == "_nvngx.dll" || lCaseFilename == "dlss-enabler-upscaler.dll")
        {
            LOG_INFO("OptiScaler working as native upscaler: {0}", filename);

            dllNameA = "OptiScaler_DontLoad.dll";
            dllNameExA = "OptiScaler_DontLoad";
            dllNameW = L"OptiScaler_DontLoad.dll";
            dllNameExW = L"OptiScaler_DontLoad";

            isNvngxMode = true;
            isWorkingWithEnabler = lCaseFilename == "dlss-enabler-upscaler.dll";

            if (isWorkingWithEnabler)
                Config::Instance()->LogToNGX = true;

            modeFound = true;
            break;
        }

        // version.dll
        if (lCaseFilename == "version.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"version.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"version-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, version-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"version.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as version.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                dllNameA = "version.dll";
                dllNameExA = "version";
                dllNameW = L"version.dll";
                dllNameExW = L"version";

                shared.LoadOriginalLibrary(dll);
                version.LoadOriginalLibrary(dll);

                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original version.dll!");
            }

            break;
        }

        // winmm.dll
        if (lCaseFilename == "winmm.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"winmm.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winmm-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, winmm-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winmm.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winmm.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                dllNameA = "winmm.dll";
                dllNameExA = "winmm";
                dllNameW = L"winmm.dll";
                dllNameExW = L"winmm";

                shared.LoadOriginalLibrary(dll);
                winmm.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winmm.dll!");
            }

            break;
        }

        // wininet.dll
        if (lCaseFilename == "wininet.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"wininet.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"wininet-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, wininet-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"wininet.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as wininet.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                dllNameA = "wininet.dll";
                dllNameExA = "wininet";
                dllNameW = L"wininet.dll";
                dllNameExW = L"wininet";

                shared.LoadOriginalLibrary(dll);
                wininet.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original wininet.dll!");
            }

            break;
        }

        // optiscaler.dll
        if (lCaseFilename == "optiscaler.asi")
        {
            LOG_INFO("OptiScaler working as OptiScaler.asi");

            // quick hack for testing
            dll = dllModule;

            dllNameA = "optiscaler.asi";
            dllNameExA = "optiscaler";
            dllNameW = L"optiscaler.asi";
            dllNameExW = L"optiscaler";

            modeFound = true;
            break;
        }

        // winhttp.dll
        if (lCaseFilename == "winhttp.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"winhttp.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winhttp-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, winhttp-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winhttp.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winhttp.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                dllNameA = "winhttp.dll";
                dllNameExA = "winhttp";
                dllNameW = L"winhttp.dll";
                dllNameExW = L"winhttp";

                shared.LoadOriginalLibrary(dll);
                winhttp.LoadOriginalLibrary(dll);
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winhttp.dll!");
            }

            break;
        }

        // dxgi.dll
        if (lCaseFilename == "dxgi.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"dxgi.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"dxgi-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, dxgi-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"dxgi.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as dxgi.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                dllNameA = "dxgi.dll";
                dllNameExA = "dxgi";
                dllNameW = L"dxgi.dll";
                dllNameExW = L"dxgi";

                shared.LoadOriginalLibrary(dll);
                dxgi.LoadOriginalLibrary(dll);

                Config::Instance()->IsDxgiMode = true;
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original dxgi.dll!");
            }

            break;
        }

    } while (false);

    // hook dxgi when not working as dxgi.dll
    if (!isWorkingWithEnabler && !Config::Instance()->IsDxgiMode && Config::Instance()->DxgiSpoofing.value_or(true))
    {
        LOG_INFO("DxgiSpoofing is enabled loading dxgi.dll");
        
        dxgi.CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory");
        dxgi.CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY)DetourFindFunction("dxgi.dll", "CreateDXGIFactory1");
        dxgi.CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)DetourFindFunction("dxgi.dll", "CreateDXGIFactory2");

        if (dxgi.CreateDxgiFactory != nullptr || dxgi.CreateDxgiFactory1 != nullptr || dxgi.CreateDxgiFactory2 != nullptr)
        {
            LOG_INFO("dxgi.dll found, hooking CreateDxgiFactory methods");

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (dxgi.CreateDxgiFactory != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory, _CreateDXGIFactory);

            if (dxgi.CreateDxgiFactory1 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory1, _CreateDXGIFactory1);

            if (dxgi.CreateDxgiFactory2 != nullptr)
                DetourAttach(&(PVOID&)dxgi.CreateDxgiFactory2, _CreateDXGIFactory2);

            DetourTransactionCommit();
        }
    }

    if (modeFound)
    {
        AttachHooks();

        if ((!isNvngxMode || isWorkingWithEnabler) && !Config::Instance()->DisableEarlyHooking.value_or(false) && Config::Instance()->OverlayMenu.value_or(true))
        {
            ImGuiOverlayDx12::Dx12Bind();
            ImGuiOverlayVk::HookVK();
            //ImGuiOverlayDx12::FSR3Bind();
        }

        return;
    }

    LOG_ERROR("Unsupported dll name: {0}", filename);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            if (loadCount > 1)
            {
                LOG_INFO("DLL_PROCESS_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
                return TRUE;
            }

            DisableThreadLibraryCalls(hModule);

            loadCount++;

            dllModule = hModule;
            processId = GetCurrentProcessId();

#ifdef VER_PRE_RELEASE
            // Enable file logging for pre builds
            Config::Instance()->LogToFile = true;

            // Set log level to debug
            if (Config::Instance()->LogLevel.value_or(2) > 1)
                Config::Instance()->LogLevel = 1;
#endif

            PrepareLogger();
            spdlog::warn("{0} loaded", VER_PRODUCT_NAME);
            spdlog::warn("---------------------------------");
            spdlog::warn("OptiScaler is freely downloadable from");
            spdlog::warn("GitHub : https://github.com/cdozdil/OptiScaler/releases"); 
            spdlog::warn("Nexus  : https://www.nexusmods.com/site/mods/986"); 
            spdlog::warn("If you paid for these files, you've been scammed!");
            spdlog::warn("DO NOT USE IN MULTIPLAYER GAMES");
            spdlog::warn("");
            spdlog::warn("LogLevel: {0}", Config::Instance()->LogLevel.value_or(2));

            // Check for Wine
            Config::Instance()->IsRunningOnLinux = IsRunningOnWine();

            // Check if real DLSS available
            if (Config::Instance()->DLSSEnabled.value_or(true))
            {
                spdlog::info("");
                NVNGXProxy::InitNVNGX();

                if (NVNGXProxy::NVNGXModule() == nullptr)
                {
                    spdlog::info("Can't load nvngx.dll, disabling DLSS");
                    Config::Instance()->DLSSEnabled = false;
                }
                else
                {
                    spdlog::info("nvngx.dll loaded, setting DLSS as default upscaler and disabling spoofing options set to auto");

                    Config::Instance()->DLSSEnabled = true;

                    if (!Config::Instance()->DxgiSpoofing.has_value())
                        Config::Instance()->DxgiSpoofing = false;

                    if (!Config::Instance()->VulkanSpoofing.has_value())
                        Config::Instance()->VulkanSpoofing = false;

                    if (!Config::Instance()->VulkanExtensionSpoofing.has_value())
                        Config::Instance()->VulkanExtensionSpoofing = false;

                    // Disable Overlay Menu because of crashes on Linux with Nvidia GPUs
                    if (Config::Instance()->IsRunningOnLinux && !Config::Instance()->OverlayMenu.has_value())
                        Config::Instance()->OverlayMenu = false;

                    isNvngxAvailable = true;
                }
            }

            // Check for working mode and attach hooks
            spdlog::info("");
            CheckWorkingMode();
            spdlog::info("");

            break;

        case DLL_PROCESS_DETACH:
#ifdef _DEBUG
            LOG_TRACE("DLL_PROCESS_DETACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
#endif
            spdlog::info("");
            spdlog::info("Unloading OptiScaler");
            CloseLogger();
            DetachHooks();

            break;

        case DLL_THREAD_ATTACH:
#ifdef _DEBUG
            // LOG_TRACE("DllMain DLL_THREAD_ATTACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
#endif
            break;

        case DLL_THREAD_DETACH:
#ifdef _DEBUG
            // LOG_TRACE("DLL_THREAD_DETACH from module: {0:X}, count: {1}", (UINT64)hModule, loadCount);
#endif
            break;

        default:
            LOG_WARN("Call reason: {0:X}", ul_reason_for_call);
            break;
    }

    return TRUE;
}
