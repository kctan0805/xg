// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_IMAGE_H_
#define XG_IMAGE_H_

#include "xg/layout.h"

namespace xg {

class Image {
 public:
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(Image&&) = delete;
  Image& operator=(Image&&) = delete;
  virtual ~Image() = default;

  virtual Result Init(const LayoutImage& limage) = 0;
  virtual void Exit() = 0;

 protected:
  Image() = default;
};

}  // namespace xg

#endif  // XG_IMAGE_H_
