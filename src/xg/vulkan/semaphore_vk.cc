// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/semaphore_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

SemaphoreVK::~SemaphoreVK() {
  if (semaphore_ && device_) {
    XG_TRACE("destroySemaphore: {}", (void*)(VkSemaphore)semaphore_);

    device_.destroySemaphore(semaphore_);
  }
}

}  // namespace xg
