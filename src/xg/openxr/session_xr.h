// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OPENXR_SESSION_XR_H_
#define XG_OPENXR_SESSION_XR_H_

#include <memory>

#include "openxr/openxr.hpp"
#include "xg/reference_space.h"
#include "xg/session.h"

namespace xg {

class SessionXR : public Session {
 public:
  SessionXR() = default;
  virtual ~SessionXR();

  std::shared_ptr<ReferenceSpace> CreateReferenceSpace(
      const LayoutReferenceSpace& lreference_space) override;

 protected:
  xr::Session session_;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_OPENXR_SESSION_XR_H_
