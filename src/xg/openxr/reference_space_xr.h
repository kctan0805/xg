// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OPENXR_REFERENCE_SPACE_XR_H_
#define XG_OPENXR_REFERENCE_SPACE_XR_H_

#include "openxr/openxr.h"
#include "xg/reference_space.h"

namespace xg {

class ReferenceSpaceXR : public ReferenceSpace {
 public:
  ReferenceSpaceXR() = default;
  virtual ~ReferenceSpaceXR();

 protected:
  XrSpace space_ = nullptr;

  friend class RealityXR;
  friend class SessionXR;
  friend class RealityViewerXR;
};

}  // namespace xg

#endif  // XG_OPENXR_REFERENCE_SPACE_XR_H_
