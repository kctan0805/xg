// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#ifndef XG_OPENXR_REALITY_VIEWER_XR_H_
#define XG_OPENXR_REALITY_VIEWER_XR_H_

#include <memory>
#include <vector>

#include "openxr/openxr.h"
#include "xg/layout.h"
#include "xg/reality_viewer.h"

namespace xg {

class RealityViewerXR : public RealityViewer {
 public:
  RealityViewerXR() = default;
  RealityViewerXR(const RealityViewerXR&) = delete;
  RealityViewerXR& operator=(const RealityViewerXR&) = delete;
  RealityViewerXR(RealityViewerXR&&) = delete;
  RealityViewerXR& operator=(RealityViewerXR&&) = delete;
  virtual ~RealityViewerXR() = default;

  void PollEvents() override;
  bool ShouldClose() const override { return false; }

 protected:
  bool Init(const LayoutRealityViewer& lreality_viewer);
  Result AcquireNextImage(View* view);
  Result Draw() override;
  Result PostUpdate() override;

  XrInstance instance_ = nullptr;
  XrSession session_ = nullptr;
  XrFrameState frame_state_ = {XR_TYPE_FRAME_STATE};
  XrViewLocateInfo view_locate_info_ = {XR_TYPE_VIEW_LOCATE_INFO};
  XrViewState view_state_ = {XR_TYPE_VIEW_STATE};
  std::vector<XrView> xr_views_;
  XrFrameEndInfo frame_end_info_ = {XR_TYPE_FRAME_END_INFO};
  std::vector<XrCompositionLayerProjection*> composition_layer_projections_;
  std::vector<XrCompositionLayerBaseHeader*> composition_layers_;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_OPENXR_REALITY_VIEWER_XR_H_
#endif  // XG_ENABLE_REALITY
