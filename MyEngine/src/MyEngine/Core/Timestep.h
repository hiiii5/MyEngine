#pragma once

namespace MyEngine {
class Timestep {
public:
  Timestep(unsigned long milliseconds = 0.0f) : m_Milliseconds(milliseconds) {}

  operator float() const { return m_Milliseconds; }

  unsigned long GetSeconds() const { return m_Milliseconds * 1000; }
  unsigned long GetMilliseconds() const { return m_Milliseconds; }

private:
  unsigned long m_Milliseconds;
};
} // namespace MyEngine
