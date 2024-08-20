#include "vk_engine.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <set>
#include <stdexcept>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Log.h"
#include "file_helper.hpp"
#include "vk_types.hpp"

using namespace MyEngine;

constexpr bool useValidationLayers = true;

void VulkanEngine::Init() {
  // Init SDL for window
  SDL_Init(SDL_INIT_VIDEO);

  SDL_WindowFlags windowFlags =
      (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  Window = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, WindowExtent.width,
                            WindowExtent.height, windowFlags);

  InitVulkan();

  IsInitialized = true;
}

void VulkanEngine::CleanupSwapChain() {
  for (VkFramebuffer buffer : SwapchainFrameBuffer) {
    vkDestroyFramebuffer(Device, buffer, nullptr);
  }

  vkFreeCommandBuffers(Device, CommandPool,
                       static_cast<uint32_t>(CommandBuffers.size()),
                       CommandBuffers.data());

  vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
  vkDestroyRenderPass(Device, RenderPass, nullptr);

  for (VkImageView image : SwapChainImageViews) {
    vkDestroyImageView(Device, image, nullptr);
  }

  vkDestroySwapchainKHR(Device, SwapChain, nullptr);
}

void VulkanEngine::Cleanup() {
  if (IsInitialized) {
    if (EnableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    }

    CleanupSwapChain();

    vkDestroyBuffer(Device, VertexBuffer, nullptr);
    vkFreeMemory(Device, VertexBufferMemory, nullptr);
    vkDestroyBuffer(Device, IndexBuffer, nullptr);
    vkFreeMemory(Device, IndexBufferMemory, nullptr);

    for (unsigned long long i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(Device, RenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(Device, ImageAvailableSemaphores[i], nullptr);
      vkDestroyFence(Device, InFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(Device, CommandPool, nullptr);

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyDevice(Device, nullptr);
    vkDestroyInstance(Instance, nullptr);
    SDL_DestroyWindow(Window);
  }
}

void VulkanEngine::RecreateSwapChain() {
  int width = 0, height = 0;
  SDL_GetWindowSize(Window, &width, &height);

  while (width == 0 || height == 0) {
    SDL_GetWindowSize(Window, &width, &height);
  }

  vkDeviceWaitIdle(Device);

  CleanupSwapChain();
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFrameBuffers();
  CreateCommandBuffers();

  ImagesInFlight.resize(SwapChainImages.size(), VK_NULL_HANDLE);
}

void VulkanEngine::Draw() {
  vkWaitForFences(Device, 1, &InFlightFences[CurrentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      Device, SwapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame],
      VK_NULL_HANDLE, &imageIndex);

  // NOTE: For Window Resizing
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    VK_CHECK(result);
  }

  if (ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(Device, 1, &ImagesInFlight[imageIndex], VK_TRUE,
                    UINT64_MAX);
  }

  ImagesInFlight[imageIndex] = InFlightFences[CurrentFrame];

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {ImageAvailableSemaphores[CurrentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &CommandBuffers[imageIndex];

  VkSemaphore signalSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(Device, 1, &InFlightFences[CurrentFrame]);

  VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &submitInfo,
                         InFlightFences[CurrentFrame]));

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {SwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(PresentationQueue, &presentInfo);

  // NOTE: For Window Resize
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      FrameBufferResized) {
    RecreateSwapChain();
    FrameBufferResized = false;
  } else if (result != VK_SUCCESS) {
    VK_CHECK(result);
  }

  CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanEngine::Run() {
  SDL_Event e;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_WINDOWEVENT:
        switch (e.window.event) {
        case SDL_WINDOWEVENT:
          StopRendering = true;
          break;
        case SDL_WINDOWEVENT_RESTORED:
          StopRendering = false;
          break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          FrameBufferResized = true;
          break;
        }
        break;
      }
    }

    if (FrameBufferResized) {
      RecreateSwapChain();
    }

    if (StopRendering) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    } else {
      Draw();
    }
  }

  vkDeviceWaitIdle(Device);
}

void VulkanEngine::InitVulkan() {
  CreateInstance();
  SetupDebugMessenger();
  CreateSurface();
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFrameBuffers();
  CreateCommandPool();
  CreateVertexBuffer();
  CreateIndexBuffer();
  CreateCommandBuffers();
  CreateSyncObjects();
}

