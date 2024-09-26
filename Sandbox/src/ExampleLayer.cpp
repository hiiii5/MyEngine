#include "ExampleLayer.h"

#include "imgui.h"

ExampleLayer::ExampleLayer() {
  m_VertexArray = MyEngine::VertexArray::Create();

  m_Vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

  MyEngine::Ref<MyEngine::VertexBuffer> vertexBuffer =
      MyEngine::VertexBuffer::Create(m_Vertices.data(), m_Vertices.size());
  MyEngine::BufferLayout layout = {
      {MyEngine::ShaderDataType::Float2, "a_position"},
      {MyEngine::ShaderDataType::Float3, "a_color"}};

  vertexBuffer->SetLayout(layout);
  m_VertexArray->AddVertexBuffer(vertexBuffer);

  m_Indices = {0, 1, 2, 2, 3, 0};
  MyEngine::Ref<MyEngine::IndexBuffer> indexBuffer =
      MyEngine::IndexBuffer::Create(m_Indices.data(), m_Indices.size());
  m_VertexArray->SetIndexBuffer(indexBuffer);

  MyEngine::Ref<MyEngine::ShaderModule> vertModule =
      MyEngine::ShaderModule::Create("shaders/shader.vert.spv",
                                     MyEngine::ShaderModule::Vertex);
  MyEngine::Ref<MyEngine::ShaderModule> fragModule =
      MyEngine::ShaderModule::Create("shaders/shader.frag.spv",
                                     MyEngine::ShaderModule::Fragment);

  std::vector<MyEngine::Ref<MyEngine::ShaderModule>> modules{vertModule,
                                                             fragModule};
  m_Shader = MyEngine::Shader::Create(modules);
}

void ExampleLayer::OnAttach() {}

void ExampleLayer::OnDetach() {}

void ExampleLayer::OnUpdate(MyEngine::Timestep ts) {
  m_Shader->Bind();
  MyEngine::Renderer::Submit(m_VertexArray);
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
        // MyEngine::Application::Get().Shutdown();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar(2);

  bool showTriangleEditMenu;
  if (ImGui::Begin("Color Editor", &showTriangleEditMenu)) {
    // ImGui::ColorEdit4("Color0", &m_Vertices[0].Color.x,
    //                   ImGuiColorEditFlags_NoAlpha);
    // ImGui::ColorEdit4("Color1", &m_Vertices[1].Color.x,
    //                   ImGuiColorEditFlags_NoAlpha);
    // ImGui::ColorEdit4("Color2", &m_Vertices[2].Color.x,
    //                   ImGuiColorEditFlags_NoAlpha);
    // ImGui::ColorEdit4("Color3", &m_Vertices[3].Color.x,
    //                   ImGuiColorEditFlags_NoAlpha);
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

void ExampleLayer::OnEvent(MyEngine::Event &e, void *pData) {}
