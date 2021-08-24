// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#include "xg/reality_viewer.h"

#include <cassert>
#include <memory>
#include <vector>

#include "xg/camera.h"
#include "xg/fence.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/logger.h"
#include "xg/swapchain.h"
#include "xg/utility.h"
#include "xg/viewer.h"

namespace xg {

bool RealityViewer::Init(const LayoutRealityViewer& lreality_viewer) {
  assert(views_.size() == 0);
  views_.reserve(lreality_viewer.lviews.size());

  for (const auto& lview : lreality_viewer.lviews) {
    View view;

    view.lframe_ = lview->lframe;
    if (!view.lframe_) {
      XG_ERROR("frame not found");
      return false;
    }

    if (lview->lcamera) {
      view.camera_ = std::static_pointer_cast<Camera>(lview->lcamera->instance);
      if (!view.camera_) {
        XG_ERROR("camera not found");
        return false;
      }
    }

    for (const auto& lcmd_context : lview->lcmd_contexts) {
      view.cmd_contexts_.emplace_back(
          std::static_pointer_cast<CommandContext>(lcmd_context->instance));
    }

    if (lview->lupdater) view.InitUpdater(*lview->lupdater);

    view.lqueue_submits_ = lview->lqueue_submits;

    views_.emplace_back(view);
  }

  SetDrawHandler([this]() -> Result { return this->Draw(); });
  SetShouldExitHandler([this]() -> bool { return this->ShouldClose(); });

  return true;
}

Result RealityViewer::BuildCommandBuffers() const {
  for (const auto& view : views_) {
    auto result = view.BuildCommandBuffers();
    if (result != Result::kSuccess) return result;
  }

  return Result::kSuccess;
}

}  // namespace xg
#endif  // XG_ENABLE_REALITY
