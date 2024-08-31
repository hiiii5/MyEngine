#pragma once

#include "MyEngine/Renderer/RendererAPI.h"

namespace MyEngine {
class Renderer {
public:
  static void Init();
  static void Shutdown();
  static void BeginFrame();
  static void EndFrame();
  static void PresentFrame();
  static void WaitForIdle();

  static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
};
} // namespace MyEngine
