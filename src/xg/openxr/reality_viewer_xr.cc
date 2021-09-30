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
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "openxr/openxr.h"
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
      static_cast<XrViewConfigurationType>(lreality_viewer.view_config_type);

  const auto& lspace = lreality_viewer.lspace;
  if (lspace->layout_type == LayoutType::kReferenceSpace) {
    const auto* ref_space =
        static_cast<ReferenceSpaceXR*>(lspace->instance.get());
    view_locate_info_.space = ref_space->space_;
  } else {
    assert(0);
  }

  if (lreality_viewer.llocate_space) {
    for (const auto& lspace : lreality_viewer.llocate_space->lspaces) {
      if (lspace->layout_type == LayoutType::kReferenceSpace) {
        const auto* ref_space =
            static_cast<ReferenceSpaceXR*>(lspace->instance.get());
        xr_spaces_.emplace_back(ref_space->space_);
      } else {
        assert(0);
      }
    }
    space_locations_.resize(xr_spaces_.size());
  }

  xr_views_.resize(views_.size());

  for (const auto& llayer : lreality_viewer.lend_frame->llayers) {
    if (llayer->layout_type == xg::LayoutType::kCompositionLayerProjection) {
      auto* composition_layer_projection_xr =
          static_cast<CompositionLayerProjectionXR*>(llayer->instance.get());
      composition_layer_projections_.emplace_back(
          &composition_layer_projection_xr->composition_layer_projection_);
      composition_layers_.emplace_back(
          reinterpret_cast<XrCompositionLayerBaseHeader*>(
              &composition_layer_projection_xr->composition_layer_projection_));
    } else {
      assert(0);
    }
  }

  frame_end_info_.environmentBlendMode = static_cast<XrEnvironmentBlendMode>(
      lreality_viewer.lend_frame->env_blend_mode);

  return true;
}

void RealityViewerXR::PollEvents() {
  XrEventDataBuffer ev = {XR_TYPE_EVENT_DATA_BUFFER};

  for (;;) {
    auto result = xrPollEvent(instance_, &ev);
    if (result == XR_EVENT_UNAVAILABLE) {
      break;
    } else if (result == XR_SUCCESS) {
      switch (ev.type) {
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
          const auto state =
              reinterpret_cast<XrEventDataSessionStateChanged&>(ev).state;
          switch (state) {
            case XR_SESSION_STATE_READY: {
              XrSessionBeginInfo info = {
                  XR_TYPE_SESSION_BEGIN_INFO, nullptr,
                  view_locate_info_.viewConfigurationType};
              result = xrBeginSession(session_, &info);
              if (result != XR_SUCCESS)
                XG_WARN(RealityResultString(static_cast<Result>(result)));
            } break;

            case XR_SESSION_STATE_STOPPING: {
              result = xrEndSession(session_);
              if (result != XR_SUCCESS)
                XG_WARN(RealityResultString(static_cast<Result>(result)));
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
  XrFrameWaitInfo frame_wait_info = {XR_TYPE_FRAME_WAIT_INFO};

  auto result = xrWaitFrame(session_, &frame_wait_info, &frame_state_);
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  // begin frame
  XrFrameBeginInfo begin_info = {XR_TYPE_FRAME_BEGIN_INFO};
  result = xrBeginFrame(session_, &begin_info);
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  if (frame_state_.shouldRender) {
    // update views
    view_locate_info_.displayTime = frame_state_.predictedDisplayTime;
    uint32_t count = 0;
    result = xrLocateViews(session_, &view_locate_info_, &view_state_,
                           static_cast<uint32_t>(views_.size()), &count,
                           xr_views_.data());
    if (result != XR_SUCCESS) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
      return static_cast<Result>(result);
    }
    assert(views_.size() == count);

    // update space locations
    XrSpaceLocation xr_space_location = {XR_TYPE_SPACE_LOCATION};
    for (int i = 0; i < xr_spaces_.size(); ++i) {
      auto* space_location = &space_locations_[i];
      const auto& xr_space = xr_spaces_[i];

      result = xrLocateSpace(xr_space, view_locate_info_.space,
                             view_locate_info_.displayTime, &xr_space_location);
      if (result != XR_SUCCESS) {
        XG_WARN(RealityResultString(static_cast<Result>(result)));
        return static_cast<Result>(result);
      }

      if ((xr_space_location.locationFlags &
           XR_SPACE_LOCATION_POSITION_VALID_BIT) &&
          (xr_space_location.locationFlags &
           XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
        const auto& pose = xr_space_location.pose;
        const glm::quat orientation =
            glm::quat(pose.orientation.w, pose.orientation.x,
                      pose.orientation.y, pose.orientation.z);
        const glm::vec3 position =
            glm::vec3(pose.position.x, -pose.position.y, pose.position.z);
        glm::mat4 rotation = glm::mat4_cast(orientation);
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        *space_location = translation * rotation;
      }
    }

    for (uint32_t i = 0; i < count; ++i) {
      auto* view = &views_[i];
      const auto& xr_view = xr_views_[i];

      // update camera
      const auto& camera = view->GetCamera();

      const auto& fov = xr_view.fov;
      camera->ComputeProjectionFromFov(fov.angleLeft, fov.angleRight,
                                       fov.angleUp, fov.angleDown);
      const auto& pose = xr_view.pose;
      const glm::quat orientation =
          glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y,
                    pose.orientation.z);
      const glm::vec3 position =
          glm::vec3(pose.position.x, -pose.position.y, pose.position.z);
      camera->ComputeViewFromPose(orientation, position);

      view->UpdateUpdaterData();

      auto ret = AcquireNextImage(view);
      if (ret != Result::kSuccess) return ret;

      ret = view->update_handler_();
      if (ret != Result::kSuccess) return ret;

      for (auto& cmd_context : view->cmd_contexts_)
        cmd_context->Update(view->curr_image_);

      view->UpdateQueueSubmits();
    }
  }

  return Result::kSuccess;
}

Result RealityViewerXR::PostUpdate() {
  if (frame_state_.shouldRender) {
    // release swapchain
    for (const auto& view : views_) {
      const auto& swapchain = view.GetSwapchain();
      auto swapchain_xr = static_cast<SwapchainXR*>(swapchain.get());
      swapchain_xr->ReleaseSwapchainImage();
    }

    // end frame
    frame_end_info_.layerCount =
        static_cast<uint32_t>(composition_layers_.size());
    frame_end_info_.layers = composition_layers_.data();

    for (auto* projection : composition_layer_projections_) {
      assert(xr_views_.size() == projection->viewCount);
      for (int i = 0; i < xr_views_.size(); ++i) {
        const auto& xr_view = xr_views_[i];
        auto& projection_view =
            const_cast<XrCompositionLayerProjectionView&>(projection->views[i]);
        projection_view.pose = xr_view.pose;
        projection_view.fov = xr_view.fov;
      }
    }
  } else {
    frame_end_info_.layerCount = 0;
    frame_end_info_.layers = nullptr;
  }
  frame_end_info_.displayTime = frame_state_.predictedDisplayTime;
  const auto result = xrEndFrame(session_, &frame_end_info_);
  if (result != XR_SUCCESS) {
    XG_WARN(RealityResultString(static_cast<Result>(result)));
  }

  return static_cast<Result>(result);
}

}  // namespace xg
#endif  // XG_ENABLE_REALITY
