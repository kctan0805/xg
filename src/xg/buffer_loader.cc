// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/buffer_loader.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "xg/device.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/resource_loader.h"
#include "xg/thread_pool.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

std::shared_ptr<BufferLoader> BufferLoader::Load(const BufferLoaderInfo& info) {
  const auto& task = std::make_shared<BufferLoader>();
  if (!task) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  task->info_ = info;
  ThreadPool::Get().Post(ThreadPool::Job(task));
  return task;
}

void BufferLoader::Run(std::shared_ptr<Task> self) {
  const auto deleter = [&](void*) {
    barrier_.set_value(nullptr);
    status_ = ResourceLoaderStatus::kEnded;
  };
  std::unique_ptr<void, decltype(deleter)> raii(static_cast<void*>(this),
                                                deleter);

  assert(info_.dst_buffers.size() > 0);
  auto* dst_buffer = info_.dst_buffers[0];
  auto data_size = info_.size;
  if (data_size == -1) data_size = dst_buffer->GetSize();

  CommandBufferBeginInfo begin_info = {};
  begin_info.usage = CommandBufferUsage::kOneTimeSubmit;
  xg::CommandBuffer* cmd = nullptr;

  if (dst_buffer->GetMemoryUsage() == MemoryUsage::kGpuOnly) {
    LayoutBuffer lbuffer;
    lbuffer.size = data_size;

    lbuffer.usage = BufferUsage::kTransferSrc;
    lbuffer.alloc_flags = MemoryAllocFlags::kCreateMapped;
    lbuffer.mem_usage = MemoryUsage::kCpuToGpu;

    context_ = ResourceLoader::AcquireNextContext(self);
    assert(context_);
    status_ = ResourceLoaderStatus::kRunning;

    context_->staging_buffer =
        ResourceLoader::GetDevice()->CreateBuffer(lbuffer);
    if (!context_->staging_buffer) return;

    const auto& staging_data =
        static_cast<uint8_t*>(context_->staging_buffer->MapMemory());

    if (info_.src_ptr) {
      const auto* src_ptr =
          static_cast<const uint8_t*>(info_.src_ptr) + info_.src_offset;
      std::copy(src_ptr, src_ptr + lbuffer.size, staging_data);
    } else {
      std::vector<uint8_t> file_data;
      if (!LoadFile(info_.file_path, &file_data)) return;

      const auto* src_ptr = file_data.data() + info_.src_offset;
      std::copy(src_ptr, src_ptr + lbuffer.size, staging_data);
    }

    context_->staging_buffer->UnmapMemory();

    cmd = context_->cmd_buffer.get();

    if (cmd->Begin(begin_info) != Result::kSuccess) return;

    PipelineBarrierInfo pipeline_barrier_info = {};
    pipeline_barrier_info.src_stage_mask = PipelineStageFlags::kTopOfPipe;
    pipeline_barrier_info.dst_stage_mask = PipelineStageFlags::kTransfer;
    pipeline_barrier_info.buffer_barriers.resize(info_.dst_buffers.size());

    int i = 0;
    for (const auto& buffer : info_.dst_buffers) {
      auto buf_barrier = &pipeline_barrier_info.buffer_barriers[i];
      buf_barrier->src_access_mask = AccessFlags::kUndefined;
      buf_barrier->dst_access_mask = AccessFlags::kTransferWrite;
      buf_barrier->src_queue_family_index =
          context_->queue->GetQueueFamilyIndex();
      buf_barrier->dst_queue_family_index = buf_barrier->src_queue_family_index;
      buf_barrier->offset = info_.dst_offset;
      buf_barrier->size = lbuffer.size;
      buf_barrier->buffer = buffer;
      ++i;
    }
    cmd->PipelineBarrier(pipeline_barrier_info);

    CopyBufferInfo copy_buf_info = {};
    copy_buf_info.src_buffer = context_->staging_buffer.get();
    copy_buf_info.regions.resize(1);
    copy_buf_info.regions[0].src_offset = 0;
    copy_buf_info.regions[0].dst_offset = info_.dst_offset;
    copy_buf_info.regions[0].size = lbuffer.size;

    for (const auto& buffer : info_.dst_buffers) {
      copy_buf_info.dst_buffer = buffer;
      cmd->CopyBuffer(copy_buf_info);
    }

    pipeline_barrier_info.src_stage_mask = PipelineStageFlags::kTransfer;
    pipeline_barrier_info.dst_stage_mask = info_.dst_stage_mask;

    i = 0;
    for (const auto& buffer : info_.dst_buffers) {
      auto buf_barrier = &pipeline_barrier_info.buffer_barriers[i];
      buf_barrier->src_access_mask = AccessFlags::kTransferWrite;
      buf_barrier->dst_access_mask = info_.dst_access_mask;

      if (info_.dst_queue) {
        buf_barrier->src_queue_family_index =
            context_->queue->GetQueueFamilyIndex();
        buf_barrier->dst_queue_family_index =
            info_.dst_queue->GetQueueFamilyIndex();
      } else {
        buf_barrier->src_queue_family_index = -1;
        buf_barrier->dst_queue_family_index = -1;
      }

      buf_barrier->offset = info_.dst_offset;
      buf_barrier->size = lbuffer.size;
      buf_barrier->buffer = buffer;
      ++i;
    }
  } else {
    auto* data = static_cast<uint8_t*>(dst_buffer->MapMemory());

    if (info_.src_ptr) {
      const auto* src_ptr =
          static_cast<const uint8_t*>(info_.src_ptr) + info_.src_offset;
      std::copy(src_ptr, src_ptr + data_size, data);
    } else {
      std::vector<uint8_t> file_data;
      if (!LoadFile(info_.file_path, &file_data)) return;

      const auto* src_ptr = file_data.data() + info_.src_offset;
      std::copy(src_ptr, src_ptr + data_size, data);
    }

    for (int i = 1; i < info_.dst_buffers.size(); ++i) {
      auto* dst_data = static_cast<uint8_t*>(info_.dst_buffers[i]->MapMemory());
      std::copy(data, data + data_size, dst_data);
      info_.dst_buffers[i]->UnmapMemory();
    }

    dst_buffer->UnmapMemory();

    cmd = context_->cmd_buffer.get();

    if (cmd->Begin(begin_info) != Result::kSuccess) return;

    PipelineBarrierInfo pipeline_barrier_info = {};
    pipeline_barrier_info.src_stage_mask = PipelineStageFlags::kTopOfPipe;
    pipeline_barrier_info.dst_stage_mask = info_.dst_stage_mask;
    pipeline_barrier_info.buffer_barriers.resize(info_.dst_buffers.size());

    int i = 0;
    for (const auto& buffer : info_.dst_buffers) {
      auto buf_barrier = &pipeline_barrier_info.buffer_barriers[i];
      buf_barrier->src_access_mask = AccessFlags::kHostWrite;
      buf_barrier->dst_access_mask = info_.dst_access_mask;

      if (info_.dst_queue) {
        buf_barrier->src_queue_family_index =
            context_->queue->GetQueueFamilyIndex();
        buf_barrier->dst_queue_family_index =
            info_.dst_queue->GetQueueFamilyIndex();
      } else {
        buf_barrier->src_queue_family_index = -1;
        buf_barrier->dst_queue_family_index = -1;
      }

      buf_barrier->offset = info_.dst_offset;
      buf_barrier->size = data_size;
      buf_barrier->buffer = buffer;
      ++i;
    }
    cmd->PipelineBarrier(pipeline_barrier_info);
  }

  cmd->End();

  context_->queue_submit_info.submit_infos[0].cmd_buffers[0] = cmd;

  const auto& result = context_->queue->Submit(context_->queue_submit_info);
  if (result != Result::kSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return;
  }

  result_ = 0;
}

}  // namespace xg
