// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/framebuffer_vk.h"

#include <cassert>
#include <memory>

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/image_view_vk.h"
#include "xg/vulkan/render_pass_vk.h"
#include "xg/vulkan/swapchain_vk.h"

namespace xg {

FramebufferVK::~FramebufferVK() { Exit(); }

Result FramebufferVK::Init(const LayoutFramebuffer& lframebuffer) {
  assert(device_);
  assert(!framebuffer_);

  auto render_pass = std::static_pointer_cast<RenderPassVK>(
      lframebuffer.lrender_pass->instance);

  std::vector<vk::ImageView> attachments;

  XG_TRACE("createFramebuffer: {} {} {} {} {}", lframebuffer.id,
           static_cast<void*>((VkRenderPass)render_pass->render_pass_),
           lframebuffer.width, lframebuffer.height, lframebuffer.layers);

  for (const auto& lattachment : lframebuffer.lattachments) {
    std::shared_ptr<ImageViewVK> image_view;
    if (lattachment.lswapchain) {
      auto swapchain = std::static_pointer_cast<SwapchainVK>(
          lattachment.lswapchain->instance);
      image_view = std::static_pointer_cast<ImageViewVK>(
          swapchain->GetImageView(lframebuffer.frame));
    } else {
      assert(lattachment.limage_view);
      image_view = std::static_pointer_cast<ImageViewVK>(
          lattachment.limage_view->instance);
    }
    assert(image_view);
    attachments.emplace_back(image_view->image_view_);

    XG_TRACE("  Attachment: {}",
             static_cast<void*>((VkImageView)image_view->image_view_));
  }

  auto& create_info =
      vk::FramebufferCreateInfo()
          .setRenderPass(render_pass->render_pass_)
          .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
          .setPAttachments(attachments.data())
          .setWidth(static_cast<uint32_t>(lframebuffer.width))
          .setHeight(static_cast<uint32_t>(lframebuffer.height))
          .setLayers(static_cast<uint32_t>(lframebuffer.layers));

  const auto& result =
      device_.createFramebuffer(&create_info, nullptr, &framebuffer_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }
  width_ = lframebuffer.width;
  height_ = lframebuffer.height;

  XG_TRACE("  Framebuffer: {} {} ",
           static_cast<void*>((VkFramebuffer)framebuffer_), lframebuffer.id);

  return Result::kSuccess;
}

void FramebufferVK::Exit() {
  if (framebuffer_ && device_) {
    XG_TRACE("destroyFramebuffer: {} ",
             static_cast<void*>((VkFramebuffer)framebuffer_));

    device_.destroyFramebuffer(framebuffer_);
    framebuffer_ = nullptr;
  }
}

}  // namespace xg
