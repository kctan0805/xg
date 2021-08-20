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
#include "xg/openxr/reference_space_xr.h"
#include "xg/swapchain.h"
#include "xg/utility.h"

namespace xg {

bool RealityViewerXR::Init(const LayoutRealityViewer& lreality_viewer) {
  if (!RealityViewer::Init(lreality_viewer)) return false;

  view_locate_info_.viewConfigurationType =
      static_cast<xr::ViewConfigurationType>(lreality_viewer.view_config_type);

  const auto& lspace = lreality_viewer.lspace;
  if (lspace->layout_type == LayoutType::kReferenceSpace) {
    const auto* ref_space =
        static_cast<ReferenceSpaceXR*>(lspace->instance.get());
    view_locate_info_.space = ref_space->space_;
  } else {
    assert(0);
  }

  for (const auto& llayer : lreality_viewer.lend_frame->llayers) {
    if (llayer->layout_type == xg::LayoutType::kCompositionLayerProjection) {
      auto* composition_layer_projection_xr =
          static_cast<CompositionLayerProjectionXR*>(llayer->instance.get());
      composition_layer_projections_.emplace_back(
          &composition_layer_projection_xr->composition_layer_projection_);
      composition_layers_.emplace_back(
          static_cast<xr::CompositionLayerBaseHeader*>(
              &composition_layer_projection_xr->composition_layer_projection_));
    } else {
      assert(0);
    }
  }

  frame_end_info_.environmentBlendMode = static_cast<xr::EnvironmentBlendMode>(
      lreality_viewer.lend_frame->env_blend_mode);
  frame_end_info_.layerCount =
      static_cast<uint32_t>(composition_layers_.size());
  frame_end_info_.layers = composition_layers_.data();

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
            xr::SessionBeginInfo info(view_locate_info_.viewConfigurationType);
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
  xr::FrameWaitInfo frame_wait_info;

  auto result = session_.waitFrame(frame_wait_info, frame_state_);
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

  if (frame_state_.shouldRender) {
    // update views
    view_locate_info_.displayTime = frame_state_.predictedDisplayTime;
    uint32_t count = 0;
    result = session_.locateViews(view_locate_info_, &view_state_,
                                  views_.size(), &count, views_.data());
    if (result != xr::Result::Success) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
      return static_cast<Result>(result);
    }

    UpdateUpdaterData();

    auto ret = update_handler_();
    if (ret != Result::kSuccess) return ret;

    for (auto& cmd_context : cmd_contexts_) cmd_context->Update(curr_image_);

    UpdateQueueSubmits();
  }

  return Result::kSuccess;
}

Result RealityViewerXR::PostUpdate() {
  // end frame
  for (auto* projection : composition_layer_projections_) {
    assert(views_.size() == projection->viewCount);
    for (int i = 0; i < views_.size(); ++i) {
      const auto& view = views_[i];
      auto projection_view = projection->views[i];
      projection_view.pose = view.pose;
      projection_view.fov = view.fov;
    }
  }
  frame_end_info_.displayTime = frame_state_.predictedDisplayTime;
  session_.endFrame(frame_end_info_);

  return Result::kSuccess;
}

}  // namespace xg
#endif  // XG_ENABLE_REALITY
