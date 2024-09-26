#include "mepch.h"

#include "MyEngine/Renderer/RendererAPI.h"
#include "MyEngine/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace MyEngine {
Ref<Shader> Shader::Create(const std::vector<Ref<ShaderModule>> modules) {
  switch (RendererAPI::GetAPI()) {
  case RendererAPI::API::Vulkan: {
    return CreateRef<VulkanShader>(modules);
  }

  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}
} // namespace MyEngine
