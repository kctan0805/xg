// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/command_buffer_vk.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "xg/logger.h"
#include "xg/types.h"
#include "xg/vulkan/buffer_vk.h"
#include "xg/vulkan/descriptor_set_vk.h"
#include "xg/vulkan/event_vk.h"
#include "xg/vulkan/framebuffer_vk.h"
#include "xg/vulkan/image_vk.h"
#include "xg/vulkan/pipeline_layout_vk.h"
#include "xg/vulkan/pipeline_vk.h"
#include "xg/vulkan/query_pool_vk.h"
#include "xg/vulkan/render_pass_vk.h"

namespace xg {

CommandBufferVK::~CommandBufferVK() {
  if (cmd_buffer_ && command_pool_ && device_) {
    XG_TRACE("freeCommandBuffers: {}",
             static_cast<void*>((VkCommandBuffer)cmd_buffer_));

    device_.freeCommandBuffers(command_pool_, 1, &cmd_buffer_);
  }
}

Result CommandBufferVK::Begin(const CommandBufferBeginInfo& info) const {
  const auto& begin_info = vk::CommandBufferBeginInfo().setFlags(
      static_cast<vk::CommandBufferUsageFlagBits>(info.usage));

  const auto& result = cmd_buffer_.begin(&begin_info);

  XG_TRACE("begin: {} {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           vk::to_string(begin_info.flags));

  return static_cast<Result>(result);
}

void CommandBufferVK::End() const {
  XG_TRACE("end: {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_));
  cmd_buffer_.end();
}

void CommandBufferVK::Reset() const {
  XG_TRACE("reset: {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_));
  cmd_buffer_.reset(vk::CommandBufferResetFlags());
}

void CommandBufferVK::PipelineBarrier(const PipelineBarrierInfo& info) const {
  std::vector<vk::MemoryBarrier> memory_barriers;
  std::vector<vk::BufferMemoryBarrier> buffer_barriers;
  std::vector<vk::ImageMemoryBarrier> image_barriers;

  memory_barriers.reserve(info.memory_barriers.size());
  buffer_barriers.reserve(info.buffer_barriers.size());
  image_barriers.reserve(info.image_barriers.size());

  const auto src_stage_mask =
      static_cast<vk::PipelineStageFlagBits>(info.src_stage_mask);
  const auto dst_stage_mask =
      static_cast<vk::PipelineStageFlagBits>(info.dst_stage_mask);

  XG_TRACE("pipelineBarrier: {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           vk::to_string(src_stage_mask), vk::to_string(dst_stage_mask));

  for (const auto& barrier : info.memory_barriers) {
    const auto src_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.src_access_mask);
    const auto dst_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.dst_access_mask);

    memory_barriers.emplace_back(vk::MemoryBarrier()
                                     .setSrcAccessMask(src_access_mask)
                                     .setDstAccessMask(dst_access_mask));

    XG_TRACE("  MemoryBarrier: {} {}", vk::to_string(src_access_mask),
             vk::to_string(dst_access_mask));
  }

  for (const auto& barrier : info.buffer_barriers) {
    const auto src_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.src_access_mask);
    const auto dst_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.dst_access_mask);
    const auto buf = reinterpret_cast<const BufferVK*>(barrier.buffer);

    buffer_barriers.emplace_back(
        vk::BufferMemoryBarrier()
            .setSrcAccessMask(src_access_mask)
            .setDstAccessMask(dst_access_mask)
            .setSrcQueueFamilyIndex(
                static_cast<uint32_t>(barrier.src_queue_family_index))
            .setDstQueueFamilyIndex(
                static_cast<uint32_t>(barrier.dst_queue_family_index))
            .setBuffer(buf->buffer_)
            .setOffset(static_cast<vk::DeviceSize>(barrier.offset))
            .setSize(static_cast<vk::DeviceSize>(barrier.size)));

    XG_TRACE("  BufferMemoryBarrier: {} {} {} {} {} {} {}",
             vk::to_string(src_access_mask), vk::to_string(dst_access_mask),
             barrier.src_queue_family_index, barrier.dst_queue_family_index,
             static_cast<void*>((VkBuffer)buf->buffer_), barrier.offset,
             static_cast<int64_t>(barrier.size));
  }

  for (const auto& barrier : info.image_barriers) {
    const auto src_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.src_access_mask);
    const auto dst_access_mask =
        static_cast<vk::AccessFlagBits>(barrier.dst_access_mask);
    const auto old_layout = static_cast<vk::ImageLayout>(barrier.old_layout);
    const auto new_layout = static_cast<vk::ImageLayout>(barrier.new_layout);
    const auto image = reinterpret_cast<const ImageVK*>(barrier.image);
    const auto aspect_mask = static_cast<vk::ImageAspectFlagBits>(
        barrier.subresource_range.aspect_mask);
    image_barriers.emplace_back(
        vk::ImageMemoryBarrier()
            .setSrcAccessMask(src_access_mask)
            .setDstAccessMask(dst_access_mask)
            .setOldLayout(old_layout)
            .setNewLayout(new_layout)
            .setSrcQueueFamilyIndex(
                static_cast<uint32_t>(barrier.src_queue_family_index))
            .setDstQueueFamilyIndex(
                static_cast<uint32_t>(barrier.dst_queue_family_index))
            .setImage(image->image_)
            .setSubresourceRange(
                vk::ImageSubresourceRange()
                    .setAspectMask(aspect_mask)
                    .setBaseMipLevel(static_cast<uint32_t>(
                        barrier.subresource_range.base_mip_level))
                    .setLevelCount(static_cast<uint32_t>(
                        barrier.subresource_range.layer_count))
                    .setBaseArrayLayer(
                        barrier.subresource_range.base_array_layer)
                    .setLayerCount(static_cast<uint32_t>(
                        barrier.subresource_range.layer_count))));

    XG_TRACE("  ImageMemoryBarrier: {} {} {} {} {} {} {} {} {} {} {} {}",
             vk::to_string(src_access_mask), vk::to_string(dst_access_mask),
             vk::to_string(old_layout), vk::to_string(new_layout),
             barrier.src_queue_family_index, barrier.dst_queue_family_index,
             static_cast<void*>((VkImage)image->image_),
             vk::to_string(aspect_mask),
             barrier.subresource_range.base_mip_level,
             barrier.subresource_range.layer_count,
             barrier.subresource_range.base_array_layer,
             barrier.subresource_range.layer_count);
  }

