// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_SEMAPHORE_VK_H_
#define XG_VULKAN_SEMAPHORE_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/semaphore.h"

namespace xg {

class SemaphoreVK : public Semaphore {
 public:
  virtual ~SemaphoreVK();

 protected:
  vk::Device device_;
  vk::Semaphore semaphore_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class QueueVK;
  friend class SwapchainVK;
};

}  // namespace xg

#endif  // XG_VULKAN_SEMAPHORE_VK_H_
