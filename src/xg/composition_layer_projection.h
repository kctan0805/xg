// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_COMPOSITION_LAYER_PROJECTION_H_
#define XG_COMPOSITION_LAYER_PROJECTION_H_

#include <memory>

#include "xg/layout.h"

namespace xg {

class CompositionLayerProjection {
 public:
  CompositionLayerProjection() = default;
  CompositionLayerProjection(const CompositionLayerProjection&) = delete;
  CompositionLayerProjection& operator=(const CompositionLayerProjection&) = delete;
  CompositionLayerProjection(CompositionLayerProjection&&) = delete;
  CompositionLayerProjection& operator=(CompositionLayerProjection&&) = delete;
  virtual ~CompositionLayerProjection() = default;
};

}  // namespace xg

#endif  // XG_COMPOSITION_LAYER_PROJECTION_H_
