#pragma once

#include "MyEngine/Core/Base.h"

namespace MyEngine {
class ShaderModule {
public:
  enum ShaderType { Vertex, Fragment };

  virtual ~ShaderModule() = default;
  virtual ShaderType GetType() const = 0;

  static Ref<ShaderModule> Create(const std::string &filepath, ShaderType type);
};
} // namespace MyEngine
