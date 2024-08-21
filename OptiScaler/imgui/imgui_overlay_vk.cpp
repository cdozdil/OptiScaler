#include "imgui_overlay_base.h"
#include "imgui_overlay_vk.h"

#include "../Util.h"

#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"
#include "../Config.h"

// Vulkan overlay code adopted from here:
// https://gist.github.com/mem99/0ec31ca302927457f86b1d6756aaa8c4
// Need to check resize & recreate fixes

static bool _isInited = false;

static VkDevice _device = VK_NULL_HANDLE;
static VkInstance _instance = VK_NULL_HANDLE;
static VkPhysicalDevice _PD = VK_NULL_HANDLE;
static HWND _hwnd;

static bool g_bInitialized = false;
static bool g_bEnabled = false;
inline static std::mutex _vkCleanMutex;

// vk hook stuff
static VkRenderPass g_vkRenderPass = VK_NULL_HANDLE;
struct ImGui_ImplVulkan_InitInfo g_ImVulkan_Info = { };
struct ImGui_ImplVulkanH_Frame* g_ImVulkan_Frames = NULL;
static VkSemaphore* g_ImVulkan_Semaphores = NULL;
static VkSwapchainCreateInfoKHR lastSwapchainInfo = {};

typedef VkResult(__fastcall* PFN_QueuePresentKHR)(VkQueue, const VkPresentInfoKHR*);
typedef VkResult(__fastcall* PFN_CreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*);

PFN_vkCreateDevice o_vkCreateDevice = nullptr;
PFN_vkCreateInstance o_vkCreateInstance = nullptr;
PFN_vkCreateDevice o_SL_vkCreateDevice = nullptr;
PFN_QueuePresentKHR oQueuePresentKHR = nullptr;
PFN_CreateSwapchainKHR oCreateSwapchainKHR = nullptr;

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
static VkResult VKAPI_CALL hkQueuePresentKHR(VkQueue queue, VkPresentInfoKHR* pPresentInfo);
static VkResult VKAPI_CALL hkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

static int createCounter = 0;

static void DestroyVulkanObjects(bool shutdown)
{
    vkDeviceWaitIdle(_device);

    if (shutdown)
    {
        if (g_vkRenderPass)
            vkDestroyRenderPass(g_ImVulkan_Info.Device, g_vkRenderPass, NULL);

        if (g_ImVulkan_Info.DescriptorPool)
            vkDestroyDescriptorPool(g_ImVulkan_Info.Device, g_ImVulkan_Info.DescriptorPool, NULL);
    }

    for (uint32_t i = 0; i < g_ImVulkan_Info.ImageCount; i++)
    {
        ImGui_ImplVulkanH_Frame* fd = &g_ImVulkan_Frames[i];

        vkDestroyFence(g_ImVulkan_Info.Device, fd->Fence, NULL);
        fd->Fence = NULL;

        vkFreeCommandBuffers(g_ImVulkan_Info.Device, fd->CommandPool, 1, &fd->CommandBuffer);
        fd->CommandBuffer = NULL;

        vkDestroyCommandPool(g_ImVulkan_Info.Device, fd->CommandPool, NULL);
        fd->CommandPool = NULL;

        vkDestroyImageView(g_ImVulkan_Info.Device, fd->BackbufferView, NULL);
        fd->BackbufferView = NULL;

        vkDestroyFramebuffer(g_ImVulkan_Info.Device, fd->Framebuffer, NULL);
        fd->Framebuffer = NULL;

        vkDestroySemaphore(g_ImVulkan_Info.Device, g_ImVulkan_Semaphores[i], NULL);
        g_ImVulkan_Semaphores[i] = NULL;
    }
}

