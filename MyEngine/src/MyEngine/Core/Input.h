#pragma once

#include "MyEngine/Core/KeyCodes.h"
#include "MyEngine/Core/MouseCodes.h"

#include "MyEngine/Math/Math.h"

namespace MyEngine {
class Input {
public:
  static bool IsKeyPressed(KeyCode key);
  static bool IsMouseButtonPressed(MouseCode button);

  static Vector2 GetMousePosition();
  static float GetMouseX();
  static float GetMouseY();
};
} // namespace MyEngine
