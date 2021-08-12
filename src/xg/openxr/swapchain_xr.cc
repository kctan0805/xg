// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/openxr/swapchain_xr.h"

#include <cassert>

// clang-format off
#include "vulkan/vulkan.hpp"
#include "openxr/openxr_platform.h"
#include "openxr/openxr.hpp"
// clang-format on
#include "xg/logger.h"
#include "xg/utility.h"

namespace xg {

SwapchainXR::~SwapchainXR() { Exit(true); }

Result SwapchainXR::Init(const LayoutSwapchain& lswapchain) {
  assert(swapchain_);

  width_ = lswapchain.width;
  height_ = lswapchain.height;
  format_ = lswapchain.image_format;

  return Result::kSuccess;
}

void SwapchainXR::Exit(bool destroy_swapchain) {
  for (auto& image_view : image_views_) {
    image_view->Exit();
  }

  if (destroy_swapchain && swapchain_) {
    XG_TRACE("destroy: {}", (void*)(XrSwapchain)swapchain_);
    swapchain_.destroy();
  }
}

Result SwapchainXR::AcquireNextImage(const AcquireNextImageInfo& info,
                                     int* image_index) {
  return Result::kSuccess;
}

}  // namespace xg
