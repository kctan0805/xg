// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/command.h"

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "xg/buffer.h"
#include "xg/command_buffer.h"
#include "xg/descriptor_set.h"
#include "xg/event.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/query_pool.h"
#include "xg/queue.h"
#include "xg/render_pass.h"
#include "xg/swapchain.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

void CommandList::Build(const CommandInfo& cmd_info) const {
  for (const auto& cmd : commands_) {
    cmd->Build(cmd_info);
  }
}

void CommandGroup::Build(const CommandInfo& cmd_info) const {
  for (const auto& node : nodes_) {
    node->Build(cmd_info);
  }
}

bool CommandContext::Init(const LayoutCommandContext& lcmd_context) {
  cmd_group_ =
      std::static_pointer_cast<CommandGroup>(lcmd_context.lcmd_group->instance);
  lcmd_buffer_ = lcmd_context.lcmd_buffer;

  if (lcmd_buffer_->lframe) {
    auto cmd_buffers =
        std::static_pointer_cast<std::vector<std::shared_ptr<CommandBuffer>>>(
            lcmd_buffer_->instance);

    need_rebuilds_ = std::vector<bool>(cmd_buffers->size(), false);
  } else {
    need_rebuilds_ = std::vector<bool>(1, false);
  }

  return true;
}

void CommandContext::Rebuild() {
  for (auto need_rebuild : need_rebuilds_) need_rebuild = true;
}

Result CommandContext::Update(int frame) {
  CommandBuffer* cmd_buffer;

  if (lcmd_buffer_->lframe) {
    assert(frame < need_rebuilds_.size());
    if (!need_rebuilds_[frame]) return Result::kSuccess;

    auto cmd_buffers =
        std::static_pointer_cast<std::vector<std::shared_ptr<CommandBuffer>>>(
            lcmd_buffer_->instance);

    cmd_buffer = (*cmd_buffers)[frame].get();

  } else {
    if (!need_rebuilds_[0]) return Result::kSuccess;

    cmd_buffer = static_cast<CommandBuffer*>(lcmd_buffer_->instance.get());

    frame = 0;
  }
  assert(cmd_group_);
  assert(lcmd_buffer_);

  cmd_buffer->Reset();

  CommandBufferBeginInfo begin_info = {};
  begin_info.usage = CommandBufferUsage::kSimultaneousUse;

  Result result = cmd_buffer->Begin(begin_info);
  if (result != Result::kSuccess) return result;

  CommandInfo info = {};
  info.cmd_buffer = cmd_buffer;
  info.frame = frame;

  cmd_group_->Build(info);
  cmd_buffer->End();

  need_rebuilds_[frame] = false;

  return Result::kSuccess;
}

Result CommandContext::Build() {
  assert(cmd_group_);
  assert(lcmd_buffer_);

  CommandBufferBeginInfo begin_info = {};
  begin_info.usage = CommandBufferUsage::kSimultaneousUse;
  CommandInfo info = {};

  if (lcmd_buffer_->lframe) {
    auto cmd_buffers =
        std::static_pointer_cast<std::vector<std::shared_ptr<CommandBuffer>>>(
            lcmd_buffer_->instance);

    for (int i = 0; i < cmd_buffers->size(); ++i) {
      auto& cmd_buffer = (*cmd_buffers)[i];

      info.cmd_buffer = cmd_buffer.get();
      info.frame = i;

      cmd_buffer->Reset();

      Result result = cmd_buffer->Begin(begin_info);
      if (result != Result::kSuccess) return result;

      cmd_group_->Build(info);
      cmd_buffer->End();

      need_rebuilds_[i] = false;
    }
  } else {
    auto cmd_buffer =
        std::static_pointer_cast<CommandBuffer>(lcmd_buffer_->instance);

    cmd_buffer->Reset();

    Result result = cmd_buffer->Begin(begin_info);
    if (result != Result::kSuccess) return result;

    info.cmd_buffer = cmd_buffer.get();

    cmd_group_->Build(info);
    cmd_buffer->End();

    need_rebuilds_[0] = false;
  }
  return Result::kSuccess;
}

