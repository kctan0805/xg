// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_QUEUE_H_
#define XG_QUEUE_H_

#include <memory>
#include <vector>

#include "xg/command_buffer.h"
#include "xg/command_pool.h"
#include "xg/fence.h"
#include "xg/layout.h"
#include "xg/semaphore.h"
#include "xg/swapchain.h"
#include "xg/types.h"

namespace xg {

struct SubmitInfo {
  std::vector<Semaphore*> wait_semaphores;
  std::vector<PipelineStageFlags> wait_dst_stage_masks;
  std::vector<CommandBuffer*> cmd_buffers;
  std::vector<Semaphore*> signal_semaphores;
};

struct QueueSubmitInfo {
  std::vector<SubmitInfo> submit_infos;
  Fence* fence = nullptr;
};

struct PresentInfo {
  std::vector<Semaphore*> wait_semaphores;
  std::vector<Swapchain*> swapchains;
  std::vector<int> image_indices;
  std::vector<Result> results;
};

class Queue {
 public:
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue(Queue&&) = delete;
  Queue& operator=(Queue&&) = delete;
  virtual ~Queue() = default;

  int GetQueueFamilyIndex() const { return family_index_; }

  virtual std::shared_ptr<CommandPool> CreateCommandPool(
      const LayoutCommandPool& lcmd_pool) = 0;

  virtual Result Submit(const QueueSubmitInfo& info) = 0;
  virtual Result Present(const PresentInfo& info) = 0;
  virtual void WaitIdle() = 0;

 protected:
  Queue() = default;

  int family_index_ = 0;
  float priority_ = 0.0f;
};

}  // namespace xg

#endif  // XG_QUEUE_H_
