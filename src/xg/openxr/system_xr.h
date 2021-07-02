// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OPENXR_SYSTEM_XR_H_
#define XG_OPENXR_SYSTEM_XR_H_

#include "openxr/openxr.hpp"
#include "xg/system.h"

namespace xg {

class SystemXR : public System {
 public:
  SystemXR() = default;
  virtual ~SystemXR();

 protected:
  xr::SystemId system_id_;

  friend class RealityXR;
};

}  // namespace xg

#endif  // XG_OPENXR_SYSTEM_XR_H_
