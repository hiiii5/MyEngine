#include "ExampleLayer.h"

#include "imgui.h"

ExampleLayer::ExampleLayer() {}

void ExampleLayer::OnAttach() {}

void ExampleLayer::OnDetach() {}

void ExampleLayer::OnUpdate(MyEngine::Timestep ts) {}

void ExampleLayer::OnImGuiRender() {
  bool showDemo = true;
  ImGui::ShowDemoWindow(&showDemo);
}

void ExampleLayer::OnEvent(MyEngine::Event &e, void *pData) {}
