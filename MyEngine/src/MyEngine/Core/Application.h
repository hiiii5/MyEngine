#pragma once

#include "MyEngine/Core/Base.h"

namespace MyEngine {
class EXPORTED Application {
public:
  Application();
  virtual ~Application();

  void Run();
};

Application *CreateApplication();
} // namespace MyEngine