CommandBuffer* CommandContext::GetCommandBuffer(int frame) const {
  if (lcmd_buffer_->lframe) {
    auto cmd_buffers =
        std::static_pointer_cast<std::vector<std::shared_ptr<CommandBuffer>>>(
            lcmd_buffer_->instance);
    return (*cmd_buffers)[frame].get();
  } else {
    return static_cast<CommandBuffer*>(lcmd_buffer_->instance.get());
  }
}

void CommandFunction::Init(LayoutFunction* lfunction) {
  if (lfunction->ldata) {
    data_ = lfunction->ldata->data;
  }
}

bool CommandPipelineBarrier::Init(
    const LayoutPipelineBarrier& lpipeline_barrier) {
  info_.src_stage_mask = lpipeline_barrier.src_stage_mask;
  info_.dst_stage_mask = lpipeline_barrier.dst_stage_mask;
  info_.dependency_flags = lpipeline_barrier.dependency_flags;

  for (const auto& lbuffer_memory_barrier :
       lpipeline_barrier.lbuffer_memory_barriers) {
    BufferMemoryBarrier barrier = {};
    barrier.src_access_mask = lbuffer_memory_barrier->src_access_mask;
    barrier.dst_access_mask = lbuffer_memory_barrier->dst_access_mask;

    if (lbuffer_memory_barrier->lsrc_queue) {
      auto queue = std::static_pointer_cast<Queue>(
                       lbuffer_memory_barrier->lsrc_queue->instance)
                       .get();
      barrier.src_queue_family_index = queue->GetQueueFamilyIndex();
    }

    if (lbuffer_memory_barrier->ldst_queue) {
      auto queue = std::static_pointer_cast<Queue>(
                       lbuffer_memory_barrier->ldst_queue->instance)
                       .get();
      barrier.dst_queue_family_index = queue->GetQueueFamilyIndex();
    }

    barrier.offset = lbuffer_memory_barrier->offset;
    barrier.size = lbuffer_memory_barrier->size;

    info_.buffer_barriers.emplace_back(barrier);
  }
  lbuffer_memory_barriers_ = lpipeline_barrier.lbuffer_memory_barriers;

  int i = 0;
  for (const auto& limage_memory_barrier :
       lpipeline_barrier.limage_memory_barriers) {
    ImageMemoryBarrier barrier = {};
    barrier.src_access_mask = limage_memory_barrier->src_access_mask;
    barrier.dst_access_mask = limage_memory_barrier->dst_access_mask;
    barrier.old_layout = limage_memory_barrier->old_layout;
    barrier.new_layout = limage_memory_barrier->new_layout;

    if (limage_memory_barrier->lsrc_queue) {
      auto queue = std::static_pointer_cast<Queue>(
                       limage_memory_barrier->lsrc_queue->instance)
                       .get();
      barrier.src_queue_family_index = queue->GetQueueFamilyIndex();
    }

    if (limage_memory_barrier->ldst_queue) {
      auto queue = std::static_pointer_cast<Queue>(
                       limage_memory_barrier->ldst_queue->instance)
                       .get();
      barrier.dst_queue_family_index = queue->GetQueueFamilyIndex();
    }

    if (limage_memory_barrier->limage) {
      barrier.image = std::static_pointer_cast<Image>(
                          limage_memory_barrier->limage->instance)
                          .get();
    } else {
      assert(limage_memory_barrier->lswapchain);

      if (!swapchains_) {
        swapchains_ = std::make_unique<std::vector<Swapchain*>>(
            lpipeline_barrier.limage_memory_barriers.size());
        if (!swapchains_) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return false;
        }
      }

      (*swapchains_)[i] = std::static_pointer_cast<Swapchain>(
                              limage_memory_barrier->lswapchain->instance)
                              .get();
    }

    barrier.subresource_range.aspect_mask =
        limage_memory_barrier->image_subresource_range.aspect_mask;
    barrier.subresource_range.base_mip_level =
        limage_memory_barrier->image_subresource_range.base_mip_level;
    barrier.subresource_range.level_count =
        limage_memory_barrier->image_subresource_range.level_count;
    barrier.subresource_range.base_array_layer =
        limage_memory_barrier->image_subresource_range.base_array_layer;
    barrier.subresource_range.layer_count =
        limage_memory_barrier->image_subresource_range.layer_count;
    info_.image_barriers.emplace_back(barrier);

    ++i;
  }
  return true;
}

