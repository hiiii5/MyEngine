#include <MyEngine.h>
#include <MyEngine/Core/EntryPoint.h>

class Sandbox : public MyEngine::Application {
public:
  Sandbox() {}
  ~Sandbox() {}
};

MyEngine::Application *MyEngine::CreateApplication() { return new Sandbox(); }
