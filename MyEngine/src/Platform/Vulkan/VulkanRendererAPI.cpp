#include "mepch.h"

#include "MyEngine/Core/Base.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include <SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL_video.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace MyEngine {

VulkanRendererAPI::~VulkanRendererAPI() {}

void VulkanRendererAPI::Init() {
  Application &app = Application::Get();
  Window &win = app.GetWindow();
  VulkanContext *ctx = static_cast<VulkanContext *>(win.GetGraphicsContext());
  SetupVulkan(ctx);
}

void VulkanRendererAPI::Shutdown() {
  Application &app = Application::Get();
  Window &win = app.GetWindow();
  VulkanContext *ctx = static_cast<VulkanContext *>(win.GetGraphicsContext());
  CleanupVulkan(ctx);
}

void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width,
                                    uint32_t height) {
  Application &app = Application::Get();
  Window &win = app.GetWindow();
  VulkanContext *ctx = static_cast<VulkanContext *>(win.GetGraphicsContext());
  CreateOrResizeWindow(ctx, x, y, width, height);
}

void VulkanRendererAPI::SetClearColor(const glm::vec4 &color) {
  m_ClearColor = color;
}

void VulkanRendererAPI::Clear() {}

void VulkanRendererAPI::WaitForIdle() {
  Application &app = Application::Get();
  Window &win = app.GetWindow();
  VulkanContext *ctx = static_cast<VulkanContext *>(win.GetGraphicsContext());
  VkResult res = vkDeviceWaitIdle(ctx->LogicalDevice);
  ME_CORE_ASSERT(res == VK_SUCCESS,
                 "Unable to wait for device idle when cleaning up vulkan!");
}

void VulkanRendererAPI::SetLineWidth(float width) {}

static bool
IsExtensionAvailable(const std::vector<VkExtensionProperties> &properties,
                     const char *extension) {
  for (const VkExtensionProperties &p : properties)
    if (strcmp(p.extensionName, extension) == 0)
      return true;
  return false;
}

#ifdef ME_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
            uint64_t object, size_t location, int32_t messageCode,
            const char *pLayerPrefix, const char *pMessage, void *pUserData) {
  // Unused arguments
  (void)flags;
  (void)object;
  (void)location;
  (void)messageCode;
  (void)pUserData;
  (void)pLayerPrefix;
  ME_CORE_ERROR("[vulkan] Debug report from ObjectType: {0}\nMessage: {1}",
                (int)objectType, pMessage);
  return VK_FALSE;
}
#endif