void CommandPipelineBarrier::Build(const CommandInfo& cmd_info) const {
  auto info = info_;

  int i = 0;
  for (const auto& lbuffer_memory_barrier : lbuffer_memory_barriers_) {
    assert(lbuffer_memory_barrier->lbuffer);
    if (lbuffer_memory_barrier->lbuffer->lframe) {
      const auto& buffers =
          std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
              lbuffer_memory_barrier->lbuffer->instance);
      info.buffer_barriers[i].buffer = (*buffers)[cmd_info.frame].get();
    } else {
      info.buffer_barriers[i].buffer =
          std::static_pointer_cast<Buffer>(
              lbuffer_memory_barrier->lbuffer->instance)
              .get();
    }
    ++i;
  }

  if (swapchains_) {
    i = 0;
    for (auto& swapchain : *swapchains_) {
      if (swapchain) {
        assert(cmd_info.frame < swapchain->GetFrameCount());
        info.image_barriers[i].image =
            const_cast<Image*>(swapchain->GetImage(cmd_info.frame).get());
      }
      ++i;
    }
  }
  cmd_info.cmd_buffer->PipelineBarrier(info);
}

void CommandCopyBuffer::Init(const LayoutCopyBuffer& lcopy_buffer) {
  info_.src_buffer =
      static_cast<Buffer*>(lcopy_buffer.lsrc_buffer->instance.get());
  info_.dst_buffer =
      static_cast<Buffer*>(lcopy_buffer.ldst_buffer->instance.get());
  info_.regions = lcopy_buffer.regions;
}

void CommandCopyBuffer::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->CopyBuffer(info_);
}

void CommandDispatch::Init(const LayoutDispatch& ldispatch) {
  info_.group_count_x = ldispatch.group_count_x;
  info_.group_count_y = ldispatch.group_count_y;
  info_.group_count_z = ldispatch.group_count_z;
}

void CommandDispatch::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->Dispatch(info_);
}

void CommandBeginRenderPass::Init(
    const LayoutBeginRenderPass& lbegin_render_pass) {
  info_.render_pass = std::static_pointer_cast<RenderPass>(
                          lbegin_render_pass.lrender_pass->instance)
                          .get();

  if (lbegin_render_pass.lframebuffer->lframe) {
    frame_framebuffers_ =
        std::static_pointer_cast<std::vector<std::shared_ptr<Framebuffer>>>(
            lbegin_render_pass.lframebuffer->instance);
  } else {
    info_.framebuffer = static_cast<Framebuffer*>(
        lbegin_render_pass.lframebuffer->instance.get());
  }

  info_.clear_values = lbegin_render_pass.clear_values;

  rect_x_ = lbegin_render_pass.rect_x;
  rect_y_ = lbegin_render_pass.rect_y;
  rect_width_ = lbegin_render_pass.rect_width;
  rect_height_ = lbegin_render_pass.rect_height;

  info_.rect.x = static_cast<int>(rect_x_);
  info_.rect.y = static_cast<int>(rect_y_);
  info_.rect.width = static_cast<int>(rect_width_);
  info_.rect.height = static_cast<int>(rect_height_);
}

void CommandBeginRenderPass::Build(const CommandInfo& cmd_info) const {
  auto info = info_;
  if (frame_framebuffers_) {
    assert(cmd_info.frame < frame_framebuffers_->size());
    info.framebuffer =
        const_cast<Framebuffer*>((*frame_framebuffers_)[cmd_info.frame].get());
  }

  if (rect_x_ < 1.0f) {
    info.rect.x = static_cast<int>(rect_x_ * info.framebuffer->GetWidth());
  }

  if (rect_y_ < 1.0f) {
    info.rect.y = static_cast<int>(rect_y_ * info.framebuffer->GetHeight());
  }

  if (rect_width_ == 0.0f) {
    info.rect.width = static_cast<int>(info.framebuffer->GetWidth());
  } else if (rect_width_ < 1.0f) {
    info.rect.width =
        static_cast<int>(rect_width_ * info.framebuffer->GetWidth());
  }

  if (rect_height_ == 0.0f) {
    info.rect.height = static_cast<int>(info.framebuffer->GetHeight());
  } else if (rect_height_ < 1.0f) {
    info.rect.height =
        static_cast<int>(rect_height_ * info.framebuffer->GetHeight());
  }

  cmd_info.cmd_buffer->BeginRenderPass(info);
}