void VulkanEngine::CreateInstance() {
  bool supportsValidation =
      EnableValidationLayers && !CheckValidationLayerSupport();
  assert(!supportsValidation &&
         "Validation layers requested but not available!");

  ME_CORE_INFO("Creating Vulkan instance!");
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = nullptr;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "TestVulkan";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount =
      static_cast<unsigned int>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (EnableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<unsigned int>(ValidationLayers.size());
    createInfo.ppEnabledLayerNames = ValidationLayers.data();

    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  VK_CHECK(vkCreateInstance(&createInfo, nullptr, &Instance));
}

void VulkanEngine::PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = DebugCallback;
}

void VulkanEngine::SetupDebugMessenger() {
  if (!EnableValidationLayers) {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  PopulateDebugMessengerCreateInfo(createInfo);

  VK_CHECK(CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr,
                                        &DebugMessenger));
}

void VulkanEngine ::CreateSurface() {
  bool createSuccess = SDL_Vulkan_CreateSurface(Window, Instance, &Surface);
  assert(createSuccess && "Unable to create vulkan surface with SDL");
}

void VulkanEngine::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

  assert(deviceCount > 0 && "No physical device to choose from!");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

  for (const VkPhysicalDevice device : devices) {
    if (IsDeviceSuitable(device)) {
      ChosenGPU = device;
      break;
    }
  }

  assert(ChosenGPU != VK_NULL_HANDLE && "No suitable physical device chosen!");
}

void VulkanEngine::CreateLogicalDevice() {
  QueueFamilyIndices indices = FindQueueFamilies(ChosenGPU);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(),
                                            indices.PresentationFamily.value()};

  float queuePriority = 1.0f;

  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;

  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(DeviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

  if (EnableValidationLayers) {
    deviceCreateInfo.enabledLayerCount =
        static_cast<uint32_t>(ValidationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
  } else {
    deviceCreateInfo.enabledLayerCount = 0;
  }

  VK_CHECK(vkCreateDevice(ChosenGPU, &deviceCreateInfo, nullptr, &Device));

  vkGetDeviceQueue(Device, indices.GraphicsFamily.value(), 0, &GraphicsQueue);
  vkGetDeviceQueue(Device, indices.PresentationFamily.value(), 0,
                   &PresentationQueue);
}

void VulkanEngine::CreateSwapChain() {
  SwapChainSupportDetails swapchainSupport = QuerySwapChainSupport(ChosenGPU);

  VkSurfaceFormatKHR surfaceFormat =
      ChooseSwapSurfaceFormat(swapchainSupport.Formats);
  VkPresentModeKHR presentMode =
      ChooseSwapPresentMode(swapchainSupport.PresentModes);
  VkExtent2D extent = ChooseSwapExtent(swapchainSupport.Capabilities);

  uint32_t imageCount = swapchainSupport.Capabilities.minImageCount + 1;
  if (swapchainSupport.Capabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.Capabilities.maxImageCount) {
    imageCount = swapchainSupport.Capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapCreateInfo{};
  swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapCreateInfo.pNext = nullptr;
  swapCreateInfo.surface = Surface;
  swapCreateInfo.minImageCount = imageCount;
  swapCreateInfo.imageFormat = surfaceFormat.format;
  swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapCreateInfo.imageExtent = extent;
  swapCreateInfo.imageArrayLayers = 1;
  swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(ChosenGPU);
  uint32_t queueFamilyIndices[] = {indices.GraphicsFamily.value(),
                                   indices.PresentationFamily.value()};

  if (indices.GraphicsFamily != indices.PresentationFamily) {
    swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapCreateInfo.queueFamilyIndexCount = 2;
    swapCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  swapCreateInfo.preTransform = swapchainSupport.Capabilities.currentTransform;
  swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapCreateInfo.presentMode = presentMode;
  swapCreateInfo.clipped = VK_TRUE;

  //  swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(Device, &swapCreateInfo, nullptr, &SwapChain));

  vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, nullptr);
  SwapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount,
                          SwapChainImages.data());

  SwapChainImageFormat = surfaceFormat.format;
  SwapChainExtent = extent;
}

void VulkanEngine::CreateImageViews() {
  SwapChainImageViews.resize(SwapChainImages.size());

  for (size_t i = 0; i < SwapChainImages.size(); i++) {
    VkImageViewCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.image = SwapChainImages[i];
    // NOTE: This is for 2d images only, this is where view is changed
    imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageCreateInfo.format = SwapChainImageFormat;
    imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCreateInfo.subresourceRange.baseMipLevel = 0;
    imageCreateInfo.subresourceRange.levelCount = 1;
    imageCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(Device, &imageCreateInfo, nullptr,
                               &SwapChainImageViews[i]));
  }
}

