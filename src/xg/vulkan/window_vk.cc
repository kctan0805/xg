// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/window_vk.h"

#pragma warning(push)
#pragma warning(disable : 4819)
#include "GLFW/glfw3.h"
#pragma warning(pop)
#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/renderer_vk.h"

namespace xg {

const char** WindowVK::GetRequiredExtensions(int* count) {
  return glfwGetRequiredInstanceExtensions(reinterpret_cast<uint32_t*>(count));
}

bool WindowVK::IsPhysicalDevicePresentationSupport(VkInstance instance,
                                                   VkPhysicalDevice device,
                                                   QueueFamily family) {
  return glfwGetPhysicalDevicePresentationSupport(instance, device,
                                                  static_cast<uint32_t>(family))
             ? true
             : false;
}

WindowVK::~WindowVK() {
  if (instance_ && surface_) {
    XG_TRACE("destroySurfaceKHR: {}",
             static_cast<void*>((VkSurfaceKHR)surface_));

    instance_.destroySurfaceKHR(surface_);
  }
}

bool WindowVK::Init(const LayoutWindow& lwin) {
  if (!WindowGLFW::Init(lwin)) return false;
  if (!CreateSurface(lwin)) return false;
  return true;
}

bool WindowVK::CreateSurface(const LayoutWindow& lwin) {
  const auto renderer =
      std::static_pointer_cast<RendererVK>(lwin.lrenderer->instance);
  instance_ = renderer->GetVkInstance();

  const auto& result =
      glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
  if (result != VK_SUCCESS) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  XG_TRACE("glfwCreateWindowSurface: {}",
           static_cast<void*>((VkSurfaceKHR)surface_));

  return true;
}

}  // namespace xg
