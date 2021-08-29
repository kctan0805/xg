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
#include "xg/layout.h"
#include "xg/swapchain.h"
#include "xg/types.h"

namespace xg {

class UpdateData {
 public:
  void* Map();
  void Unmap();

 protected:
  Buffer* buffer_ = nullptr;

  friend class View;
};

class View {
 public:
  View() = default;

  int GetCurrentFrame() const { return curr_frame_; }
  int GetCurrentImage() const { return curr_image_; }
  std::shared_ptr<Camera> GetCamera() const { return camera_; }
  UpdateData* GetUpdateData(int index) { return &updater_.update_data_[index]; }
  std::shared_ptr<Swapchain> GetSwapchain() const {
    return std::static_pointer_cast<Swapchain>(lframe_->lswapchain->instance);
  }
  void* GetCustomData() const { return custom_data_; }
  void SetCustomData(void* data) { custom_data_ = data; }

  void InitAcquireNextImage(const LayoutAcquireNextImage& lacquire_next_image);
  void InitUpdater(const LayoutUpdater& lupdater);
  Result BuildCommandBuffers() const;
  void RebuildCommandBuffers();
  void UpdateUpdaterData();
  void UpdateQueueSubmits();

  using UpdateHandlerType = Result();

  void SetUpdateHandler(std::function<UpdateHandlerType> handler) {
    update_handler_ = handler;
  }

 protected:
  std::function<UpdateHandlerType> update_handler_ = []() -> Result {
    return Result::kSuccess;
  };

  int curr_frame_ = 0;
  int curr_image_ = 0;
  bool first_round_ = true;
  void* custom_data_ = nullptr;

  std::shared_ptr<LayoutFrame> lframe_;
  std::shared_ptr<Camera> camera_;
  std::vector<std::shared_ptr<CommandContext>> cmd_contexts_;
  std::vector<Fence*> wait_fences_;
  std::vector<Fence*> wait_image_fences_;
  std::vector<AcquireNextImageInfo> acquire_next_image_infos_;
  std::vector<std::shared_ptr<LayoutQueueSubmit>> lqueue_submits_;

  struct {
    std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;
    std::vector<UpdateData> update_data_;
  } updater_;

  friend class Viewer;
  friend class WindowViewer;
  friend class RealityViewer;
  friend class RealityViewerXR;
};

class Viewer {
 public:
  Viewer() = default;
  Viewer(const Viewer&) = delete;
  Viewer& operator=(const Viewer&) = delete;
  Viewer(Viewer&&) = delete;
  Viewer& operator=(Viewer&&) = delete;
  virtual ~Viewer() = default;

  virtual void PollEvents() = 0;
  virtual bool ShouldClose() const = 0;
  virtual Result BuildCommandBuffers() const = 0;

  using DrawHandlerType = Result();

  void SetDrawHandler(std::function<DrawHandlerType> handler) {
    draw_handler_ = handler;
  }

  using ShouldExitHandlerType = bool();

  void SetShouldExitHandler(std::function<ShouldExitHandlerType> handler) {
    should_exit_handler_ = handler;
  }

 protected:
  virtual Result Draw() = 0;
  virtual Result PostUpdate() = 0;

  std::function<DrawHandlerType> draw_handler_ = []() -> Result {
    return Result::kSuccess;
  };
  std::function<ShouldExitHandlerType> should_exit_handler_ = []() -> bool {
    return false;
  };

  friend class Engine;
  friend class Renderer;
};

}  // namespace xg

#endif  // XG_VIEWER_H_
