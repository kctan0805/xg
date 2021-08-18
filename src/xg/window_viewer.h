// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_WINDOW_VIEWER_H_
#define XG_WINDOW_VIEWER_H_

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
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

class WindowViewer : public Viewer {
 public:
  WindowViewer() = default;
  WindowViewer(const WindowViewer&) = delete;
  WindowViewer& operator=(const WindowViewer&) = delete;
  WindowViewer(WindowViewer&&) = delete;
  WindowViewer& operator=(WindowViewer&&) = delete;
  virtual ~WindowViewer() = default;

  void PollEvents() override { win_->PollEvents(); }
  std::shared_ptr<Window> GetWindow() const { return win_; }
  std::shared_ptr<Swapchain> GetSwapchain() const {
    return std::static_pointer_cast<Swapchain>(lframe_->lswapchain->instance);
  }

  bool ShouldClose() const override { return win_->ShouldClose(); }
  bool IsEnabled() const;
  void Enable();
  void Disable();
  Result Resize();

  using ResizeHandlerType = void(int, int);

  void SetResizeHandler(std::function<ResizeHandlerType> handler) {
    resize_handler_ = handler;
  }

  using DrawOverlayHandlerType = void();

  void SetDrawOverlayHandler(std::function<DrawOverlayHandlerType> handler) {
    draw_overlay_handler_ = handler;
  }

 protected:
  bool Init(const LayoutWindowViewer& lwin_viewer);
  void InitAcquireNextImage(const LayoutWindowViewer& lviewer);
  bool InitQueuePresent(const LayoutWindowViewer& lviewer);
  Result AcquireNextImage();
  void UpdateQueueSubmits();
  void UpdateQueuePresent();
  Result Draw() override;
  Result PostUpdate() override;

  std::shared_ptr<Device> device_;
  std::shared_ptr<Window> win_;
  std::shared_ptr<LayoutFrame> lframe_;
  std::shared_ptr<Overlay> overlay_;

  std::function<ResizeHandlerType> resize_handler_ = [](int, int) {};
  std::function<DrawOverlayHandlerType> draw_overlay_handler_ = []() {};
  std::vector<AcquireNextImageInfo> acquire_next_image_infos_;
  std::shared_ptr<LayoutQueuePresent> lqueue_present_;

  struct {
    bool enabled = false;
    std::vector<std::shared_ptr<LayoutImage>> limages;
    std::vector<std::shared_ptr<LayoutImageView>> limage_views;
    std::vector<std::shared_ptr<LayoutGraphicsPipeline>> lgraphics_pipelines;
    std::vector<std::shared_ptr<LayoutFramebuffer>> lframebuffers;
  } resizer_;

  friend class Engine;
  friend class Renderer;
};

}  // namespace xg

#endif  // XG_WINDOW_VIEWER_H_
