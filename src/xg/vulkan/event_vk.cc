// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/event_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

EventVK::~EventVK() {
  if (event_ && device_) {
    XG_TRACE("destroyEvent: {}", (void*)(VkEvent)event_);

    device_.destroyEvent(event_);
  }
}

Result EventVK::GetStatus() const {
  return static_cast<Result>(device_.getEventStatus(event_));
}

void EventVK::Set() { device_.setEvent(event_); }

void EventVK::Reset() { device_.resetEvent(event_); }

}  // namespace xg
