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
  xr::SwapchainImageAcquireInfo acquire_info;

  auto result = swapchain_.acquireSwapchainImage(
      acquire_info, reinterpret_cast<uint32_t*>(image_index));
  if (result != xr::Result::Success) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  xr::SwapchainImageWaitInfo wait_info;
  wait_info.timeout = xr::Duration(info.timeout);

  result = swapchain_.waitSwapchainImage(wait_info);
  if (result != xr::Result::Success) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
  }

  return static_cast<Result>(result);
}

void SwapchainXR::ReleaseSwapchainImage() {
  xr::SwapchainImageReleaseInfo release_info;

  swapchain_.releaseSwapchainImage(release_info);
}

}  // namespace xg
