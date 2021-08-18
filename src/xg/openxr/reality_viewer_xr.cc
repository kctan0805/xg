// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#include "xg/openxr/reality_viewer_xr.h"

#include <cassert>
#include <memory>
#include <vector>

#include "openxr/openxr.hpp"
#include "xg/camera.h"
#include "xg/fence.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/logger.h"
#include "xg/openxr/composition_layer_projection_xr.h"
#include "xg/swapchain.h"
#include "xg/utility.h"

namespace xg {

bool RealityViewerXR::Init(const LayoutRealityViewer& lreality_viewer) {
  if (!RealityViewer::Init(lreality_viewer)) return false;

  view_config_type_ =
      static_cast<xr::ViewConfigurationType>(lreality_viewer.view_config_type);

  for (const auto& llayer : lreality_viewer.lend_frame->llayers) {
    if (llayer->layout_type == xg::LayoutType::kCompositionLayerProjection) {
      auto* composition_layer_projection_xr =
          static_cast<CompositionLayerProjectionXR*>(llayer->instance.get());
      composition_layers_.emplace_back(
          static_cast<xr::CompositionLayerBaseHeader*>(
              &composition_layer_projection_xr->composition_layer_projection_));
    } else {
      assert(0);
    }
  }

  return true;
}

void RealityViewerXR::PollEvents() {
  xr::EventDataBuffer ev;

  const auto result = instance_.pollEvent(ev);
  if (result == xr::Result::Success) {
    switch (ev.type) {
      case xr::StructureType::EventDataSessionStateChanged: {
        const auto state =
            reinterpret_cast<xr::EventDataSessionStateChanged&>(ev).state;
        switch (state) {
          case xr::SessionState::Ready: {
            xr::SessionBeginInfo info(view_config_type_);
            session_.beginSession(info);
          } break;

          case xr::SessionState::Stopping: {
            session_.endSession();
          } break;
        }
      } break;
    }
  } else if (result != xr::Result::EventUnavailable) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
  }
}

Result RealityViewerXR::AcquireNextImage() { return Result::kSuccess; }

Result RealityViewerXR::Draw() {
  // wait frame
  xr::FrameWaitInfo wait_info;
  xr::FrameState state;

  auto result = session_.waitFrame(wait_info, state);
  if (result != xr::Result::Success) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  // begin frame
  xr::FrameBeginInfo begin_info;
  result = session_.beginFrame(begin_info);
  if (result != xr::Result::Success) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  // end frame
  xr::FrameEndInfo end_info;
  end_info.displayTime = state.predictedDisplayTime;
  end_info.environmentBlendMode = env_blend_mode_;
  end_info.layerCount = static_cast<uint32_t>(composition_layers_.size());
  end_info.layers = composition_layers_.data();
  session_.endFrame(end_info);

  return Result::kSuccess;
}

Result RealityViewerXR::PostUpdate() { return Result::kSuccess; }

}  // namespace xg
#endif  // XG_ENABLE_REALITY
