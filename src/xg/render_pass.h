// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_RENDER_PASS_H_
#define XG_RENDER_PASS_H_

#include "xg/layout.h"

namespace xg {

class RenderPass {
 public:
  RenderPass(const RenderPass&) = delete;
  RenderPass& operator=(const RenderPass&) = delete;
  RenderPass(RenderPass&&) = delete;
  RenderPass& operator=(RenderPass&&) = delete;
  virtual ~RenderPass() = default;

  virtual Result Init(const LayoutRenderPass& lrender_pass) = 0;
  virtual void Exit() = 0;

protected:
  RenderPass() = default;
};

}  // namespace xg

#endif  // XG_RENDER_PASS_H_
