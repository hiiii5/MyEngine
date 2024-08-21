#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Core/Window.h"
#include "MyEngine/Events/ApplicationEvent.h"

int main(int argc, char **argv);

namespace MyEngine {
class EXPORTED Application {
public:
  Application();
  virtual ~Application();

  Window &GetWindow() { return *m_Window; }

private:
  void Run();
  void OnEvent(Event &e);

  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);

  Unique<Window> m_Window;
  bool m_Running = true;

  friend int ::main(int argc, char *argv[]);
};

Application *CreateApplication();
} // namespace MyEngine