void VulkanRendererAPI::SetupVulkan(VulkanContext *context) {
  VkResult err;

  std::vector<const char *> instanceExtensions;
  uint32_t extensionsCount = 0;

  Application &app = Application::Get();
  SDL_Window *win =
      static_cast<SDL_Window *>(app.GetWindow().GetNativeWindow());
  SDL_Vulkan_GetInstanceExtensions(win, &extensionsCount, nullptr);
  instanceExtensions.resize(extensionsCount);
  SDL_Vulkan_GetInstanceExtensions(win, &extensionsCount,
                                   instanceExtensions.data());

  // Create the vulkan instance.
  {
    ME_CORE_TRACE("Creating vulkan instance!");
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Get the available extensions
    uint32_t propertiesCount;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount,
                                           properties.data());
    properties.resize(propertiesCount);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount,
                                                 properties.data());
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to enumerate instance extensions when setting up vulkan!");

    // Enable required extensions
    if (IsExtensionAvailable(
            properties,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
      instanceExtensions.push_back(
          VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

#ifdef ME_DEBUG
    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;
    instanceExtensions.push_back("VK_EXT_debug_report");
#endif

    createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    err = vkCreateInstance(&createInfo, context->AllocationCallback,
                           &context->Instance);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to create vulkan instance when setting up vulkan!");
    ME_CORE_TRACE("Vulkan instance created successfully!");

#ifdef ME_DEBUG
    ME_CORE_TRACE("Creating vulkan debug reporter!");
    auto f_vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            context->Instance, "vkCreateDebugReportCallbackEXT");
    ME_CORE_ASSERT(
        f_vkCreateDebugReportCallbackEXT != nullptr,
        "Unable to get debug report callback ext when setting up vulkan!");

    VkDebugReportCallbackCreateInfoEXT debugReportCi{};
    debugReportCi.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugReportCi.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                          VK_DEBUG_REPORT_WARNING_BIT_EXT |
                          VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debugReportCi.pfnCallback = DebugReport;
    debugReportCi.pUserData = nullptr;
    err = f_vkCreateDebugReportCallbackEXT(context->Instance, &debugReportCi,
                                           context->AllocationCallback,
                                           &context->DebugReport);
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to create vulkan debug reporter when setting up vulkan!");
    ME_CORE_TRACE("Created vulkan debug reporter successfully!");
#endif
  }

  // Select physical device
  {
    ME_CORE_TRACE("Selecting vulkan physical device!");
    uint32_t gpuCount;
    err = vkEnumeratePhysicalDevices(context->Instance, &gpuCount, nullptr);
    ME_CORE_ASSERT(gpuCount > 0, "No gpus found while selecting physical "
                                 "device when setting up vulkan!");
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to enumerate physical devices when setting up vulkan!");

    std::vector<VkPhysicalDevice> gpus;
    gpus.resize(gpuCount);
    err = vkEnumeratePhysicalDevices(context->Instance, &gpuCount, gpus.data());
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to enumerate physical devices when setting up vulkan!");

    for (VkPhysicalDevice &device : gpus) {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(device, &properties);
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        context->PhysicalDevice = device;
      }
    }

    // No discrete device found
    if (context->PhysicalDevice == nullptr) {
      ME_CORE_WARN("No discrete gpu found, using first available when "
                   "setting up vulkan!");
      context->PhysicalDevice = gpus[0];
    }

    ME_CORE_TRACE("Vulkan physical device selected successfully!");
  }

  // Select graphics queue family
  {
    ME_CORE_TRACE("Selecting graphics queue family for vulkan!");
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(context->PhysicalDevice, &count,
                                             nullptr);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(
        sizeof(VkQueueFamilyProperties) * count);

    vkGetPhysicalDeviceQueueFamilyProperties(context->PhysicalDevice, &count,
                                             queues);
    for (uint32_t i = 0; i < count; i++) {
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        context->QueueFamily = i;
        break;
      }
    }

    free(queues);
    ME_CORE_ASSERT(context->QueueFamily != (uint32_t)-1,
                   "No queue family selected when setting up vulkan!");
    ME_CORE_TRACE("Selected graphics queue family for vulkan successfully!");
  }

  // Create logical device (with 1 queue)
  {
    ME_CORE_TRACE("Creating logical device for vulkan!");
    std::vector<const char *> deviceExtensions;
    deviceExtensions.push_back("VK_KHR_swapchain");

    uint32_t propertiesCount;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(context->PhysicalDevice, nullptr,
                                         &propertiesCount, nullptr);
    properties.resize(propertiesCount);
    vkEnumerateDeviceExtensionProperties(context->PhysicalDevice, nullptr,
                                         &propertiesCount, properties.data());

    const float queuePriority[] = {1.0f};

    VkDeviceQueueCreateInfo queueInfo[1] = {};

    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].queueFamilyIndex = context->QueueFamily;
    queueInfo[0].queueCount = 1;
    queueInfo[0].pQueuePriorities = queuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
    createInfo.pQueueCreateInfos = queueInfo;
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    err = vkCreateDevice(context->PhysicalDevice, &createInfo,
                         context->AllocationCallback, &context->LogicalDevice);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to create logical device when setting up vulkan!");

    vkGetDeviceQueue(context->LogicalDevice, context->QueueFamily, 0,
                     &context->Queue);
    ME_CORE_TRACE("Created vulkan logical device successfully!");
  }

  {
    ME_CORE_TRACE("Creating descriptor pool for vulkan!");
    // Create descriptor pool
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = poolSizes;
    err = vkCreateDescriptorPool(context->LogicalDevice, &poolInfo,
                                 context->AllocationCallback,
                                 &context->DescriptorPool);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to create descriptor pool when setting up vulkan!");
    ME_CORE_TRACE("Descriptor pool created for vulkan successfully!");
  }

  // Get surface
  {
    ME_CORE_TRACE("Creating surface for vulkan!");
    SDL_Window *win = static_cast<SDL_Window *>(
        Application::Get().GetWindow().GetNativeWindow());
    if (SDL_Vulkan_CreateSurface(win, context->Instance,
                                 &context->Window.Surface) == 0) {
      ME_CORE_ASSERT(
          false,
          "Unable to create vulkan surface from SDL when setting up vulkan!");
    }

    int w, h;
    SDL_GetWindowSize(win, &w, &h);
    context->Window.Width = w;
    context->Window.Height = h;
    ME_CORE_TRACE("Created surface for vulkan successfully!");
  }

  // Select surface format
  {
    ME_CORE_TRACE("Selecting surface format for vulkan!");
    const VkFormat requestSurfaceImageFormat[] = {
        VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace =
        VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    uint32_t availCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        context->PhysicalDevice, context->Window.Surface, &availCount, nullptr);
    std::vector<VkSurfaceFormatKHR> availFormat;
    availFormat.resize((int)availCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->PhysicalDevice,
                                         context->Window.Surface, &availCount,
                                         availFormat.data());

    ME_CORE_TRACE("Selecting present mode for vulkan!");
    if (availCount == 1) {
      if (availFormat[0].format == VK_FORMAT_UNDEFINED) {
        VkSurfaceFormatKHR fmt;
        fmt.format = requestSurfaceImageFormat[0];
        fmt.colorSpace = requestSurfaceColorSpace;
        context->Window.SurfaceFormat = fmt;
      } else {
        context->Window.SurfaceFormat = availFormat[0];
      }
    } else {
      // Request several formats, the first found will be used
      bool found = false;
      for (int request = 0; request < 4 /* size of requestSurfaceImageFormat */;
           request++) {
        for (uint32_t availIndex = 0; availIndex < availCount; availIndex++) {
          if (availFormat[availIndex].format ==
                  requestSurfaceImageFormat[request] &&
              availFormat[availIndex].colorSpace == requestSurfaceColorSpace) {
            context->Window.SurfaceFormat = availFormat[availIndex];
            found = true;
          }
        }
      }

      if (!found) {
        context->Window.SurfaceFormat = availFormat[0];
      }
    }

    ME_CORE_TRACE("Surface format for vulkan selected successfully!");
  }

  // Present mode
  { context->Window.PresentMode = VK_PRESENT_MODE_FIFO_KHR; }

  // Swapchain
  {
    ME_CORE_TRACE("Creating window for vulkan!");
    CreateOrResizeWindow(context, 0, 0, context->Window.Width,
                         context->Window.Height);
    ME_CORE_ASSERT(context->IsValid() == true,
                   "Context is not valid after setting up vulkan!");
    ME_CORE_TRACE("window for vulkan created successfully!");
  }
}