  cmd_buffer_.pipelineBarrier(
      src_stage_mask, dst_stage_mask, vk::DependencyFlagBits(),
      static_cast<uint32_t>(memory_barriers.size()), memory_barriers.data(),
      static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(),
      static_cast<uint32_t>(image_barriers.size()), image_barriers.data());
}

void CommandBufferVK::Dispatch(const DispatchInfo& info) const {
  XG_TRACE("dispatch: {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_), info.group_count_x,
           info.group_count_y, info.group_count_z);

  cmd_buffer_.dispatch(static_cast<uint32_t>(info.group_count_x),
                       static_cast<uint32_t>(info.group_count_y),
                       static_cast<uint32_t>(info.group_count_z));
}

void CommandBufferVK::CopyBuffer(const CopyBufferInfo& info) const {
  const auto src_buf = reinterpret_cast<const BufferVK*>(info.src_buffer);
  const auto dst_buf = reinterpret_cast<const BufferVK*>(info.dst_buffer);
  std::vector<vk::BufferCopy> vk_buffer_copies;
  vk_buffer_copies.reserve(info.regions.size());

  XG_TRACE("copyBuffer: {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_));

  for (const auto& region : info.regions) {
    vk_buffer_copies.emplace_back(vk::BufferCopy()
                                      .setSrcOffset(region.src_offset)
                                      .setDstOffset(region.dst_offset)
                                      .setSize(region.size));

    XG_TRACE("  Region: {} {} {}", region.src_offset, region.dst_offset,
             region.size);
  }

  cmd_buffer_.copyBuffer(src_buf->buffer_, dst_buf->buffer_, vk_buffer_copies);
}

void CommandBufferVK::CopyBufferToImage(
    const CopyBufferToImageInfo& info) const {
  const auto src_buf = reinterpret_cast<const BufferVK*>(info.src_buffer);
  const auto dst_image = reinterpret_cast<const ImageVK*>(info.dst_image);
  std::vector<vk::BufferImageCopy> copies;
  const auto dst_image_layout =
      static_cast<vk::ImageLayout>(info.dst_image_layout);

  XG_TRACE("copyBufferToImage: {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkBuffer)src_buf->buffer_),
           static_cast<void*>((VkImage)dst_image->image_),
           vk::to_string(dst_image_layout));

  for (const auto& region : info.regions) {
    const auto aspect_mask = static_cast<vk::ImageAspectFlagBits>(
        region.image_subresource.aspect_mask);
    const auto& subresource =
        vk::ImageSubresourceLayers()
            .setAspectMask(aspect_mask)
            .setLayerCount(
                static_cast<uint32_t>(region.image_subresource.mip_level))
            .setBaseArrayLayer(static_cast<uint32_t>(
                region.image_subresource.base_array_layer))
            .setLayerCount(
                static_cast<uint32_t>(region.image_subresource.layer_count));

    copies
        .emplace_back(vk::BufferImageCopy()
                          .setBufferOffset(region.buffer_offset)
                          .setBufferRowLength(region.buffer_low_length)
                          .setBufferImageHeight(region.buffer_image_height)
                          .setImageSubresource(subresource)
                          .setImageOffset(vk::Offset3D()
                                              .setX(region.image_offset.x)
                                              .setY(region.image_offset.y)
                                              .setZ(region.image_offset.z)))
        .setImageExtent(
            vk::Extent3D()
                .setWidth(static_cast<uint32_t>(region.image_extent.width))
                .setHeight(static_cast<uint32_t>(region.image_extent.height))
                .setDepth(static_cast<uint32_t>(region.image_extent.depth)));

    XG_TRACE("  Region: {} {} {} {} {} {} {} {} {} {} {} {} {}",
             region.buffer_offset, region.buffer_low_length,
             region.buffer_image_height, vk::to_string(aspect_mask),
             region.image_subresource.mip_level,
             region.image_subresource.base_array_layer,
             region.image_subresource.layer_count, region.image_offset.x,
             region.image_offset.y, region.image_offset.z,
             region.image_extent.width, region.image_extent.height,
             region.image_extent.depth);
  }

  cmd_buffer_.copyBufferToImage(
      src_buf->buffer_, dst_image->image_, dst_image_layout,
      static_cast<uint32_t>(copies.size()), copies.data());
}

void CommandBufferVK::BlitImage(const BlitImageInfo& info) const {
  const auto src_image = reinterpret_cast<const ImageVK*>(info.src_image);
  const auto dst_image = reinterpret_cast<const ImageVK*>(info.dst_image);
  const auto src_image_layout =
      static_cast<vk::ImageLayout>(info.src_image_layout);
  const auto dst_image_layout =
      static_cast<vk::ImageLayout>(info.dst_image_layout);
  std::vector<vk::ImageBlit> blites;
  const auto filter = static_cast<vk::Filter>(info.filter);

  XG_TRACE("blitImage: {} {} {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkImage)src_image->image_),
           vk::to_string(src_image_layout),
           static_cast<void*>((VkImage)dst_image->image_),
           vk::to_string(dst_image_layout), vk::to_string(filter));

  for (const auto& region : info.regions) {
    const auto src_aspect_mask = static_cast<vk::ImageAspectFlagBits>(
        region.src_subresource.aspect_mask);
    const auto& src_subresource =
        vk::ImageSubresourceLayers()
            .setAspectMask(src_aspect_mask)
            .setLayerCount(
                static_cast<uint32_t>(region.src_subresource.mip_level))
            .setBaseArrayLayer(
                static_cast<uint32_t>(region.src_subresource.base_array_layer))
            .setLayerCount(
                static_cast<uint32_t>(region.src_subresource.layer_count));

    std::array<vk::Offset3D, 2> src_offsets;
    for (int i = 0; i < 2; ++i) {
      src_offsets[i]
          .setX(region.src_offsets[i].x)
          .setY(region.src_offsets[i].y)
          .setZ(region.src_offsets[i].z);
    }

    const auto dst_aspect_mask = static_cast<vk::ImageAspectFlagBits>(
        region.dst_subresource.aspect_mask);

    const auto& dst_subresource =
        vk::ImageSubresourceLayers()
            .setAspectMask(dst_aspect_mask)
            .setLayerCount(
                static_cast<uint32_t>(region.dst_subresource.mip_level))
            .setBaseArrayLayer(
                static_cast<uint32_t>(region.dst_subresource.base_array_layer))
            .setLayerCount(
                static_cast<uint32_t>(region.dst_subresource.layer_count));

    std::array<vk::Offset3D, 2> dst_offsets;
    for (int i = 0; i < 2; ++i) {
      dst_offsets[i]
          .setX(region.dst_offsets[i].x)
          .setY(region.dst_offsets[i].y)
          .setZ(region.dst_offsets[i].z);
    }

    blites.emplace_back(vk::ImageBlit()
                            .setSrcSubresource(src_subresource)
                            .setSrcOffsets(src_offsets)
                            .setDstSubresource(dst_subresource)
                            .setDstOffsets(dst_offsets));

    XG_TRACE("  Region: Src: {} {} {} {} {} {} {} {} {} {}",
             vk::to_string(src_aspect_mask), region.src_subresource.mip_level,
             region.src_subresource.base_array_layer,
             region.src_subresource.layer_count, region.src_offsets[0].x,
             region.src_offsets[0].y, region.src_offsets[0].z,
             region.src_offsets[1].x, region.src_offsets[1].y,
             region.src_offsets[1].z);
    XG_TRACE("  Region: Dst: {} {} {} {} {} {} {} {} {} {}",
             vk::to_string(dst_aspect_mask), region.dst_subresource.mip_level,
             region.dst_subresource.base_array_layer,
             region.dst_subresource.layer_count, region.dst_offsets[0].x,
             region.dst_offsets[0].y, region.dst_offsets[0].z,
             region.dst_offsets[1].x, region.dst_offsets[1].y,
             region.dst_offsets[1].z);
  }

  cmd_buffer_.blitImage(src_image->image_, src_image_layout, dst_image->image_,
                        dst_image_layout, blites, filter);
}

void CommandBufferVK::PushConstants(const PushConstantsInfo& info) const {
  const auto pipeline_layout_vk =
      reinterpret_cast<const PipelineLayoutVK*>(info.layout);
  const auto stage_flags =
      static_cast<vk::ShaderStageFlagBits>(info.stage_flags);

  XG_TRACE("pushConstants: {} {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>(
               (VkPipelineLayout)pipeline_layout_vk->pipeline_layout_),
           vk::to_string(stage_flags), info.offset, info.size);

  cmd_buffer_.pushConstants(pipeline_layout_vk->pipeline_layout_, stage_flags,
                            static_cast<uint32_t>(info.offset),
                            static_cast<uint32_t>(info.size), info.values);
}

