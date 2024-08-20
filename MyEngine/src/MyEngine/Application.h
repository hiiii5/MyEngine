#pragma once

#include "Common.hpp"

namespace MyEngine {
class EXPORTED Application {
public:
  Application();
  virtual ~Application();

  void Run();
};

Application *CreateApplication();
} // namespace MyEngine