static void CreateVulkanObjects(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain)
{
    LOG_FUNC();

    if (g_bInitialized)
    {
        LOG_DEBUG("g_bInitialized, releaseing objects");

        if (ImGui::GetIO().BackendRendererUserData != nullptr)
            ImGui_ImplVulkan_Shutdown();

        DestroyVulkanObjects(false);

        g_bInitialized = false;
    }

    // Initialize ImGui for IO.
    if (!ImGuiOverlayBase::IsInited())
    {
        LOG_DEBUG("ImGuiOverlayBase::Init");
        ImGuiOverlayBase::Init(_hwnd);
    }

    ImGuiIO& io = ImGui::GetIO();

    if (pCreateInfo != nullptr)
    {
        io.DisplaySize.x = pCreateInfo->imageExtent.width;
        io.DisplaySize.y = pCreateInfo->imageExtent.height;
    }
    else
    {
        io.DisplaySize.x = Config::Instance()->CurrentFeature->DisplayWidth();
        io.DisplaySize.y = Config::Instance()->CurrentFeature->DisplayHeight();
    }

    // Get swapchain image count.
    uint32_t imageCount;
    VkImage images[8];
    vkGetSwapchainImagesKHR(device, *pSwapchain, &imageCount, NULL);
    vkGetSwapchainImagesKHR(device, *pSwapchain, &imageCount, images);

    // Alloc ImGui frame structure/semaphores for every image.
    // For convenience, I am using ImGui_ImplVulkanH_Frame in imgui_impl_vulkan.h
    if (!g_bInitialized)
    {
        g_ImVulkan_Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * imageCount);
        g_ImVulkan_Semaphores = (VkSemaphore*)IM_ALLOC(sizeof(VkSemaphore) * imageCount);
    }

    // Select queue family.
    uint32_t queueFamily = 0;
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(_PD, &count, NULL);
        VkQueueFamilyProperties queues[8];
        vkGetPhysicalDeviceQueueFamilyProperties(_PD, &count, queues);

        for (uint32_t i = 0; i < count; i++)
        {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueFamily = i;
                break;
            }
        }
    }

    // Get device queue
    VkQueue queue;
    vkGetDeviceQueue(device, queueFamily, 0, &queue);

    // Create our vulkan objects.
    // Some code here is copied from imgui_impl_vulkan.cpp

    // Create the render pool
    VkDescriptorPool pool = VK_NULL_HANDLE;

    if (g_bInitialized == false)
    {
        VkDescriptorPoolSize sampler_pool_size = { };
        sampler_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_pool_size.descriptorCount = 1;
        VkDescriptorPoolCreateInfo desc_pool_info = { };
        desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_info.maxSets = 1;
        desc_pool_info.poolSizeCount = 1;
        desc_pool_info.pPoolSizes = &sampler_pool_size;
        vkCreateDescriptorPool(device, &desc_pool_info, NULL, &pool);
    }

    // Create the render pass
    if (g_bInitialized == false)
    {
        VkAttachmentDescription attachment_desc = { };

        if (pCreateInfo != nullptr)
            attachment_desc.format = pCreateInfo->imageFormat;
        else
            attachment_desc.format = VK_FORMAT_R8G8B8A8_UNORM;

        attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment = { };
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = { };
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;

        VkSubpassDependency dependency = { };
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = { };
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &attachment_desc;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        vkCreateRenderPass(device, &render_pass_info, NULL, &g_vkRenderPass);
    }

    // Create The Image Views
    {

        VkImageViewCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;

        if (pCreateInfo != nullptr)
            info.format = pCreateInfo->imageFormat;
        else
            info.format = VK_FORMAT_R8G8B8A8_UNORM;

        info.components.r = VK_COMPONENT_SWIZZLE_R;
        info.components.g = VK_COMPONENT_SWIZZLE_G;
        info.components.b = VK_COMPONENT_SWIZZLE_B;
        info.components.a = VK_COMPONENT_SWIZZLE_A;

        VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        info.subresourceRange = image_range;

        for (uint32_t i = 0; i < imageCount; i++)
        {
            ImGui_ImplVulkanH_Frame* fd = &g_ImVulkan_Frames[i];
            fd->Backbuffer = images[i];
            info.image = fd->Backbuffer;

            vkCreateImageView(device, &info, NULL, &fd->BackbufferView);
        }
    }

    // Create frame Buffer
    {
        VkImageView attachment[1];
        VkFramebufferCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = g_vkRenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;

        if (pCreateInfo != nullptr)
        {
            info.width = pCreateInfo->imageExtent.width;
            info.height = pCreateInfo->imageExtent.height;
        }
        else
        {
            info.width = Config::Instance()->CurrentFeature->DisplayWidth();
            info.height = Config::Instance()->CurrentFeature->DisplayHeight();
        }

        info.layers = 1;

        for (uint32_t i = 0; i < imageCount; i++)
        {
            ImGui_ImplVulkanH_Frame* fd = &g_ImVulkan_Frames[i];
            attachment[0] = fd->BackbufferView;
            vkCreateFramebuffer(device, &info, NULL, &fd->Framebuffer);
        }
    }

    // Create command pools, command buffers, fences, and semaphores for every image
    for (uint32_t i = 0; i < imageCount; i++)
    {
        ImGui_ImplVulkanH_Frame* fd = &g_ImVulkan_Frames[i];
        VkSemaphore* fsd = &g_ImVulkan_Semaphores[i];
        {
            VkCommandPoolCreateInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = queueFamily;
            vkCreateCommandPool(device, &info, NULL, &fd->CommandPool);
        }
        {
            VkCommandBufferAllocateInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->CommandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
        }
        {
            VkFenceCreateInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(device, &info, NULL, &fd->Fence);
        }
        {
            VkSemaphoreCreateInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(device, &info, NULL, fsd);
        }
    }

    // Initialize ImGui and upload fonts
    if (!ImGui::GetIO().BackendRendererUserData)
    {
        g_ImVulkan_Info.Instance = _instance;
        g_ImVulkan_Info.PhysicalDevice = _PD;
        g_ImVulkan_Info.Device = device;
        g_ImVulkan_Info.QueueFamily = queueFamily;
        g_ImVulkan_Info.Queue = queue;
        g_ImVulkan_Info.DescriptorPool = pool;
        g_ImVulkan_Info.Subpass = 0;

        if (pCreateInfo != nullptr)
            g_ImVulkan_Info.MinImageCount = pCreateInfo->minImageCount;
        else
            g_ImVulkan_Info.MinImageCount = imageCount;

        g_ImVulkan_Info.ImageCount = imageCount;
        g_ImVulkan_Info.Allocator = NULL;
        g_ImVulkan_Info.RenderPass = g_vkRenderPass;

        ImGui_ImplVulkan_Init(&g_ImVulkan_Info);

        // Upload Fonts
        // Use any command queue
        VkCommandPool command_pool = g_ImVulkan_Frames[0].CommandPool;
        VkCommandBuffer command_buffer = g_ImVulkan_Frames[0].CommandBuffer;
        vkResetCommandPool(device, command_pool, 0);

        VkCommandBufferBeginInfo begin_info = { };
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture();

        VkSubmitInfo end_info = { };
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        vkEndCommandBuffer(command_buffer);

        vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(device);
    }

    g_bInitialized = true;
    g_bEnabled = true;

    LOG_FUNC_RESULT(true);
}

