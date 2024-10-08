
#include "MyEngine/Math/Math.h"

namespace MyEngine {
struct Vertex {
  Vertex(Vector3 pos, Vector4 col) : Position(pos), Color(col) {}

  Vertex &operator=(const Vertex &v) {
    if (this != &v) {
      this->Position = v.Position;
      this->Color = v.Color;
    }

    return *this;
  }

  Vertex &operator=(Vertex &v) {
    if (this != &v) {
      this->Position = v.Position;
      this->Color = v.Color;
    }

    return *this;
  }

  Vector3 Position;
  Vector4 Color;
};
} // namespace MyEngine
