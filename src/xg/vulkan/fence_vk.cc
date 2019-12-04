// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/fence_vk.h"

#include <cstdint>
#include <limits>

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

FenceVK::~FenceVK() {
  if (fence_ && device_) {
    XG_TRACE("destroyFence: {}", static_cast<void*>((VkFence)fence_));

    device_.destroyFence(fence_);
  }
}

void FenceVK::Reset() {
  //XG_TRACE("resetFences: {}", static_cast<void*>((VkFence)fence_));

  device_.resetFences(1, &fence_);
}

void FenceVK::Wait() {
  //XG_TRACE("waitForFences: {}", static_cast<void*>((VkFence)fence_));

  device_.waitForFences(1, &fence_, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

bool FenceVK::IsSignaled() const {
  return (device_.getFenceStatus(fence_) == vk::Result::eSuccess) ? true
                                                                  : false;
}

}  // namespace xg
