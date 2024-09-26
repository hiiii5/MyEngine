#include "mepch.h"

#include "MyEngine/Renderer/VertexArray.h"

#include "MyEngine/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace MyEngine {
Ref<VertexArray> VertexArray::Create() {
  switch (Renderer::GetAPI()) {
  case RendererAPI::API::None: {
    ME_CORE_ASSERT(false,
                   "RendererAPI set to none when making a vertex array!");
    return nullptr;
  }
  case RendererAPI::API::Vulkan: {
    return CreateRef<VulkanVertexArray>();
  } break;
  }

  ME_CORE_ASSERT(false, "Unknown RendererAPI when making a vertex array!");
  return nullptr;
}
} // namespace MyEngine
