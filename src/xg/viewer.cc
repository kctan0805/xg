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

#include "xg/camera.h"
#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/fence.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/logger.h"
#include "xg/pipeline.h"
#include "xg/render_pass.h"
#include "xg/renderer.h"
#include "xg/semaphore.h"
#include "xg/swapchain.h"
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

bool Viewer::Init(const LayoutViewer& lviewer) {
  win_ = std::static_pointer_cast<Window>(lviewer.lwin->instance);
  if (!win_) {
    XG_ERROR("window not found");
    return false;
  }
  lframe_ = lviewer.lframe;
  if (!lframe_) {
    XG_ERROR("frame not found");
    return false;
  }
  camera_ = std::static_pointer_cast<Camera>(lviewer.lcamera->instance);
  if (!camera_) {
    XG_ERROR("camera not found");
    return false;
  }

  for (const auto& lcmd_context : lviewer.lcmd_contexts) {
    cmd_contexts_.emplace_back(
        std::static_pointer_cast<CommandContext>(lcmd_context->instance));
  }

  if (lviewer.lresizer) {
    const auto& lresizer = *lviewer.lresizer;
    resizer_.limages = lresizer.limages;
    resizer_.limage_views = lresizer.limage_views;
    resizer_.lgraphics_pipelines = lresizer.lgraphics_pipelines;
    resizer_.lframebuffers = lresizer.lframebuffers;
    resizer_.enabled = true;
  }

  SetDrawHandler([this]() -> Result { return this->Draw(); });
  SetUpdateHandler([this]() -> Result { return this->Update(); });
  SetShouldExitHandler([this]() -> bool { return this->ShouldClose(); });
  SetResizeHandler([this](int, int) -> void { this->Resize(); });
  win_->SetResizeHandler([this](int width, int height) -> void {
    this->resize_handler_(width, height);
  });

  InitAcquireNextImage(lviewer);
  InitUpdaterData(lviewer);

  lqueue_submits_ = lviewer.lqueue_submits;

  if (!InitQueuePresent(lviewer)) return false;

  return true;
}

bool Viewer::IsEnabled() const {
  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  return queue_present->enabled;
}

void Viewer::Enable() {
  for (auto& lqueue_submit : lqueue_submits_) {
    auto queue_submit =
        static_cast<QueueSubmit*>(lqueue_submit->instance.get());
    queue_submit->enabled = lqueue_submit->enabled;
  }

  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  queue_present->enabled = true;
}

void Viewer::Disable() {
  for (auto& lqueue_submit : lqueue_submits_) {
    auto queue_submit =
        static_cast<QueueSubmit*>(lqueue_submit->instance.get());
    queue_submit->enabled = false;
  }

  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  queue_present->enabled = false;
}

