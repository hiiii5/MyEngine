#pragma once

#include "MyEngine/Renderer/GraphicsContext.h"
#include "MyEngine/Renderer/RendererAPI.h"

namespace MyEngine {
class RenderCommand {
public:
  static void Init() { s_RendererAPI->Init(); }
  static void Shutdown() { s_RendererAPI->Shutdown(); }

  static void BeginFrame(GraphicsContext *ctx) {
    s_RendererAPI->BeginFrame(ctx);
  }

  static void EndFrame(GraphicsContext *ctx) { s_RendererAPI->EndFrame(ctx); }

  static void PresentFrame(GraphicsContext *ctx) {
    s_RendererAPI->PresentFrame(ctx);
  }

  static void WaitForIdle() { s_RendererAPI->WaitForIdle(); }

private:
  static Unique<RendererAPI> s_RendererAPI;
};
} // namespace MyEngine
