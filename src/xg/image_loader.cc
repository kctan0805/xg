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
  ktxTexture* ktx_texture = nullptr;

  const auto deleter = [&](void*) {
    barrier_.set_value(nullptr);
    status_ = ResourceLoaderStatus::kEnded;
    if (!info_.file_path.empty()) {
      if (ktx_texture) {
        ktxTexture_Destroy(ktx_texture);
      } else {
        stbi_image_free(info_.src_ptr);
      }
      info_.src_ptr = nullptr;
    }
  };
  std::unique_ptr<void, decltype(deleter)> raii(static_cast<void*>(this),
                                                deleter);

  LayoutBuffer lbuffer;
  lbuffer.size = info_.size;
  auto device = ResourceLoader::GetDevice();
  auto limage = info_.limage;

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

      info_.src_ptr = ktxTexture_GetData(ktx_texture);
      lbuffer.size = ktxTexture_GetDataSize(ktx_texture);

      if (limage->instance) {
        assert(ktx_texture->baseWidth == limage->extent.width);
        assert(ktx_texture->baseHeight == limage->extent.height);
        assert(ktx_texture->numLevels == limage->mip_levels);
        assert(ktx_texture->numLayers * ktx_texture->numFaces ==
               limage->array_layers);
      } else {
        limage->extent.width = ktx_texture->baseWidth;
        limage->extent.height = ktx_texture->baseHeight;
        limage->mip_levels = ktx_texture->numLevels;
        limage->array_layers = ktx_texture->numFaces * ktx_texture->numLayers;

        auto image = device->CreateImage(*limage);
        if (!image) return;

        limage->instance = image;
      }
    } else {
      int width, height, channels;
      int req_comp = FormatToSize(limage->format);
      info_.src_ptr = stbi_load(info_.file_path.c_str(), &width, &height,
                                &channels, req_comp);
      if (!info_.src_ptr) {
        XG_ERROR("load stb image fail: {}", info_.file_path);
        return;
      }

      if (limage->instance) {
        assert(width == limage->extent.width);
        assert(height == limage->extent.height);
      } else {
        limage->extent.width = width;
        limage->extent.height = height;

        auto image = device->CreateImage(*limage);
        if (!image) return;

        limage->instance = image;
      }
      lbuffer.size = width * height * req_comp;
    }
  }

  auto dst_image = static_cast<Image*>(limage->instance.get());
  lbuffer.usage = BufferUsage::kTransferSrc;
  lbuffer.alloc_flags = MemoryAllocFlags::kCreateMapped;
  lbuffer.mem_usage = MemoryUsage::kCpuToGpu;

  context_ = ResourceLoader::AcquireNextContext(self);
  assert(context_);
  status_ = ResourceLoaderStatus::kRunning;

  context_->staging_buffer = device->CreateBuffer(lbuffer);
  if (!context_->staging_buffer) return;

  const auto& staging_data =
      static_cast<uint8_t*>(context_->staging_buffer->MapMemory());
  const auto& src_ptr = static_cast<const uint8_t*>(info_.src_ptr);
  std::copy(src_ptr, src_ptr + lbuffer.size, staging_data);

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
  image_barrier->image = dst_image;
  image_barrier->subresource_range.aspect_mask = ImageAspectFlags::kColor;
  image_barrier->subresource_range.level_count = limage->mip_levels;
  image_barrier->subresource_range.layer_count = limage->array_layers;

  cmd->PipelineBarrier(pipeline_barrier_info);

  CopyBufferToImageInfo copy_image_info = {};
  copy_image_info.src_buffer = context_->staging_buffer.get();
  copy_image_info.dst_image = dst_image;
  copy_image_info.dst_image_layout = ImageLayout::kTransferDstOptimal;
  copy_image_info.regions.reserve(limage->array_layers * limage->mip_levels);

  if (ktx_texture) {
    for (ktx_uint32_t layer = 0; layer < ktx_texture->numLayers; ++layer) {
      for (ktx_uint32_t face = 0; face < ktx_texture->numFaces; ++face) {
        for (ktx_uint32_t level = 0; level < ktx_texture->numLevels; ++level) {
          ktx_size_t offset;
          auto ret = ktxTexture_GetImageOffset(ktx_texture, level, layer, face,
                                               &offset);
          assert(ret == KTX_SUCCESS);

          BufferImageCopy buf_image_copy = {};
          buf_image_copy.image_subresource.aspect_mask =
              ImageAspectFlags::kColor;
          buf_image_copy.image_subresource.mip_level = static_cast<int>(level);
          buf_image_copy.image_subresource.base_array_layer =
              static_cast<int>(layer * ktx_texture->numFaces + face);
          buf_image_copy.image_subresource.layer_count = 1;
          buf_image_copy.image_extent.width = limage->extent.width >> level;
          buf_image_copy.image_extent.height = limage->extent.height >> level;
          buf_image_copy.image_extent.depth = 1;
          buf_image_copy.buffer_offset = offset;

          copy_image_info.regions.emplace_back(buf_image_copy);
        }
      }
    }

  } else {
    BufferImageCopy buf_image_copy = {};
    buf_image_copy.image_subresource.aspect_mask = ImageAspectFlags::kColor;
    buf_image_copy.image_subresource.layer_count = 1;
    buf_image_copy.image_extent.width = limage->extent.width;
    buf_image_copy.image_extent.height = limage->extent.height;
    buf_image_copy.image_extent.depth = 1;

    copy_image_info.regions.emplace_back(buf_image_copy);
  }

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
  image_barrier->image = dst_image;

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
