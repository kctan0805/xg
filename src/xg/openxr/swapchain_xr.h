// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OPENXR_SWAPCHAIN_XR_H_
#define XG_OPENXR_SWAPCHAIN_XR_H_

#include "openxr/openxr.h"
#include "xg/layout.h"
#include "xg/swapchain.h"
#include "xg/types.h"

namespace xg {

class SwapchainXR : public Swapchain {
 public:
  SwapchainXR() = default;
  virtual ~SwapchainXR();

  virtual Result Init(const LayoutSwapchain& lswapchain) override;
  virtual void Exit(bool destroy_swapchain) override;
  virtual Result AcquireNextImage(const AcquireNextImageInfo& info,
                                  int* image_index) override;
  void ReleaseSwapchainImage();

 protected:
  XrSwapchain swapchain_ = nullptr;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_OPENXR_SWAPCHAIN_XR_H_
