#include "mepch.h"

#include "MyEngine/Core/Window.h"
#include "MyEngine/Renderer/RendererAPI.h"
#include "Platform/SDL/SDLWindow.h"

namespace MyEngine {
Unique<Window> Window::Create(const WindowProperties &properties) {
  switch (RendererAPI::GetAPI()) {
  case RendererAPI::API::Vulkan:
    return CreateUnique<SDLWindow>(properties);
    break;
  default:
    ME_CORE_ASSERT(false, "Unknown platform!");
    return nullptr;
  }
}
} // namespace MyEngine