Result Viewer::Resize() {
  if (win_->GetAttrib(WindowAttrib::kIconified)) {
    Disable();
    return Result::kSuccess;
  } else {
    Enable();
  }

  if (!resizer_.enabled) return Result::kSuccess;

  device_->WaitIdle();

  // release
  for (auto& lframebuffer : resizer_.lframebuffers) {
    if (lframebuffer->lframe) {
      auto framebuffers =
          static_cast<std::vector<std::shared_ptr<Framebuffer>>*>(
              lframebuffer->instance.get());
      for (auto& framebuffer : *framebuffers) {
        framebuffer->Exit();
      }
    } else {
      auto framebuffer =
          static_cast<Framebuffer*>(lframebuffer->instance.get());
      framebuffer->Exit();
    }
  }

  for (auto& lpipeline : resizer_.lgraphics_pipelines) {
    auto pipeline = static_cast<Pipeline*>(lpipeline->instance.get());
    pipeline->Exit();
  }

  for (auto& limage_view : resizer_.limage_views) {
    auto image_view = static_cast<ImageView*>(limage_view->instance.get());
    image_view->Exit();
  }

  for (auto& limage : resizer_.limages) {
    auto image = static_cast<Image*>(limage->instance.get());
    image->Exit();
  }

  auto& swapchain = GetSwapchain();
  swapchain->Exit(false);

  // recreate
  int width = 0;
  int height = 0;
  win_->GetFramebufferSize(&width, &height);

  auto lswapchain = lframe_->lswapchain.get();
  lswapchain->width = width;
  lswapchain->height = height;

  auto result = swapchain->Init(*lswapchain);
  if (result != Result::kSuccess) return result;

  for (auto& limage : resizer_.limages) {
    auto image = static_cast<Image*>(limage->instance.get());

    limage->extent.width = width;
    limage->extent.height = height;

    result = image->Init(*limage);
    if (result != Result::kSuccess) return result;
  }

  for (auto& limage_view : resizer_.limage_views) {
    auto image_view = static_cast<ImageView*>(limage_view->instance.get());

    result = image_view->Init(*limage_view);
    if (result != Result::kSuccess) return result;
  }

  std::vector<std::shared_ptr<Pipeline>> pipelines;
  pipelines.reserve(resizer_.lgraphics_pipelines.size());

  for (auto& lpipeline : resizer_.lgraphics_pipelines) {
    for (auto& lviewport : lpipeline->lviewport_state->lviewports) {
      float aspect = lviewport->viewport.width / lviewport->viewport.height;
      lviewport->viewport.width = static_cast<float>(width);
      lviewport->viewport.height = static_cast<float>(width) / aspect;
    }
    for (auto& lscissor : lpipeline->lviewport_state->lscissors) {
      float aspect =
          static_cast<float>(lscissor->rect.width) / lscissor->rect.height;
      lscissor->rect.width = width;
      lscissor->rect.height = static_cast<int>(width / aspect);
    }
    pipelines.emplace_back(
        std::static_pointer_cast<Pipeline>(lpipeline->instance));
  }
  result =
      device_->InitGraphicsPipelines(resizer_.lgraphics_pipelines, &pipelines);
  if (result != Result::kSuccess) return result;

  for (auto& lframebuffer : resizer_.lframebuffers) {
    lframebuffer->width = width;
    lframebuffer->height = height;

    if (lframebuffer->lframe) {
      auto framebuffers =
          static_cast<std::vector<std::shared_ptr<Framebuffer>>*>(
              lframebuffer->instance.get());

      int i = 0;
      for (auto& framebuffer : *framebuffers) {
        lframebuffer->frame = i++;

        result = framebuffer->Init(*lframebuffer);
        if (result != Result::kSuccess) return result;
      }
    } else {
      auto framebuffer =
          static_cast<Framebuffer*>(lframebuffer->instance.get());

      result = framebuffer->Init(*lframebuffer);
      if (result != Result::kSuccess) return result;
    }
  }

  return BuildCommandBuffers();
}

void Viewer::RebuildCommandBuffers() {
  for (const auto& cmd_context : cmd_contexts_) cmd_context->Rebuild();
}

Result Viewer::BuildCommandBuffers() const {
  for (const auto& cmd_context : cmd_contexts_) {
    auto result = cmd_context->Build();
    if (result != Result::kSuccess) return result;
  }
  return Result::kSuccess;
}

Result Viewer::AcquireNextImage() {
  auto fence = wait_fences_[curr_frame_];
  assert(fence);

  fence->Wait();
  fence->Reset();

  auto& info = acquire_next_image_infos_[curr_frame_];
  auto& swapchain = GetSwapchain();

  auto result = swapchain->AcquireNextImage(info, &curr_image_);
  if (result == Result::kSuboptimal || result == Result::kErrorOutOfDate) {
    result = Resize();
  }

  return result;
}

