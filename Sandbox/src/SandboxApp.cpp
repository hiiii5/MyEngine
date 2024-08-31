#include "MyEngine/Core/Application.h"
#include <MyEngine.h>
#include <MyEngine/Core/EntryPoint.h>

class Sandbox : public MyEngine::Application {
public:
  Sandbox(const MyEngine::ApplicationSpecification &specification)
      : MyEngine::Application(specification) {}

  ~Sandbox() {}
};

MyEngine::Application *MyEngine::CreateApplication() {
  ApplicationSpecification spec;
  spec.Name = "Sandbox";

  return new Sandbox(spec);
}