void VulkanRendererAPI::CleanupVulkan(VulkanContext *context) {
  context->Cleanup();
}

void VulkanRendererAPI::CreateOrResizeWindow(VulkanContext *context, uint32_t x,
                                             uint32_t y, uint32_t width,
                                             uint32_t height) {
  CreateWindowSwapchain(context, width, height);
  CreateWindowCommandBuffers(context);
}

void VulkanRendererAPI::CreateWindowSwapchain(VulkanContext *context,
                                              uint32_t width, uint32_t height) {
  VkResult err;
  VkSwapchainKHR oldSwapchain = context->Window.Swapchain;
  context->Window.Swapchain = VK_NULL_HANDLE;
  err = vkDeviceWaitIdle(context->LogicalDevice);
  ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to wait for device idle to create "
                                    "swapchain when setting up vulkan!");

  // Cleanup old memory
  {
    // Destroy frames
    for (uint32_t i = 0; i < context->Window.ImageCount; i++) {
      VulkanFrame *fd = &context->Window.Frames[i];
      vkDestroyFence(context->LogicalDevice, fd->Fence,
                     context->AllocationCallback);
      vkFreeCommandBuffers(context->LogicalDevice, fd->CommandPool, 1,
                           &fd->CommandBuffer);
      vkDestroyCommandPool(context->LogicalDevice, fd->CommandPool,
                           context->AllocationCallback);
      fd->Fence = VK_NULL_HANDLE;
      fd->CommandBuffer = VK_NULL_HANDLE;
      fd->CommandPool = VK_NULL_HANDLE;

      vkDestroyImageView(context->LogicalDevice, fd->BackBufferView,
                         context->AllocationCallback);
      vkDestroyFramebuffer(context->LogicalDevice, fd->Framebuffer,
                           context->AllocationCallback);
    }

    // Destroy Semaphores
    for (uint32_t i = 0; i < context->Window.SemaphoreCount; i++) {
      VulkanFrameSemaphores *fsd = &context->Window.FrameSemaphores[i];
      vkDestroySemaphore(context->LogicalDevice, fsd->ImageAcquiredSemaphore,
                         context->AllocationCallback);
      vkDestroySemaphore(context->LogicalDevice, fsd->RenderCompleteSemaphore,
                         context->AllocationCallback);
      fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore =
          VK_NULL_HANDLE;
    }

    delete[] context->Window.Frames;
    delete[] context->Window.FrameSemaphores;

    context->Window.Frames = nullptr;
    context->Window.FrameSemaphores = nullptr;
    context->Window.ImageCount = 0;

    if (context->Window.RenderPass) {
      vkDestroyRenderPass(context->LogicalDevice, context->Window.RenderPass,
                          context->AllocationCallback);
    }
    // if (context->Window.Pipeline) {
    //   vkDestroyPipeline(context->LogicalDevice, context->Window.Pipeline,
    //                     context->AllocationCallback);
    // }
  }

  if (context->MinImageCount == 0) {
    context->MinImageCount = 2; // VK_PRESENT_MODE_FIFO_KHR
  }

  // Create swapchain
  {
    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = context->Window.Surface;
    info.minImageCount = context->MinImageCount;
    info.imageFormat = context->Window.SurfaceFormat.format;
    info.imageColorSpace = context->Window.SurfaceFormat.colorSpace;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // Assume graphics family == present family
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = context->Window.PresentMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = oldSwapchain;

    VkSurfaceCapabilitiesKHR cap;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context->PhysicalDevice, context->Window.Surface, &cap);
    ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to get physical device surface "
                                      "capabilities when creating swapchain!");

    if (info.minImageCount < cap.minImageCount) {
      info.minImageCount = cap.minImageCount;
    } else if (cap.maxImageCount != 0 &&
               info.minImageCount > cap.maxImageCount) {
      info.minImageCount = cap.maxImageCount;
    }

    if (cap.currentExtent.width == 0xffffffff) {
      info.imageExtent.width = context->Window.Width = width;
      info.imageExtent.height = context->Window.Height = height;
    } else {
      info.imageExtent.width = context->Window.Width = cap.currentExtent.width;
      info.imageExtent.height = context->Window.Height =
          cap.currentExtent.height;
    }

    err = vkCreateSwapchainKHR(context->LogicalDevice, &info,
                               context->AllocationCallback,
                               &context->Window.Swapchain);
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to create swapchain when resizing or creating vulkan window!");

    err = vkGetSwapchainImagesKHR(context->LogicalDevice,
                                  context->Window.Swapchain,
                                  &context->Window.ImageCount, nullptr);
    ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to get swapchain images when "
                                      "resizing or creating vulkan window!");

    VkImage backbuffers[16] = {};
    ME_CORE_ASSERT(context->Window.ImageCount >= context->MinImageCount);
    ME_CORE_ASSERT(context->Window.ImageCount < 16);

    err = vkGetSwapchainImagesKHR(context->LogicalDevice,
                                  context->Window.Swapchain,
                                  &context->Window.ImageCount, backbuffers);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to get swapchain images for backbuffer when "
                   "resizing or creating vulkan window!");

    ME_CORE_ASSERT(context->Window.Frames == nullptr &&
                       context->Window.FrameSemaphores == nullptr,
                   "Window frames or frame semaphores are not nullptr when "
                   "resizing or creating vulkan window!");

    context->Window.SemaphoreCount = context->Window.ImageCount + 1;
    context->Window.Frames = new VulkanFrame[context->Window.ImageCount];
    context->Window.FrameSemaphores =
        new VulkanFrameSemaphores[context->Window.SemaphoreCount];

    memset(context->Window.Frames, 0,
           sizeof(context->Window.Frames[0]) * context->Window.ImageCount);
    memset(context->Window.FrameSemaphores, 0,
           sizeof(context->Window.FrameSemaphores[0]) *
               context->Window.SemaphoreCount);
    for (uint32_t i = 0; i < context->Window.ImageCount; i++) {
      context->Window.Frames[i].BackBuffer = backbuffers[i];
    }
  }

  if (oldSwapchain) {
    vkDestroySwapchainKHR(context->LogicalDevice, oldSwapchain,
                          context->AllocationCallback);
  }

  if (!context->Window.UseDynamicRendering) {
    VkAttachmentDescription attachment{};
    attachment.format = context->Window.SurfaceFormat.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = context->Window.ClearEnable
                            ? VK_ATTACHMENT_LOAD_OP_CLEAR
                            : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachment{};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    err = vkCreateRenderPass(context->LogicalDevice, &info,
                             context->AllocationCallback,
                             &context->Window.RenderPass);
    ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to create render pass when "
                                      "resizing or creating vulkan window!");
  }

  // Create the image views
  {
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = context->Window.SurfaceFormat.format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;

    VkImageSubresourceRange imageRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0,
                                          1};
    info.subresourceRange = imageRange;
    for (uint32_t i = 0; i < context->Window.ImageCount; i++) {
      VulkanFrame *fd = &context->Window.Frames[i];
      info.image = fd->BackBuffer;
      err = vkCreateImageView(context->LogicalDevice, &info,
                              context->AllocationCallback, &fd->BackBufferView);
      ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to create image view when "
                                        "resizing or creating vulkan window!");
    }
  }

  // Create frame buffer
  if (!context->Window.UseDynamicRendering) {
    VkImageView attachment[1];
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = context->Window.RenderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = context->Window.Width;
    info.height = context->Window.Height;
    info.layers = 1;
    for (uint32_t i = 0; i < context->Window.ImageCount; i++) {
      VulkanFrame *fd = &context->Window.Frames[i];
      attachment[0] = fd->BackBufferView;
      err = vkCreateFramebuffer(context->LogicalDevice, &info,
                                context->AllocationCallback, &fd->Framebuffer);
      ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to create frame buffer when "
                                        "resizing or creating vulkan window!");
    }
  }
}

