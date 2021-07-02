// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SESSION_H_
#define XG_SESSION_H_

#include <memory>

#include "xg/layout.h"
#include "xg/reference_space.h"

namespace xg {

class Session {
 public:
  Session() = default;
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session(Session&&) = delete;
  Session& operator=(Session&&) = delete;
  virtual ~Session() = default;

  virtual std::shared_ptr<ReferenceSpace> CreateReferenceSpace(
      const LayoutReferenceSpace& lreference_space) = 0;
};

}  // namespace xg

#endif  // XG_SESSION_H_
