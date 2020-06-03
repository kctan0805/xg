// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/image_loader.h"

#include <cassert>
#include <memory>

#include "ktx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "xg/device.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/resource_loader.h"
#include "xg/thread_pool.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

std::shared_ptr<ImageLoader> ImageLoader::Load(const ImageLoaderInfo& info) {
  const auto& task = std::make_shared<ImageLoader>();
  if (!task) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  task->info_ = info;
  ThreadPool::Get().Post(ThreadPool::Job(task));
  return task;
}

static inline bool ends_with(std::string const& value,
                             std::string const& ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void ImageLoader::Run(std::shared_ptr<Task> self) {
  const auto deleter = [&](void*) {
    barrier_.set_value(nullptr);
    status_ = ResourceLoaderStatus::kEnded;
    if (!info_.file_path.empty()) stbi_image_free(info_.src_ptr);
  };
  std::unique_ptr<void, decltype(deleter)> raii(static_cast<void*>(this),
                                                deleter);

  LayoutBuffer lbuffer;
  lbuffer.size = info_.size;
  ktxTexture* ktx_texture = nullptr;

  if (info_.src_ptr == nullptr) {
    assert(!info_.file_path.empty());

    if (ends_with(info_.file_path, "ktx")) {
      auto result = ktxTexture_CreateFromNamedFile(
          info_.file_path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
          &ktx_texture);
      if (result != KTX_SUCCESS) {
        XG_ERROR("load ktx image fail: {} result: {}", info_.file_path, result);
        return;
      }

      assert(ktx_texture->baseWidth == info_.width);
      assert(ktx_texture->baseHeight == info_.height);

      info_.src_ptr = ktx_texture->pData;
      lbuffer.size = ktx_texture->dataSize;

    } else {
      int width, height, channels;
      int req_comp = FormatToSize(info_.format);
      info_.src_ptr = stbi_load(info_.file_path.c_str(), &width, &height,
                                &channels, req_comp);
      if (!info_.src_ptr) {
        XG_ERROR("load stb image fail: {}", info_.file_path);
        return;
      }

      assert(width == info_.width);
      assert(height == info_.height);

      lbuffer.size = width * height * req_comp;
    }
  }

  lbuffer.usage = BufferUsage::kTransferSrc;
  lbuffer.alloc_flags = MemoryAllocFlags::kCreateMapped;
  lbuffer.mem_usage = MemoryUsage::kCpuToGpu;

  context_ = ResourceLoader::AcquireNextContext(self);
  assert(context_);
  status_ = ResourceLoaderStatus::kRunning;

  context_->staging_buffer = ResourceLoader::GetDevice()->CreateBuffer(lbuffer);
  if (!context_->staging_buffer) return;

  const auto& staging_data =
      static_cast<uint8_t*>(context_->staging_buffer->MapMemory());
  const auto& src_ptr = static_cast<const uint8_t*>(info_.src_ptr);
  std::copy(src_ptr, src_ptr + lbuffer.size, staging_data);

  if (!info_.file_path.empty()) {
    if (ktx_texture) {
      ktxTexture_Destroy(ktx_texture);
    } else {
      stbi_image_free(info_.src_ptr);
    }
    info_.src_ptr = nullptr;
  }

  context_->staging_buffer->UnmapMemory();

  CommandBufferBeginInfo begin_info = {};
  begin_info.usage = CommandBufferUsage::kOneTimeSubmit;

  const auto& cmd = context_->cmd_buffer;

  if (cmd->Begin(begin_info) != Result::kSuccess) return;

  PipelineBarrierInfo pipeline_barrier_info = {};
  pipeline_barrier_info.src_stage_mask = PipelineStageFlags::kTopOfPipe;
  pipeline_barrier_info.dst_stage_mask = PipelineStageFlags::kTransfer;
  pipeline_barrier_info.image_barriers.resize(1);
  auto image_barrier = &pipeline_barrier_info.image_barriers[0];
  image_barrier->src_access_mask = AccessFlags::kUndefined;
  image_barrier->dst_access_mask = AccessFlags::kTransferWrite;
  image_barrier->old_layout = ImageLayout::kUndefined;
  image_barrier->new_layout = ImageLayout::kTransferDstOptimal;
  image_barrier->src_queue_family_index =
      context_->queue->GetQueueFamilyIndex();
  image_barrier->dst_queue_family_index = image_barrier->src_queue_family_index;
  image_barrier->image = info_.dst_image;
  image_barrier->subresource_range.aspect_mask = ImageAspectFlags::kColor;
  image_barrier->subresource_range.level_count = 1;
  image_barrier->subresource_range.layer_count = 1;

  cmd->PipelineBarrier(pipeline_barrier_info);

  CopyBufferToImageInfo copy_image_info = {};
  copy_image_info.src_buffer = context_->staging_buffer.get();
  copy_image_info.dst_image = info_.dst_image;
  copy_image_info.dst_image_layout = ImageLayout::kTransferDstOptimal;
  copy_image_info.regions.resize(1);
  auto buf_image_copy = &copy_image_info.regions[0];
  buf_image_copy->image_subresource.aspect_mask = ImageAspectFlags::kColor;
  buf_image_copy->image_subresource.layer_count = 1;
  buf_image_copy->image_extent.width = info_.width;
  buf_image_copy->image_extent.height = info_.height;
  buf_image_copy->image_extent.depth = 1;

  cmd->CopyBufferToImage(copy_image_info);

  pipeline_barrier_info.src_stage_mask = PipelineStageFlags::kTransfer;
  pipeline_barrier_info.dst_stage_mask = info_.dst_stage_mask;
  image_barrier->src_access_mask = AccessFlags::kTransferWrite;
  image_barrier->dst_access_mask = info_.dst_access_mask;
  image_barrier->old_layout = ImageLayout::kTransferDstOptimal;
  image_barrier->new_layout = info_.new_layout;

  if (info_.dst_queue) {
    image_barrier->src_queue_family_index =
        context_->queue->GetQueueFamilyIndex();
    image_barrier->dst_queue_family_index =
        info_.dst_queue->GetQueueFamilyIndex();
  } else {
    image_barrier->src_queue_family_index = -1;
    image_barrier->dst_queue_family_index = -1;
  }
  image_barrier->image = info_.dst_image;

  cmd->PipelineBarrier(pipeline_barrier_info);

  cmd->End();

  context_->queue_submit_info.submit_infos[0].cmd_buffers[0] = cmd.get();
  const auto& result = context_->queue->Submit(context_->queue_submit_info);
  if (result != Result::kSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return;
  }

  result_ = 0;
}

}  // namespace xg
