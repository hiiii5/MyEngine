#pragma once

#include "MyEngine/Renderer/ShaderStage.h"

#include "MyEngine/Math/Math.h"

namespace MyEngine {
class Shader {
public:
  virtual ~Shader() = default;
  virtual void Bind() = 0;

  /* virtual void SetInt(const std::string &name, int value) = 0;
  virtual void SetIntArray(const std::string &name, int *value,
                           uint32_t count) = 0;
  virtual void SetFloat(const std::string &name, float value) = 0;
  virtual void SetFloat2(const std::string &name, const Vector2 &value) = 0;
  virtual void SetFloat3(const std::string &name, const Vector3 &value) = 0;
  virtual void SetFloat4(const std::string &name, const Vector4 &value) = 0;
  virtual void SetMat4(const std::string &name, const Matrix4 &value) = 0; */

  static Ref<Shader> Create(const std::string &name,
                            const std::vector<Ref<ShaderStage>> modules);
};
} // namespace MyEngine
