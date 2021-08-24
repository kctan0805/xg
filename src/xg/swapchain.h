// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SWAPCHAIN_H_
#define XG_SWAPCHAIN_H_

#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "xg/fence.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/layout.h"
#include "xg/semaphore.h"
#include "xg/types.h"

namespace xg {

struct AcquireNextImageInfo {
  int64_t timeout = std::numeric_limits<int64_t>::max();
  Semaphore* semaphore = nullptr;
  Fence* fence = nullptr;
};

class Swapchain {
 public:
  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = delete;
  Swapchain& operator=(Swapchain&&) = delete;
  virtual ~Swapchain() = default;

  virtual Result Init(const LayoutSwapchain& lswapchain) = 0;
  virtual void Exit(bool destroy_swapchain) = 0;

  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
  Format GetFormat() const { return format_; }
  int GetFrameCount() const { return static_cast<int>(image_views_.size()); }
  const std::shared_ptr<Image>& GetImage(int index) const {
    return images_[index];
  }
  const std::shared_ptr<ImageView>& GetImageView(int index) const {
    return image_views_[index];
  }
  virtual Result AcquireNextImage(const AcquireNextImageInfo& info,
                                  int* image_index) = 0;

 protected:
  Swapchain() = default;

  int width_ = 0;
  int height_ = 0;
  Format format_ = Format::kR8G8B8A8Unorm;
  std::vector<std::shared_ptr<Image>> images_;
  std::vector<std::shared_ptr<ImageView>> image_views_;
};

}  // namespace xg

#endif  // XG_SWAPCHAIN_H_
