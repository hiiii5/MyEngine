#include "ExampleLayer.h"

#include "imgui.h"

ExampleLayer::ExampleLayer() {}

void ExampleLayer::OnAttach() {}

void ExampleLayer::OnDetach() {}

void ExampleLayer::OnUpdate(MyEngine::Timestep ts) {}

void ExampleLayer::OnImGuiRender() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

  bool show = true;
  ImGui::Begin("CentralDockspace", &show, windowFlags);
  ImGuiID centralId = ImGui::GetID("Dockspace");
  ImGui::DockSpace(centralId, ImVec2(0.0f, 0.0f),
                   ImGuiDockNodeFlags_PassthruCentralNode);

  bool showDemo = true;
  ImGui::ShowDemoWindow(&showDemo);

  ImGui::End();
  ImGui::PopStyleVar(3);
}

void ExampleLayer::OnEvent(MyEngine::Event &e, void *pData) {}
