#include "mepch.h"

#include "MyEngine/Renderer/RendererAPI.h"
#include "MyEngine/Renderer/ShaderStage.h"
#include "Platform/Vulkan/VulkanShaderStage.h"

namespace MyEngine {
Ref<ShaderStage> ShaderStage::Create(const std::string &filepath,
                                     StageType type) {
  switch (RendererAPI::GetAPI()) {
  case RendererAPI::API::Vulkan: {
    return CreateRef<VulkanShaderStage>(filepath, type);
  } break;
  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}
} // namespace MyEngine
