// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_QUEUE_VK_H_
#define XG_VULKAN_QUEUE_VK_H_

#include <mutex>

#include "vulkan/vulkan.hpp"
#include "xg/command_pool.h"
#include "xg/layout.h"
#include "xg/queue.h"
#include "xg/types.h"

namespace xg {

class QueueVK : public Queue {
 public:
  virtual ~QueueVK() = default;

  const vk::Queue& GetVkQueue() const { return queue_; }

  std::shared_ptr<CommandPool> CreateCommandPool(
      const LayoutCommandPool& lcmd_pool) override;

  Result Submit(const QueueSubmitInfo& info) override;
  Result Present(const PresentInfo& info) override;
  void WaitIdle() override;

 protected:
  vk::Device device_;
  vk::Queue queue_;
  int use_count_ = 0;
  std::mutex mutex_;

  friend class RendererVK;
  friend class DeviceVK;
};

}  // namespace xg

#endif  // XG_VULKAN_QUEUE_VK_H_
