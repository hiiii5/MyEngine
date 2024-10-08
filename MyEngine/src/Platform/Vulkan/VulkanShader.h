#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Math/Math.h"
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

  /* virtual void SetInt(const std::string &name, int value) override;
  virtual void SetIntArray(const std::string &name, int *value,
                           uint32_t count) override;
  virtual void SetFloat(const std::string &name, float value) override;
  virtual void SetFloat2(const std::string &name,
                         const Vector2 &value) override;
  virtual void SetFloat3(const std::string &name,
                         const Vector3 &value) override;
  virtual void SetFloat4(const std::string &name,
                         const Vector4 &value) override;
  virtual void SetMat4(const std::string &name, const Matrix4 &value) override;
*/

private:
  std::string m_Name;
  std::vector<Ref<ShaderStage>> m_Stages;
  VkPipeline m_ShaderPipeline;
  VkPipelineLayout m_PipelineLayout;
};
} // namespace MyEngine
