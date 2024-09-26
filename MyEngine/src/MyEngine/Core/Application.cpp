
#include "Application.h"
#include "MyEngine/ImGui/ImGuiLayer.h"
#include "MyEngine/Renderer/Renderer.h"
#include "MyEngine/Time/Time.h"

namespace MyEngine {
Application *Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification &specification) {
  s_Instance = this;

  m_Window = Window::Create();
  m_Window->Init();
  ME_CORE_ASSERT(m_Window != nullptr, "Window is null after creation!");
  m_Window->SetEventCallback(ME_BIND_EVENT_FN(Application::OnEvent));

  Renderer::Init();

  m_ImGuiLayer = new ImGuiLayer();
  PushOverlay(m_ImGuiLayer);
}

Application::~Application() { Shutdown(); }

void Application::Shutdown() {
  if (!m_IsShuttingDown) {
    m_IsShuttingDown = true;
    Renderer::WaitForIdle();
    m_LayerStack.Cleanup();
    Renderer::Shutdown();
  }
}

void Application::PushLayer(Layer *layer) {
  m_LayerStack.PushLayer(layer);
  layer->OnAttach();
}

void Application::PushOverlay(Layer *layer) {
  m_LayerStack.PushOverlay(layer);
  layer->OnAttach();
}

void Application::Run() {
  while (m_Running) {
    unsigned long milliseconds = Time::GetTime();
    Timestep timestep = milliseconds - m_LastFrameTime;
    m_LastFrameTime = milliseconds;

    if (m_Window->IsMinimized()) {
      continue;
    }

    if (Renderer::BeginFrame()) {
      for (Layer *layer : m_LayerStack) {
        layer->OnUpdate(timestep);
      }

      m_ImGuiLayer->Begin();
      for (Layer *layer : m_LayerStack) {
        layer->OnImGuiRender();
      }
      m_ImGuiLayer->End();

      Renderer::EndFrame();
      Renderer::PresentFrame();
    }

    m_Window->OnUpdate();
  }
}

void Application::OnEvent(Event &e, void *pData) {
  EventDispatcher dispatcher(e);

  dispatcher.Dispatch<WindowCloseEvent>(
      ME_BIND_EVENT_FN(Application::OnWindowClose));
  dispatcher.Dispatch<WindowResizeEvent>(
      ME_BIND_EVENT_FN(Application::OnWindowResize));

  for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
    if (e.Handled) {
      break;
    }
    (*it)->OnEvent(e, pData);
  }
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
