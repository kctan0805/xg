// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_COMMAND_POOL_H_
#define XG_COMMAND_POOL_H_

#include <memory>
#include <vector>

#include "xg/command_buffer.h"
#include "xg/layout.h"

namespace xg {

class CommandPool {
 public:
  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;
  CommandPool(CommandPool&&) = delete;
  CommandPool& operator=(CommandPool&&) = delete;
  virtual ~CommandPool() = default;

  virtual bool AllocateCommandBuffers(
      const std::vector<std::shared_ptr<LayoutCommandBuffer>>& lcmd_buffers,
      std::vector<std::shared_ptr<CommandBuffer>>* cmd_buffers) = 0;

 protected:
  CommandPool() = default;
};

}  // namespace xg

#endif  // XG_COMMAND_POOL_H_