void CommandBufferVK::BeginRenderPass(const RenderPassBeginInfo& info) const {
  const auto& render_pass = reinterpret_cast<RenderPassVK*>(info.render_pass);
  const auto& framebuffer = reinterpret_cast<FramebufferVK*>(info.framebuffer);
  const auto& area =
      vk::Rect2D()
          .setOffset(vk::Offset2D()
                         .setX(static_cast<int32_t>(info.rect.x))
                         .setY(static_cast<int32_t>(info.rect.y)))
          .setExtent(vk::Extent2D()
                         .setWidth(static_cast<uint32_t>(info.rect.width))
                         .setHeight(static_cast<uint32_t>(info.rect.height)));
  std::vector<vk::ClearValue> clear_values;

  XG_TRACE("beginRenderPass: {} {} {} {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkRenderPass)render_pass->render_pass_),
           static_cast<void*>((VkFramebuffer)framebuffer->framebuffer_),
           info.rect.x, info.rect.y, info.rect.width, info.rect.height);

  for (const auto& clear_value : info.clear_values) {
    if (clear_value.index() == 0) {
      const auto& v = std::get<ClearColorValue>(clear_value);
      clear_values.emplace_back(vk::ClearColorValue(
          std::array{v.value.r, v.value.g, v.value.b, v.value.a}));

      XG_TRACE("  ClearColorValue: {} {} {} {}", v.value.r, v.value.g,
               v.value.b, v.value.a);
    } else {
      assert(clear_value.index() == 1);
      const auto& v = std::get<ClearDepthStencilValue>(clear_value);
      clear_values.emplace_back(vk::ClearDepthStencilValue(v.depth, v.stencil));

      XG_TRACE("  ClearDepthStencilValue: {} {}", v.depth, v.stencil);
    }
  }

  auto& render_pass_begin_info =
      vk::RenderPassBeginInfo()
          .setRenderPass(render_pass->render_pass_)
          .setFramebuffer(framebuffer->framebuffer_)
          .setRenderArea(area)
          .setClearValueCount(static_cast<uint32_t>(clear_values.size()))
          .setPClearValues(clear_values.data());

  cmd_buffer_.beginRenderPass(&render_pass_begin_info,
                              vk::SubpassContents::eInline);
}

