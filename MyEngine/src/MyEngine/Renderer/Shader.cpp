#include "mepch.h"

#include "MyEngine/Renderer/RendererAPI.h"
#include "MyEngine/Renderer/Shader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace MyEngine {
Ref<Shader> Shader::Create(const std::string &name,
                           const std::vector<Ref<ShaderStage>> modules) {
  switch (RendererAPI::GetAPI()) {
  case RendererAPI::API::Vulkan: {
    return CreateRef<VulkanShader>(name, modules);
  }

  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}
} // namespace MyEngine
