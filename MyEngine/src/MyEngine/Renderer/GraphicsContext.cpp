#include "mepch.h"

#include "MyEngine/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace MyEngine {
Unique<GraphicsContext> GraphicsContext::Create() {
#ifdef ME_PLATFORM_WINDOWS
  return CreateUnique<VulkanContext>();
#elif defined(ME_PLATFORM_LINUX)
  return CreateUnique<VulkanContext>();
#else
  ME_CORE_ASSERT(false, "Unknown platform!");
  return nullptr;
#endif
}
} // namespace MyEngine
