// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/renderer.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

#include "xg/buffer.h"
#include "xg/camera.h"
#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/command_pool.h"
#include "xg/descriptor_set.h"
#include "xg/fence.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/queue.h"
#include "xg/render_pass.h"
#include "xg/semaphore.h"
#include "xg/swapchain.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/viewer.h"

namespace xg {

std::shared_ptr<std::vector<std::shared_ptr<Framebuffer>>>
Renderer::CreateFramebuffersOfFrame(LayoutFramebuffer* lframebuffer) {
  assert(lframebuffer->lframe);
  const auto& swapchain = std::static_pointer_cast<Swapchain>(
      lframebuffer->lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  if (!lframebuffer->width || !lframebuffer->height) {
    lframebuffer->width = swapchain->GetWidth();
    lframebuffer->height = swapchain->GetHeight();
  }

  auto framebuffers =
      std::make_shared<std::vector<std::shared_ptr<Framebuffer>>>(frame_count);
  if (!framebuffers) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    lframebuffer->frame = i;

    auto framebuffer = device_->CreateFramebuffer(*lframebuffer);
    if (!framebuffer) return nullptr;

    (*framebuffers)[i] = framebuffer;
  }

  return framebuffers;
}

std::shared_ptr<std::vector<std::shared_ptr<Fence>>>
Renderer::CreateFencesOfFrame(const LayoutFence& lfence) {
  assert(lfence.lframe);
  const auto& swapchain =
      std::static_pointer_cast<Swapchain>(lfence.lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  auto fences =
      std::make_shared<std::vector<std::shared_ptr<Fence>>>(frame_count);
  if (!fences) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    auto fence = device_->CreateFence(lfence);
    if (!fence) return nullptr;

    (*fences)[i] = fence;
  }

  return fences;
}

std::shared_ptr<std::vector<std::shared_ptr<Semaphore>>>
Renderer::CreateSemaphoresOfFrame(const LayoutSemaphore& lsemaphore) {
  assert(lsemaphore.lframe);
  const auto& swapchain = std::static_pointer_cast<Swapchain>(
      lsemaphore.lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  auto semaphores =
      std::make_shared<std::vector<std::shared_ptr<Semaphore>>>(frame_count);
  if (!semaphores) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    auto sem = device_->CreateSemaphore(lsemaphore);
    if (!sem) return nullptr;

    (*semaphores)[i] = sem;
  }

  return semaphores;
}

std::shared_ptr<std::vector<std::shared_ptr<CommandBuffer>>>
Renderer::CreateCommandBuffersOfFrame(
    const std::shared_ptr<LayoutCommandBuffer> lcmd_buffer) {
  assert(lcmd_buffer->lframe);
  const auto& swapchain = std::static_pointer_cast<Swapchain>(
      lcmd_buffer->lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  const auto cmd_pool =
      std::static_pointer_cast<CommandPool>(lcmd_buffer->lcmd_pool->instance);

  std::vector<std::shared_ptr<LayoutCommandBuffer>> lcmd_buffers(frame_count,
                                                                 lcmd_buffer);

  std::vector<std::shared_ptr<CommandBuffer>> cmd_buffers;

  if (!cmd_pool->AllocateCommandBuffers(lcmd_buffers, &cmd_buffers))
    return nullptr;

  auto& frame_cmd_buffers =
      std::make_shared<std::vector<std::shared_ptr<CommandBuffer>>>(
          cmd_buffers);
  if (!frame_cmd_buffers) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  return frame_cmd_buffers;
}

std::shared_ptr<std::vector<std::shared_ptr<Buffer>>>
Renderer::CreateBuffersOfFrame(const LayoutBuffer& lbuffer) {
  assert(lbuffer.lframe);
  const auto& swapchain =
      std::static_pointer_cast<Swapchain>(lbuffer.lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  auto buffers =
      std::make_shared<std::vector<std::shared_ptr<Buffer>>>(frame_count);
  if (!buffers) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    auto buffer = device_->CreateBuffer(lbuffer);
    if (!buffer) return nullptr;

    (*buffers)[i] = buffer;
  }

  return buffers;
}

std::shared_ptr<std::vector<std::shared_ptr<DescriptorSet>>>
Renderer::CreateDescriptorSetsOfFrame(
    const std::shared_ptr<LayoutDescriptorSet> ldesc_set) {
  assert(ldesc_set->lframe);
  const auto& swapchain = std::static_pointer_cast<Swapchain>(
      ldesc_set->lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  const auto desc_pool =
      std::static_pointer_cast<DescriptorPool>(ldesc_set->ldesc_pool->instance);

  std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets(frame_count,
                                                               ldesc_set);
  std::vector<std::shared_ptr<DescriptorSet>> desc_sets;

  if (!desc_pool->AllocateDescriptorSets(ldesc_sets, &desc_sets))
    return nullptr;

  const auto& frame_desc_sets =
      std::make_shared<std::vector<std::shared_ptr<DescriptorSet>>>(desc_sets);
  if (!frame_desc_sets) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  return frame_desc_sets;
}

std::shared_ptr<std::vector<std::shared_ptr<QueryPool>>>
Renderer::CreateQueryPoolsOfFrame(const LayoutQueryPool& lquery_pool) {
  assert(lquery_pool.lframe);
  const auto& swapchain = std::static_pointer_cast<Swapchain>(
      lquery_pool.lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  auto query_pools =
      std::make_shared<std::vector<std::shared_ptr<QueryPool>>>(frame_count);
  if (!query_pools) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    auto query_pool = device_->CreateQueryPool(lquery_pool);
    if (!query_pool) return nullptr;

    (*query_pools)[i] = query_pool;
  }

  return query_pools;
}

std::shared_ptr<std::vector<std::shared_ptr<Event>>>
Renderer::CreateEventsOfFrame(const LayoutEvent& levent) {
  assert(levent.lframe);
  const auto& swapchain =
      std::static_pointer_cast<Swapchain>(levent.lframe->lswapchain->instance);
  int frame_count = swapchain->GetFrameCount();

  auto events =
      std::make_shared<std::vector<std::shared_ptr<Event>>>(frame_count);
  if (!events) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (int i = 0; i < frame_count; ++i) {
    auto event = device_->CreateEvent(levent);
    if (!event) return nullptr;

    (*events)[i] = event;
  }

  return events;
}

std::shared_ptr<Camera> Renderer::CreateCamera(const LayoutCamera& lcamera) {
  auto camera = std::make_shared<Camera>();
  if (!camera) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  camera->Perspective(lcamera.fov, lcamera.width, lcamera.height,
                      lcamera.z_near, lcamera.z_far);
  camera->LookAt(lcamera.eye, lcamera.center, lcamera.up);

  return camera;
}

std::shared_ptr<CommandList> Renderer::CreateCommandList(
    const LayoutCommandList& lcmd_list) {
  auto cmd_list = std::make_shared<CommandList>();
  if (!cmd_list) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  for (const auto& lcmd : lcmd_list.lcmds) {
    switch (lcmd->layout_type) {
      case LayoutType::kFunction: {
        auto cmd = std::make_shared<CommandFunction>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lfunction = std::static_pointer_cast<LayoutFunction>(lcmd);
        cmd->Init(lfunction.get());

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kPipelineBarrier: {
        auto cmd = std::make_shared<CommandPipelineBarrier>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lpipeline_barrier =
            std::static_pointer_cast<LayoutPipelineBarrier>(lcmd);
        if (!cmd->Init(*lpipeline_barrier)) return nullptr;

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kCopyBuffer: {
        auto cmd = std::make_shared<CommandCopyBuffer>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lcopy_buffer = std::static_pointer_cast<LayoutCopyBuffer>(lcmd);
        cmd->Init(*lcopy_buffer);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kDispatch: {
        auto cmd = std::make_shared<CommandDispatch>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto ldispatch = std::static_pointer_cast<LayoutDispatch>(lcmd);
        cmd->Init(*ldispatch);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBeginRenderPass: {
        auto cmd = std::make_shared<CommandBeginRenderPass>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lbegin_render_pass =
            std::static_pointer_cast<LayoutBeginRenderPass>(lcmd);
        cmd->Init(*lbegin_render_pass);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kEndRenderPass: {
        auto cmd = std::make_shared<CommandEndRenderPass>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kSetViewport: {
        auto cmd = std::make_shared<CommandSetViewport>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lset_viewport = std::static_pointer_cast<LayoutSetViewport>(lcmd);
        cmd->Init(*lset_viewport);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kSetScissor: {
        auto cmd = std::make_shared<CommandSetScissor>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lset_scissor = std::static_pointer_cast<LayoutSetScissor>(lcmd);
        cmd->Init(*lset_scissor);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBindDescriptorSets: {
        auto cmd = std::make_shared<CommandBindDescriptorSets>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lbind_desc_sets =
            std::static_pointer_cast<LayoutBindDescriptorSets>(lcmd);

        int align = device_->GetMinUniformBufferOffsetAlignment();

        for (auto& ldynamic_offset : lbind_desc_sets->ldynamic_offsets) {
          auto offset = ldynamic_offset.unit_size;
          if (align > 0) offset = (offset + align - 1) & ~(offset - 1);
          ldynamic_offset.dynamic_offset = offset * ldynamic_offset.unit;
        }

        if (!cmd->Init(*lbind_desc_sets)) return nullptr;

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBindPipeline: {
        auto cmd = std::make_shared<CommandBindPipeline>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lbind_pipeline =
            std::static_pointer_cast<LayoutBindPipeline>(lcmd);
        cmd->Init(*lbind_pipeline);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBindVertexBuffers: {
        auto cmd = std::make_shared<CommandBindVertexBuffers>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lbind_vertex_buffers =
            std::static_pointer_cast<LayoutBindVertexBuffers>(lcmd);
        cmd->Init(*lbind_vertex_buffers);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBindIndexBuffer: {
        auto cmd = std::make_shared<CommandBindIndexBuffer>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lbind_index_buffer =
            std::static_pointer_cast<LayoutBindIndexBuffer>(lcmd);
        cmd->Init(*lbind_index_buffer);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kDraw: {
        auto cmd = std::make_shared<CommandDraw>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto ldraw = std::static_pointer_cast<LayoutDraw>(lcmd);
        cmd->Init(*ldraw);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kDrawIndexed: {
        auto cmd = std::make_shared<CommandDrawIndexed>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto ldraw_indexed = std::static_pointer_cast<LayoutDrawIndexed>(lcmd);
        cmd->Init(*ldraw_indexed);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kDrawIndexedIndirect: {
        auto cmd = std::make_shared<CommandDrawIndexedIndirect>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto ldraw_indexed_indirect =
            std::static_pointer_cast<LayoutDrawIndexedIndirect>(lcmd);

        if (ldraw_indexed_indirect->offset == 0) {
          int align = device_->GetMinUniformBufferOffsetAlignment();
          auto offset = ldraw_indexed_indirect->unit_size;
          if (align > 0) offset = (offset + align - 1) & ~(offset - 1);
          ldraw_indexed_indirect->offset =
              offset * ldraw_indexed_indirect->unit;
        }
        cmd->Init(*ldraw_indexed_indirect);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kBlitImage: {
        auto cmd = std::make_shared<CommandBlitImage>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lblit_image = std::static_pointer_cast<LayoutBlitImage>(lcmd);
        cmd->Init(*lblit_image);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kPushConstants: {
        auto cmd = std::make_shared<CommandPushConstants>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lpush_constants =
            std::static_pointer_cast<LayoutPushConstants>(lcmd);
        cmd->Init(lpush_constants.get());

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kResetQueryPool: {
        auto cmd = std::make_shared<CommandResetQueryPool>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lreset_query_pool =
            std::static_pointer_cast<LayoutResetQueryPool>(lcmd);
        cmd->Init(*lreset_query_pool);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kSetEvent: {
        auto cmd = std::make_shared<CommandSetEvent>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lset_event = std::static_pointer_cast<LayoutSetEvent>(lcmd);
        cmd->Init(*lset_event);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      case LayoutType::kResetEvent: {
        auto cmd = std::make_shared<CommandResetEvent>();
        if (!cmd) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return nullptr;
        }
        auto lreset_event = std::static_pointer_cast<LayoutResetEvent>(lcmd);
        cmd->Init(*lreset_event);

        cmd_list->commands_.emplace_back(cmd);
        lcmd->instance = std::move(cmd);
        break;
      }

      default:
        XG_ERROR("unknown command layout: {}",
                 static_cast<int>(lcmd->layout_type));
        return false;
    }
  }
  return cmd_list;
}

std::shared_ptr<CommandGroup> Renderer::CreateCommandGroup(
    const LayoutCommandGroup& lcmd_group) {
  auto cmd_group = std::make_shared<CommandGroup>();
  if (!cmd_group) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  return cmd_group;
}

std::shared_ptr<CommandContext> Renderer::CreateCommandContext(
    const LayoutCommandContext& lcmd_context) {
  auto cmd_context = std::make_shared<CommandContext>();
  if (!cmd_context) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  if (!cmd_context->Init(lcmd_context)) return nullptr;

  return cmd_context;
}

std::shared_ptr<QueueSubmit> Renderer::CreateQueueSubmit(
    const LayoutQueueSubmit& lqueue_submit) {
  auto queue_submit = std::make_shared<QueueSubmit>();
  if (!queue_submit) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  queue_submit->queue =
      static_cast<Queue*>(lqueue_submit.lqueue->instance.get());

  queue_submit->queue_submit_info.submit_infos.reserve(
      lqueue_submit.lsubmits.size());

  for (const auto& lsubmit : lqueue_submit.lsubmits) {
    SubmitInfo info = {};

    info.wait_semaphores =
        std::vector<Semaphore*>(lsubmit->lwait_semaphores.size(), nullptr);
    info.wait_dst_stage_masks = lsubmit->wait_dst_stage_masks;
    info.cmd_buffers =
        std::vector<CommandBuffer*>(lsubmit->lcmd_buffers.size(), nullptr);
    info.signal_semaphores =
        std::vector<Semaphore*>(lsubmit->lsignal_semaphores.size(), nullptr);

    int i = 0;
    for (const auto& lwait_semaphore : lsubmit->lwait_semaphores) {
      if (!lwait_semaphore->lframe) {
        info.wait_semaphores[i] =
            static_cast<Semaphore*>(lwait_semaphore->instance.get());
      }
      ++i;
    }

    i = 0;
    for (const auto& lcmd_buffer : lsubmit->lcmd_buffers) {
      if (!lcmd_buffer->lframe) {
        info.cmd_buffers[i] =
            static_cast<CommandBuffer*>(lcmd_buffer->instance.get());
      }
      ++i;
    }

    i = 0;
    for (const auto& lsignal_semaphore : lsubmit->lsignal_semaphores) {
      if (!lsignal_semaphore->lframe) {
        info.signal_semaphores[i] =
            static_cast<Semaphore*>(lsignal_semaphore->instance.get());
      }
      ++i;
    }
    queue_submit->queue_submit_info.submit_infos.emplace_back(info);
  }

  if (lqueue_submit.lfence && !lqueue_submit.lfence->lframe) {
    queue_submit->queue_submit_info.fence =
        static_cast<Fence*>(lqueue_submit.lfence->instance.get());
  }

  queue_submit->enabled = lqueue_submit.enabled;

  return queue_submit;
}

std::shared_ptr<Viewer> Renderer::CreateViewer(const LayoutViewer& lviewer) {
  auto viewer = std::make_shared<Viewer>();
  if (!viewer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  viewer->device_ = device_;

  if (!viewer->Init(lviewer)) return nullptr;

  return viewer;
}

std::shared_ptr<QueuePresent> Renderer::CreateQueuePresent(
    const LayoutQueuePresent& lqueue_present) {
  auto queue_present = std::make_shared<QueuePresent>();
  if (!queue_present) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  queue_present->queue =
      static_cast<Queue*>(lqueue_present.lqueue->instance.get());

  auto& present_info = queue_present->present_info;

  present_info.wait_semaphores.resize(lqueue_present.lwait_semaphores.size());

  int i = 0;
  for (const auto& lwait_semaphore : lqueue_present.lwait_semaphores) {
    if (!lwait_semaphore->lframe) {
      present_info.wait_semaphores[i] =
          static_cast<Semaphore*>(lwait_semaphore->instance.get());
    }
    ++i;
  }

  if (lqueue_present.lswapchains.size() > 0) {
    present_info.swapchains.reserve(lqueue_present.lswapchains.size());
    present_info.image_indices.reserve(lqueue_present.lswapchains.size());
    present_info.results.reserve(lqueue_present.lswapchains.size());

    for (const auto& lswapchain : lqueue_present.lswapchains) {
      present_info.swapchains.emplace_back(
          static_cast<Swapchain*>(lswapchain->instance.get()));
      present_info.image_indices.emplace_back(0);
      present_info.results.emplace_back(Result::kSuccess);
    }
  }
  queue_present->enabled = true;

  return queue_present;
}

}  // namespace xg
