#pragma once

#include "MyEngine/Core/Application.h"
#include "MyEngine/Core/Base.h"

#ifdef ME_PLATFORM_LINUX

extern MyEngine::Application *MyEngine::CreateApplication();

int main(int argc, char *argv[]) {
  MyEngine::Log::Init();

  auto app = MyEngine::CreateApplication();
  app->Run();
  delete app;

  return 0;
}

#endif
