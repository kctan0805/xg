// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_SWAPCHAIN_VK_H_
#define XG_VULKAN_SWAPCHAIN_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/swapchain.h"
#include "xg/types.h"

namespace xg {

class SwapchainVK : public Swapchain {
 public:
  virtual ~SwapchainVK();

  virtual Result Init(const LayoutSwapchain& lswapchain) override;
  virtual void Exit(bool destroy_swapchain) override;

  virtual Result AcquireNextImage(const AcquireNextImageInfo& info,
                                      int* image_index) override;

 protected:
  vk::Device device_;
  vk::SwapchainKHR swapchain_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class QueueVK;
};

}  // namespace xg

#endif  // XG_VULKAN_SWAPCHAIN_VK_H_
