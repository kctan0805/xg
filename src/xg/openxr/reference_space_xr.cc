// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/openxr/reference_space_xr.h"

#include "openxr/openxr.h"
#include "xg/logger.h"
#include "xg/utility.h"

namespace xg {

ReferenceSpaceXR::~ReferenceSpaceXR() {
  if (space_) {
    XG_TRACE("destroy: {}", (void*)space_);
    const auto result = xrDestroySpace(space_);
    if (result != XR_SUCCESS) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
    }
  }
}

}  // namespace xg
