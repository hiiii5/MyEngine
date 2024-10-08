#include "ExampleLayer.h"

#include "imgui.h"

using namespace MyEngine;

ExampleLayer::ExampleLayer() {
  m_VertexArray = VertexArray::Create();

  m_Vertices = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}};

  Ref<VertexBuffer> vertexBuffer =
      VertexBuffer::Create(m_Vertices.data(), m_Vertices.size());
  BufferLayout layout = {{ShaderDataType::Float3, "a_position"},
                         {ShaderDataType::Float4, "a_color"}};

  vertexBuffer->SetLayout(layout);
  m_VertexArray->AddVertexBuffer(vertexBuffer);

  m_Indices = {0, 1, 2, 2, 3, 0};
  Ref<MyEngine::IndexBuffer> indexBuffer =
      IndexBuffer::Create(m_Indices.data(), m_Indices.size());
  m_VertexArray->SetIndexBuffer(indexBuffer);

  Ref<MyEngine::ShaderStage> vertModule =
      ShaderStage::Create("shaders/vertexColor.vert.glsl", ShaderStage::Vertex);
  Ref<MyEngine::ShaderStage> fragModule = ShaderStage::Create(
      "shaders/vertexColor.frag.glsl", ShaderStage::Fragment);

  std::vector<Ref<MyEngine::ShaderStage>> modules{vertModule, fragModule};
  m_Shader = Shader::Create("VertexColorShader", modules);
}

void ExampleLayer::OnAttach() {}

void ExampleLayer::OnDetach() {}

void ExampleLayer::OnUpdate(Timestep ts) {
  Renderer::Submit(m_Shader, m_VertexArray);
}

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
  ImGui::End();
  ImGui::PopStyleVar(3);

  // bool showDemo = true;
  // ImGui::ShowDemoWindow(&showDemo);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Menu")) {
      if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "CTRL+Z")) {
        }
        if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
        } // Disabled item
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "CTRL+X")) {
        }
        if (ImGui::MenuItem("Copy", "CTRL+C")) {
        }
        if (ImGui::MenuItem("Paste", "CTRL+V")) {
        }
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Alt+F4")) {
        ImGui::EndMenu();
        ImGui::End();
        Application::Get().Shutdown();
        return;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar(2);

  bool showTriangleEditMenu;
  if (ImGui::Begin("Color Editor", &showTriangleEditMenu)) {
    for (int i = 0; i < m_Vertices.size(); i++) {
      std::string label = "Color" + std::to_string(i);
      ImGui::ColorEdit4(label.c_str(), &m_Vertices[i].Color.x,
                        ImGuiColorEditFlags_NoAlpha);
    }
    m_VertexArray->GetVertexBuffers()[0]->SetData(m_Vertices.data(),
                                                  m_Vertices.size());
  }
  ImGui::End();
}

void ExampleLayer::OnEvent(Event &e, void *pData) {}
