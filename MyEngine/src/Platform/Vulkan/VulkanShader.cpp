#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "MyEngine/Renderer/Vertex.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanShaderStage.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace MyEngine {
VulkanShader::VulkanShader(const std::string &name,
                           const std::vector<Ref<ShaderStage>> stages)
    : m_Name(name), m_Stages(std::move(stages)) {
  VulkanContext *context =
      Application::Get().GetGraphicsContext<VulkanContext>();

  VkPipelineShaderStageCreateInfo shaderStages[m_Stages.size()];
  for (int i = 0; i < m_Stages.size(); i++) {
    VulkanShaderStage *stage =
        static_cast<VulkanShaderStage *>(stages[i].get());
    if (stage == nullptr) {
      ME_CORE_ASSERT(false, "Shader module is null!");
    }
    VkPipelineShaderStageCreateInfo info = stage->GetStageInfo();
    shaderStages[i] = info;
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

  // Position
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, Position);
  // Color
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, Color);

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)context->Window.Width;
  viewport.height = (float)context->Window.Height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {(uint32_t)context->Window.Width,
                    (uint32_t)context->Window.Height};

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterState{};
  rasterState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterState.depthClampEnable = VK_FALSE;
  rasterState.rasterizerDiscardEnable = VK_FALSE;
  rasterState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterState.lineWidth = 1.0f;
  rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterState.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  VkResult res =
      vkCreatePipelineLayout(context->LogicalDevice, &pipelineLayoutInfo,
                             context->AllocationCallback, &m_PipelineLayout);
  ME_CORE_ASSERT(res == VK_SUCCESS, "Could not create pipeline layout!");

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = m_Stages.size();
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterState;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = m_PipelineLayout;
  pipelineInfo.renderPass = context->Window.RenderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  res = vkCreateGraphicsPipelines(context->LogicalDevice, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, context->AllocationCallback,
                                  &m_ShaderPipeline);
  ME_CORE_ASSERT(res == VK_SUCCESS, "Unable to create shader pipeline!");
}

VulkanShader::~VulkanShader() {
  VulkanContext *context =
      Application::Get().GetGraphicsContext<VulkanContext>();

  // TODO: Maybe move to the module so that its responsible for both creation
  // and deletion
  for (int i = 0; i < m_Stages.size(); i++) {
    vkDestroyShaderModule(
        context->LogicalDevice,
        static_cast<VulkanShaderStage *>(m_Stages[i].get())->GetShaderModule(),
        nullptr);
  }

  vkDestroyPipelineLayout(context->LogicalDevice, m_PipelineLayout,
                          context->AllocationCallback);
  vkDestroyPipeline(context->LogicalDevice, m_ShaderPipeline,
                    context->AllocationCallback);
}

void VulkanShader::Bind() {
  VulkanContext *context =
      Application::Get().GetGraphicsContext<VulkanContext>();
  vkCmdBindPipeline(context->Window.GetCurrentFrame()->CommandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShaderPipeline);
}

/* void SetInt(const std::string &name, int value) {}
void SetIntArray(const std::string &name, int *value, uint32_t count) {}
void SetFloat(const std::string &name, float value) {}
void SetFloat2(const std::string &name, const Vector2 &value) {}
void SetFloat3(const std::string &name, const Vector3 &value) {}
void SetFloat4(const std::string &name, const Vector4 &value) {}
void SetMat4(const std::string &name, const Matrix4 &value) {} */
} // namespace MyEngine
