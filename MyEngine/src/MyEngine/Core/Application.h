#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Core/LayerStack.h"
#include "MyEngine/Core/Window.h"
#include "MyEngine/Events/ApplicationEvent.h"
#include "MyEngine/ImGui/ImGuiLayer.h"

int main(int argc, char **argv);

namespace MyEngine {
struct ApplicationSpecification {
  std::string Name = "MyEngine Application";
};

class MYENGINE_API Application {
public:
  Application(const ApplicationSpecification &specification);
  virtual ~Application();

  void PushLayer(Layer *layer);
  void PushOverlay(Layer *layer);

  Window &GetWindow() { return *m_Window; }

  static Application &Get() { return *s_Instance; }

  const ApplicationSpecification &GetSpecification() const {
    return m_Specification;
  }

private:
  void Run();
  void OnEvent(Event &e, void *pData);

  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);

  Unique<Window> m_Window;
  ImGuiLayer *m_ImGuiLayer;
  ApplicationSpecification m_Specification;
  bool m_Running = true;
  LayerStack m_LayerStack;

  unsigned long m_LastFrameTime = 0.0f;

  static Application *s_Instance;

  friend int ::main(int argc, char *argv[]);
};

Application *CreateApplication();
} // namespace MyEngine