void CommandEndRenderPass::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->EndRenderPass();
}

void CommandSetViewport::Init(const LayoutSetViewport& lset_viewport) {
  info_.first_viewport = lset_viewport.first_viewport;

  for (const auto& lviewport : lset_viewport.lviewports) {
    info_.viewports.emplace_back(lviewport->viewport);
  }
}

void CommandSetViewport::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->SetViewport(info_);
}

void CommandSetScissor::Init(const LayoutSetScissor& lset_scissor) {
  info_.first_scissor = lset_scissor.first_scissor;

  for (const auto& lscissor : lset_scissor.lscissors) {
    std::array<float, 4> rect = {lscissor->x, lscissor->y, lscissor->width,
                                 lscissor->height};
    info_.scissors.emplace_back(std::move(rect));
  }
}

void CommandSetScissor::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->SetScissor(info_);
}

bool CommandBindDescriptorSets::Init(
    const LayoutBindDescriptorSets& lbind_desc_sets) {
  info_.bind_point = lbind_desc_sets.pipeline_bind_point;
  info_.pipeline_layout =
      std::static_pointer_cast<PipelineLayout>(lbind_desc_sets.layout->instance)
          .get();
  info_.first_set = lbind_desc_sets.first_set;

  info_.desc_sets =
      std::vector<DescriptorSet*>(lbind_desc_sets.ldesc_sets.size());

  for (int i = 0; i < lbind_desc_sets.ldesc_sets.size(); ++i) {
    const auto ldesc_set = lbind_desc_sets.ldesc_sets[i];
    if (ldesc_set->lframe) {
      if (!frame_desc_sets_) {
        frame_desc_sets_ = std::make_unique<std::vector<
            std::shared_ptr<std::vector<std::shared_ptr<DescriptorSet>>>>>(
            lbind_desc_sets.ldesc_sets.size());
        if (!frame_desc_sets_) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return false;
        }
      }

      (*frame_desc_sets_)[i] =
          std::static_pointer_cast<std::vector<std::shared_ptr<DescriptorSet>>>(
              ldesc_set->instance);
    } else {
      info_.desc_sets[i] =
          std::static_pointer_cast<DescriptorSet>(ldesc_set->instance).get();
    }
  }

  info_.dynamic_offsets.resize(lbind_desc_sets.ldynamic_offsets.size());
  for (int i = 0; i < info_.dynamic_offsets.size(); ++i) {
    info_.dynamic_offsets[i] =
        lbind_desc_sets.ldynamic_offsets[i].dynamic_offset;
  }
  return true;
}

void CommandBindDescriptorSets::Build(const CommandInfo& cmd_info) const {
  if (frame_desc_sets_) {
    auto info = info_;
    int i = 0;
    for (const auto& desc_sets : *frame_desc_sets_) {
      if (desc_sets) {
        assert(cmd_info.frame < desc_sets->size());
        auto desc_set = std::static_pointer_cast<DescriptorSet>(
            (*desc_sets)[cmd_info.frame]);
        assert(desc_set);
        info.desc_sets[i] = desc_set.get();
      }
      ++i;
    }
    cmd_info.cmd_buffer->BindDescriptorSets(info);
  } else {
    cmd_info.cmd_buffer->BindDescriptorSets(info_);
  }
}

void CommandBindPipeline::Init(const LayoutBindPipeline& lbind_pipeline) {
  pipeline_ =
      std::static_pointer_cast<Pipeline>(lbind_pipeline.lpipeline->instance)
          .get();
}

void CommandBindPipeline::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->BindPipeline(*pipeline_);
}

