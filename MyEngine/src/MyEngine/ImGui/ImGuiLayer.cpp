#include "MyEngine/Renderer/Renderer.h"
#include "mepch.h"

#include "MyEngine/ImGui/ImGuiLayer.h"

#include <SDL_timer.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "MyEngine/Core/Application.h"
#include "MyEngine/Events/Event.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace MyEngine {
static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  ME_CORE_ERROR("[ImGui Vulkan] VkResult = {0}\n", (int)err);
  ME_CORE_ASSERT(false);
}

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach() {
  ME_CORE_INFO("Attaching ImGui layer!");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  // io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  style.Colors[ImGuiCol_WindowBg].w = 1.0f;

  ME_CORE_INFO("Getting app for ImGui layer!");
  Application &app = Application::Get();
  ME_CORE_INFO("Getting window for ImGui layer!");
  SDL_Window *window =
      static_cast<SDL_Window *>(app.GetWindow().GetNativeWindow());

  ME_CORE_INFO("Getting graphics context for ImGui layer!");
  VulkanContext *context =
      static_cast<VulkanContext *>(app.GetWindow().GetGraphicsContext());

  ME_CORE_INFO("Initializing ImGui layer!");
  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForVulkan(window);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = context->Instance;
  init_info.PhysicalDevice = context->PhysicalDevice;
  init_info.Device = context->LogicalDevice;
  init_info.QueueFamily = context->QueueFamily;
  init_info.Queue = context->Queue;
  init_info.PipelineCache = context->PipelineCache;
  init_info.DescriptorPool = context->DescriptorPool;
  init_info.RenderPass = context->Window.RenderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = 2;
  init_info.ImageCount = 2;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = context->AllocationCallback;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);
  ME_CORE_INFO("Initialized ImGui layer successfully!");
}

void ImGuiLayer::OnDetach() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::Begin() {
  Application &app = Application::Get();
  Window &window = app.GetWindow();
  VulkanContext *context =
      static_cast<VulkanContext *>(window.GetGraphicsContext());

  ImGui_ImplVulkan_SetMinImageCount(context->MinImageCount);
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() {
  Application &app = Application::Get();
  Window &window = app.GetWindow();
  VulkanContext *context =
      static_cast<VulkanContext *>(window.GetGraphicsContext());

  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(window.GetWidth(), window.GetHeight());

  ImGui::Render();
  ImDrawData *drawData = ImGui::GetDrawData();
  const bool isMinimized =
      (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
  if (!isMinimized) {
    ImGui_ImplVulkan_RenderDrawData(
        drawData, context->Window.GetCurrentFrame()->CommandBuffer);
  }

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}

void ImGuiLayer::OnEvent(Event &event, void *pData) {
  ImGuiIO &io = ImGui::GetIO();
  event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
  event.Handled |=
      event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

  if (event.Handled) {
    ImGui_ImplSDL2_ProcessEvent((SDL_Event *)pData);
  }
}

} // namespace MyEngine
