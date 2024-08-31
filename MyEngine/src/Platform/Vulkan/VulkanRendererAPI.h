#pragma once

#include "MyEngine/Renderer/GraphicsContext.h"
#include "MyEngine/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace MyEngine {
class VulkanRendererAPI : public RendererAPI {
public:
  virtual ~VulkanRendererAPI();

  virtual void Init() override;
  virtual void Shutdown() override;
  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) override;
  virtual void SetClearColor(const glm::vec4 &color) override;
  virtual void Clear() override;
  virtual void WaitForIdle() override;

  virtual void SetLineWidth(float width) override;

  void SetupVulkan(VulkanContext *context);
  void CleanupVulkan(VulkanContext *context);

  void CreateOrResizeWindow(VulkanContext *context, uint32_t x, uint32_t y,
                            uint32_t width, uint32_t height);
  void CreateWindowSwapchain(VulkanContext *context, uint32_t width,
                             uint32_t height);
  void CreateWindowCommandBuffers(VulkanContext *context);

  virtual void BeginFrame(GraphicsContext *ctx) override;
  virtual void EndFrame(GraphicsContext *ctx) override;
  virtual void PresentFrame(GraphicsContext *ctx) override;

private:
  // +============+
  // | CLASS VARS |
  // +============+
  glm::vec4 m_ClearColor;
  bool m_IsInitialized = false;
};
} // namespace MyEngine