static void HookDevice(VkDevice InDevice)
{
    if (oCreateSwapchainKHR != nullptr)
        return;

    LOG_FUNC();

    oQueuePresentKHR = (PFN_QueuePresentKHR)(vkGetDeviceProcAddr(InDevice, "vkQueuePresentKHR"));
    oCreateSwapchainKHR = (PFN_CreateSwapchainKHR)(vkGetDeviceProcAddr(InDevice, "vkCreateSwapchainKHR"));

    if (oCreateSwapchainKHR)
    {
        LOG_DEBUG("Hooking VkDevice");

        // Hook
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)oQueuePresentKHR, hkQueuePresentKHR);
        DetourAttach(&(PVOID&)oCreateSwapchainKHR, hkCreateSwapchainKHR);

        DetourTransactionCommit();
    }
}

static VkResult hkvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LOG_FUNC();

    auto result = o_vkCreateInstance(pCreateInfo, pAllocator, pInstance);

    if (result == VK_SUCCESS)
    {
        LOG_DEBUG("Assigning instance");
        _instance = *pInstance;
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult hkvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LOG_FUNC();

    auto result = o_vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (result == VK_SUCCESS)
    {
        LOG_DEBUG("Assigning PD and Device, hooking device");
        _PD = physicalDevice;
        _device = *pDevice;
        HookDevice(_device);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

static VkResult VKAPI_CALL hkQueuePresentKHR(VkQueue queue, VkPresentInfoKHR* pPresentInfo)
{
    LOG_FUNC();
    auto e_result = oQueuePresentKHR(queue, pPresentInfo);
    return e_result;

    if (!g_bInitialized)
    {
        LOG_DEBUG("!g_bInitialized");

        if (pPresentInfo->swapchainCount > 0)
        {
            LOG_DEBUG("CreateVulkanObjects");
            auto sc = pPresentInfo->pSwapchains[0];

            if (lastSwapchainInfo.minImageCount > 0)
                CreateVulkanObjects(_device, &lastSwapchainInfo, &sc);
            else
                CreateVulkanObjects(_device, nullptr, &sc);
        }
        else
        {
            LOG_DEBUG("return oQueuePresentKHR");
            return oQueuePresentKHR(queue, pPresentInfo);
        }
    }

    if (!ImGuiOverlayBase::IsInited())
        ImGuiOverlayBase::Init(Util::GetProcessWindow());
    else if (ImGuiOverlayBase::IsResetRequested())
    {
        LOG_INFO("Reset request detected, shutting down ImGui!");
        ImGuiOverlayVk::ReInitVk(Util::GetProcessWindow());
        return oQueuePresentKHR(queue, pPresentInfo);
    }

    if (!ImGuiOverlayBase::IsInited() || !ImGuiOverlayBase::IsVisible() || !Config::Instance()->RenderMenu)
        return oQueuePresentKHR(queue, pPresentInfo);

    LOG_DEBUG("rendering menu, swapchain count: {0}", pPresentInfo->swapchainCount);

    bool skipMenu = true;
    VkSemaphore signalSemaphores[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    for (size_t scIndex = 0; scIndex < pPresentInfo->swapchainCount; scIndex++)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGuiOverlayBase::RenderMenu();

        uint32_t idx = pPresentInfo->pImageIndices[scIndex];
        ImGui_ImplVulkanH_Frame* fd = &g_ImVulkan_Frames[idx];

        vkWaitForFences(g_ImVulkan_Info.Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
        vkResetFences(g_ImVulkan_Info.Device, 1, &fd->Fence);

        {
            vkResetCommandPool(g_ImVulkan_Info.Device, fd->CommandPool, 0);
            VkCommandBufferBeginInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(fd->CommandBuffer, &info);
        }

        {
            VkRenderPassBeginInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = g_vkRenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = ImGui::GetIO().DisplaySize.x;
            info.renderArea.extent.height = ImGui::GetIO().DisplaySize.y;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        auto ecbResult = vkEndCommandBuffer(fd->CommandBuffer);
        if (ecbResult != VK_SUCCESS)
        {
            LOG_ERROR("vkQueueSubmit error: {0:X}", (UINT)ecbResult);
            break;
        }

        // Submit queue and semaphores
        LOG_DEBUG("waitSemaphoreCount: {0}", pPresentInfo->waitSemaphoreCount);
        VkPipelineStageFlags waitStages[8] = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };

        VkSubmitInfo submit_info = { };
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &fd->CommandBuffer;
        submit_info.pWaitDstStageMask = waitStages;

        submit_info.waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
        submit_info.pWaitSemaphores = pPresentInfo->pWaitSemaphores;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &g_ImVulkan_Semaphores[idx];

        auto qResult = vkQueueSubmit(g_ImVulkan_Info.Queue, 1, &submit_info, fd->Fence);
        if (qResult != VK_SUCCESS)
        {
            LOG_ERROR("vkQueueSubmit error: {0:X}", (UINT)qResult);
            break;
        }

        signalSemaphores[scIndex] = g_ImVulkan_Semaphores[idx];
        skipMenu = false;
    }

    LOG_DEBUG("rendering done {0} errors", skipMenu ? "with" : "without");

    if (!skipMenu)
    {
        // already waited original calls semaphores when running commands
        // set menu draw signal semaphores as wait semaphores for present
        pPresentInfo->waitSemaphoreCount = pPresentInfo->swapchainCount;
        pPresentInfo->pWaitSemaphores = signalSemaphores;
    }

    // original call
    auto result = oQueuePresentKHR(queue, pPresentInfo);
    LOG_FUNC_RESULT(result);
    return result;
}

static VkResult VKAPI_CALL hkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator,
                                                VkSwapchainKHR* pSwapchain)
{
    LOG_FUNC();

    auto result = oCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

    //std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (result == VK_SUCCESS && device != NULL && pCreateInfo != NULL && pSwapchain != NULL)
    {
        _device = device;
        lastSwapchainInfo.imageExtent = pCreateInfo->imageExtent;
        lastSwapchainInfo.minImageCount = pCreateInfo->minImageCount;
        lastSwapchainInfo.imageFormat = pCreateInfo->imageFormat;
        //CreateVulkanObjects(device, pCreateInfo, pSwapchain);
    }

    LOG_FUNC_RESULT(result);

    return result;
}

bool ImGuiOverlayVk::IsInitedVk()
{
    ImGuiOverlayBase::VulkanReady();
    return _isInited;
}

HWND ImGuiOverlayVk::Handle()
{
    return ImGuiOverlayBase::Handle();
}

void ImGuiOverlayVk::InitVk(HWND InHwnd, VkDevice InDevice, VkInstance InInstance, VkPhysicalDevice InPD)
{
    _hwnd = InHwnd;

    if (_instance == nullptr)
        _instance = InInstance;

    if (_device == nullptr)
        _device = InDevice;

    if (_PD == nullptr)
        _PD = InPD;

    HookDevice(InDevice);

    _isInited = true;
}

void ImGuiOverlayVk::HookVK()
{
    if (o_vkCreateDevice != nullptr)
        return;

    //o_vkCreateDevice = (PFN_vkCreateDevice)DetourFindFunction("sl.interposer.dll", "vkCreateDevice");
    //o_vkCreateInstance = (PFN_vkCreateInstance)DetourFindFunction("sl.interposer.dll", "vkCreateInstance");

    if (o_vkCreateDevice == nullptr)
    {
        o_vkCreateDevice = (PFN_vkCreateDevice)DetourFindFunction("vulkan-1.dll", "vkCreateDevice");
        o_vkCreateInstance = (PFN_vkCreateInstance)DetourFindFunction("vulkan-1.dll", "vkCreateInstance");
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (o_vkCreateDevice != nullptr)
        DetourAttach(&(PVOID&)o_vkCreateDevice, hkvkCreateDevice);

    if (o_vkCreateInstance != nullptr)
        DetourAttach(&(PVOID&)o_vkCreateInstance, hkvkCreateInstance);

    DetourTransactionCommit();
}

void ImGuiOverlayVk::ShutdownVk()
{
    if (_isInited)
        ImGui_ImplVulkan_Shutdown();

    ImGuiOverlayBase::Shutdown();

    if (g_bInitialized)
        DestroyVulkanObjects(true);

    if (_isInited)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&(PVOID&)oQueuePresentKHR, hkQueuePresentKHR);
        DetourDetach(&(PVOID&)oCreateSwapchainKHR, hkCreateSwapchainKHR);

        DetourTransactionCommit();
    }

    _isInited = false;
    g_bInitialized = false;
}

void ImGuiOverlayVk::ReInitVk(HWND InNewHwnd)
{
    if (!_isInited || !g_bInitialized)
        return;

    _vkCleanMutex.lock();

    LOG_DEBUG("hwnd: {0:X}", (UINT64)InNewHwnd);

    ImGui_ImplVulkan_Shutdown();
    ImGuiOverlayBase::Shutdown();
    ImGuiOverlayBase::Init(InNewHwnd);
    g_bInitialized = false;
    createCounter = 0;

    _vkCleanMutex.unlock();
}
