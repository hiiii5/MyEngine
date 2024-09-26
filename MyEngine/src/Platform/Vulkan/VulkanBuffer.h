#pragma once

#include "MyEngine/Renderer/Buffer.h"

#include <vulkan/vulkan_core.h>

namespace MyEngine {
class VulkanVertexBuffer : public VertexBuffer {
public:
  VulkanVertexBuffer(uint32_t size);
  VulkanVertexBuffer(Vertex *vertices, uint32_t size);
  virtual ~VulkanVertexBuffer();

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual void SetData(const void *data, uint32_t size) override;

  virtual const BufferLayout &GetLayout() const override { return m_Layout; }
  virtual void SetLayout(const BufferLayout &layout) override {
    m_Layout = layout;
  }

private:
  VkBuffer m_Buffer;
  VkDeviceMemory m_BufferMemory;
  BufferLayout m_Layout;
};

class VulkanIndexBuffer : public IndexBuffer {
public:
  VulkanIndexBuffer(uint32_t *indices, uint32_t count);
  virtual ~VulkanIndexBuffer();

  virtual void Bind() const override;
  virtual void Unbind() const override;

  virtual uint32_t GetCount() const override { return m_Count; }

private:
  VkBuffer m_Buffer;
  VkDeviceMemory m_BufferMemory;
  uint32_t m_Count;
};

class VulkanBufferHelper {
public:
  static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice,
                                 uint32_t typeFilter,
                                 VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) &&
          (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
        return i;
      }
    }

    ME_CORE_ASSERT(false, "Unable to find suitable memory type!");
    return 0;
  }

  static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device,
                           VkDeviceSize size, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer &buffer,
                           VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult res = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    ME_CORE_ASSERT(res == VK_SUCCESS, "Unable to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = FindMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties);

    res = vkAllocateMemory(device, &memAllocInfo, nullptr, &bufferMemory);
    ME_CORE_ASSERT(res == VK_SUCCESS, "Unable to allocate memory for buffer!");
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
  }

  static void CopyBuffer(VkDevice device, VkCommandPool commandPool,
                         VkQueue graphicsQueue, VkBuffer srcBuffer,
                         VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }
};

} // namespace MyEngine
