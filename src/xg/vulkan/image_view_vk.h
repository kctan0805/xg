// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_IMAGE_VIEW_VK_H_
#define XG_VULKAN_IMAGE_VIEW_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/image_view.h"
#include "xg/layout.h"

namespace xg {

class ImageViewVK : public ImageView {
 public:
  virtual ~ImageViewVK();

  Result Init(const LayoutImageView& limage_view) override;
  virtual void Exit() override;

 protected:
  vk::Device device_;
  vk::ImageView image_view_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class SwapchainVK;
  friend class FramebufferVK;
  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_VULKAN_IMAGE_VIEW_VK_H_
