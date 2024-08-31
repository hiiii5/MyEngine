#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Renderer/RenderCommand.h"
#include "MyEngine/Renderer/Renderer.h"

namespace MyEngine {
void Renderer::Init() { RenderCommand::Init(); }

void Renderer::Shutdown() { RenderCommand::Shutdown(); }

void Renderer::BeginFrame() {
  RenderCommand::BeginFrame(
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
} // namespace MyEngine
