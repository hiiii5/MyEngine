#pragma once

#include "Application.h"
#include "Log.h"

extern MyEngine::Application *MyEngine::CreateApplication();

int main(int argc, char *argv[]) {
  MyEngine::Log::Init();
  ME_CORE_WARN("Initialized log!");
  ME_INFO("Hello!");

  auto app = MyEngine::CreateApplication();
  app->Run();
  delete app;
  return 0;
}
