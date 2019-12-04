// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_IMAGE_VIEW_H_
#define XG_IMAGE_VIEW_H_

#include "xg/layout.h"

namespace xg {

class ImageView {
 public:
  ImageView(const ImageView&) = delete;
  ImageView& operator=(const ImageView&) = delete;
  ImageView(ImageView&&) = delete;
  ImageView& operator=(ImageView&&) = delete;
  virtual ~ImageView() = default;

  virtual Result Init(const LayoutImageView& limage_view) = 0;
  virtual void Exit() = 0;

 protected:
  ImageView() = default;
};

}  // namespace xg

#endif  // XG_IMAGE_VIEW_H_
