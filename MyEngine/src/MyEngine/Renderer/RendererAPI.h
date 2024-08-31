#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Renderer/GraphicsContext.h"

#include <glm/glm.hpp>

namespace MyEngine {
class RendererAPI {
public:
  enum class API { None = 0, Vulkan = 1 };

  virtual ~RendererAPI() = default;

  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) = 0;
  virtual void SetClearColor(const glm::vec4 &color) = 0;
  virtual void Clear() = 0;

  // virtual uint64_t CreateVertexBuffer(uint32_t size) = 0;
  // virtual uint64_t CreateVertexBuffer(float *vertices, uint32_t size) = 0;
  // virtual uint64_t CreateIndexBuffer(uint32_t *indices, uint32_t count) = 0;

  virtual void BeginFrame(GraphicsContext *ctx) = 0;
  virtual void EndFrame(GraphicsContext *ctx) = 0;
  virtual void PresentFrame(GraphicsContext *ctx) = 0;
  virtual void WaitForIdle() = 0;
  // virtual void DrawIndexed(const Ref<>)

  virtual void SetLineWidth(float width) = 0;

  static API GetAPI() { return s_API; }
  static Unique<RendererAPI> Create();

private:
  static API s_API;
};
} // namespace MyEngine
