#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <vector>

#include "Common.hpp"
#include "vk_types.hpp"

struct SDL_Window;

namespace MyEngine {

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> ValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char *> DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif

struct FrameData {
  VkSemaphore SwapchainSemaphore, RenderSemaphore;
  VkFence RenderFence;

  VkCommandPool CommandPool;
  VkCommandBuffer MainCommandBuffer;
};

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }

  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void
DestroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger,
                              const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

struct QueueFamilyIndices {
  std::optional<uint32_t> GraphicsFamily;
  std::optional<uint32_t> PresentationFamily;

  bool IsCompleted() {
    return GraphicsFamily.has_value() && PresentationFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR Capabilities;
  std::vector<VkSurfaceFormatKHR> Formats;
  std::vector<VkPresentModeKHR> PresentModes;
};

const std::vector<Vertex> Vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<uint16_t> Indices{0, 1, 2, 2, 3, 0};

class EXPORTED VulkanEngine {
public:
  bool IsInitialized{false};
  int FrameNumber{0};
  bool StopRendering{false};
  VkExtent2D WindowExtent{1700, 900};

  SDL_Window *Window{nullptr};
  bool FrameBufferResized = false;

  VkInstance Instance;                     // Vulkan library handle
  VkDebugUtilsMessengerEXT DebugMessenger; // Vulkan debug output handle
  VkPhysicalDevice ChosenGPU;              // GPU chosen as the default device
  VkDevice Device;                         // Vulkan device for commands
  VkQueue GraphicsQueue;                   // Graphics queue
  VkQueue PresentationQueue;               // Presentation queue
  VkSwapchainKHR SwapChain;                // Graphics swap chain
  VkSurfaceKHR Surface;                    // Vulkan window surface
  std::vector<VkImage> SwapChainImages;    // Swapchain image buffer
  VkFormat SwapChainImageFormat;           // Swapchain formatting
  VkExtent2D SwapChainExtent;              // Size of swapchain area
  std::vector<VkImageView> SwapChainImageViews; // Images in the chain
  VkPipelineLayout PipelineLayout; // Graphics pipeline layout information
  VkRenderPass RenderPass;
  VkPipeline GraphicsPipeline; // Actual graphics pipeline
  std::vector<VkFramebuffer> SwapchainFrameBuffer;
  VkCommandPool CommandPool;
  VkBuffer VertexBuffer;
  VkDeviceMemory VertexBufferMemory;
  VkBuffer IndexBuffer;
  VkDeviceMemory IndexBufferMemory;
  std::vector<VkCommandBuffer> CommandBuffers;
  std::vector<VkSemaphore> ImageAvailableSemaphores;
  std::vector<VkSemaphore> RenderFinishedSemaphores;
  std::vector<VkFence> InFlightFences;
  std::vector<VkFence> ImagesInFlight;
  unsigned long long CurrentFrame = 0;

  void Init();
  void Cleanup();
  void Draw();
  void Run();

private:
  void InitVulkan();
  void CleanupSwapChain();
  void RecreateSwapChain();

  void CreateInstance();

  void PopulateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateRenderPass();
  void CreateGraphicsPipeline();
  void CreateFrameBuffers();
  void CreateCommandPool();
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags);

  void CreateCommandBuffers();
  void CreateSyncObjects();
  VkShaderModule CreateShaderModule(const std::string &byteCode);

  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
  bool IsDeviceSuitable(VkPhysicalDevice device);
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

  std::vector<const char *> GetRequiredExtensions();
  bool CheckValidationLayerSupport();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    fmt::print("ValidationLayer: {}\n", pCallbackData->pMessage);
    return VK_FALSE;
  }
};
} // namespace MyEngine
