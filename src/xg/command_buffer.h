// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_COMMAND_BUFFER_H_
#define XG_COMMAND_BUFFER_H_

#include <cstdint>
#include <variant>
#include <vector>

#include "xg/buffer.h"
#include "xg/descriptor_set.h"
#include "xg/event.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/pipeline.h"
#include "xg/pipeline_layout.h"
#include "xg/query_pool.h"
#include "xg/render_pass.h"
#include "xg/types.h"

namespace xg {

struct CommandBufferBeginInfo {
  CommandBufferUsage usage;
};

struct MemoryBarrier {
  AccessFlags src_access_mask;
  AccessFlags dst_access_mask;
};

struct BufferMemoryBarrier {
  AccessFlags src_access_mask;
  AccessFlags dst_access_mask;
  int src_queue_family_index;
  int dst_queue_family_index;
  Buffer* buffer;
  size_t offset;
  size_t size;
};

struct ImageMemoryBarrier {
  AccessFlags src_access_mask;
  AccessFlags dst_access_mask;
  ImageLayout old_layout;
  ImageLayout new_layout;
  int src_queue_family_index;
  int dst_queue_family_index;
  Image* image;
  ImageSubresourceRange subresource_range;
};

struct PipelineBarrierInfo {
  PipelineStageFlags src_stage_mask;
  PipelineStageFlags dst_stage_mask;
  std::vector<MemoryBarrier> memory_barriers;
  std::vector<BufferMemoryBarrier> buffer_barriers;
  std::vector<ImageMemoryBarrier> image_barriers;
};

struct DispatchInfo {
  int group_count_x;
  int group_count_y;
  int group_count_z;
};

struct CopyBufferInfo {
  Buffer* src_buffer;
  Buffer* dst_buffer;
  std::vector<BufferCopy> regions;
};

struct BufferImageCopy {
  size_t buffer_offset;
  int buffer_low_length;
  int buffer_image_height;
  ImageSubresourceLayers image_subresource;
  Offset3D image_offset;
  Extent3D image_extent;
};

struct CopyBufferToImageInfo {
  Buffer* src_buffer;
  Image* dst_image;
  ImageLayout dst_image_layout;
  size_t offset;
  std::vector<BufferImageCopy> regions;
};

struct BlitImageInfo {
  Image* src_image;
  ImageLayout src_image_layout;
  Image* dst_image;
  ImageLayout dst_image_layout;
  std::vector<ImageBlit> regions;
  Filter filter;
};

struct PushConstantsInfo {
  PipelineLayout* layout;
  ShaderStageFlags stage_flags;
  size_t offset;
  size_t size;
  const void* values;
};

struct RenderPassBeginInfo {
  RenderPass* render_pass;
  Framebuffer* framebuffer;
  Rect2D rect;
  std::vector<std::variant<ClearColorValue, ClearDepthStencilValue>>
      clear_values;
};

struct SetViewportInfo {
  int first_viewport;
  std::vector<Viewport> viewports;
};

struct SetScissorInfo {
  int first_scissor;
  std::vector<Rect2D> scissors;
};

struct BindDescriptorSetsInfo {
  PipelineBindPoint bind_point;
  PipelineLayout* pipeline_layout;
  int first_set;
  std::vector<DescriptorSet*> desc_sets;
  std::vector<int> dynamic_offsets;
};

struct BindVertexBuffersInfo {
  int first_binding;
  std::vector<Buffer*> buffers;
  std::vector<size_t> offsets;
};

struct BindIndexBufferInfo {
  size_t offset;
  Buffer* buffer;
  IndexType index_type;
};

struct DrawInfo {
  int vertex_count;
  int instance_count;
  int first_vertex;
  int first_instance;
};

struct DrawIndexedInfo {
  int index_count;
  int instance_count;
  int first_index;
  int vertex_offset;
  int first_instance;
};

struct DrawIndexedIndirectCommand {
  int index_count;
  int instance_count;
  int first_index;
  int vertex_offset;
  int first_instance;
};

struct DrawIndexedIndirectInfo {
  Buffer* buffer;
  size_t offset;
  int draw_count;
  int stride;
};

struct ResetQueryPoolInfo {
  QueryPool* query_pool;
  int first_query;
  int query_count;
};

struct QueryInfo {
  QueryPool* query_pool;
  int query;
};

struct EventInfo {
  Event* event;
  PipelineStageFlags stage_mask;
};

struct NextSubpassInfo {
  SubpassContents contents;
};

class CommandBuffer {
 public:
  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer(CommandBuffer&&) = delete;
  CommandBuffer& operator=(CommandBuffer&&) = delete;
  virtual ~CommandBuffer() = default;

  virtual Result Begin(const CommandBufferBeginInfo& info) const = 0;
  virtual void End() const = 0;
  virtual void Reset() const = 0;

  virtual void PipelineBarrier(const PipelineBarrierInfo& info) const = 0;
  virtual void Dispatch(const DispatchInfo& info) const  = 0;
  virtual void CopyBuffer(const CopyBufferInfo& info) const = 0;
  virtual void CopyBufferToImage(const CopyBufferToImageInfo& info) const = 0;
  virtual void BlitImage(const BlitImageInfo& info) const = 0;
  virtual void PushConstants(const PushConstantsInfo& info) const = 0;
  virtual void BeginRenderPass(const RenderPassBeginInfo& info) const = 0;
  virtual void EndRenderPass() const = 0;
  virtual void SetViewport(const SetViewportInfo& info) const = 0;
  virtual void SetScissor(const SetScissorInfo& info) const = 0;
  virtual void BindDescriptorSets(const BindDescriptorSetsInfo& info) const = 0;
  virtual void BindPipeline(const Pipeline& pipeline) const = 0;
  virtual void BindVertexBuffers(const BindVertexBuffersInfo& info) const = 0;
  virtual void BindIndexBuffer(const BindIndexBufferInfo& info) const = 0;
  virtual void Draw(const DrawInfo& info) const = 0;
  virtual void DrawIndexed(const DrawIndexedInfo& info) const = 0;
  virtual void DrawIndexedIndirect(
      const DrawIndexedIndirectInfo& info) const = 0;
  virtual void ResetQueryPool(const ResetQueryPoolInfo& info) const = 0;
  virtual void BeginQuery(const QueryInfo& info) const = 0;
  virtual void EndQuery(const QueryInfo& info) const = 0;
  virtual void SetEvent(const EventInfo& info) const = 0;
  virtual void ResetEvent(const EventInfo& info) const = 0;
  virtual void NextSubpass(const NextSubpassInfo& info) const = 0;

 protected:
  CommandBuffer() = default;
};

}  // namespace xg

#endif  // XG_COMMAND_BUFFER_H_
