// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/window_viewer.h"

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
#include "xg/viewer.h"

namespace xg {

bool WindowViewer::Init(const LayoutWindowViewer& lwin_viewer) {
  win_ = std::static_pointer_cast<Window>(lwin_viewer.lwin->instance);
  if (!win_) {
    XG_ERROR("window not found");
    return false;
  }

  lframe_ = lwin_viewer.lframe;
  if (!lframe_) {
    XG_ERROR("frame not found");
    return false;
  }

  for (const auto& lcamera : lwin_viewer.lcameras) {
    cameras_.emplace_back(std::static_pointer_cast<Camera>(lcamera->instance));
  }

  if (lwin_viewer.loverlay) {
    overlay_ =
        std::static_pointer_cast<Overlay>(lwin_viewer.loverlay->instance);
    if (!overlay_) {
      XG_ERROR("overlay not found");
      return false;
    }

    overlay_->SetDrawHandler(
        [this]() -> void { this->draw_overlay_handler_(); });
  }

  for (const auto& lcmd_context : lwin_viewer.lcmd_contexts) {
    cmd_contexts_.emplace_back(
        std::static_pointer_cast<CommandContext>(lcmd_context->instance));
  }

  if (lwin_viewer.lresizer) {
    const auto& lresizer = *lwin_viewer.lresizer;
    resizer_.limages = lresizer.limages;
    resizer_.limage_views = lresizer.limage_views;
    resizer_.lgraphics_pipelines = lresizer.lgraphics_pipelines;
    resizer_.lframebuffers = lresizer.lframebuffers;
    resizer_.enabled = true;
  }

  SetDrawHandler([this]() -> Result { return this->Draw(); });
  SetShouldExitHandler([this]() -> bool { return this->ShouldClose(); });
  SetResizeHandler([this](int, int) -> void { this->Resize(); });
  win_->SetResizeHandler([this](int width, int height) -> void {
    this->resize_handler_(width, height);
  });

  if (lwin_viewer.lacquire_next_image)
    InitAcquireNextImage(*lwin_viewer.lacquire_next_image);

  if (lwin_viewer.lupdater) InitUpdater(*lwin_viewer.lupdater);

  lqueue_submits_ = lwin_viewer.lqueue_submits;

  if (!InitQueuePresent(lwin_viewer)) return false;

  return true;
}

bool WindowViewer::IsEnabled() const {
  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  return queue_present->enabled;
}

void WindowViewer::Enable() {
  for (auto& lqueue_submit : lqueue_submits_) {
    auto queue_submit =
        static_cast<QueueSubmit*>(lqueue_submit->instance.get());
    queue_submit->enabled = lqueue_submit->enabled;
  }

  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  queue_present->enabled = true;
}

void WindowViewer::Disable() {
  for (auto& lqueue_submit : lqueue_submits_) {
    auto queue_submit =
        static_cast<QueueSubmit*>(lqueue_submit->instance.get());
    queue_submit->enabled = false;
  }

  auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  queue_present->enabled = false;
}

Result WindowViewer::Resize() {
  if (static_cast<unsigned int>(win_->GetWindowFlags() &
                                WindowFlags::kMinimized)) {
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

  auto swapchain = GetSwapchain();
  swapchain->Exit(false);

  // recreate
  int width = 0;
  int height = 0;
  win_->GetDrawableSize(&width, &height);

  auto lswapchain = lframe_->lswapchain.get();
  lswapchain->width = width;
  lswapchain->height = height;

  auto result = swapchain->Init(*lswapchain);
  if (result != Result::kSuccess) return result;

  for (auto& limage : resizer_.limages) {
    auto image = static_cast<Image*>(limage->instance.get());

    limage->width = static_cast<float>(width);
    limage->height = static_cast<float>(height);

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
      float aspect = lscissor->width / lscissor->height;
      lscissor->width = static_cast<float>(width);
      lscissor->height = static_cast<float>(width) / aspect;
    }
    pipelines.emplace_back(
        std::static_pointer_cast<Pipeline>(lpipeline->instance));
  }
  result =
      device_->InitGraphicsPipelines(resizer_.lgraphics_pipelines, &pipelines);
  if (result != Result::kSuccess) return result;

  for (auto& lframebuffer : resizer_.lframebuffers) {
    lframebuffer->width = static_cast<float>(width);
    lframebuffer->height = static_cast<float>(height);

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

  if (overlay_) overlay_->Resize();

  RebuildCommandBuffers();

  return Result::kSuccess;
}

Result WindowViewer::AcquireNextImage() {
  auto fence = wait_fences_[curr_frame_];
  assert(fence);

  fence->Wait();

  auto& info = acquire_next_image_infos_[curr_frame_];
  auto swapchain = GetSwapchain();

  auto result = swapchain->AcquireNextImage(info, &curr_image_);
  if (result == Result::kSuboptimal || result == Result::kErrorOutOfDate) {
    result = Resize();
  }

  if (wait_image_fences_[curr_image_]) wait_image_fences_[curr_image_]->Wait();

  wait_image_fences_[curr_image_] = wait_fences_[curr_frame_];

  fence->Reset();

  return result;
}

void WindowViewer::UpdateQueuePresent() {
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

Result WindowViewer::Draw() {
  const auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  if (queue_present->enabled) {
    auto result = AcquireNextImage();
    if (result != Result::kSuccess) return result;

    UpdateUpdaterData();

    result = update_handler_();
    if (result != Result::kSuccess) return result;

    for (auto& cmd_context : cmd_contexts_) cmd_context->Update(curr_image_);

    UpdateQueueSubmits();
    UpdateQueuePresent();
  }
  return Result::kSuccess;
}

Result WindowViewer::PostUpdate() {
  const auto queue_present =
      static_cast<QueuePresent*>(lqueue_present_->instance.get());
  if (queue_present->enabled) {
    const auto& present_info = queue_present->present_info;

    auto result = present_info.results[0];
    if (result == Result::kSuboptimal || result == Result::kErrorOutOfDate) {
      result = Resize();
    }
    if (result != Result::kSuccess) return result;

    auto swapchain = GetSwapchain();

    curr_frame_ = (curr_frame_ + 1) % swapchain->GetFrameCount();
    if (first_round_ && !curr_frame_) first_round_ = false;
    lframe_->curr_frame = curr_frame_;
  }
  return Result::kSuccess;
}

bool WindowViewer::InitQueuePresent(const LayoutWindowViewer& lviewer) {
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
