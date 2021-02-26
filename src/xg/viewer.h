// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VIEWER_H_
#define XG_VIEWER_H_

#include <functional>
#include <memory>
#include <vector>

#include "xg/buffer.h"
#include "xg/camera.h"
#include "xg/command.h"
#include "xg/device.h"
#include "xg/layout.h"
#include "xg/overlay.h"
#include "xg/swapchain.h"
#include "xg/types.h"
#include "xg/window.h"

namespace xg {

class UpdateData {
 public:
  void* Map();
  void Unmap();

 protected:
  Buffer* buffer_ = nullptr;

  friend class Viewer;
};

class Viewer {
 public:
  Viewer() = default;
  Viewer(const Viewer&) = delete;
  Viewer& operator=(const Viewer&) = delete;
  Viewer(Viewer&&) = delete;
  Viewer& operator=(Viewer&&) = delete;
  virtual ~Viewer() = default;

  void PollEvents() const { win_->PollEvents(); }
  std::shared_ptr<Window> GetWindow() const { return win_; }
  std::shared_ptr<Swapchain> GetSwapchain() const {
    return std::static_pointer_cast<Swapchain>(lframe_->lswapchain->instance);
  }
  std::shared_ptr<Camera> GetCamera() const { return camera_; }

  int GetCurrentFrame() const { return curr_frame_; }
  int GetCurrentImage() const { return curr_image_; }
  UpdateData& GetUpdateData(int index) { return updater_.update_data_[index]; }
  bool ShouldClose() const { return win_->ShouldClose(); }
  bool IsEnabled() const;
  void Enable();
  void Disable();
  Result Resize();
  Result BuildCommandBuffers() const;
  void RebuildCommandBuffers();

  using DrawHandlerType = Result();

  void SetDrawHandler(std::function<DrawHandlerType> handler) {
    draw_handler_ = handler;
  }

  using UpdateHandlerType = Result();

  void SetUpdateHandler(std::function<UpdateHandlerType> handler) {
    update_handler_ = handler;
  }

  using ShouldExitHandlerType = bool();

  void SetShouldExitHandler(std::function<ShouldExitHandlerType> handler) {
    should_exit_handler_ = handler;
  }

  using ResizeHandlerType = void(int, int);

  void SetResizeHandler(std::function<ResizeHandlerType> handler) {
    resize_handler_ = handler;
  }

  using DrawOverlayHandlerType = void();

  void SetDrawOverlayHandler(std::function<DrawOverlayHandlerType> handler) {
    draw_overlay_handler_ = handler;
  }

 protected:
  bool Init(const LayoutViewer& lviewer);
  void InitAcquireNextImage(const LayoutViewer& lviewer);
  void InitUpdaterData(const LayoutViewer& lviewer);
  bool InitQueuePresent(const LayoutViewer& lviewer);
  Result AcquireNextImage();
  void UpdateUpdaterData();
  void UpdateQueueSubmits();
  void UpdateQueuePresent();
  Result Draw();
  Result Update() { return Result::kSuccess; }
  Result PostUpdate();

  std::shared_ptr<Device> device_;
  std::shared_ptr<Window> win_;
  std::shared_ptr<LayoutFrame> lframe_;
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<Overlay> overlay_;
  int curr_frame_ = 0;
  int curr_image_ = 0;
  bool first_round_ = true;
  std::function<DrawHandlerType> draw_handler_ = []() -> Result {
    return Result::kSuccess;
  };
  std::function<UpdateHandlerType> update_handler_ = []() -> Result {
    return Result::kSuccess;
  };
  std::function<ShouldExitHandlerType> should_exit_handler_ = []() -> bool {
    return false;
  };
  std::function<ResizeHandlerType> resize_handler_ = [](int, int) {};
  std::function<DrawOverlayHandlerType> draw_overlay_handler_ = []() {};
  std::vector<std::shared_ptr<CommandContext>> cmd_contexts_;
  std::vector<Fence*> wait_fences_;
  std::vector<AcquireNextImageInfo> acquire_next_image_infos_;
  std::vector<std::shared_ptr<LayoutQueueSubmit>> lqueue_submits_;
  std::shared_ptr<LayoutQueuePresent> lqueue_present_;

  struct {
    bool enabled = false;
    std::vector<std::shared_ptr<LayoutImage>> limages;
    std::vector<std::shared_ptr<LayoutImageView>> limage_views;
    std::vector<std::shared_ptr<LayoutGraphicsPipeline>> lgraphics_pipelines;
    std::vector<std::shared_ptr<LayoutFramebuffer>> lframebuffers;
  } resizer_;

  struct {
    std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;
    std::vector<UpdateData> update_data_;
  } updater_;

  friend class Engine;
  friend class Renderer;
};

}  // namespace xg

#endif  // XG_VIEWER_H_