void VulkanEngine::CreateRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = SwapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VK_CHECK(vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass));
}

void VulkanEngine::CreateGraphicsPipeline() {
  std::string vertShaderCode;
  fs::FsReadStatus readStatus =
      fs::ReadFile("shaders/shader.vert.spv", &vertShaderCode);
  assert(readStatus == fs::SUCCESS && "Unable to read vert shader!");

  std::string fragShaderCode;
  readStatus = fs::ReadFile("shaders/shader.frag.spv", &fragShaderCode);
  assert(readStatus == fs::SUCCESS && "Unable to read frag shader!");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescriptions = Vertex::GetAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)SwapChainExtent.width;
  viewport.height = (float)SwapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = SwapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterState{};
  rasterState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterState.depthClampEnable = VK_FALSE;
  rasterState.rasterizerDiscardEnable = VK_FALSE;
  rasterState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterState.lineWidth = 1.0f;
  rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterState.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  VK_CHECK(vkCreatePipelineLayout(Device, &pipelineLayoutInfo, nullptr,
                                  &PipelineLayout));

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterState;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = PipelineLayout;
  pipelineInfo.renderPass = RenderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &GraphicsPipeline));

  vkDestroyShaderModule(Device, fragShaderModule, nullptr);
  vkDestroyShaderModule(Device, vertShaderModule, nullptr);
}

void VulkanEngine::CreateFrameBuffers() {
  SwapchainFrameBuffer.resize(SwapChainImageViews.size());

  for (size_t i = 0; i < SwapChainImageViews.size(); i++) {
    VkImageView attachments[] = {SwapChainImageViews[i]};

    VkFramebufferCreateInfo frameBufferInfo{};
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.renderPass = RenderPass;
    frameBufferInfo.attachmentCount = 1;
    frameBufferInfo.pAttachments = attachments;
    frameBufferInfo.width = SwapChainExtent.width;
    frameBufferInfo.height = SwapChainExtent.height;
    frameBufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(Device, &frameBufferInfo, nullptr,
                                 &SwapchainFrameBuffer[i]));
  }
}

void VulkanEngine::CreateCommandPool() {
  QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(ChosenGPU);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

  VK_CHECK(vkCreateCommandPool(Device, &poolInfo, nullptr, &CommandPool));
}

void VulkanEngine::CreateVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(Device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, Vertices.data(), (unsigned long long)bufferSize);
  vkUnmapMemory(Device, stagingBufferMemory);

  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);
  CopyBuffer(stagingBuffer, VertexBuffer, bufferSize);
  vkDestroyBuffer(Device, stagingBuffer, nullptr);
  vkFreeMemory(Device, stagingBufferMemory, nullptr);
}

void VulkanEngine::CreateIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(Indices[0]) * Indices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(Device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, Indices.data(), (unsigned long long)bufferSize);
  vkUnmapMemory(Device, stagingBufferMemory);

  CreateBuffer(
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);
  CopyBuffer(stagingBuffer, IndexBuffer, bufferSize);
  vkDestroyBuffer(Device, stagingBuffer, nullptr);
  vkFreeMemory(Device, stagingBufferMemory, nullptr);
}

void VulkanEngine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer &buffer,
                                VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(Device, &bufferInfo, nullptr, &buffer));

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(Device, buffer, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex =
      FindMemoryType(memRequirements.memoryTypeBits, properties);

  VK_CHECK(vkAllocateMemory(Device, &memAllocInfo, nullptr, &bufferMemory));

  vkBindBufferMemory(Device, buffer, bufferMemory, 0);
}

