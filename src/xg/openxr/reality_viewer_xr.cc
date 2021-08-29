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

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "openxr/openxr.hpp"
#include "xg/camera.h"
#include "xg/fence.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/logger.h"
#include "xg/openxr/composition_layer_projection_xr.h"
#include "xg/openxr/reference_space_xr.h"
#include "xg/openxr/swapchain_xr.h"
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

  xr_views_.resize(views_.size());

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

  return true;
}

void RealityViewerXR::PollEvents() {
  xr::EventDataBuffer ev;

  for (;;) {
    const auto result = instance_.pollEvent(ev);
    if (result == xr::Result::EventUnavailable) {
      break;
    } else if (result == xr::Result::Success) {
      switch (ev.type) {
        case xr::StructureType::EventDataSessionStateChanged: {
          const auto state =
              reinterpret_cast<xr::EventDataSessionStateChanged&>(ev).state;
          switch (state) {
            case xr::SessionState::Ready: {
              xr::SessionBeginInfo info(
                  view_locate_info_.viewConfigurationType);
              session_.beginSession(info);
            } break;

            case xr::SessionState::Stopping: {
              session_.endSession();
            } break;
          }
        } break;
      }
    } else {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
      break;
    }
  }
}

Result RealityViewerXR::AcquireNextImage(View* view) {
  auto fence = view->wait_fences_[view->curr_frame_];
  assert(fence);

  fence->Wait();

  auto& info = view->acquire_next_image_infos_[view->curr_frame_];
  auto swapchain = view->GetSwapchain();

  auto result = swapchain->AcquireNextImage(info, &view->curr_image_);
  if (result != Result::kSuccess) return result;

  if (view->wait_image_fences_[view->curr_image_])
    view->wait_image_fences_[view->curr_image_]->Wait();

  view->wait_image_fences_[view->curr_image_] =
      view->wait_fences_[view->curr_frame_];

  fence->Reset();

  return result;
}

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
                                  static_cast<uint32_t>(views_.size()), &count, xr_views_.data());
    if (result != xr::Result::Success) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
      return static_cast<Result>(result);
    }
    assert(views_.size() == count);

    for (uint32_t i = 0; i < count; ++i) {
      auto* view = &views_[i];
      const auto& xr_view = xr_views_[i];

      // updat camera
      const auto& camera = view->GetCamera();

      const auto& fov = xr_view.fov;
      camera->ComputeProjectionFromFov(fov.angleLeft, fov.angleRight,
                                       fov.angleUp, fov.angleDown);
      const auto& pose = xr_view.pose;
      const glm::quat orientation =
          glm::quat(pose.orientation.w * -1.0f, pose.orientation.x,
                    pose.orientation.y * -1.0f, pose.orientation.z);
      const glm::vec4 position =
          glm::vec4(pose.position.x, -pose.position.y, pose.position.z, 1.0f);
      camera->ComputeViewFromPose(orientation, position);

      view->UpdateUpdaterData();

      auto ret = AcquireNextImage(view);
      if (ret != Result::kSuccess) return ret;

      ret = view->update_handler_();
      if (ret != Result::kSuccess) return ret;

      for (auto& cmd_context : view->cmd_contexts_)
        cmd_context->Update(view->curr_image_);

      view->UpdateQueueSubmits();

      // release swapchain
      auto swapchain_xr = static_cast<SwapchainXR*>(view->GetSwapchain().get());
      swapchain_xr->ReleaseSwapchainImage();
    }
  }

  return Result::kSuccess;
}

Result RealityViewerXR::PostUpdate() {
  // end frame
  if (frame_state_.shouldRender) {
    frame_end_info_.layerCount =
        static_cast<uint32_t>(composition_layers_.size());
    frame_end_info_.layers = composition_layers_.data();

    for (auto* projection : composition_layer_projections_) {
      assert(xr_views_.size() == projection->viewCount);
      for (int i = 0; i < xr_views_.size(); ++i) {
        const auto& xr_view = xr_views_[i];
        auto projection_view = projection->views[i];
        projection_view.pose = xr_view.pose;
        projection_view.fov = xr_view.fov;
      }
    }
  } else {
    frame_end_info_.layerCount = 0;
    frame_end_info_.layers = nullptr;
  }
  frame_end_info_.displayTime = frame_state_.predictedDisplayTime;
  session_.endFrame(frame_end_info_);

  return Result::kSuccess;
}

}  // namespace xg
#endif  // XG_ENABLE_REALITY
