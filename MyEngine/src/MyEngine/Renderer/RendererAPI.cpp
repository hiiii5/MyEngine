#include "mepch.h"

#include "MyEngine/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace MyEngine {
RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

Unique<RendererAPI> RendererAPI::Create() {
  switch (s_API) {
  case RendererAPI::API::None: {
    ME_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
    return nullptr;
  }
  case RendererAPI::API::Vulkan: {
    return CreateUnique<VulkanRendererAPI>();
  }
  default:
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
}
} // namespace MyEngine
