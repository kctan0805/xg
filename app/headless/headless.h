// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef HEADLESS_H_
#define HEADLESS_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "xg/buffer.h"
#include "xg/engine.h"
#include "xg/fence.h"
#include "xg/layout.h"
#include "xg/renderer.h"

class Application {
 public:
  std::shared_ptr<xg::Layout> CreateLayout();
  bool Init(xg::Engine* engine);
  bool Run();

 private:
  std::shared_ptr<xg::Queue> queue_;
  std::shared_ptr<xg::Fence> fence_;
  std::shared_ptr<xg::Buffer> buffer_;
  std::shared_ptr<xg::QueueSubmit> queue_submit_;
  std::vector<uint32_t> compute_input_;
  std::vector<uint32_t> compute_output_;
};

#endif  // HEADLESS_H_