void VulkanEngine::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                              VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = CommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(Device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(GraphicsQueue);
  vkFreeCommandBuffers(Device, CommandPool, 1, &commandBuffer);
}

uint32_t VulkanEngine::FindMemoryType(uint32_t typeFilter,
                                      VkMemoryPropertyFlags flags) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(ChosenGPU, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
      return i;
    }
  }

  throw(std::runtime_error("Unable to find suitable memory type!"));
}

void VulkanEngine::CreateCommandBuffers() {
  CommandBuffers.resize(SwapchainFrameBuffer.size());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = CommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

  VK_CHECK(vkAllocateCommandBuffers(Device, &allocInfo, CommandBuffers.data()));

  for (size_t i = 0; i < CommandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(CommandBuffers[i], &beginInfo));

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = RenderPass;
    renderPassInfo.framebuffer = SwapchainFrameBuffer[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = SwapChainExtent;

    VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      GraphicsPipeline);

    VkBuffer vertexBuffers[] = {VertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(CommandBuffers[i], IndexBuffer, 0,
                         VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(CommandBuffers[i], static_cast<uint32_t>(Indices.size()),
                     1, 0, 0, 0);
    vkCmdEndRenderPass(CommandBuffers[i]);

    VK_CHECK(vkEndCommandBuffer(CommandBuffers[i]));
  }
}

void VulkanEngine::CreateSyncObjects() {
  ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  ImagesInFlight.resize(SwapChainImages.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceCreateInfo{};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (unsigned long long i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VK_CHECK(vkCreateSemaphore(Device, &semaphoreInfo, nullptr,
                               &ImageAvailableSemaphores[i]));
    VK_CHECK(vkCreateSemaphore(Device, &semaphoreInfo, nullptr,
                               &RenderFinishedSemaphores[i]));
    VK_CHECK(
        vkCreateFence(Device, &fenceCreateInfo, nullptr, &InFlightFences[i]));
  }
}

VkShaderModule VulkanEngine::CreateShaderModule(const std::string &byteCode) {
  VkShaderModuleCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = byteCode.size();
  shaderCreateInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

  VkShaderModule shaderModule;

  VK_CHECK(
      vkCreateShaderModule(Device, &shaderCreateInfo, nullptr, &shaderModule));

  return shaderModule;
}

VkSurfaceFormatKHR VulkanEngine::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR VulkanEngine::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return availablePresentModes[0];
}

VkExtent2D
VulkanEngine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  }

  int width, height;
  SDL_Vulkan_GetDrawableSize(Window, &width, &height);

  VkExtent2D actualExtent{static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height)};

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);

  return actualExtent;
}

SwapChainSupportDetails
VulkanEngine::QuerySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Surface,
                                            &details.Capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface, &formatCount, nullptr);

  if (formatCount > 0) {
    details.Formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface, &formatCount,
                                         details.Formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, Surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount > 0) {
    details.PresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, Surface, &presentModeCount, details.PresentModes.data());
  }

  return details;
}

bool VulkanEngine::IsDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = FindQueueFamilies(device);

  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool swapChainAdequate = false;

  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.Formats.empty() &&
                        !swapChainSupport.PresentModes.empty();
  }

  return indices.IsCompleted() && extensionsSupported && swapChainAdequate;
}

bool VulkanEngine::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(DeviceExtensions.begin(),
                                           DeviceExtensions.end());

  for (const VkExtensionProperties &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanEngine::FindQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           families.data());

  int i = 0;
  for (const VkQueueFamilyProperties family : families) {
    if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.GraphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Surface, &presentSupport);

    if (presentSupport) {
      indices.PresentationFamily = i;
    }

    if (indices.IsCompleted()) {
      break;
    }

    i++;
  }

  return indices;
}

std::vector<const char *> VulkanEngine::GetRequiredExtensions() {
  unsigned int pCount = 0;
  SDL_Vulkan_GetInstanceExtensions(Window, &pCount, nullptr);

  std::vector<const char *> names(pCount);
  SDL_Vulkan_GetInstanceExtensions(Window, &pCount, names.data());

  if (EnableValidationLayers) {
    names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return names;
}

bool VulkanEngine::CheckValidationLayerSupport() {
  unsigned int layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : ValidationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}