void CommandBindVertexBuffers::Init(
    const LayoutBindVertexBuffers& lbind_vertex_buffers) {
  info_.first_binding = lbind_vertex_buffers.first_binding;
  info_.buffers.resize(lbind_vertex_buffers.lbuffers.size());
  info_.offsets = lbind_vertex_buffers.offsets;
  lbuffers_ = lbind_vertex_buffers.lbuffers;
}

void CommandBindVertexBuffers::Build(const CommandInfo& cmd_info) const {
  auto info = info_;

  int i = 0;
  for (const auto& lbuffer : lbuffers_) {
    info.buffers[i] = static_cast<Buffer*>(lbuffer->instance.get());
    ++i;
  }

  cmd_info.cmd_buffer->BindVertexBuffers(info);
}

void CommandBindIndexBuffer::Init(
    const LayoutBindIndexBuffer& lbind_index_buffer) {
  info_.offset = lbind_index_buffer.offset;
  info_.index_type = lbind_index_buffer.index_type;
  lbuffer_ = lbind_index_buffer.lbuffer;
}

void CommandBindIndexBuffer::Build(const CommandInfo& cmd_info) const {
  auto info = info_;

  info.buffer = static_cast<Buffer*>(lbuffer_->instance.get());

  cmd_info.cmd_buffer->BindIndexBuffer(info);
}

void CommandDraw::Init(const LayoutDraw& ldraw) {
  info_.vertex_count = ldraw.vertex_count;
  info_.instance_count = ldraw.instance_count;
  info_.first_vertex = ldraw.first_vertex;
  info_.first_instance = ldraw.first_instance;
}

void CommandDraw::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->Draw(info_);
}

void CommandDrawIndexed::Init(const LayoutDrawIndexed& ldraw_indexed) {
  info_.index_count = ldraw_indexed.index_count;
  info_.instance_count = ldraw_indexed.instance_count;
  info_.first_index = ldraw_indexed.first_index;
  info_.vertex_offset = ldraw_indexed.vertex_offset;
  info_.first_instance = ldraw_indexed.first_instance;
}

void CommandDrawIndexed::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->DrawIndexed(info_);
}

void CommandDrawIndexedIndirect::Init(
    const LayoutDrawIndexedIndirect& ldraw_indexed_indirect) {
  assert(!ldraw_indexed_indirect.lbuffer->lframe);
  info_.buffer =
      std::static_pointer_cast<Buffer>(ldraw_indexed_indirect.lbuffer->instance)
          .get();
  info_.offset = ldraw_indexed_indirect.offset;
  info_.draw_count = ldraw_indexed_indirect.draw_count;
  info_.stride = ldraw_indexed_indirect.stride;
}

void CommandDrawIndexedIndirect::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->DrawIndexedIndirect(info_);
}

