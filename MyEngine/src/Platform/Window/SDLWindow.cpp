#include "mepch.h"

#include "Platform/Window/SDLWindow.h"

#include "MyEngine/Events/ApplicationEvent.h"
#include "MyEngine/Events/KeyEvent.h"
#include "MyEngine/Events/MouseEvent.h"

// TODO: Import graphics context

namespace MyEngine {
static uint8_t s_SDLWindowCount = 0;

SDLWindow::SDLWindow(const WindowProperties &properties) { Init(properties); }

SDLWindow::~SDLWindow() { Shutdown(); }

static int windowEventCallback(void *pData, SDL_Event *event) {
  SDLWindow::WindowData data = *static_cast<SDLWindow::WindowData *>(pData);

  if (event->type == SDL_WINDOWEVENT) {
    SDL_Window *win = SDL_GetWindowFromID(event->window.windowID);

    if (event->window.event == SDL_WINDOWEVENT_RESIZED) {

      int width = 0, height = 0;
      SDL_GetWindowSize(win, &width, &height);
      data.Width = width;
      data.Height = height;

      WindowResizeEvent event(width, height);
      data.EventCallback(event);
    }

    if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
      WindowCloseEvent event;
      ME_CORE_INFO("Closing window");
      data.EventCallback(event);
    }
  }

  switch (event->type) {
  case SDL_KEYDOWN: {
    Key::KeyCode code = (Key::KeyCode)event->key.keysym.sym;

    if (event->key.repeat == 0) {
      KeyPressedEvent keyPressEvent(code, false);
      data.EventCallback(keyPressEvent);
    } else {
      KeyPressedEvent keyPressEvent(code, true);
      data.EventCallback(keyPressEvent);
    }
    break;
  }
  case SDL_KEYUP: {
    KeyReleasedEvent keyReleaseEvent((Key::KeyCode)event->key.keysym.sym);
    data.EventCallback(keyReleaseEvent);
    break;
  }
  case SDL_MOUSEBUTTONDOWN: {
    MouseButtonPressedEvent mouseDownEvent(event->button.button);
    data.EventCallback(mouseDownEvent);
    break;
  }
  case SDL_MOUSEBUTTONUP: {
    MouseButtonReleasedEvent mouseUpEvent(event->button.button);
    data.EventCallback(mouseUpEvent);
    break;
  }
  case SDL_MOUSEWHEEL: {
    MouseScrolledEvent scrollEvent(event->wheel.preciseX,
                                   event->wheel.preciseY);
    data.EventCallback(scrollEvent);
    break;
  }
  case SDL_MOUSEMOTION: {
    MouseMovedEvent moveEvent((float)event->motion.x, (float)event->motion.y);
    data.EventCallback(moveEvent);
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
}

void SDLWindow::SetVSync(bool enabled) { m_Data.VSync = enabled; }

bool SDLWindow::IsVsyncEnabled() const { return m_Data.VSync; }
} // namespace MyEngine
