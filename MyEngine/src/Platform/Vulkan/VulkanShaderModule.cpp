#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Filesystem/Filesystem.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShaderModule.h"

namespace MyEngine {
VulkanShaderModule::VulkanShaderModule(const std::string &filepath,
                                       ShaderType type) {
  std::string shaderModuleCode;
  Filesystem::FsReadStatus readStatus =
      Filesystem::ReadFile(filepath, &shaderModuleCode);
  ME_CORE_ASSERT(readStatus == Filesystem::SUCCESS,
                 "Unable to read shader file.");

  VkShaderModuleCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = shaderModuleCode.size();
  shaderCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(shaderModuleCode.data());

  VulkanContext *context =
      Application::Get().GetGraphicsContext<VulkanContext>();
  VkResult res = vkCreateShaderModule(context->LogicalDevice, &shaderCreateInfo,
                                      nullptr, &m_ShaderModule);
  ME_CORE_ASSERT(res == VK_SUCCESS, "Unable to create shader module!");

  m_StageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  m_StageInfo.module = m_ShaderModule;
  m_StageInfo.pName = "main";

  switch (type) {
  case ShaderModule::Vertex: {
    m_StageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  } break;
  case ShaderModule::Fragment: {
    m_StageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  } break;
  }
}

VulkanShaderModule::~VulkanShaderModule() {}

ShaderModule::ShaderType VulkanShaderModule::GetType() const { return m_Type; }
} // namespace MyEngine
