#pragma once

#include <vulkan/vulkan.h>

#include "MyEngine/Renderer/GraphicsContext.h"

namespace MyEngine {
struct VulkanFrame {
  VkCommandPool CommandPool;
  VkCommandBuffer CommandBuffer;
  VkFence Fence;
  VkImage BackBuffer;
  VkImageView BackBufferView;
  VkFramebuffer Framebuffer;
};

struct VulkanFrameSemaphores {
  VkSemaphore ImageAcquiredSemaphore;
  VkSemaphore RenderCompleteSemaphore;
};

struct VulkanWindow {
  int Width;
  int Height;
  VkSwapchainKHR Swapchain;
  VkSurfaceKHR Surface;
  VkSurfaceFormatKHR SurfaceFormat;
  VkPresentModeKHR PresentMode;
  VkRenderPass RenderPass;
  VkPipeline Pipeline;

  bool UseDynamicRendering;
  bool ClearEnable;
  VkClearValue ClearValue;
  uint32_t FrameIndex;
  uint32_t ImageCount;
  uint32_t SemaphoreCount;
  uint32_t SemaphoreIndex;

  VulkanFrame *Frames;
  VulkanFrameSemaphores *FrameSemaphores;

  VkSemaphore GetImageAcquiredSemaphore() {
    return FrameSemaphores[SemaphoreIndex].ImageAcquiredSemaphore;
  }

  VkSemaphore GetRenderCompleteSemaphore() {
    return FrameSemaphores[SemaphoreIndex].RenderCompleteSemaphore;
  }

  VulkanFrame *GetCurrentFrame() { return &Frames[FrameIndex]; }

  VulkanWindow() {
    ClearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    ClearValue.depthStencil = {1.0f, 0};
    ClearEnable = true;
  }

  bool IsValid() {
    return Swapchain != VK_NULL_HANDLE && Surface != VK_NULL_HANDLE &&
           RenderPass != VK_NULL_HANDLE && Frames != nullptr &&
           FrameSemaphores != nullptr;
  }
};

class VulkanContext : public GraphicsContext {
public:
  VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
  VkDevice LogicalDevice = VK_NULL_HANDLE;
  VkInstance Instance = VK_NULL_HANDLE;
  VkAllocationCallbacks *AllocationCallback = nullptr;
  uint32_t QueueFamily = (uint32_t)-1;
  VkQueue Queue = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT DebugReport = VK_NULL_HANDLE;
  VkPipelineCache PipelineCache = VK_NULL_HANDLE;
  VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
  uint32_t MinImageCount = 2;

  VulkanWindow Window;

  bool IsValid() {
    return PhysicalDevice != VK_NULL_HANDLE &&
           LogicalDevice != VK_NULL_HANDLE && Instance != VK_NULL_HANDLE &&
           QueueFamily != (uint32_t)-1 && Queue != VK_NULL_HANDLE &&
           DescriptorPool != VK_NULL_HANDLE && Window.IsValid();
  }

  void Cleanup() {
    VkResult res = vkDeviceWaitIdle(this->LogicalDevice);
    ME_CORE_ASSERT(res == VK_SUCCESS,
                   "Unable to wait for device idle when cleaning up vulkan!");

    for (uint32_t i = 0; i < this->Window.ImageCount; i++) {
      DestroyFrame(&this->Window.Frames[i]);
    }
    for (uint32_t i = 0; i < this->Window.SemaphoreCount; i++) {
      DestroyFrameSemaphores(&this->Window.FrameSemaphores[i]);
    }
    delete[] this->Window.Frames;
    delete[] this->Window.FrameSemaphores;
    this->Window.Frames = nullptr;
    this->Window.FrameSemaphores = nullptr;

    vkDestroyPipeline(this->LogicalDevice, this->Window.Pipeline,
                      this->AllocationCallback);
    vkDestroyRenderPass(this->LogicalDevice, this->Window.RenderPass,
                        this->AllocationCallback);
    vkDestroySwapchainKHR(this->LogicalDevice, this->Window.Swapchain,
                          this->AllocationCallback);
    vkDestroySurfaceKHR(this->Instance, this->Window.Surface,
                        this->AllocationCallback);

    vkDestroyDescriptorPool(this->LogicalDevice, this->DescriptorPool,
                            this->AllocationCallback);

#ifdef ME_DEBUG
    auto f_vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            this->Instance, "vkDestroyDebugReportCallbackEXT");

    f_vkDestroyDebugReportCallbackEXT(this->Instance, this->DebugReport,
                                      this->AllocationCallback);
#endif

    vkDestroyDevice(this->LogicalDevice, this->AllocationCallback);
    vkDestroyInstance(this->Instance, this->AllocationCallback);
  }

private:
  void DestroyFrame(VulkanFrame *fd) {
    vkDestroyFence(this->LogicalDevice, fd->Fence, this->AllocationCallback);
    vkFreeCommandBuffers(this->LogicalDevice, fd->CommandPool, 1,
                         &fd->CommandBuffer);
    vkDestroyCommandPool(this->LogicalDevice, fd->CommandPool,
                         this->AllocationCallback);
    fd->Fence = VK_NULL_HANDLE;
    fd->CommandBuffer = VK_NULL_HANDLE;
    fd->CommandPool = VK_NULL_HANDLE;

    vkDestroyImageView(this->LogicalDevice, fd->BackBufferView,
                       this->AllocationCallback);
    vkDestroyFramebuffer(this->LogicalDevice, fd->Framebuffer,
                         this->AllocationCallback);
  }

  void DestroyFrameSemaphores(VulkanFrameSemaphores *fsd) {
    vkDestroySemaphore(this->LogicalDevice, fsd->ImageAcquiredSemaphore,
                       this->AllocationCallback);
    vkDestroySemaphore(this->LogicalDevice, fsd->RenderCompleteSemaphore,
                       this->AllocationCallback);
    fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = VK_NULL_HANDLE;
  }
};
} // namespace MyEngine
