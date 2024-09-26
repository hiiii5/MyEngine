#pragma once

#include "MyEngine/Core/Base.h"

#include "MyEngine/Renderer/Shader.h"
#include "MyEngine/Renderer/ShaderModule.h"
#include <vulkan/vulkan_core.h>

namespace MyEngine {
class VulkanShader : public Shader {
public:
  VulkanShader(const std::vector<Ref<ShaderModule>> modules);
  virtual ~VulkanShader() override;
  virtual void Bind() override;

private:
  std::vector<Ref<ShaderModule>> m_Modules;
  VkPipeline m_ShaderPipeline;
  VkPipelineLayout m_PipelineLayout;
};
} // namespace MyEngine
