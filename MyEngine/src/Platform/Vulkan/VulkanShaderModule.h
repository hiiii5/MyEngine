#pragma once

#include "MyEngine/Renderer/ShaderModule.h"

#include <vulkan/vulkan.h>

namespace MyEngine {
class VulkanShaderModule : public ShaderModule {
public:
  VulkanShaderModule(const std::string &filepath, ShaderType type);
  virtual ~VulkanShaderModule() override;

  virtual ShaderType GetType() const override;

  VkShaderModule GetShaderModule() const { return m_ShaderModule; }
  VkPipelineShaderStageCreateInfo GetStageInfo() const { return m_StageInfo; }

private:
  VkShaderModule m_ShaderModule;
  VkPipelineShaderStageCreateInfo m_StageInfo{};
  ShaderType m_Type;
};
} // namespace MyEngine
