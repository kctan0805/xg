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
#include "openxr/openxr.h"
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
    XG_TRACE("destroy: {}", (void*)swapchain_);
    const auto result = xrDestroySwapchain(swapchain_);
    if (result != XR_SUCCESS) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
    }
  }
}

Result SwapchainXR::AcquireNextImage(const AcquireNextImageInfo& info,
                                     int* image_index) {
  XrSwapchainImageAcquireInfo acquire_info = {
      XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
  };

  auto result = xrAcquireSwapchainImage(
      swapchain_, &acquire_info, reinterpret_cast<uint32_t*>(image_index));
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  XrSwapchainImageWaitInfo wait_info = {XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
  wait_info.timeout = static_cast<XrDuration>(info.timeout);

  result = xrWaitSwapchainImage(swapchain_, &wait_info);
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
  }

  return static_cast<Result>(result);
}

void SwapchainXR::ReleaseSwapchainImage() {
  XrSwapchainImageReleaseInfo release_info = {
      XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};

  const auto result = xrReleaseSwapchainImage(swapchain_, &release_info);
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
  }
}

}  // namespace xg
