#pragma once

#include "MyEngine/Core/Timestep.h"
#include "MyEngine/Events/Event.h"
#include "MyEngine/Events/MouseEvent.h"
#include "MyEngine/Renderer/Camera.h"

namespace MyEngine {
class EditorCamera : public Camera {
public:
  EditorCamera() = default;
  EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

  void OnUpdate(Timestep ts);
  void OnEvent(Event &e);

  inline float GetDistance() const { return m_Distance; }
  inline void SetDistance(float distance) { m_Distance = distance; }

  inline void SetViewportSize(float width, float height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
  }

  const Matrix4 &GetViewMatrix() const { return m_ViewMatrix; }
  Matrix4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

  Vector3 GetUpDirection() const;
  Vector3 GetRightDirection() const;
  Vector3 GetForwardDirection() const;

  const Vector3 &GetPosition() const { return m_Position; }
  Quaternion GetOrientation() const;

  float GetPitch() const { return m_Pitch; }
  float GetYaw() const { return m_Yaw; }

private:
  void UpdateProjection();
  void UpdateView();

  bool OnMouseScroll(MouseScrolledEvent &e);

  void MousePan(const Vector2 &delta);
  void MouseRotate(const Vector2 &delta);
  void MouseZoom(float delta);

  Vector3 CalculatePosition() const;

  Vector2 PanSpeed() const;
  float RotationSpeed() const;
  float ZoomSpeed() const;

  float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f,
        m_FarClip = 1000.0f;

  Matrix4 m_ViewMatrix;
  Vector3 m_Position = {0.0f, 0.0f, 0.0f};
  Vector3 m_FocalPoint = {0.0f, 0.0f, 0.0f};

  Vector2 m_InitialMousePosition = {0.0f, 0.0f};

  float m_Distance = 10.0f;
  float m_Pitch = 0.0f, m_Yaw = 0.0f;

  float m_ViewportWidth = 1280, m_ViewportHeight = 720;
};
} // namespace MyEngine
