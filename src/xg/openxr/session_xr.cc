// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/openxr/session_xr.h"

#include <memory>

#include "openxr/openxr.hpp"
#include "xg/logger.h"
#include "xg/openxr/reference_space_xr.h"
#include "xg/utility.h"

namespace xg {

SessionXR::~SessionXR() {
  if (session_) {
    XG_TRACE("destroy: {}", (void*)(XrSession)session_);
    session_.destroy();
  }
}

std::shared_ptr<ReferenceSpace> SessionXR::CreateReferenceSpace(
    const LayoutReferenceSpace& lreference_space) {
  auto reference_space = std::make_shared<ReferenceSpaceXR>();
  if (!reference_space) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const auto& orientation = lreference_space.orientation_;
  const auto& position = lreference_space.position_;
  xr::ReferenceSpaceCreateInfo info;

  info.referenceSpaceType = static_cast<xr::ReferenceSpaceType>(
      lreference_space.reference_space_type_);
  info.poseInReferenceSpace.orientation = xr::Quaternionf(
      orientation.x, orientation.y, orientation.z, orientation.w);
  info.poseInReferenceSpace.position =
      xr::Vector3f(position.x, position.y, position.z);

  const auto result =
      session_.createReferenceSpace(info, reference_space->space_);
  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  return reference_space;
}

}  // namespace xg
