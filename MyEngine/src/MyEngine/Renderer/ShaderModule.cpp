#include "mepch.h"

#include "MyEngine/Renderer/RendererAPI.h"
#include "MyEngine/Renderer/ShaderModule.h"
#include "Platform/Vulkan/VulkanShaderModule.h"

namespace MyEngine {
Ref<ShaderModule> ShaderModule::Create(const std::string &filepath,
                                       ShaderType type) {
  switch (RendererAPI::GetAPI()) {
  case RendererAPI::API::Vulkan: {
    return CreateRef<VulkanShaderModule>(filepath, type);
  } break;
  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}
} // namespace MyEngine
