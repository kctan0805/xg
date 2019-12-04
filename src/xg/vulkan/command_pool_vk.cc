// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/command_pool_vk.h"

#include <cassert>
#include <memory>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/command_buffer_vk.h"

namespace xg {

CommandPoolVK::~CommandPoolVK() {
  if (cmd_pool_ && device_) {
    XG_TRACE("destroyCommandPool: {}",
             static_cast<void*>((VkCommandPool)cmd_pool_));

    device_.destroyCommandPool(cmd_pool_);
  }
}

bool CommandPoolVK::AllocateCommandBuffers(
    const std::vector<std::shared_ptr<LayoutCommandBuffer>>& lcmd_buffers,
    std::vector<std::shared_ptr<CommandBuffer>>* cmd_buffers) {
  assert(cmd_buffers);

  auto alloc_info =
      vk::CommandBufferAllocateInfo()
          .setCommandPool(cmd_pool_)
          .setLevel(vk::CommandBufferLevel::ePrimary)
          .setCommandBufferCount(static_cast<uint32_t>(lcmd_buffers.size()));

  std::vector<vk::CommandBuffer> vk_cmd_buffers(lcmd_buffers.size());

  XG_TRACE("allocateCommandBuffers: {}",
           static_cast<void*>((VkCommandPool)cmd_pool_));

  const auto& result =
      device_.allocateCommandBuffers(&alloc_info, vk_cmd_buffers.data());
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  int i = 0;
  for (const auto& vk_cmd_buffer : vk_cmd_buffers) {
    auto cmd_buffer = std::make_shared<CommandBufferVK>();
    if (!cmd_buffer) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return nullptr;
    }
    cmd_buffer->device_ = device_;
    cmd_buffer->command_pool_ = cmd_pool_;
    cmd_buffer->cmd_buffer_ = vk_cmd_buffer;
    cmd_buffers->emplace_back(cmd_buffer);

    XG_TRACE("  CommandBuffer: {} {}",
             static_cast<void*>((VkCommandBuffer)vk_cmd_buffer),
             lcmd_buffers[i++]->id);
  }
  return true;
}

}  // namespace xg
