// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/render_pass_vk.h"

#include <cassert>

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

RenderPassVK::~RenderPassVK() { Exit(); }

Result RenderPassVK::Init(const LayoutRenderPass& lrender_pass) {
  assert(device_);
  assert(!render_pass_);

  std::vector<vk::AttachmentDescription> attachments;
  std::vector<vk::SubpassDescription> subpasses;
  std::vector<std::shared_ptr<std::vector<vk::AttachmentReference>>>
      color_references_array;
  std::vector<std::shared_ptr<vk::AttachmentReference>> depth_references;
  std::vector<vk::SubpassDependency> dependencies;

  XG_TRACE("createRenderPass: {}", lrender_pass.id);

  for (auto lattachment : lrender_pass.lattachments) {
    const auto format = static_cast<vk::Format>(lattachment->format);
    const auto samples =
        static_cast<vk::SampleCountFlagBits>(lattachment->samples);
    const auto load_op =
        static_cast<vk::AttachmentLoadOp>(lattachment->load_op);
    const auto store_op =
        static_cast<vk::AttachmentStoreOp>(lattachment->store_op);
    const auto stencil_load_op =
        static_cast<vk::AttachmentLoadOp>(lattachment->stencil_load_op);
    const auto stencil_store_op =
        static_cast<vk::AttachmentStoreOp>(lattachment->stencil_store_op);
    const auto initial_layout =
        static_cast<vk::ImageLayout>(lattachment->initial_layout);
    const auto final_layout =
        static_cast<vk::ImageLayout>(lattachment->final_layout);

    attachments.emplace_back(vk::AttachmentDescription()
                                 .setFormat(format)
                                 .setSamples(samples)
                                 .setLoadOp(load_op)
                                 .setStoreOp(store_op)
                                 .setStencilLoadOp(stencil_load_op)
                                 .setStencilStoreOp(stencil_store_op)
                                 .setInitialLayout(initial_layout)
                                 .setFinalLayout(final_layout));

    XG_TRACE("  Attachment: {} {} {} {} {} {} {} {}", vk::to_string(format),
             vk::to_string(samples), vk::to_string(load_op),
             vk::to_string(store_op), vk::to_string(stencil_load_op),
             vk::to_string(stencil_store_op), vk::to_string(initial_layout),
             vk::to_string(final_layout));
  }

  for (auto lsubpass : lrender_pass.lsubpasses) {
    auto subpass = vk::SubpassDescription().setPipelineBindPoint(
        vk::PipelineBindPoint::eGraphics);

    if (lsubpass->lcolor_attachments.size() > 0) {
      auto color_references =
          std::make_shared<std::vector<vk::AttachmentReference>>();
      if (!color_references) return Result::kErrorOutOfHostMemory;

      for (auto lcolor_attachment : lsubpass->lcolor_attachments) {
        const auto layout =
            static_cast<vk::ImageLayout>(lcolor_attachment->layout);

        color_references->emplace_back(vk::AttachmentReference()
                                           .setAttachment(static_cast<uint32_t>(
                                               lcolor_attachment->attachment))
                                           .setLayout(layout));

        XG_TRACE("  ColorAttachment: {} {}", lcolor_attachment->attachment,
                 vk::to_string(layout));
      }
      color_references_array.emplace_back(color_references);
      subpass
          .setColorAttachmentCount(
              static_cast<uint32_t>(color_references->size()))
          .setPColorAttachments(color_references->data());
    }

    const auto& ldepth_stencil_attachment = lsubpass->ldepth_stencil_attachment;
    if (ldepth_stencil_attachment) {
      auto depth_reference = std::make_shared<vk::AttachmentReference>();
      if (!depth_reference) return Result::kErrorOutOfHostMemory;

      depth_reference->setAttachment(
          static_cast<uint32_t>(ldepth_stencil_attachment->attachment));

      const auto layout =
          static_cast<vk::ImageLayout>(ldepth_stencil_attachment->layout);
      depth_reference->setLayout(layout);

      depth_references.emplace_back(depth_reference);
      subpass.setPDepthStencilAttachment(depth_reference.get());

      XG_TRACE("  DepthStencilAttachment: {} {}",
               ldepth_stencil_attachment->attachment, vk::to_string(layout));
    }

    subpasses.emplace_back(subpass);
  }

  for (auto ldependency : lrender_pass.ldependencies) {
    const auto src_stage_mask =
        static_cast<vk::PipelineStageFlagBits>(ldependency->src_stage_mask);
    const auto dst_stage_mask =
        static_cast<vk::PipelineStageFlagBits>(ldependency->dst_stage_mask);
    const auto src_access_mask =
        static_cast<vk::AccessFlagBits>(ldependency->src_access_mask);
    const auto dst_access_mask =
        static_cast<vk::AccessFlagBits>(ldependency->dst_access_mask);

    dependencies.emplace_back(
        vk::SubpassDependency()
            .setSrcSubpass(static_cast<uint32_t>(ldependency->src_subpass))
            .setDstSubpass(static_cast<uint32_t>(ldependency->dst_subpass))
            .setSrcStageMask(src_stage_mask)
            .setDstStageMask(dst_stage_mask)
            .setSrcAccessMask(src_access_mask)
            .setDstAccessMask(dst_access_mask)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion));

    XG_TRACE("  Dependency: {} {} {} {} {} {}", ldependency->src_subpass,
             ldependency->dst_subpass, vk::to_string(src_stage_mask),
             vk::to_string(dst_stage_mask), vk::to_string(src_access_mask),
             vk::to_string(dst_access_mask));
  }

  auto& render_pass_create_info =
      vk::RenderPassCreateInfo()
          .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
          .setPAttachments(attachments.data())
          .setSubpassCount(static_cast<uint32_t>(subpasses.size()))
          .setPSubpasses(subpasses.data())
          .setDependencyCount(static_cast<uint32_t>(dependencies.size()))
          .setPDependencies(dependencies.data());

  static_assert(sizeof(uint32_t) == sizeof(int));

  vk::RenderPassMultiviewCreateInfo render_pass_multiview_create_info;
  if (lrender_pass.lmultiview) {
    const auto& lmultiview = lrender_pass.lmultiview;
    render_pass_multiview_create_info
        .setSubpassCount(static_cast<uint32_t>(lmultiview->view_masks.size()))
        .setPViewMasks(
            reinterpret_cast<uint32_t*>(lmultiview->view_masks.data()))
        .setDependencyCount(
            static_cast<uint32_t>(lmultiview->view_offsets.size()))
        .setPViewOffsets(
            reinterpret_cast<int32_t*>(lmultiview->view_offsets.data()))
        .setCorrelationMaskCount(
            static_cast<uint32_t>(lmultiview->correlation_masks.size()))
        .setPCorrelationMasks(
            reinterpret_cast<uint32_t*>(lmultiview->correlation_masks.data()));

    render_pass_create_info.setPNext(&render_pass_multiview_create_info);

    XG_TRACE("  Multiview: {} {} {}", lmultiview->view_masks.size(),
             lmultiview->view_offsets.size(),
             lmultiview->correlation_masks.size());
  }

  const auto& result = device_.createRenderPass(&render_pass_create_info,
                                                nullptr, &render_pass_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  XG_TRACE("  RenderPass: {}", static_cast<void*>((VkRenderPass)render_pass_));

  return Result::kSuccess;
}

void RenderPassVK::Exit() {
  if (render_pass_ && device_) {
    XG_TRACE("destroyRenderPass: {}",
             static_cast<void*>((VkRenderPass)render_pass_));

    device_.destroyRenderPass(render_pass_);
    render_pass_ = nullptr;
  }
}

}  // namespace xg
