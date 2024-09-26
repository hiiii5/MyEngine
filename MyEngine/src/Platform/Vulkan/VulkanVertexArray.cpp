#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

#include <vulkan/vulkan.h>

namespace MyEngine {
VulkanVertexArray::VulkanVertexArray() {}
VulkanVertexArray::~VulkanVertexArray() {}

void VulkanVertexArray::Bind() const {
  for (auto &buffer : m_VertexBuffers) {
    buffer->Bind();
  }

  m_IndexBuffer->Bind();
}

void VulkanVertexArray::Unbind() const {}

void VulkanVertexArray::Draw() const {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());
  uint32_t count = m_IndexBuffer->GetCount();
  vkCmdDrawIndexed(context->Window.GetCurrentFrame()->CommandBuffer, count, 1,
                   0, 0, 0);
}

void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) {
  m_VertexBuffers.push_back(vertexBuffer);
}

void VulkanVertexArray::SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) {
  m_IndexBuffer = indexBuffer;
}
} // namespace MyEngine
