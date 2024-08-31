#include "mepch.h"

#include "MyEngine/Renderer/Buffer.h"

#include "MyEngine/Renderer/Renderer.h"

namespace MyEngine {
Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
  switch (Renderer::GetAPI()) {

  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}

Ref<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size) {
  switch (Renderer::GetAPI()) {

  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}

Ref<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t count) {
  switch (Renderer::GetAPI()) {

  default: {
    ME_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
  }
}
} // namespace MyEngine
