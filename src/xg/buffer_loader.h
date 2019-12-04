// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_BUFFER_LOADER_H_
#define XG_BUFFER_LOADER_H_

#include <memory>
#include <string>
#include <vector>

#include "xg/buffer.h"
#include "xg/queue.h"
#include "xg/resource_loader.h"
#include "xg/types.h"

namespace xg {

struct BufferLoaderInfo {
  std::string file_path;
  const void* src_ptr = nullptr;
  std::vector<Buffer*> dst_buffers;
  size_t src_offset = 0;
  size_t dst_offset = 0;
  size_t size = static_cast<size_t>(-1);
  AccessFlags dst_access_mask = AccessFlags::kMemoryRead;
  Queue* dst_queue = nullptr;
  PipelineStageFlags dst_stage_mask = PipelineStageFlags::kAllCommands;
};

class BufferLoader : public ResourceLoader {
 public:
  static std::shared_ptr<BufferLoader> Load(const BufferLoaderInfo& info);

  void Run(std::shared_ptr<Task> self) override;

 protected:
  BufferLoaderInfo info_;
};

}  // namespace xg

#endif  // XG_BUFFER_LOADER_H_
