#pragma once

namespace MyEngine {
class GraphicsContext {
public:
  virtual ~GraphicsContext() = default;

  // virtual void Init() = 0;
  // virtual void BeginDraw() = 0;
  // virtual void EndDraw() = 0;
  // virtual void SwapBuffers() = 0;

  static Unique<GraphicsContext> Create();
};
} // namespace MyEngine