void CommandBlitImage::Init(const LayoutBlitImage& lblit_image) {
  if (lblit_image.lsrc_image) {
    info_.src_image =
        std::static_pointer_cast<Image>(lblit_image.lsrc_image->instance).get();
  } else {
    assert(lblit_image.lsrc_swapchain);
    src_swapchain_ = std::static_pointer_cast<Swapchain>(
                         lblit_image.lsrc_swapchain->instance)
                         .get();
  }
  info_.src_image_layout = lblit_image.src_image_layout;

  if (lblit_image.ldst_image) {
    info_.dst_image =
        std::static_pointer_cast<Image>(lblit_image.ldst_image->instance).get();
  } else {
    assert(lblit_image.ldst_swapchain);
    dst_swapchain_ = std::static_pointer_cast<Swapchain>(
                         lblit_image.ldst_swapchain->instance)
                         .get();
  }
  info_.dst_image_layout = lblit_image.dst_image_layout;

  for (const auto& region : lblit_image.regions) {
    xg::ImageBlit blit = {};

    blit.src_subresource = region.src_subresource;
    blit.src_offsets[0].x = static_cast<int>(region.src_offsets[0][0]);
    blit.src_offsets[0].y = static_cast<int>(region.src_offsets[0][1]);
    blit.src_offsets[0].z = static_cast<int>(region.src_offsets[0][2]);
    blit.src_offsets[1].x = static_cast<int>(region.src_offsets[1][0]);
    blit.src_offsets[1].y = static_cast<int>(region.src_offsets[1][1]);
    blit.src_offsets[1].z = static_cast<int>(region.src_offsets[1][2]);
    blit.dst_subresource = region.dst_subresource;
    blit.dst_offsets[0].x = static_cast<int>(region.dst_offsets[0][0]);
    blit.dst_offsets[0].y = static_cast<int>(region.dst_offsets[0][1]);
    blit.dst_offsets[0].z = static_cast<int>(region.dst_offsets[0][2]);
    blit.dst_offsets[1].x = static_cast<int>(region.dst_offsets[1][0]);
    blit.dst_offsets[1].y = static_cast<int>(region.dst_offsets[1][1]);
    blit.dst_offsets[1].z = static_cast<int>(region.dst_offsets[1][2]);

    info_.regions.emplace_back(std::move(blit));

    CommandImageBlitOffsets offsets;

    offsets.src_offsets[0][0] = region.src_offsets[0][0];
    offsets.src_offsets[0][1] = region.src_offsets[0][1];
    offsets.src_offsets[1][0] = region.src_offsets[1][0];
    offsets.src_offsets[1][1] = region.src_offsets[1][1];
    offsets.dst_offsets[0][0] = region.dst_offsets[0][0];
    offsets.dst_offsets[0][1] = region.dst_offsets[0][1];
    offsets.dst_offsets[1][0] = region.dst_offsets[1][0];
    offsets.dst_offsets[1][1] = region.dst_offsets[1][1];

    offsets_.emplace_back(std::move(offsets));
  }

  info_.filter = lblit_image.filter;
}

void CommandBlitImage::Build(const CommandInfo& cmd_info) const {
  auto info = info_;

  if (src_swapchain_) {
    assert(cmd_info.frame < src_swapchain_->GetFrameCount());
    info.src_image = src_swapchain_->GetImage(cmd_info.frame).get();
  }

  if (dst_swapchain_) {
    assert(cmd_info.frame < dst_swapchain_->GetFrameCount());
    info.dst_image = dst_swapchain_->GetImage(cmd_info.frame).get();
  }

  for (int i = 0; i < offsets_.size(); ++i) {
    const auto& offsets = offsets_[i];
    auto& region = info.regions[i];

    if (offsets.src_offsets[0][0] < 1.0f) {
      region.src_offsets[0].x = static_cast<int>(offsets.src_offsets[0][0] *
                                                 info.src_image->GetWidth());
    }

    if (offsets.src_offsets[0][1] < 1.0f) {
      region.src_offsets[0].y = static_cast<int>(offsets.src_offsets[0][1] *
                                                 info.src_image->GetHeight());
    }

    if (offsets.src_offsets[1][0] == 0.0f) {
      region.src_offsets[1].x = static_cast<int>(info.src_image->GetWidth());
    } else if (offsets.src_offsets[1][0] < 1.0f) {
      region.src_offsets[1].x = static_cast<int>(offsets.src_offsets[1][0] *
                                                 info.src_image->GetWidth());
    }

    if (offsets.src_offsets[1][1] == 0.0f) {
      region.src_offsets[1].y = static_cast<int>(info.src_image->GetHeight());
    } else if (offsets.src_offsets[1][1] < 1.0f) {
      region.src_offsets[1].y = static_cast<int>(offsets.src_offsets[1][1] *
                                                 info.src_image->GetHeight());
    }

    if (offsets.dst_offsets[0][0] < 1.0f) {
      region.dst_offsets[0].x = static_cast<int>(offsets.dst_offsets[0][0] *
                                                 info.dst_image->GetWidth());
    }

    if (offsets.dst_offsets[0][1] < 1.0f) {
      region.dst_offsets[0].y = static_cast<int>(offsets.dst_offsets[0][1] *
                                                 info.dst_image->GetHeight());
    }

    if (offsets.dst_offsets[1][0] == 0.0f) {
      region.dst_offsets[1].x = static_cast<int>(info.dst_image->GetWidth());
    } else if (offsets.dst_offsets[1][0] < 1.0f) {
      region.dst_offsets[1].x = static_cast<int>(offsets.dst_offsets[1][0] *
                                                 info.dst_image->GetWidth());
    }

    if (offsets.dst_offsets[1][1] == 0.0f) {
      region.dst_offsets[1].y = static_cast<int>(info.dst_image->GetHeight());
    } else if (offsets.dst_offsets[1][1] < 1.0f) {
      region.dst_offsets[1].y = static_cast<int>(offsets.dst_offsets[1][1] *
                                                 info.dst_image->GetHeight());
    }
  }

  cmd_info.cmd_buffer->BlitImage(info);
}

