// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_EVENT_VK_H_
#define XG_VULKAN_EVENT_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/event.h"
#include "xg/types.h"

namespace xg {

class EventVK : public Event {
 public:
  virtual ~EventVK();

  Result GetStatus() const override;
  void Set() override;
  void Reset() override;

 protected:
  vk::Device device_;
  vk::Event event_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_EVENT_VK_H_
