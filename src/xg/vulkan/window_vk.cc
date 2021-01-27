// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/window_vk.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/renderer_vk.h"

namespace xg {

bool WindowVK::Initialize() {
  if (!WindowSDL::Initialize()) return false;

  if (SDL_Vulkan_LoadLibrary(nullptr) < 0) {
    XG_ERROR("SDL_Vulkan_LoadLibrary() fail");
    return false;
  }

  return true;
}

void WindowVK::GetInstanceExtensions(std::vector<const char*>* extensions) {
  unsigned int count = 0;
  if (!SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr)) {
    XG_ERROR("SDL_Vulkan_GetInstanceExtensions() fail");
    return;
  }

  extensions->resize(count);

  if (!SDL_Vulkan_GetInstanceExtensions(nullptr, &count, extensions->data())) {
    XG_ERROR("SDL_Vulkan_GetInstanceExtensions() fail");
    return;
  }
}

WindowVK::~WindowVK() {
  if (instance_ && surface_) {
    XG_TRACE("destroySurfaceKHR: {}", (void*)(VkSurfaceKHR)surface_);

    instance_.destroySurfaceKHR(surface_);
  }
}

bool WindowVK::Init(const LayoutWindow& lwin) {
  Uint32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;
  flags |= lwin.resizable ? SDL_WINDOW_RESIZABLE : 0;
  int x = lwin.xpos == -1 ? SDL_WINDOWPOS_UNDEFINED : lwin.xpos;
  int y = lwin.ypos == -1 ? SDL_WINDOWPOS_UNDEFINED : lwin.ypos;
  window_ = SDL_CreateWindow(lwin.title.c_str(), x, y, lwin.width, lwin.height,
                             flags);
  if (!window_) {
    XG_ERROR(SDL_GetError());
    return false;
  }

  if (!CreateSurface(lwin)) return false;
  return true;
}

bool WindowVK::CreateSurface(const LayoutWindow& lwin) {
  const auto renderer =
      std::static_pointer_cast<RendererVK>(lwin.lrenderer->instance);
  instance_ = renderer->GetVkInstance();

  const auto result = SDL_Vulkan_CreateSurface(window_, instance_, &surface_);
  if (!result) {
    XG_ERROR("SDL_Vulkan_CreateSurface() fail");
    return false;
  }

  XG_TRACE("SDL_Vulkan_CreateSurface: {}", (void*)(VkSurfaceKHR)surface_);

  return true;
}

}  // namespace xg
