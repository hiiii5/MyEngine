#pragma once

#include "MyEngine/Core/Window.h"

#include <SDL2/SDL.h>

namespace MyEngine {
class SDLWindow : public Window {
public:
  SDLWindow(const WindowProperties &properties);
  virtual ~SDLWindow();

  void OnUpdate() override;

  unsigned int GetWidth() const override { return m_Data.Width; }
  unsigned int GetHeight() const override { return m_Data.Height; }

  void SetEventCallback(const EventCallbackFn &callback) override {
    m_Data.EventCallback = callback;
  }

  virtual void SetVSync(bool enabled) override;
  virtual bool IsVsyncEnabled() const override;

  virtual void *GetNativeWindow() const override { return m_Window; }

  struct WindowData {
    std::string Title;
    unsigned int Width, Height;
    bool VSync;

    EventCallbackFn EventCallback;
  };

private:
  virtual void Init(const WindowProperties &properties);
  virtual void Shutdown();

  SDL_Window *m_Window;

  WindowData m_Data;
};
}; // namespace MyEngine
