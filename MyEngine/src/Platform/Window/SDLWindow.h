#pragma once

#include "MyEngine/Core/Window.h"

#include "MyEngine/Renderer/GraphicsContext.h"

#include <SDL2/SDL.h>

namespace MyEngine {
class SDLWindow : public Window {
public:
  SDLWindow(const WindowProperties &properties);
  virtual ~SDLWindow();
  virtual void Init(const WindowProperties &properties) override;

  virtual void OnUpdate() override;

  virtual unsigned int GetWidth() const override;
  virtual unsigned int GetHeight() const override;

  virtual bool IsMinimized() const override;

  void SetEventCallback(const EventCallbackFn &callback) override {
    m_Data.EventCallback = callback;
  }

  virtual void SetVSync(bool enabled) override;
  virtual bool IsVsyncEnabled() const override;

  virtual void *GetNativeWindow() const override { return m_Window; }

  virtual GraphicsContext *GetGraphicsContext() const override {
    return m_GraphicsContext.get();
  }

  struct WindowData {
    std::string Title;
    unsigned int Width, Height;
    bool VSync;

    EventCallbackFn EventCallback;
  };

private:
  virtual void Shutdown();

  SDL_Window *m_Window;

  WindowData m_Data;

  Unique<GraphicsContext> m_GraphicsContext;
};
}; // namespace MyEngine
