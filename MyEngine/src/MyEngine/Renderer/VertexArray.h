#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Renderer/Buffer.h"

namespace MyEngine {
class VertexArray {
public:
  virtual ~VertexArray() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
  virtual void Draw() const = 0;

  virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) = 0;
  virtual void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) = 0;

  virtual const std::vector<Ref<VertexBuffer>> &GetVertexBuffers() const = 0;
  virtual const Ref<IndexBuffer> &GetIndexBuffer() const = 0;

  static Ref<VertexArray> Create();
};
} // namespace MyEngine
