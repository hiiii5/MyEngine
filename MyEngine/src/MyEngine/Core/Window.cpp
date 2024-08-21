#include "mepch.h"

#include "MyEngine/Core/Window.h"

#ifdef ME_PLATFORM_WINDOWS
#include "Platform/Window/SDLWindow.h"
#elif defined(ME_PLATFORM_LINUX)
#include "Platform/Window/SDLWindow.h"
#endif

namespace MyEngine {
Unique<Window> Window::Create(const WindowProperties &properties) {
#ifdef ME_PLATFORM_WINDOWS
  return CreateUnique<SDLWindow>(properties);
#elif defined(ME_PLATFORM_LINUX)
  return CreateUnique<SDLWindow>(properties);
#else
  ME_CORE_ASSERT(false, "Unknown platform!");
  return nullptr;
#endif
}
} // namespace MyEngine
