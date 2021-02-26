// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_WINDOW_VK_H_
#define XG_VULKAN_WINDOW_VK_H_

#include <vector>

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/types.h"
#include "xg/window.h"
#include "xg/window_sdl.h"

namespace xg {

class WindowVK : public WindowSDL {
 public:
  static bool Initialize();
  static void GetInstanceExtensions(std::vector<const char*>* extensions);

  ~WindowVK();

  void GetDrawableSize(int* width, int* height) const override {
    SDL_Vulkan_GetDrawableSize(window_, width, height);
  }

 protected:
  bool Init(const LayoutWindow& lwin) override;

  bool CreateSurface(const LayoutWindow& lwin);

  vk::Instance instance_;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;

  friend class DeviceVK;
  friend class RendererVK;
  friend class SwapchainVK;
};

}  // namespace xg

#endif  // XG_VULKAN_WINDOW_VK_H_
