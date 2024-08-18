extern MyEngine::Application *MyEngine::CreateApplication();

int main(int argc, char *argv[]) {
  auto app = MyEngine::CreateApplication();
  app->Run();
  delete app;
  return 0;
}
