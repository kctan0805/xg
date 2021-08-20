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
  lframe_ = lreality_viewer.lframe;
  if (!lframe_) {
    XG_ERROR("frame not found");
    return false;
  }

  for (const auto& lcamera : lreality_viewer.lcameras) {
    cameras_.emplace_back(std::static_pointer_cast<Camera>(lcamera->instance));
  }

  for (const auto& lcmd_context : lreality_viewer.lcmd_contexts) {
    cmd_contexts_.emplace_back(
        std::static_pointer_cast<CommandContext>(lcmd_context->instance));
  }

  SetDrawHandler([this]() -> Result { return this->Draw(); });
  SetShouldExitHandler([this]() -> bool { return this->ShouldClose(); });

  if (lreality_viewer.lupdater) InitUpdater(*lreality_viewer.lupdater);

  lqueue_submits_ = lreality_viewer.lqueue_submits;

  return true;
}

}  // namespace xg
#endif  // XG_ENABLE_REALITY
