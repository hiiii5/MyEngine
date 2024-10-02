#pragma once

#include "MyEngine/Core/Base.h"

#include "MyEngine/Renderer/Shader.h"
#include "MyEngine/Renderer/ShaderStage.h"
#include <vulkan/vulkan_core.h>

namespace MyEngine {
class VulkanShader : public Shader {
public:
  VulkanShader(const std::string &name,
               const std::vector<Ref<ShaderStage>> stages);
  virtual ~VulkanShader() override;
  virtual void Bind() override;

private:
  std::string m_Name;
  std::vector<Ref<ShaderStage>> m_Stages;
  VkPipeline m_ShaderPipeline;
  VkPipelineLayout m_PipelineLayout;
};
} // namespace MyEngine
