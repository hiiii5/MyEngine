#pragma once

#include "MyEngine/Renderer/ShaderModule.h"

namespace MyEngine {
class Shader {
public:
  virtual ~Shader() = default;
  virtual void Bind() = 0;

  static Ref<Shader> Create(const std::vector<Ref<ShaderModule>> modules);
};
} // namespace MyEngine
