// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/buffer_vk.h"

#include <cassert>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

BufferVK::~BufferVK() { Exit(); }

Result BufferVK::Init(const LayoutBuffer& lbuffer) {
  assert(!buffer_);
  assert(!alloc_);

  size_t buffer_size = lbuffer.size;
  if (buffer_size == 0) {
    auto unit_size = lbuffer.unit_size;
    if (min_uniform_buffer_offset_align_ > 0) {
      unit_size = (unit_size + min_uniform_buffer_offset_align_ - 1) &
                  ~(min_uniform_buffer_offset_align_ - 1);
    }
    buffer_size = unit_size * lbuffer.unit;
    unit_size_ = unit_size;
  }

  const auto& buf_create_info =
      vk::BufferCreateInfo()
          .setSize(buffer_size)
          .setUsage(static_cast<vk::BufferUsageFlagBits>(lbuffer.usage));

  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.flags =
      static_cast<VmaAllocationCreateFlags>(lbuffer.alloc_flags);
  alloc_create_info.usage = static_cast<VmaMemoryUsage>(lbuffer.mem_usage);

  const auto& vk_buf_create_info =
      static_cast<VkBufferCreateInfo>(buf_create_info);
  VmaAllocationInfo alloc_info = {};

  auto result =
      vmaCreateBuffer(vma_allocator_, &vk_buf_create_info, &alloc_create_info,
                      &buffer_, &alloc_, &alloc_info);
  if (result != VK_SUCCESS) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  size_ = buffer_size;
  mapped_data_ = alloc_info.pMappedData;

  XG_TRACE("vmaCreateBuffer: {} {}", static_cast<void*>(buffer_), lbuffer.id);

  return Result::kSuccess;
}

void BufferVK::Exit() {
  if (buffer_ && vma_allocator_) {
    XG_TRACE("vmaDestroyBuffer: {}", static_cast<void*>(buffer_));
    vmaDestroyBuffer(vma_allocator_, buffer_, alloc_);
  }
}

void BufferVK::InvalidateRange(const MemoryRangeInfo& info) {
  if (mapped_data_ == nullptr) return;

  vmaInvalidateAllocation(vma_allocator_, alloc_,
                          static_cast<VkDeviceSize>(info.offset),
                          static_cast<VkDeviceSize>(info.size));
}

void BufferVK::FlushRange(const MemoryRangeInfo& info) {
  if (mapped_data_ == nullptr) return;

  vmaFlushAllocation(vma_allocator_, alloc_,
                     static_cast<VkDeviceSize>(info.offset),
                     static_cast<VkDeviceSize>(info.size));
}

void* BufferVK::MapMemory() {
  void* data = nullptr;
  VkResult result = vmaMapMemory(vma_allocator_, alloc_, &data);
  if (result != VK_SUCCESS) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  return data;
}

void BufferVK::UnmapMemory() { vmaUnmapMemory(vma_allocator_, alloc_); }

}  // namespace xg
