// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/overlay_vk.h"

#include "SDL2/SDL.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/imgui.h"
#include "xg/logger.h"
#include "xg/renderer.h"
#include "xg/swapchain.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/command_buffer_vk.h"
#include "xg/vulkan/descriptor_pool_vk.h"
#include "xg/vulkan/device_vk.h"
#include "xg/vulkan/queue_vk.h"
#include "xg/vulkan/render_pass_vk.h"
#include "xg/vulkan/renderer_vk.h"
#include "xg/window.h"

namespace xg {

void OverlayVK::Terminate() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
}

static void CheckVkResult(VkResult result) {
  if (result > VK_SUCCESS)
    XG_TRACE(ResultString(static_cast<Result>(result)));
  else if (result < VK_SUCCESS)
    XG_ERROR(ResultString(static_cast<Result>(result)));
}

bool OverlayVK::Init(const LayoutOverlay& loverlay) {
  ctxt_ = ImGui::CreateContext();

  win_ = std::static_pointer_cast<Window>(loverlay.lwin->instance);
  auto result =
      ImGui_ImplSDL2_InitForVulkan(static_cast<SDL_Window*>(win_->GetHandle()));
  if (!result) return false;

  ImGui_ImplVulkan_InitInfo init_info = {};

  auto renderer_vk =
      static_cast<RendererVK*>(loverlay.lwin->lrenderer->instance.get());
  init_info.Instance = (VkInstance)renderer_vk->GetVkInstance();

  auto device_vk = static_cast<DeviceVK*>(renderer_vk->GetDevice().get());
  init_info.PhysicalDevice = (VkPhysicalDevice)device_vk->GetVkPhysicalDevice();
  init_info.Device = (VkDevice)device_vk->GetVkDevice();

  auto queue_vk = static_cast<QueueVK*>(loverlay.lqueue->instance.get());
  init_info.QueueFamily = queue_vk->GetQueueFamilyIndex();
  init_info.Queue = (VkQueue)queue_vk->GetVkQueue();

  init_info.PipelineCache = (VkPipelineCache)device_vk->GetVkPipelineCache();

  auto desc_pool_vk =
      static_cast<DescriptorPoolVK*>(loverlay.ldesc_pool->instance.get());
  init_info.DescriptorPool =
      (VkDescriptorPool)desc_pool_vk->GetVkDescriptorPool();

  init_info.MinImageCount = min_image_count_ =
      loverlay.lswapchain->min_image_count;

  auto swapchain = static_cast<Swapchain*>(loverlay.lswapchain->instance.get());
  init_info.ImageCount = swapchain->GetFrameCount();

  init_info.CheckVkResultFn = CheckVkResult;

  auto render_pass_vk =
      static_cast<RenderPassVK*>(loverlay.lrender_pass->instance.get());
  result = ImGui_ImplVulkan_Init(
      &init_info, (VkRenderPass)render_pass_vk->GetVkRenderPass());

  return result;
}

bool OverlayVK::CreateFontsTexture(const CommandBuffer* cmd) {
  auto cmd_vk = static_cast<const CommandBufferVK*>(cmd);
  auto result = ImGui_ImplVulkan_CreateFontsTexture(
      (VkCommandBuffer)cmd_vk->GetVkCommandBuffer());
  return result;
}

void OverlayVK::DestroyFontUploadObjects() {
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void OverlayVK::Draw(const CommandBuffer* cmd) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame(static_cast<SDL_Window*>(win_->GetHandle()));
  ImGui::NewFrame();

  draw_handler_();

  ImGui::Render();
  auto* draw_data = ImGui::GetDrawData();
  auto cmd_vk = static_cast<const CommandBufferVK*>(cmd);
  ImGui_ImplVulkan_RenderDrawData(
      draw_data, (VkCommandBuffer)cmd_vk->GetVkCommandBuffer());
}

void OverlayVK::Resize() {
  ImGui_ImplVulkan_SetMinImageCount(min_image_count_);
}

}  // namespace xg
