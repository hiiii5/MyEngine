#pragma once

#include "MyEngine/Core/Base.h"
#include "MyEngine/Events/Event.h"

namespace MyEngine {
class Layer {
public:
  Layer(const std::string &debugName = "Layer");
  virtual ~Layer() = default;

  virtual void OnAttach() {}
  virtual void OnDetach() {}
  virtual void OnUpdate() {}
  virtual void OnImGuiRender() {}
  virtual void OnEvent(Event &event, void *pData) {}

  const std::string &GetName() const { return m_DebugName; }

protected:
  std::string m_DebugName;
};
} // namespace MyEngine
