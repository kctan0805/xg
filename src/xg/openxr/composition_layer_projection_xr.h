// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_COMPOSITION_LAYER_PROJECTION_XR_H_
#define XG_COMPOSITION_LAYER_PROJECTION_XR_H_

#include <memory>
#include <vector>

#include "openxr/openxr.hpp"
#include "xg/composition_layer_projection.h"

namespace xg {

class CompositionLayerProjectionXR : public CompositionLayerProjection {
 public:
  CompositionLayerProjectionXR() = default;
  virtual ~CompositionLayerProjectionXR();

 protected:
  xr::CompositionLayerProjection composition_layer_projection_;
  std::vector<xr::CompositionLayerProjectionView>
      composition_layer_projection_views_;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_COMPOSITION_LAYER_PROJECTION_XR_H_
