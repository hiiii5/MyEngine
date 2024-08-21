#pragma once

#include "MyEngine/Core/Base.h"

#include <spdlog/spdlog.h>

namespace MyEngine {
class Log {
public:
  static void Init();

  inline static Ref<spdlog::logger> &GetCoreLogger() { return s_CoreLogger; }
  inline static Ref<spdlog::logger> &GetClientLogger() {
    return s_ClientLogger;
  }

private:
  static Ref<spdlog::logger> s_CoreLogger;
  static Ref<spdlog::logger> s_ClientLogger;
};
} // namespace MyEngine

#define ME_CORE_TRACE(...) ::MyEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ME_CORE_INFO(...) ::MyEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ME_CORE_WARN(...) ::MyEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ME_CORE_ERROR(...) ::MyEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ME_CORE_CRITICAL(...)                                                  \
  ::MyEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define ME_TRACE(...) ::MyEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ME_INFO(...) ::MyEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ME_WARN(...) ::MyEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ME_ERROR(...) ::MyEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ME_CRITICAL(...)                                                       \
  ::MyEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
