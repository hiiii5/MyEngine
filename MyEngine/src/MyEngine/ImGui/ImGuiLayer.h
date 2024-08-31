#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Core/Layer.h"

namespace MyEngine {
class MYENGINE_API ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  ~ImGuiLayer();

  virtual void OnAttach() override;
  virtual void OnDetach() override;
  virtual void OnUpdate() override;
  virtual void OnEvent(Event &event, void *pData) override;

private:
  float m_Time = 0.0f;
};
} // namespace MyEngine
