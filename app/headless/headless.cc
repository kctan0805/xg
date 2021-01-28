// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "headless.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>

#include "xg/buffer.h"
#include "xg/command.h"
#include "xg/fence.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/queue.h"
#include "xg/renderer.h"
#include "xg/types.h"

static const int kElementCount = 32;

std::shared_ptr<xg::Layout> Application::CreateLayout() {
  auto layout = xg::Parser::Get().ParseFile("headless.xml");

  auto lbuffer_loader = std::static_pointer_cast<xg::LayoutBufferLoader>(
      layout->Find("deviceBufferLoader"));
  assert(lbuffer_loader);

  compute_input_.resize(kElementCount);

  uint32_t n = 0;
  std::generate(compute_input_.begin(), compute_input_.end(),
                [&n] { return n++; });

  lbuffer_loader->data = compute_input_.data();

  return layout;
}

bool Application::Init(xg::Engine* engine) {
  queue_ = std::static_pointer_cast<xg::Queue>(engine->Find("mainQueue"));
  assert(queue_);

  fence_ =
      std::static_pointer_cast<xg::Fence>(engine->Find("computeCompleteFence"));
  assert(fence_);

  buffer_ = std::static_pointer_cast<xg::Buffer>(engine->Find("hostBuffer"));
  assert(buffer_);

  queue_submit_ = std::static_pointer_cast<xg::QueueSubmit>(
      engine->Find("mainQueueSubmit"));
  assert(queue_submit_);

  auto cmd_ctxt = std::static_pointer_cast<xg::CommandContext>(
      engine->Find("mainCommandContext"));
  assert(cmd_ctxt);

  if (cmd_ctxt->Build() != xg::Result::kSuccess) return false;

  compute_output_.resize(kElementCount);

  return true;
}

bool Application::Run() {
  const auto& result = queue_->Submit(queue_submit_->queue_submit_info);
  if (result != xg::Result::kSuccess) return false;

  fence_->Wait();

  auto mapped = static_cast<uint32_t*>(buffer_->MapMemory());
  xg::MemoryRangeInfo range = {0, static_cast<size_t>(-1)};
  buffer_->InvalidateRange(range);
  std::copy(mapped, mapped + kElementCount, compute_output_.begin());
  buffer_->UnmapMemory();

  std::cout << "Compute input:\n";
  for (auto v : compute_input_) {
    std::cout << v << " \t";
  }
  std::cout << std::endl;

  std::cout << "Compute output:\n";
  for (auto v : compute_output_) {
    std::cout << v << " \t";
  }
  std::cout << std::endl;

  return true;
}
