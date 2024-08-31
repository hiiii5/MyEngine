#include "MyEngine/Time/Time.h"

#include <SDL_timer.h>

namespace MyEngine {
unsigned long Time::GetTime() { return SDL_GetTicks64(); }
} // namespace MyEngine