void VulkanRendererAPI::CreateWindowCommandBuffers(VulkanContext *context) {
  ME_CORE_ASSERT(context->PhysicalDevice != VK_NULL_HANDLE &&
                     context->LogicalDevice != VK_NULL_HANDLE,
                 "Devices were not setup properly when creating window command "
                 "buffers for vulkan!");

  VkResult err;
  for (uint32_t i = 0; i < context->Window.ImageCount; i++) {
    VulkanFrame *fd = &context->Window.Frames[i];

    {
      VkCommandPoolCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      info.flags = 0;
      info.queueFamilyIndex = context->QueueFamily;
      err = vkCreateCommandPool(context->LogicalDevice, &info,
                                context->AllocationCallback, &fd->CommandPool);
      ME_CORE_ASSERT(err == VK_SUCCESS,
                     "Unable to create command pool when creating window "
                     "command buffers for vulkan!");
    }

    {
      VkCommandBufferAllocateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      info.commandPool = fd->CommandPool;
      info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      info.commandBufferCount = 1;
      err = vkAllocateCommandBuffers(context->LogicalDevice, &info,
                                     &fd->CommandBuffer);
      ME_CORE_ASSERT(err == VK_SUCCESS,
                     "Unable to allocate command buffer when creating window "
                     "command buffers for vulkan!");
    }

    {
      VkFenceCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      err = vkCreateFence(context->LogicalDevice, &info,
                          context->AllocationCallback, &fd->Fence);
      ME_CORE_ASSERT(err == VK_SUCCESS, "Unable to create fence when creating "
                                        "window command buffers for vulkan!");
    }
  }

  for (uint32_t i = 0; i < context->Window.SemaphoreCount; i++) {
    VulkanFrameSemaphores *fsd = &context->Window.FrameSemaphores[i];

    {
      VkSemaphoreCreateInfo info{};
      info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      err = vkCreateSemaphore(context->LogicalDevice, &info,
                              context->AllocationCallback,
                              &fsd->ImageAcquiredSemaphore);
      ME_CORE_ASSERT(err == VK_SUCCESS,
                     "Cannot create image acquired semaphore when creating "
                     "window command buffers for vulkan");
      err = vkCreateSemaphore(context->LogicalDevice, &info,
                              context->AllocationCallback,
                              &fsd->RenderCompleteSemaphore);
      ME_CORE_ASSERT(err == VK_SUCCESS,
                     "Cannot create render complete semaphore when creating "
                     "window command buffers for vulkan");
    }
  }
}

