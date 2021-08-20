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
#include "xg/types.h"

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

  virtual void PollEvents() = 0;
  std::shared_ptr<Camera> GetCamera(int index = 0) const { return cameras_[index]; }

  int GetCurrentFrame() const { return curr_frame_; }
  int GetCurrentImage() const { return curr_image_; }

  UpdateData& GetUpdateData(int index) { return updater_.update_data_[index]; }
  std::shared_ptr<Swapchain> GetSwapchain() const {
    return std::static_pointer_cast<Swapchain>(lframe_->lswapchain->instance);
  }
  virtual bool ShouldClose() const = 0;
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

 protected:
  void InitAcquireNextImage(const LayoutAcquireNextImage& lacquire_next_image);
  void InitUpdater(const LayoutUpdater& lupdater);
  void UpdateUpdaterData();
  void UpdateQueueSubmits();
  virtual Result Draw() = 0;
  virtual Result PostUpdate() = 0;

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

  std::shared_ptr<LayoutFrame> lframe_;
  std::vector<std::shared_ptr<Camera>> cameras_;
  std::vector<std::shared_ptr<CommandContext>> cmd_contexts_;
  std::vector<Fence*> wait_fences_;
  std::vector<Fence*> wait_image_fences_;
  std::vector<AcquireNextImageInfo> acquire_next_image_infos_;
  std::vector<std::shared_ptr<LayoutQueueSubmit>> lqueue_submits_;

  struct {
    std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;
    std::vector<UpdateData> update_data_;
  } updater_;

  friend class Engine;
  friend class Renderer;
};

}  // namespace xg

#endif  // XG_VIEWER_H_
