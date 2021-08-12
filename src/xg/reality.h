// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_REALITY_H_
#define XG_REALITY_H_

#include <memory>
#include <vector>

#include "xg/composition_layer_projection.h"
#include "xg/layout.h"
#include "xg/session.h"
#include "xg/swapchain.h"

namespace xg {

class Reality {
 public:
  Reality() = default;
  Reality(const Reality&) = delete;
  Reality& operator=(const Reality&) = delete;
  Reality(Reality&&) = delete;
  Reality& operator=(Reality&&) = delete;
  virtual ~Reality() = default;

  const std::vector<std::shared_ptr<Swapchain>>& GetSwapchains() const {
    return swapchains_;
  }
  std::shared_ptr<Session> GetSession() const { return session_; }

  virtual std::shared_ptr<CompositionLayerProjection>
  CreateCompositionLayerProjection(
      const LayoutCompositionLayerProjection& lprojection) = 0;

 protected:
  std::vector<std::shared_ptr<Swapchain>> swapchains_;
  std::shared_ptr<Session> session_;
};

}  // namespace xg

#endif  // XG_REALITY_H_
