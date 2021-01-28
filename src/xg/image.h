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

  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
  Format GetFormat() const { return format_; }

 protected:
  Image() = default;

  int width_ = 0;
  int height_ = 0;
  Format format_ = Format::kUndefined;
};

}  // namespace xg

#endif  // XG_IMAGE_H_
