// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_FRAMEBUFFER_VK_H_
#define XG_VULKAN_FRAMEBUFFER_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/framebuffer.h"
#include "xg/layout.h"

namespace xg {

class FramebufferVK : public Framebuffer {
 public:
  virtual ~FramebufferVK();

  Result Init(const LayoutFramebuffer& lframebuffer) override;
  virtual void Exit() override;

 protected:
  vk::Device device_;
  vk::Framebuffer framebuffer_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_FRAMEBUFFER_VK_H_
