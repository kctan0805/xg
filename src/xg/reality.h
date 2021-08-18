// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#ifndef XG_REALITY_H_
#define XG_REALITY_H_

#include <memory>
#include <vector>

#include "xg/composition_layer_projection.h"
#include "xg/layout.h"
#include "xg/session.h"
#include "xg/swapchain.h"
#include "xg/viewer.h"

namespace xg {

class Reality {
 public:
  Reality() = default;
  Reality(const Reality&) = delete;
  Reality& operator=(const Reality&) = delete;
  Reality(Reality&&) = delete;
  Reality& operator=(Reality&&) = delete;
  virtual ~Reality() = default;

  std::shared_ptr<Session> GetSession() const { return session_; }

  virtual std::shared_ptr<Swapchain> CreateSwapchain(
      LayoutSwapchain* lswapchain) const = 0;
  virtual std::shared_ptr<CompositionLayerProjection>
  CreateCompositionLayerProjection(
      const LayoutCompositionLayerProjection& lprojection) = 0;
  virtual std::shared_ptr<Viewer> CreateRealityViewer(
      const LayoutRealityViewer& lreality_viewer) = 0;

 protected:
  std::shared_ptr<Session> session_;
};

}  // namespace xg

#endif  // XG_REALITY_H_
#endif  // XG_ENABLE_REALITY