void CommandPushConstants::Init(LayoutPushConstants* lpush_constants) {
  info_.layout = std::static_pointer_cast<PipelineLayout>(
                     lpush_constants->llayout->instance)
                     .get();
  info_.stage_flags = lpush_constants->stage_flags;
  info_.offset = lpush_constants->offset;

  if (lpush_constants->ldata) {
    data_ = lpush_constants->ldata->data;
    info_.size = data_.size();
    info_.values = data_.data();
  } else {
    info_.size = lpush_constants->size;
    info_.values = lpush_constants->values;
  }
}

void CommandPushConstants::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->PushConstants(info_);
}

void CommandResetQueryPool::Init(
    const LayoutResetQueryPool& lreset_query_pool) {
  if (lreset_query_pool.lquery_pool->lframe) {
    lquery_pool_ = lreset_query_pool.lquery_pool;
  } else {
    info_.query_pool =
        static_cast<QueryPool*>(lreset_query_pool.lquery_pool->instance.get());
  }
  info_.first_query = lreset_query_pool.first_query;
  info_.query_count = lreset_query_pool.query_count;
}  // namespace xg

void CommandResetQueryPool::Build(const CommandInfo& cmd_info) const {
  if (info_.query_pool) {
    cmd_info.cmd_buffer->ResetQueryPool(info_);
  } else {
    assert(lquery_pool_);
    assert(lquery_pool_->lframe);
    auto info = info_;
    auto query_pools =
        std::static_pointer_cast<std::vector<std::shared_ptr<QueryPool>>>(
            lquery_pool_->instance);
    info.query_pool = (*query_pools)[cmd_info.frame].get();
  }
}

void CommandSetEvent::Init(const LayoutSetEvent& lset_event) {
  if (lset_event.levent->lframe) {
    frame_events_ =
        std::static_pointer_cast<std::vector<std::shared_ptr<Event>>>(
            lset_event.levent->instance);
  } else {
    info_.event = static_cast<Event*>(lset_event.levent->instance.get());
  }
  info_.stage_mask = lset_event.stage_mask;
}

void CommandSetEvent::Build(const CommandInfo& cmd_info) const {
  if (frame_events_) {
    assert(cmd_info.frame < frame_events_->size());
    auto info = info_;
    info.event = const_cast<Event*>((*frame_events_)[cmd_info.frame].get());
  } else {
    cmd_info.cmd_buffer->SetEvent(info_);
  }
}

void CommandResetEvent::Init(const LayoutResetEvent& lreset_event) {
  if (lreset_event.levent->lframe) {
    frame_events_ =
        std::static_pointer_cast<std::vector<std::shared_ptr<Event>>>(
            lreset_event.levent->instance);
  } else {
    info_.event = static_cast<Event*>(lreset_event.levent->instance.get());
  }
  info_.stage_mask = lreset_event.stage_mask;
}

void CommandResetEvent::Build(const CommandInfo& cmd_info) const {
  if (frame_events_) {
    assert(cmd_info.frame < frame_events_->size());
    auto info = info_;
    info.event = const_cast<Event*>((*frame_events_)[cmd_info.frame].get());
  } else {
    cmd_info.cmd_buffer->SetEvent(info_);
  }
}

void CommandNextSubpass::Init(const LayoutNextSubpass& lnext_subpass) {
  info_.contents = lnext_subpass.contents;
}

void CommandNextSubpass::Build(const CommandInfo& cmd_info) const {
  cmd_info.cmd_buffer->NextSubpass(info_);
}

}  // namespace xg
