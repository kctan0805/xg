// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_COMMAND_POOL_VK_H_
#define XG_VULKAN_COMMAND_POOL_VK_H_

#include <memory>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "xg/command_pool.h"
#include "xg/layout.h"

namespace xg {

class CommandPoolVK : public CommandPool {
 public:
  virtual ~CommandPoolVK();

  bool AllocateCommandBuffers(
      const std::vector<std::shared_ptr<LayoutCommandBuffer>>& lcmd_buffers,
      std::vector<std::shared_ptr<CommandBuffer>>* cmd_buffers) override;

 protected:
  vk::Device device_;
  vk::CommandPool cmd_pool_;

  friend class RendererVK;
  friend class QueueVK;
};

}  // namespace xg

#endif  // XG_VULKAN_COMMAND_POOL_VK_H_
