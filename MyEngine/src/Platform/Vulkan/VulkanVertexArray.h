#pragma once

#include "MyEngine/Renderer/VertexArray.h"

namespace MyEngine {
class VulkanVertexArray : public VertexArray {
public:
  VulkanVertexArray();
  virtual ~VulkanVertexArray();

  virtual void Bind() const override;
  virtual void Unbind() const override;
  virtual void Draw() const override;

  virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) override;
  virtual void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) override;

  virtual const std::vector<Ref<VertexBuffer>> &
  GetVertexBuffers() const override {
    return m_VertexBuffers;
  }
  virtual const Ref<IndexBuffer> &GetIndexBuffer() const override {
    return m_IndexBuffer;
  }

private:
  uint32_t m_VertexBufferIndex = 0;
  std::vector<Ref<VertexBuffer>> m_VertexBuffers;
  Ref<IndexBuffer> m_IndexBuffer;
};
} // namespace MyEngine
