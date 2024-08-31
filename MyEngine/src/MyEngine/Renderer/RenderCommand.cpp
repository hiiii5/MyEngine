#include "mepch.h"

#include "MyEngine/Renderer/RenderCommand.h"

namespace MyEngine {
Unique<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
