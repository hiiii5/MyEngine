#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Filesystem/Filesystem.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShaderStage.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace MyEngine {
VulkanShaderStage::VulkanShaderStage(const std::string &filepath,
                                     ShaderStage::StageType type) {
  CompileOrLoadFromCache(filepath, type);

  VkShaderModuleCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = m_SPIRV.size() * sizeof(uint32_t);
  shaderCreateInfo.pCode = m_SPIRV.data();

  VulkanContext *context =
      Application::Get().GetGraphicsContext<VulkanContext>();
  VkResult res =
      vkCreateShaderModule(context->LogicalDevice, &shaderCreateInfo,
                           context->AllocationCallback, &m_ShaderModule);
  ME_CORE_ASSERT(res == VK_SUCCESS, "Unable to create shader VkShaderModule!");

  m_StageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  m_StageInfo.module = m_ShaderModule;
  m_StageInfo.pName = "main";

  switch (type) {
  case ShaderStage::Vertex: {
    m_StageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  } break;
  case ShaderStage::Fragment: {
    m_StageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  } break;
  }
}

VulkanShaderStage::~VulkanShaderStage() {}

ShaderStage::StageType VulkanShaderStage::GetType() const { return m_Type; }

static shaderc_shader_kind StageTypeToShaderC(ShaderStage::StageType type) {
  switch (type) {
  case ShaderStage::Vertex:
    return shaderc_glsl_vertex_shader;
  case ShaderStage::Fragment:
    return shaderc_glsl_fragment_shader;
  }

  ME_CORE_ASSERT(false);
  return (shaderc_shader_kind)0;
}

void VulkanShaderStage::CompileOrLoadFromCache(const std::string &filepath,
                                               StageType type) {
  const std::string cachePath = GetCachePath(filepath, type);

  if (Filesystem::Exists(cachePath)) {
    ME_CORE_INFO("Loading shader stage {0} from cache",
                 Filesystem::GetFilename(filepath));
    if (Filesystem::ReadSpvFile(cachePath, &m_SPIRV) ==
        Filesystem::READ_SUCCESS) {
      return;
    }
    ME_CORE_ERROR("Unable to read shader stage from cache, attempting to "
                  "compile from source");
  }

  std::string glsl;
  if (Filesystem::ReadFile(filepath, &glsl) != Filesystem::READ_SUCCESS) {
    ME_CORE_ERROR("Unable to read shader stage source file");
    ME_CORE_ASSERT(false);
  }

  const std::string preprocessed =
      PreProcess(Filesystem::GetFilename(filepath), type, glsl);

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  options.SetTargetEnvironment(shaderc_target_env_vulkan,
                               shaderc_env_version_vulkan_1_3);
#ifndef ME_DEBUG
  options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

  shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
      preprocessed, StageTypeToShaderC(type), filepath.c_str(), options);
  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    ME_CORE_ERROR(module.GetErrorMessage());
    ME_CORE_ASSERT(false);
  }

  m_SPIRV = std::vector<uint32_t>(module.cbegin(), module.cend());

  if (Filesystem::WriteSpvFile(cachePath, m_SPIRV) !=
      Filesystem::WRITE_SUCCESS) {
    ME_CORE_ERROR("Unable to write shader stage spv binary to file");
    ME_CORE_ASSERT(false);
  }
}

std::string VulkanShaderStage::PreProcess(const std::string &fileName,
                                          ShaderStage::StageType type,
                                          const std::string &source) {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  options.SetTargetEnvironment(shaderc_target_env_vulkan,
                               shaderc_env_version_vulkan_1_0);

  shaderc_shader_kind kind = StageTypeToShaderC(type);

  shaderc::PreprocessedSourceCompilationResult result =
      compiler.PreprocessGlsl(source, kind, fileName.c_str(), options);
  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    ME_CORE_ERROR("Unable to preprocess shader stage", "file", fileName);
    ME_CORE_ASSERT(false);
    return "";
  }

  return {result.cbegin(), result.cend()};
}

std::string VulkanShaderStage::GetCachePath(const std::string &filepath,
                                            ShaderStage::StageType type) {
  return Filesystem::GetCacheDirectory() + "/shaders/vulkan/" + "cached." +
         Filesystem::GetFilename(filepath) + ".spv";
}
} // namespace MyEngine
