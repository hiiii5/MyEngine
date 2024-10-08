#include "mepch.h"

#include "MyEngine/Core/Input.h"
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

namespace MyEngine {
bool Input::IsKeyPressed(const KeyCode key) {
  const Uint8 *state = SDL_GetKeyboardState(nullptr);
  return state[key];
}

bool Input::IsMouseButtonPressed(const MouseCode button) {
  const Uint32 state = SDL_GetMouseState(nullptr, nullptr);
  return SDL_BUTTON(state) == button;
}

Vector2 Input::GetMousePosition() {
  int x, y;
  const Uint32 state = SDL_GetMouseState(&x, &y);
  return {x, y};
}

float Input::GetMouseX() {
  int x;
  const Uint32 state = SDL_GetMouseState(&x, nullptr);
  return x;
}

float Input::GetMouseY() {
  int y;
  const Uint32 state = SDL_GetMouseState(nullptr, &y);
  return y;
}
} // namespace MyEngine
