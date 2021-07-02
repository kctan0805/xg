// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SYSTEM_H_
#define XG_SYSTEM_H_

#include "xg/layout.h"

namespace xg {

class System {
 public:
  System() = default;
  System(const System&) = delete;
  System& operator=(const System&) = delete;
  System(System&&) = delete;
  System& operator=(System&&) = delete;
  virtual ~System() = default;
};

}  // namespace xg

#endif  // XG_SYSTEM_H_
