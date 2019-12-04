// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_FRAMEBUFFER_H_
#define XG_FRAMEBUFFER_H_

#include "xg/layout.h"

namespace xg {

class Framebuffer {
 public:
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;
  Framebuffer(Framebuffer&&) = delete;
  Framebuffer& operator=(Framebuffer&&) = delete;
  virtual ~Framebuffer() = default;

  virtual Result Init(const LayoutFramebuffer& lframebuffer) = 0;
  virtual void Exit() = 0;

  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }

protected:
  Framebuffer() = default;

  int width_ = 0;
  int height_ = 0;
};

}  // namespace xg

#endif  // XG_FRAMEBUFFER_H_