void CommandBufferVK::EndRenderPass() const {
  XG_TRACE("endRenderPass: {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_));

  cmd_buffer_.endRenderPass();
}

void CommandBufferVK::SetViewport(const SetViewportInfo& info) const {
  std::vector<vk::Viewport> viewports;
  viewports.reserve(info.viewports.size());

  XG_TRACE("setViewport: {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_));

  for (const auto& viewport : info.viewports) {
    viewports.emplace_back(vk::Viewport()
                               .setX(viewport.x)
                               .setY(viewport.y)
                               .setWidth(viewport.width)
                               .setHeight(viewport.height)
                               .setMinDepth(viewport.min_depth)
                               .setMaxDepth(viewport.max_depth));

    XG_TRACE("  Viewport: {} {} {} {} {} {}", viewport.x, viewport.y,
             viewport.width, viewport.height, viewport.min_depth,
             viewport.max_depth);
  }

  cmd_buffer_.setViewport(static_cast<uint32_t>(info.first_viewport),
                          static_cast<uint32_t>(viewports.size()),
                          viewports.data());
}

void CommandBufferVK::SetScissor(const SetScissorInfo& info) const {
  std::vector<vk::Rect2D> scissors;
  scissors.reserve(info.scissors.size());

  XG_TRACE("setScissor: {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           info.first_scissor);

  for (const auto& scissor : info.scissors) {
    scissors.emplace_back(
        vk::Rect2D()
            .setOffset(vk::Offset2D()
                           .setX(static_cast<int32_t>(scissor.x))
                           .setY(static_cast<int32_t>(scissor.y)))
            .setExtent(vk::Extent2D()
                           .setWidth(static_cast<uint32_t>(scissor.width))
                           .setHeight(static_cast<uint32_t>(scissor.height))));

    XG_TRACE("  Scissor: {} {} {} {} {} {}", scissor.x, scissor.y,
             scissor.width, scissor.height);
  }

  cmd_buffer_.setScissor(static_cast<uint32_t>(info.first_scissor),
                         static_cast<uint32_t>(scissors.size()),
                         scissors.data());
}

void CommandBufferVK::BindDescriptorSets(
    const BindDescriptorSetsInfo& info) const {
  const auto bind_point = static_cast<vk::PipelineBindPoint>(info.bind_point);
  const auto pipeline_layout_vk =
      reinterpret_cast<const PipelineLayoutVK*>(info.pipeline_layout);
  std::vector<vk::DescriptorSet> vk_desc_sets;
  vk_desc_sets.reserve(info.desc_sets.size());

  XG_TRACE("bindDescriptorSets: {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           vk::to_string(bind_point),
           static_cast<void*>(
               (VkPipelineLayout)pipeline_layout_vk->pipeline_layout_),
           info.first_set);

  for (const auto& desc_set : info.desc_sets) {
    const auto desc_set_vk = reinterpret_cast<const DescriptorSetVK*>(desc_set);
    vk_desc_sets.emplace_back(desc_set_vk->desc_set_);

    XG_TRACE("  descriptorSet: {}",
             static_cast<void*>((VkDescriptorSet)desc_set_vk->desc_set_));
  }

  std::vector<uint32_t> dynamic_offsets(info.dynamic_offsets.size());
  dynamic_offsets.assign(
      info.dynamic_offsets.begin(),
      info.dynamic_offsets.begin() + info.dynamic_offsets.size());

  if (Logger::Get().IsTraceEnabled()) {
    for (const auto& dynamic_offset : dynamic_offsets) {
      XG_TRACE("  dynamicOffset: {}", dynamic_offset);
    }
  }

  cmd_buffer_.bindDescriptorSets(
      bind_point, pipeline_layout_vk->pipeline_layout_,
      static_cast<uint32_t>(info.first_set), vk_desc_sets, dynamic_offsets);
}

void CommandBufferVK::BindPipeline(const Pipeline& pipeline) const {
  const auto& pipeline_vk = reinterpret_cast<const PipelineVK&>(pipeline);
  const auto bind_point =
      static_cast<vk::PipelineBindPoint>(pipeline_vk.GetBindPoint());

  XG_TRACE("bindPipeline: {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           vk::to_string(bind_point),
           static_cast<void*>((VkPipeline)pipeline_vk.pipeline_));

  cmd_buffer_.bindPipeline(bind_point, pipeline_vk.pipeline_);
}

void CommandBufferVK::BindVertexBuffers(
    const BindVertexBuffersInfo& info) const {
  std::vector<vk::Buffer> vk_buffers;
  vk_buffers.reserve(info.buffers.size());

  XG_TRACE("bindVertexBuffers: {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           info.first_binding);

  int i = 0;
  for (const auto& buf : info.buffers) {
    const auto buf_vk = reinterpret_cast<const BufferVK*>(buf);
    vk_buffers.emplace_back(buf_vk->buffer_);

    XG_TRACE("Buffer: {} {}", static_cast<void*>((VkBuffer)buf_vk->buffer_),
             info.offsets[i++]);
  }

  static_assert(sizeof(size_t) == sizeof(vk::DeviceSize));

  cmd_buffer_.bindVertexBuffers(static_cast<uint32_t>(info.first_binding),
                                static_cast<uint32_t>(vk_buffers.size()),
                                vk_buffers.data(), info.offsets.data());
}

void CommandBufferVK::BindIndexBuffer(const BindIndexBufferInfo& info) const {
  const auto buf_vk = reinterpret_cast<const BufferVK*>(info.buffer);
  const auto index_type = static_cast<vk::IndexType>(info.index_type);

  XG_TRACE("bindIndexBuffer: {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkBuffer)buf_vk->buffer_), info.offset,
           vk::to_string(index_type));

  cmd_buffer_.bindIndexBuffer(buf_vk->buffer_, info.offset, index_type);
}

void CommandBufferVK::DrawIndexed(const DrawIndexedInfo& info) const {
  XG_TRACE("drawIndexed: {} {} {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_), info.index_count,
           info.instance_count, info.first_index, info.vertex_offset,
           info.first_instance);

  cmd_buffer_.drawIndexed(static_cast<uint32_t>(info.index_count),
                          static_cast<uint32_t>(info.instance_count),
                          static_cast<uint32_t>(info.first_index),
                          static_cast<uint32_t>(info.vertex_offset),
                          static_cast<uint32_t>(info.first_instance));
}

void CommandBufferVK::DrawIndexedIndirect(
    const DrawIndexedIndirectInfo& info) const {
  const auto buf_vk = reinterpret_cast<const BufferVK*>(info.buffer);

  XG_TRACE("drawIndexedIndirect: {} {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkBuffer)buf_vk->buffer_), info.offset,
           info.draw_count, info.stride);

  cmd_buffer_.drawIndexedIndirect(buf_vk->buffer_,
                                  static_cast<vk::DeviceSize>(info.offset),
                                  static_cast<uint32_t>(info.draw_count),
                                  static_cast<uint32_t>(info.stride));
}

void CommandBufferVK::ResetQueryPool(const ResetQueryPoolInfo& info) const {
  const auto query_pool_vk =
      reinterpret_cast<const QueryPoolVK*>(info.query_pool);

  XG_TRACE("resetQueryPool: {} {} {} {}",
           static_cast<void*>((VkCommandBuffer)cmd_buffer_),
           static_cast<void*>((VkQueryPool)query_pool_vk->query_pool_),
           info.first_query, info.query_count);

  cmd_buffer_.resetQueryPool(query_pool_vk->query_pool_, info.first_query,
                             info.query_count);
}

void CommandBufferVK::BeginQuery(const QueryInfo& info) const {
  const auto query_pool_vk =
      reinterpret_cast<const QueryPoolVK*>(info.query_pool);

  XG_TRACE(
      "beginQuery: {} {} {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_),
      static_cast<void*>((VkQueryPool)query_pool_vk->query_pool_), info.query);

  cmd_buffer_.beginQuery(query_pool_vk->query_pool_, info.query,
                         vk::QueryControlFlags());
}

void CommandBufferVK::EndQuery(const QueryInfo& info) const {
  const auto query_pool_vk =
      reinterpret_cast<const QueryPoolVK*>(info.query_pool);

  XG_TRACE(
      "endQuery: {} {} {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_),
      static_cast<void*>((VkQueryPool)query_pool_vk->query_pool_), info.query);

  cmd_buffer_.endQuery(query_pool_vk->query_pool_, info.query);
}

void CommandBufferVK::SetEvent(const EventInfo& info) const {
  const auto event_vk = reinterpret_cast<const EventVK*>(info.event);
  const auto stage_mask =
      static_cast<vk::PipelineStageFlagBits>(info.stage_mask);

  XG_TRACE(
      "setEvent: {} {} {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_),
      static_cast<void*>((VkEvent)event_vk->event_), vk::to_string(stage_mask));

  cmd_buffer_.setEvent(event_vk->event_, stage_mask);
}

void CommandBufferVK::ResetEvent(const EventInfo& info) const {
  const auto event_vk = reinterpret_cast<const EventVK*>(info.event);
  const auto stage_mask =
      static_cast<vk::PipelineStageFlagBits>(info.stage_mask);

  XG_TRACE(
      "resetEvent: {} {} {}", static_cast<void*>((VkCommandBuffer)cmd_buffer_),
      static_cast<void*>((VkEvent)event_vk->event_), vk::to_string(stage_mask));

  cmd_buffer_.resetEvent(event_vk->event_, stage_mask);
}

}  // namespace xg