bool VulkanRendererAPI::BeginFrame(GraphicsContext *ctx) {
  Application &app = Application::Get();
  Window &window = app.GetWindow();
  VulkanContext *context = static_cast<VulkanContext *>(ctx);

  if (window.GetWidth() > 0 && window.GetHeight() > 0 &&
      (context->RebuildSwapchain ||
       context->Window.Width != window.GetWidth() ||
       context->Window.Height != window.GetHeight())) {
    SetViewport(0, 0, window.GetWidth(), window.GetHeight());
    context->Window.FrameIndex = 0;
    context->RebuildSwapchain = false;
  }

  VkResult err;

  VkSemaphore imageAcquiredSemaphore =
      context->Window.GetImageAcquiredSemaphore();

  err = vkAcquireNextImageKHR(context->LogicalDevice, context->Window.Swapchain,
                              UINT64_MAX, imageAcquiredSemaphore,
                              VK_NULL_HANDLE, &context->Window.FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    context->RebuildSwapchain = true;
    return false;
  }
  ME_CORE_ASSERT(err == VK_SUCCESS,
                 "Unable to acquire next image when beginning vulkan frame!");

  VulkanFrame *fd = context->Window.GetCurrentFrame();
  {
    err = vkWaitForFences(context->LogicalDevice, 1, &fd->Fence, VK_TRUE,
                          UINT64_MAX);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to wait for fences when beginning vulkan frame!");

    err = vkResetFences(context->LogicalDevice, 1, &fd->Fence);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to reset fences when beginning vulkan frame!");
  }
  {
    err = vkResetCommandPool(context->LogicalDevice, fd->CommandPool, 0);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to reset command pool when beginning vulkan frame!");
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    ME_CORE_ASSERT(
        err == VK_SUCCESS,
        "Unable to begin command buffer when beginning vulkan frame!");
  }
  {
    VkRenderPassBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = context->Window.RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.offset = {0, 0};
    info.renderArea.extent.width = context->Window.Width;
    info.renderArea.extent.height = context->Window.Height;
    VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    info.clearValueCount = 1;
    context->Window.ClearValue = clearValue;
    info.pClearValues = &context->Window.ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  return true;
}

void VulkanRendererAPI::EndFrame(GraphicsContext *ctx) {
  VulkanContext *context = static_cast<VulkanContext *>(ctx);

  VulkanFrame *fd = context->Window.GetCurrentFrame();
  VkSemaphore imageAcquiredSemaphore =
      context->Window.GetImageAcquiredSemaphore();
  VkSemaphore renderCompleteSemaphore =
      context->Window.GetRenderCompleteSemaphore();

  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &imageAcquiredSemaphore;
    info.pWaitDstStageMask = &waitStage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &renderCompleteSemaphore;

    VkResult err = vkEndCommandBuffer(fd->CommandBuffer);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to end command buffer when ending vulkan frame!");
    err = vkQueueSubmit(context->Queue, 1, &info, fd->Fence);
    ME_CORE_ASSERT(err == VK_SUCCESS,
                   "Unable to submit queue when ending vulkan frame!");
  }
}

void VulkanRendererAPI::PresentFrame(GraphicsContext *ctx) {
  VulkanContext *context = static_cast<VulkanContext *>(ctx);

  if (context->RebuildSwapchain) {
    return;
  }

  VkSemaphore renderCompleteSemaphore =
      context->Window.GetRenderCompleteSemaphore();

  VkPresentInfoKHR info{};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &renderCompleteSemaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &context->Window.Swapchain;
  info.pImageIndices = &context->Window.FrameIndex;
  VkResult err = vkQueuePresentKHR(context->Queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    context->RebuildSwapchain = true;
    return;
  }
  ME_CORE_ASSERT(err == VK_SUCCESS,
                 "Unable to present current frame from vulkan!");

  // Get the next set of semaphores
  context->Window.SemaphoreIndex =
      (context->Window.SemaphoreIndex + 1) % context->Window.SemaphoreCount;
}

} // namespace MyEngine
