#pragma once

#include "MyEngine/Renderer/ShaderStage.h"

#include <vulkan/vulkan.h>

namespace MyEngine {
class VulkanShaderStage : public ShaderStage {
public:
  VulkanShaderStage(const std::string &filepath, StageType type);
  virtual ~VulkanShaderStage() override;

  virtual StageType GetType() const override;

  VkShaderModule GetShaderModule() const { return m_ShaderModule; }
  VkPipelineShaderStageCreateInfo GetStageInfo() const { return m_StageInfo; }

private:
  void CompileOrLoadFromCache(const std::string &filepath, StageType type);
  std::string PreProcess(const std::string &fileName, StageType type,
                         const std::string &source);
  std::string GetCachePath(const std::string &filepath, StageType type);

  VkShaderModule m_ShaderModule;
  VkPipelineShaderStageCreateInfo m_StageInfo{};
  StageType m_Type;
  std::vector<uint32_t> m_SPIRV;
};
} // namespace MyEngine
