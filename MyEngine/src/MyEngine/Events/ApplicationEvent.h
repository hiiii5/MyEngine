#pragma once

#include "MyEngine/Events/Event.h"

namespace MyEngine {
class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(unsigned int width, unsigned int height)
      : m_Width(width), m_Height(height) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
    return ss.str();
  }

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
  unsigned int m_Width;
  unsigned int m_Height;
};

class WindowCloseEvent : public Event {
public:
  WindowCloseEvent() = default;

  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};
} // namespace MyEngine
