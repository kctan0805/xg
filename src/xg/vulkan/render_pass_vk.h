// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_RENDER_PASS_VK_H_
#define XG_VULKAN_RENDER_PASS_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/render_pass.h"

namespace xg {

class RenderPassVK : public RenderPass {
 public:
  virtual ~RenderPassVK();

  const vk::RenderPass& GetVkRenderPass() const { return render_pass_; }

  Result Init(const LayoutRenderPass& lrender_pass) override;
  virtual void Exit() override;

 protected:
  vk::Device device_;
  vk::RenderPass render_pass_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class FramebufferVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_RENDER_PASS_VK_H_
