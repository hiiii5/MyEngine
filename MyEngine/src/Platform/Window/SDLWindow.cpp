#include "mepch.h"

#include "Platform/Window/SDLWindow.h"

#include "MyEngine/Events/ApplicationEvent.h"
#include "MyEngine/Events/KeyEvent.h"
#include "MyEngine/Events/MouseEvent.h"
#include "MyEngine/Renderer/GraphicsContext.h"
#include "MyEngine/Renderer/Renderer.h"

// TODO: Import graphics context

namespace MyEngine {
static uint8_t s_SDLWindowCount = 0;

SDLWindow::SDLWindow(const WindowProperties &properties) {}

SDLWindow::~SDLWindow() { Shutdown(); }

static int windowEventCallback(void *pData, SDL_Event *e) {
  SDLWindow::WindowData data = *static_cast<SDLWindow::WindowData *>(pData);

  switch (e->type) {
  case SDL_WINDOWEVENT: {
    SDL_Window *win = SDL_GetWindowFromID(e->window.windowID);

    if (e->window.event == SDL_WINDOWEVENT_RESIZED) {

      int width = 0, height = 0;
      SDL_GetWindowSize(win, &width, &height);
      data.Width = width;
      data.Height = height;

      WindowResizeEvent event(width, height);
      data.EventCallback(event, (void *)e);
    }

    if (e->window.event == SDL_WINDOWEVENT_CLOSE) {
      WindowCloseEvent event;
      ME_CORE_INFO("Closing window");
      data.EventCallback(event, (void *)e);
    }
  }
  case SDL_KEYDOWN: {
    Key::KeyCode code = (Key::KeyCode)e->key.keysym.sym;

    if (e->key.repeat == 0) {
      KeyPressedEvent keyPressEvent(code, false);
      data.EventCallback(keyPressEvent, (void *)e);
    } else {
      KeyPressedEvent keyPressEvent(code, true);
      data.EventCallback(keyPressEvent, (void *)e);
    }
    break;
  }
  case SDL_KEYUP: {
    KeyReleasedEvent keyReleaseEvent((Key::KeyCode)e->key.keysym.sym);
    data.EventCallback(keyReleaseEvent, (void *)e);
    break;
  }
  case SDL_MOUSEBUTTONDOWN: {
    MouseButtonPressedEvent mouseDownEvent(e->button.button);
    data.EventCallback(mouseDownEvent, (void *)e);
    break;
  }
  case SDL_MOUSEBUTTONUP: {
    MouseButtonReleasedEvent mouseUpEvent(e->button.button);
    data.EventCallback(mouseUpEvent, (void *)e);
    break;
  }
  case SDL_MOUSEWHEEL: {
    MouseScrolledEvent scrollEvent(e->wheel.preciseX, e->wheel.preciseY);
    data.EventCallback(scrollEvent, (void *)e);
    break;
  }
  case SDL_MOUSEMOTION: {
    MouseMovedEvent moveEvent((float)e->motion.x, (float)e->motion.y);
    data.EventCallback(moveEvent, (void *)e);
    break;
  }
  }

  return 0;
}

void SDLWindow::Init(const WindowProperties &properties) {
  m_Data.Title = properties.Title;
  m_Data.Width = properties.Width;
  m_Data.Height = properties.Height;

  ME_CORE_INFO("Creating window {0} ({1}, {2})", properties.Title,
               properties.Width, properties.Height);

  if (s_SDLWindowCount == 0) {
    // Init SDL for window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      ME_CORE_CRITICAL("Failed to initialize SDL for SDLWindow: {0}",
                       SDL_GetError());
    }

    // TODO: Add api switch
    SDL_WindowFlags windowFlags =
        (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    m_Window = SDL_CreateWindow("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, (int)m_Data.Width,
                                (int)m_Data.Height, windowFlags);

    s_SDLWindowCount++;

    m_GraphicsContext = GraphicsContext::Create();

    SDL_AddEventWatch(windowEventCallback, &m_Data);
  }
}

void SDLWindow::Shutdown() {
  SDL_DestroyWindow(m_Window);
  --s_SDLWindowCount;

  if (s_SDLWindowCount == 0) {
    SDL_Quit();
  }
}

void SDLWindow::OnUpdate() {
  // TODO: swap context buffer;
  SDL_PumpEvents();

  // m_GraphicsContext->SwapBuffers();
}

void SDLWindow::SetVSync(bool enabled) { m_Data.VSync = enabled; }

bool SDLWindow::IsVsyncEnabled() const { return m_Data.VSync; }
} // namespace MyEngine
