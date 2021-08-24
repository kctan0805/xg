// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/viewer.h"

#include <cassert>
#include <memory>
#include <vector>

#include "xg/buffer.h"
#include "xg/camera.h"
#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/fence.h"
#include "xg/logger.h"
#include "xg/renderer.h"
#include "xg/utility.h"

namespace xg {

void* UpdateData::Map() {
  auto data = buffer_->GetMappedData();
  if (!data) data = buffer_->MapMemory();
  return data;
}

void UpdateData::Unmap() {
  auto data = buffer_->GetMappedData();
  if (!data) {
    buffer_->UnmapMemory();
  } else {
    MemoryRangeInfo mem_range_info = {0, static_cast<size_t>(-1)};
    buffer_->FlushRange(mem_range_info);
  }
}

void View::InitAcquireNextImage(
    const LayoutAcquireNextImage& lacquire_next_image) {
  auto swapchain = GetSwapchain();

  int frame_count = swapchain->GetFrameCount();
  wait_fences_.reserve(frame_count);
  wait_image_fences_.reserve(frame_count);
  acquire_next_image_infos_.reserve(frame_count);

  for (int i = 0; i < frame_count; ++i) {
    if (lacquire_next_image.lwait_fence) {
      Fence* fence;
      auto lfence = lacquire_next_image.lwait_fence.get();
      if (lfence->lframe) {
        auto fences =
            std::static_pointer_cast<std::vector<std::shared_ptr<Fence>>>(
                lfence->instance);
        fence = (*fences)[i].get();
      } else {
        fence = static_cast<Fence*>(lfence->instance.get());
      }
      assert(fence);
      wait_fences_.emplace_back(fence);
      wait_image_fences_.emplace_back(nullptr);
    }

    AcquireNextImageInfo info;

    info.timeout = lacquire_next_image.timeout;

    if (lacquire_next_image.lsemaphore) {
      auto lsemaphore = lacquire_next_image.lsemaphore.get();
      if (lsemaphore->lframe) {
        auto semaphores =
            std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
                lsemaphore->instance);
        info.semaphore = (*semaphores)[i].get();
      } else {
        info.semaphore = static_cast<Semaphore*>(lsemaphore->instance.get());
      }
    }

    if (lacquire_next_image.lfence) {
      auto lfence = lacquire_next_image.lfence.get();
      if (lfence->lframe) {
        auto fences =
            std::static_pointer_cast<std::vector<std::shared_ptr<Fence>>>(
                lfence->instance);
        info.fence = (*fences)[i].get();
      } else {
        info.fence = static_cast<Fence*>(lfence->instance.get());
      }
    }

    acquire_next_image_infos_.emplace_back(info);
  }
}

void View::InitUpdater(const LayoutUpdater& lupdater) {
  updater_.lbuffers = lupdater.lbuffers;

  for (const auto& lbuffer : updater_.lbuffers) {
    UpdateData update_data;

    if (!lbuffer->lframe) {
      update_data.buffer_ = static_cast<Buffer*>(lbuffer->instance.get());
    }
    updater_.update_data_.emplace_back(update_data);
  }
}

void View::RebuildCommandBuffers() {
  for (const auto& cmd_context : cmd_contexts_) cmd_context->Rebuild();
}

Result View::BuildCommandBuffers() const {
  for (const auto& cmd_context : cmd_contexts_) {
    auto result = cmd_context->Build();
    if (result != Result::kSuccess) return result;
  }
  return Result::kSuccess;
}

void View::UpdateUpdaterData() {
  int i = 0;
  for (const auto& lbuffer : updater_.lbuffers) {
    if (lbuffer->lframe) {
      const auto& buffers =
          std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
              lbuffer->instance);
      updater_.update_data_[i].buffer_ = (*buffers)[curr_frame_].get();
    }
    ++i;
  }
}

void View::UpdateQueueSubmits() {
  for (auto& lqueue_submit : lqueue_submits_) {
    auto queue_submit =
        static_cast<QueueSubmit*>(lqueue_submit->instance.get());
    auto& queue_submit_info = queue_submit->queue_submit_info;
    int i = 0;
    for (auto& submit_info : queue_submit_info.submit_infos) {
      const auto& lsubmit = *lqueue_submit->lsubmits[i];
      int j = 0;
      for (const auto& lwait_semaphore : lsubmit.lwait_semaphores) {
        if (lwait_semaphore->lframe) {
          const auto& semaphores =
              std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
                  lwait_semaphore->instance);

          int curr_frame = lwait_semaphore->lframe->curr_frame;
          int frame = (lsubmit.wait_frame_offsets[j] + curr_frame) %
                      static_cast<int>(semaphores->size());
          if (frame < 0) frame += static_cast<int>(semaphores->size());
          if (!first_round_ || frame <= curr_frame) {
            submit_info.wait_semaphores[j] = (*semaphores)[frame].get();
          }
        }
        ++j;
      }

      j = 0;
      for (const auto& lcmd_buffer : lsubmit.lcmd_buffers) {
        if (lcmd_buffer->lframe) {
          const auto& cmd_buffers = std::static_pointer_cast<
              std::vector<std::shared_ptr<CommandBuffer>>>(
              lcmd_buffer->instance);
          submit_info.cmd_buffers[j] = (*cmd_buffers)[curr_image_].get();
        }
        ++j;
      }

      j = 0;
      for (const auto& lsignal_semaphore : lsubmit.lsignal_semaphores) {
        if (lsignal_semaphore->lframe) {
          const auto& semaphores =
              std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
                  lsignal_semaphore->instance);
          int curr_frame = lsignal_semaphore->lframe->curr_frame;
          submit_info.signal_semaphores[j] = (*semaphores)[curr_frame].get();
        }
        ++j;
      }
      ++i;
    }

    const auto& lfence = lqueue_submit->lfence;
    if (lfence && lfence->lframe) {
      const auto& fences =
          std::static_pointer_cast<std::vector<std::shared_ptr<Fence>>>(
              lfence->instance);
      int curr_frame = lfence->lframe->curr_frame;
      queue_submit_info.fence = (*fences)[curr_frame].get();
    }
  }
}

}  // namespace xg
