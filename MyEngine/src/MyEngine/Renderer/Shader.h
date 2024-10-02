#pragma once

#include "MyEngine/Renderer/ShaderStage.h"

namespace MyEngine {
class Shader {
public:
  virtual ~Shader() = default;
  virtual void Bind() = 0;

  static Ref<Shader> Create(const std::string &name,
                            const std::vector<Ref<ShaderStage>> modules);
};
} // namespace MyEngine
