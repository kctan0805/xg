// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_REFERENCE_SPACE_H_
#define XG_REFERENCE_SPACE_H_

#include "xg/layout.h"

namespace xg {

class ReferenceSpace {
 public:
  ReferenceSpace() = default;
  ReferenceSpace(const ReferenceSpace&) = delete;
  ReferenceSpace& operator=(const ReferenceSpace&) = delete;
  ReferenceSpace(ReferenceSpace&&) = delete;
  ReferenceSpace& operator=(ReferenceSpace&&) = delete;
  virtual ~ReferenceSpace() = default;
};

}  // namespace xg

#endif  // XG_REFERENCE_SPACE_H_
