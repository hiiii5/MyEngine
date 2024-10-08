#pragma once

#include "MyEngine/Renderer/RendererAPI.h"
#include "MyEngine/Renderer/Shader.h"
#include "MyEngine/Renderer/VertexArray.h"

namespace MyEngine {
class Renderer {
public:
  static void Init();
  static void Shutdown();
  static void Update();
  static bool BeginFrame();
  static void EndFrame();
  static void PresentFrame();
  static void WaitForIdle();

  static void Submit(const Ref<Shader> &shader,
                     const Ref<VertexArray> &vertexArray);

  static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
};
} // namespace MyEngine
