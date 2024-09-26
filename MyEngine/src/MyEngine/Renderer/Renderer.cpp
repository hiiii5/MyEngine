#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Renderer/RenderCommand.h"
#include "MyEngine/Renderer/Renderer.h"

namespace MyEngine {
void Renderer::Init() { RenderCommand::Init(); }

void Renderer::Shutdown() { RenderCommand::Shutdown(); }

void Renderer::Update() {
  Application &app = Application::Get();
  uint32_t width = app.GetWindow().GetWidth();
  uint32_t height = app.GetWindow().GetHeight();
  RenderCommand::SetViewport(0, 0, width, height);
}

bool Renderer::BeginFrame() {
  return RenderCommand::BeginFrame(
      Application::Get().GetWindow().GetGraphicsContext());
}

void Renderer::EndFrame() {
  RenderCommand::EndFrame(Application::Get().GetWindow().GetGraphicsContext());
}

void Renderer::PresentFrame() {
  RenderCommand::PresentFrame(
      Application::Get().GetWindow().GetGraphicsContext());
}

void Renderer::WaitForIdle() { RenderCommand::WaitForIdle(); }

void Renderer::Submit(const Ref<VertexArray> &vertexArray) {
  vertexArray->Bind();
  RenderCommand::DrawIndexed(vertexArray);
}

} // namespace MyEngine