void Viewer::UpdateUpdaterData() {
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

void Viewer::UpdateQueueSubmits() {
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

void Viewer::UpdateQueuePresent() {
  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  auto& present_info = queue_present->present_info;

  int i = 0;
  for (const auto& lwait_semaphore : lqueue_present_->lwait_semaphores) {
    if (lwait_semaphore->lframe) {
      const auto& semaphores =
          std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
              lwait_semaphore->instance);
      int curr_frame = lwait_semaphore->lframe->curr_frame;
      present_info.wait_semaphores[i] = (*semaphores)[curr_frame].get();
    }
    ++i;
  }

  present_info.image_indices[0] = curr_image_;
}

Result Viewer::Draw() {
  const auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  if (queue_present->enabled) {
    auto result = AcquireNextImage();
    if (result != Result::kSuccess) return result;

    UpdateUpdaterData();

    result = update_handler_();
    if (result != Result::kSuccess) return result;

    for (auto& cmd_context : cmd_contexts_) cmd_context->Update(curr_frame_);

    UpdateQueueSubmits();
    UpdateQueuePresent();
  }
  return Result::kSuccess;
}

Result Viewer::PostUpdate() {
  const auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  if (queue_present->enabled) {
    const auto& present_info = queue_present->present_info;

    auto result = present_info.results[0];
    if (result == Result::kSuboptimal || result == Result::kErrorOutOfDate) {
      result = Resize();
    }
    if (result != Result::kSuccess) return result;

    auto& swapchain = GetSwapchain();

    curr_frame_ = (curr_frame_ + 1) % swapchain->GetFrameCount();
    if (first_round_ && !curr_frame_) first_round_ = false;
    lframe_->curr_frame = curr_frame_;
  }
  return Result::kSuccess;
}

void Viewer::InitAcquireNextImage(const LayoutViewer& lviewer) {
  auto& swapchain = GetSwapchain();

  if (lviewer.lacquire_next_image) {
    const auto& lacquire_next_image = lviewer.lacquire_next_image;
    int frame_count = swapchain->GetFrameCount();
    wait_fences_.reserve(frame_count);
    acquire_next_image_infos_.reserve(frame_count);

    for (int i = 0; i < frame_count; ++i) {
      if (lacquire_next_image->lwait_fence) {
        Fence* fence;
        auto lfence = lacquire_next_image->lwait_fence.get();
        if (lfence->lframe) {
          auto& fences =
              std::static_pointer_cast<std::vector<std::shared_ptr<Fence>>>(
                  lfence->instance);
          fence = (*fences)[i].get();
        } else {
          fence = static_cast<Fence*>(lfence->instance.get());
        }
        assert(fence);
        wait_fences_.emplace_back(fence);
      }

      AcquireNextImageInfo info;

      info.timeout = lacquire_next_image->timeout;

      if (lacquire_next_image->lsemaphore) {
        auto lsemaphore = lacquire_next_image->lsemaphore.get();
        if (lsemaphore->lframe) {
          auto& semaphores =
              std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
                  lsemaphore->instance);
          info.semaphore = (*semaphores)[i].get();
        } else {
          info.semaphore = static_cast<Semaphore*>(lsemaphore->instance.get());
        }
      }

      if (lacquire_next_image->lfence) {
        auto lfence = lacquire_next_image->lfence.get();
        if (lfence->lframe) {
          auto& fences =
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
}

void Viewer::InitUpdaterData(const LayoutViewer& lviewer) {
  if (lviewer.lupdater) {
    const auto& lupdater = *lviewer.lupdater;
    updater_.lbuffers = lupdater.lbuffers;
  }

  for (const auto& lbuffer : updater_.lbuffers) {
    UpdateData update_data;

    if (!lbuffer->lframe) {
      update_data.buffer_ = static_cast<Buffer*>(lbuffer->instance.get());
    }
    updater_.update_data_.emplace_back(update_data);
  }
}

bool Viewer::InitQueuePresent(const LayoutViewer& lviewer) {
  lqueue_present_ = lviewer.lqueue_present;
  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  auto& present_info = queue_present->present_info;

  present_info.swapchains.emplace_back(GetSwapchain().get());
  present_info.image_indices.emplace_back(0);
  present_info.results.emplace_back(Result::kSuccess);

  return true;
}

}  // namespace xg
