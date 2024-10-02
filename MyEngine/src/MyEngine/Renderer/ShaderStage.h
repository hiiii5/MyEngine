#pragma once

#include "MyEngine/Core/Base.h"

namespace MyEngine {
class ShaderStage {
public:
  enum StageType { Vertex, Fragment };

  virtual ~ShaderStage() = default;
  virtual StageType GetType() const = 0;

  static Ref<ShaderStage> Create(const std::string &filepath, StageType type);
};
} // namespace MyEngine
