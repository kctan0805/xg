// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_BUFFER_H_
#define XG_BUFFER_H_

#include "xg/layout.h"
#include "xg/types.h"

namespace xg {

class Buffer {
 public:
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;
  Buffer(Buffer&&) = delete;
  Buffer& operator=(Buffer&&) = delete;
  virtual ~Buffer() = default;

  virtual Result Init(const LayoutBuffer& lbuffer) = 0;
  virtual void Exit() = 0;

  size_t GetSize() const { return size_; }
  void* GetMappedData() const { return mapped_data_; }

  virtual void InvalidateRange(const MemoryRangeInfo& info) = 0;
  virtual void FlushRange(const MemoryRangeInfo& info) = 0;
  virtual void* MapMemory() = 0;
  virtual void UnmapMemory() = 0;

 protected:
  Buffer() = default;

  size_t size_ = 0;
  void* mapped_data_ = nullptr;
};

}  // namespace xg

#endif  // XG_BUFFER_H_
