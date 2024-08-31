#include "mepch.h"

#include "MyEngine/Core/Application.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"

#include <vulkan/vulkan_core.h>

namespace MyEngine {
// +===============+
// | VERTEX BUFFER |
// +===============+
VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size) {}

VulkanVertexBuffer::VulkanVertexBuffer(float *vertices, uint32_t size) {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  VkDeviceSize bufferSize = sizeof(float) * size;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VulkanBufferHelper::CreateBuffer(context->PhysicalDevice,
                                   context->LogicalDevice, bufferSize,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(context->LogicalDevice, stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, vertices, (unsigned long long)bufferSize);
  vkUnmapMemory(context->LogicalDevice, stagingBufferMemory);

  VulkanBufferHelper::CreateBuffer(
      context->PhysicalDevice, context->LogicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

  VulkanBufferHelper::CopyBuffer(
      context->LogicalDevice, context->Window.GetCurrentFrame()->CommandPool,
      context->Queue, stagingBuffer, m_Buffer, bufferSize);
  vkDestroyBuffer(context->LogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(context->LogicalDevice, stagingBufferMemory, nullptr);

  VulkanBufferHelper::BindBuffer(context->LogicalDevice, m_Buffer,
                                 m_BufferMemory);
}

VulkanVertexBuffer::~VulkanVertexBuffer() {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkDestroyBuffer(context->LogicalDevice, m_Buffer, nullptr);
  vkFreeMemory(context->LogicalDevice, m_BufferMemory, nullptr);
}

void VulkanVertexBuffer::Bind() const {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkCmdBindVertexBuffers(context->Window.GetCurrentFrame()->CommandBuffer, 0, 1,
                         &m_Buffer, 0);
}

void VulkanVertexBuffer::Unbind() const {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkCmdBindVertexBuffers(context->Window.GetCurrentFrame()->CommandBuffer, 0, 1,
                         VK_NULL_HANDLE, 0);
}

void VulkanVertexBuffer::SetData(const void *data, uint32_t size) {}

// +==============+
// | INDEX BUFFER |
// +==============+
VulkanIndexBuffer::VulkanIndexBuffer(uint32_t *indices, uint32_t count) {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  VkDeviceSize bufferSize = sizeof(uint32_t) * count;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VulkanBufferHelper::CreateBuffer(context->PhysicalDevice,
                                   context->LogicalDevice, bufferSize,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(context->LogicalDevice, stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, indices, (unsigned long long)bufferSize);
  vkUnmapMemory(context->LogicalDevice, stagingBufferMemory);

  VulkanBufferHelper::CreateBuffer(
      context->PhysicalDevice, context->LogicalDevice, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);
  VulkanBufferHelper::CopyBuffer(
      context->LogicalDevice, context->Window.GetCurrentFrame()->CommandPool,
      context->Queue, stagingBuffer, m_Buffer, bufferSize);
  vkDestroyBuffer(context->LogicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(context->LogicalDevice, stagingBufferMemory, nullptr);
}

VulkanIndexBuffer::~VulkanIndexBuffer() {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkDestroyBuffer(context->LogicalDevice, m_Buffer, nullptr);
  vkFreeMemory(context->LogicalDevice, m_BufferMemory, nullptr);
}

void VulkanIndexBuffer::Bind() {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkCmdBindIndexBuffer(context->Window.GetCurrentFrame()->CommandBuffer,
                       m_Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanIndexBuffer::Unbind() {
  VulkanContext *context = static_cast<VulkanContext *>(
      Application::Get().GetWindow().GetGraphicsContext());

  vkCmdBindIndexBuffer(context->Window.GetCurrentFrame()->CommandBuffer,
                       VK_NULL_HANDLE, 0, VK_INDEX_TYPE_UINT32);
}
} // namespace MyEngine
