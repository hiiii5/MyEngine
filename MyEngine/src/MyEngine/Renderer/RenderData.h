#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Renderer/Buffer.h"
#include <glm/glm.hpp>

namespace MyEngine {
struct Vertex {
  glm::vec3 Position;
  glm::vec4 Color;
  glm::vec2 TexCoord;
  float TexIndex;
  float TilingFactor;
};

struct RenderData {
  Ref<VertexBuffer> VertexBuffer;
};
} // namespace MyEngine
