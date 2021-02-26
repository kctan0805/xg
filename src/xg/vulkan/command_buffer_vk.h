// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_COMMAND_BUFFER_VK_H_
#define XG_VULKAN_COMMAND_BUFFER_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/command_buffer.h"
#include "xg/types.h"

namespace xg {

class CommandBufferVK : public CommandBuffer {
 public:
  virtual ~CommandBufferVK();

  const vk::CommandBuffer& GetVkCommandBuffer() const { return cmd_buffer_; }

  Result Begin(const CommandBufferBeginInfo& info) const override;
  void End() const override;
  void Reset() const override;

  void PipelineBarrier(const PipelineBarrierInfo& info) const override;
  void Dispatch(const DispatchInfo& info) const override;
  void CopyBuffer(const CopyBufferInfo& info) const override;
  void CopyBufferToImage(const CopyBufferToImageInfo& info) const override;
  void BlitImage(const BlitImageInfo& info) const override;
  void PushConstants(const PushConstantsInfo& info) const override;
  void BeginRenderPass(const RenderPassBeginInfo& info) const override;
  void EndRenderPass() const override;
  void SetViewport(const SetViewportInfo& info) const override;
  void SetScissor(const SetScissorInfo& info) const override;
  void BindDescriptorSets(const BindDescriptorSetsInfo& info) const override;
  void BindPipeline(const Pipeline& pipeline) const override;
  void BindVertexBuffers(const BindVertexBuffersInfo& info) const override;
  void BindIndexBuffer(const BindIndexBufferInfo& info) const override;
  void Draw(const DrawInfo& info) const override;
  void DrawIndexed(const DrawIndexedInfo& info) const override;
  void DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) const override;
  void ResetQueryPool(const ResetQueryPoolInfo& info) const override;
  void BeginQuery(const QueryInfo& info) const override;
  void EndQuery(const QueryInfo& info) const override;
  void SetEvent(const EventInfo& info) const override;
  void ResetEvent(const EventInfo& info) const override;
  void NextSubpass(const NextSubpassInfo& info) const override;

 protected:
  vk::Device device_;
  vk::CommandPool command_pool_;
  vk::CommandBuffer cmd_buffer_;

  friend class RendererVK;
  friend class QueueVK;
  friend class CommandPoolVK;
};

}  // namespace xg

#endif  // XG_VULKAN_COMMAND_BUFFER_VK_H_
