// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_BUFFER_VK_H_
#define XG_VULKAN_BUFFER_VK_H_

#include <memory>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/buffer.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {

class BufferVK : public Buffer {
 public:
  virtual ~BufferVK();

  Result Init(const LayoutBuffer& lbuffer) override;
  void Exit() override;

  void InvalidateRange(const MemoryRangeInfo& info) override;
  void FlushRange(const MemoryRangeInfo& info) override;
  void* MapMemory() override;
  void UnmapMemory() override;

 protected:
  VmaAllocator vma_allocator_ = VK_NULL_HANDLE;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VmaAllocation alloc_ = VK_NULL_HANDLE;
  int min_uniform_buffer_offset_align_ = 0;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_BUFFER_VK_H_
