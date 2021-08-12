// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_IMAGE_VK_H_
#define XG_VULKAN_IMAGE_VK_H_

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/image.h"
#include "xg/layout.h"

namespace xg {

class ImageVK : public Image {
 public:
  virtual ~ImageVK();

  Result Init(const LayoutImage& limage) override;
  virtual void Exit() override;

 protected:
  VmaAllocator vma_allocator_ = VK_NULL_HANDLE;
  VkImage image_ = VK_NULL_HANDLE;
  VmaAllocation alloc_ = VK_NULL_HANDLE;

  friend class RendererVK;
  friend class DeviceVK;
  friend class ImageViewVK;
  friend class SwapchainVK;
  friend class CommandBufferVK;
  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_VULKAN_IMAGE_VK_H_
