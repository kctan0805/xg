// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_FENCE_VK_H_
#define XG_VULKAN_FENCE_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/fence.h"

namespace xg {

class FenceVK : public Fence {
 public:
  virtual ~FenceVK();

  void Reset() override;
  void Wait() override;
  bool IsSignaled() const;

 protected:
  vk::Device device_;
  vk::Fence fence_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class QueueVK;
  friend class SwapchainVK;
};

}  // namespace xg

#endif  // XG_VULKAN_FENCE_VK_H_
