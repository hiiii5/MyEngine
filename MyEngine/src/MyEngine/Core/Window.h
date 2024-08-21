#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Events/Event.h"

namespace MyEngine {
struct WindowProperties {
  std::string Title;
  uint32_t Width;
  uint32_t Height;

  WindowProperties(const std::string &title = "My Engine",
                   uint32_t width = 1600, uint32_t height = 900)
      : Title(title), Width(width), Height(height) {}
};

class Window {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  virtual ~Window() = default;

  virtual void OnUpdate() = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  virtual void SetEventCallback(const EventCallbackFn &callback) = 0;
  virtual void SetVSync(bool enabled) = 0;
  virtual bool IsVsyncEnabled() const = 0;

  virtual void *GetNativeWindow() const = 0;

  static Unique<Window>
  Create(const WindowProperties &properties = WindowProperties());
};
} // namespace MyEngine
