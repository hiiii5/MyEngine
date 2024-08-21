
#include "Application.h"

namespace MyEngine {
Application::Application() {
  m_Window = Window::Create();
  m_Window->SetEventCallback(ME_BIND_EVENT_FN(Application::OnEvent));
}

Application::~Application() {}

void Application::Run() {
  while (m_Running) {
    m_Window->OnUpdate();
  }
}

void Application::OnEvent(Event &e) {
  EventDispatcher dispatcher(e);

  dispatcher.Dispatch<WindowCloseEvent>(
      ME_BIND_EVENT_FN(Application::OnWindowClose));
  dispatcher.Dispatch<WindowResizeEvent>(
      ME_BIND_EVENT_FN(Application::OnWindowResize));
}

bool Application::OnWindowClose(WindowCloseEvent &e) {
  m_Running = false;
  return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
  // TODO: Renderer resize
  return false;
}
} // namespace MyEngine
