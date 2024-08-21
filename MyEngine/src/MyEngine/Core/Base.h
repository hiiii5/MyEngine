#pragma once

#include "MyEngine/Core/Platform.h"

#ifdef ME_PLATFORM_WINDOWS
#ifdef WIN_EXPORT
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif
#endif

#ifdef ME_PLATFORM_LINUX
#define EXPORTED
#endif

#define MYENGINE_API EXPORTED

#ifdef ME_DEBUG
#ifdef ME_PLATFORM_WINDOWS
#define ME_DEBUGBREAK() __debugbreak()
#elif defined(ME_PLATFORM_LINUX)
#include <signal.h>
#define ME_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet"
#endif
#define ME_ENABLE_ASSERTS
#else
#define ME_DEBUGBREAK()
#endif

#define ME_EXPAND_MACRO(x) x
#define ME_STRINGIFY_MACRO(x) #x

#define ME_BIND_EVENT_FN(fn)                                                   \
  [this](auto &&...args) -> decltype(auto) {                                   \
    return this->fn(std::forward<decltype(args)>(args)...);                    \
  }

#include <memory>

namespace MyEngine {
template <typename T> using Unique = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Unique<T> CreateUnique(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace MyEngine

#include "MyEngine/Core/Log.h"
