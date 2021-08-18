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

#include "openxr/openxr.hpp"
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
  Result AcquireNextImage();
  Result Draw() override;
  Result PostUpdate() override;

  xr::Instance instance_;
  xr::Session session_;
  xr::ViewConfigurationType view_config_type_ =
      xr::ViewConfigurationType::PrimaryStereo;
  xr::EnvironmentBlendMode env_blend_mode_ = xr::EnvironmentBlendMode::Opaque;

  std::vector<xr::CompositionLayerBaseHeader*> composition_layers_;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_OPENXR_REALITY_VIEWER_XR_H_
#endif  // XG_ENABLE_REALITY
