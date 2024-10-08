#include "mepch.h"

#include "MyEngine/Core/Input.h"
#include "MyEngine/Events/Event.h"
#include "MyEngine/Renderer/EditorCamera.h"

#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <glm/gtx/quaternion.hpp>

namespace MyEngine {
EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip,
                           float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip),
      m_FarClip(farClip) {
  UpdateView();
}

void EditorCamera::UpdateProjection() {
  m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
  m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio,
                                  m_NearClip, m_FarClip);
}

void EditorCamera::UpdateView() {
  m_Position = CalculatePosition();
  Quaternion orientation = GetOrientation();
  m_ViewMatrix =
      glm::translate(Matrix4(1.0f), m_Position) * glm::toMat4(orientation);
  m_ViewMatrix = glm::inverse(m_ViewMatrix);
}

Vector2 EditorCamera::PanSpeed() const {
  float x = std::min(m_ViewportWidth / 1000.0f, 2.4f);
  float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

  float y = std::min(m_ViewportHeight / 1000.0f, 2.4f);
  float yFactor = 0.0366f * (y * y) - 0.1778f * x + 0.3021f;

  return {xFactor, yFactor};
}

float EditorCamera::RotationSpeed() const { return 0.8f; }

float EditorCamera::ZoomSpeed() const {
  float distance = m_Distance * 0.2f;
  distance = std::max(distance, 0.0f);
  float speed = distance * distance;
  speed = std::min(speed, 100.0f);
  return speed;
}

void EditorCamera::OnUpdate(Timestep ts) {
  if (Input::IsKeyPressed(SDL_KeyCode::SDLK_LALT)) {
    const Vector2 &mouse = Input::GetMousePosition();
    Vector2 delta = (mouse - m_InitialMousePosition) * 0.003f;
    m_InitialMousePosition = mouse;

    if (Input::IsMouseButtonPressed(SDL_BUTTON_MIDDLE)) {
      MousePan(delta);
    } else if (Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
      MouseRotate(delta);
    } else if (Input::IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      MouseZoom(delta.y);
    }
  }

  UpdateView();
}

void EditorCamera::OnEvent(Event &e) {
  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<MouseScrolledEvent>(
      ME_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
}

bool EditorCamera::OnMouseScroll(MouseScrolledEvent &e) {
  float delta = e.GetY() * 0.1f;
  MouseZoom(delta);
  UpdateView();
  return false;
}

void EditorCamera::MousePan(const Vector2 &delta) {
  const Vector2 speed = PanSpeed();
  m_FocalPoint += -GetRightDirection() * delta.x * speed.x * m_Distance;
  m_FocalPoint += GetUpDirection() * delta.y * speed.y * m_Distance;
}

void EditorCamera::MouseRotate(const Vector2 &delta) {
  float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
  m_Yaw += yawSign * delta.x * RotationSpeed();
  m_Pitch += delta.y * RotationSpeed();
}

void EditorCamera::MouseZoom(float delta) {
  m_Distance -= delta * ZoomSpeed();
  if (m_Distance < 1.0f) {
    m_FocalPoint += GetForwardDirection();
    m_Distance = 1.0f;
  }
}

Vector3 EditorCamera::GetUpDirection() const {
  return glm::rotate(GetOrientation(), Vector3(0.0f, 1.0f, 0.0f));
}

Vector3 EditorCamera::GetRightDirection() const {
  return glm::rotate(GetOrientation(), Vector3(1.0f, 0.0f, 0.0f));
}

Vector3 EditorCamera::GetForwardDirection() const {
  return glm::rotate(GetOrientation(), Vector3(0.0f, 0.0f, -1.0f));
}

Vector3 EditorCamera::CalculatePosition() const {
  return m_FocalPoint - GetForwardDirection() * m_Distance;
}

Quaternion EditorCamera::GetOrientation() const {
  return Quaternion(Vector3(-m_Pitch, -m_Yaw, 0.0f));
}
} // namespace MyEngine
