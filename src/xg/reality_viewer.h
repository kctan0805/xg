// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#ifndef XG_REALITY_VIEWER_H_
#define XG_REALITY_VIEWER_H_

#include <vector>

#include "glm/glm.hpp"
#include "xg/layout.h"
#include "xg/viewer.h"

namespace xg {

class RealityViewer : public Viewer {
 public:
  RealityViewer() = default;
  RealityViewer(const RealityViewer&) = delete;
  RealityViewer& operator=(const RealityViewer&) = delete;
  RealityViewer(RealityViewer&&) = delete;
  RealityViewer& operator=(RealityViewer&&) = delete;
  virtual ~RealityViewer() = default;

  bool ShouldClose() const override { return false; }
  Result BuildCommandBuffers() const override;

  std::vector<View>& GetViews() { return views_; }
  std::vector<glm::mat4>& GetSpaceLocations() { return space_locations_; }

 protected:
  bool Init(const LayoutRealityViewer& lreality_viewer);

  std::vector<View> views_;
  std::vector<glm::mat4> space_locations_;

  friend class Engine;
  friend class Reality;
};

}  // namespace xg

#endif  // XG_REALITY_VIEWER_H_
#endif  // XG_ENABLE_REALITY
