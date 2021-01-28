// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_COMMAND_H_
#define XG_COMMAND_H_

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "xg/command_buffer.h"
#include "xg/descriptor_set.h"
#include "xg/event.h"
#include "xg/framebuffer.h"
#include "xg/layout.h"
#include "xg/swapchain.h"
#include "xg/types.h"

namespace xg {

struct CommandInfo {
  const CommandBuffer* cmd_buffer;
  int frame;
};

class CommandNode {
 public:
  CommandNode(const CommandNode&) = delete;
  CommandNode& operator=(const CommandNode&) = delete;
  CommandNode(CommandNode&&) = delete;
  CommandNode& operator=(CommandNode&&) = delete;
  virtual ~CommandNode() = default;

  virtual void Build(const CommandInfo& cmd_info) const = 0;

 protected:
  CommandNode() = default;
};

class CommandBase;

class CommandList : public CommandNode {
 public:
  virtual ~CommandList() = default;

  void Build(const CommandInfo& cmd_info) const override;

 protected:
  std::vector<std::shared_ptr<CommandBase>> commands_;

  friend class Renderer;
};

class CommandGroup : public CommandNode {
 public:
  virtual ~CommandGroup() = default;

  void Build(const CommandInfo& cmd_info) const override;

 protected:
  std::vector<std::shared_ptr<CommandNode>> nodes_;

  friend class Engine;
  friend class Renderer;
};

class CommandContext {
 public:
  CommandContext() = default;
  CommandContext(const CommandContext&) = delete;
  CommandContext& operator=(const CommandContext&) = delete;
  CommandContext(CommandContext&&) = delete;
  CommandContext& operator=(CommandContext&&) = delete;
  virtual ~CommandContext() = default;

  bool Init(const LayoutCommandContext& lcmd_context);
  void Rebuild();
  Result Update(int frame);
  Result Build();
  CommandBuffer* GetCommandBuffer(int frame) const;

 protected:
  std::shared_ptr<CommandGroup> cmd_group_;
  std::shared_ptr<LayoutCommandBuffer> lcmd_buffer_;
  std::vector<bool> need_rebuilds_;

  friend class Renderer;
};

class CommandBase {
 public:
  CommandBase(const CommandBase&) = delete;
  CommandBase& operator=(const CommandBase&) = delete;
  CommandBase(CommandBase&&) = delete;
  CommandBase& operator=(CommandBase&&) = delete;
  virtual ~CommandBase() = default;

  virtual void Build(const CommandInfo& cmd_info) const = 0;

 protected:
  CommandBase() = default;
};

class CommandFunction : public CommandBase {
 public:
  virtual ~CommandFunction() = default;

  void Init(LayoutFunction* lfunction);
  void Build(const CommandInfo& cmd_info) const override {
    build_handler_(this, cmd_info);
  }

  using BuildHandlerType = void(const CommandFunction* cmd_func,
                                const CommandInfo& cmd_info);

  void SetBuildHandler(std::function<BuildHandlerType> handler) {
    build_handler_ = handler;
  }

  const std::vector<uint8_t>& GetData() const { return data_; }

 protected:
  std::function<BuildHandlerType> build_handler_ = [](const CommandFunction*,
                                                      const CommandInfo&) {};
  std::vector<uint8_t> data_;
};

class CommandPipelineBarrier : public CommandBase {
 public:
  virtual ~CommandPipelineBarrier() = default;

  bool Init(const LayoutPipelineBarrier& lpipeline_barrier);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  PipelineBarrierInfo info_ = {};
  std::vector<std::shared_ptr<LayoutBufferMemoryBarrier>>
      lbuffer_memory_barriers_;
  std::unique_ptr<std::vector<Swapchain*>> swapchains_;

  friend class Renderer;
};

class CommandCopyBuffer : public CommandBase {
 public:
  virtual ~CommandCopyBuffer() = default;

  void Init(const LayoutCopyBuffer& lcopy_buffer);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  CopyBufferInfo info_ = {};

  friend class Renderer;
};

class CommandDispatch : public CommandBase {
 public:
  virtual ~CommandDispatch() = default;

  void Init(const LayoutDispatch& ldispatch);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  DispatchInfo info_ = {};

  friend class Renderer;
};

class CommandBeginRenderPass : public CommandBase {
 public:
  virtual ~CommandBeginRenderPass() = default;

  void Init(const LayoutBeginRenderPass& lbegin_render_pass);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  RenderPassBeginInfo info_ = {};
  float rect_x_ = 0.0f;
  float rect_y_ = 0.0f;
  float rect_width_ = 0.0f;
  float rect_height_ = 0.0f;
  std::shared_ptr<std::vector<std::shared_ptr<Framebuffer>>>
      frame_framebuffers_;

  friend class Renderer;
};

class CommandEndRenderPass : public CommandBase {
 public:
  virtual ~CommandEndRenderPass() = default;

  void Build(const CommandInfo& cmd_info) const override;
};

class CommandSetViewport : public CommandBase {
 public:
  virtual ~CommandSetViewport() = default;

  void Init(const LayoutSetViewport& lset_viewport);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  SetViewportInfo info_ = {};

  friend class Renderer;
};

class CommandSetScissor : public CommandBase {
 public:
  virtual ~CommandSetScissor() = default;

  void Init(const LayoutSetScissor& lset_scissor);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  SetScissorInfo info_ = {};

  friend class Renderer;
};

class CommandBindDescriptorSets : public CommandBase {
 public:
  virtual ~CommandBindDescriptorSets() = default;

  bool Init(const LayoutBindDescriptorSets& lbind_desc_sets);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  BindDescriptorSetsInfo info_ = {};
  std::unique_ptr<
      std::vector<std::shared_ptr<std::vector<std::shared_ptr<DescriptorSet>>>>>
      frame_desc_sets_;

  friend class Renderer;
};

class CommandBindPipeline : public CommandBase {
 public:
  virtual ~CommandBindPipeline() = default;

  void Init(const LayoutBindPipeline& lbind_pipeline);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  Pipeline* pipeline_ = nullptr;

  friend class Renderer;
};

class CommandBindVertexBuffers : public CommandBase {
 public:
  virtual ~CommandBindVertexBuffers() = default;

  void Init(const LayoutBindVertexBuffers& lbind_vertex_buffers);
  void Build(const CommandInfo& cmd_info) const override;
  BindVertexBuffersInfo* GetInfo() { return &info_; };

 protected:
  BindVertexBuffersInfo info_ = {};
  std::vector<std::shared_ptr<LayoutBuffer>> lbuffers_;

  friend class Renderer;
};

class CommandBindIndexBuffer : public CommandBase {
 public:
  virtual ~CommandBindIndexBuffer() = default;

  void Init(const LayoutBindIndexBuffer& lbind_index_buffer);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  BindIndexBufferInfo info_ = {};
  std::shared_ptr<LayoutBuffer> lbuffer_;

  friend class Renderer;
};

class CommandDraw : public CommandBase {
 public:
  virtual ~CommandDraw() = default;

  void Init(const LayoutDraw& ldraw);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  DrawInfo info_ = {};

  friend class Renderer;
};

class CommandDrawIndexed : public CommandBase {
 public:
  virtual ~CommandDrawIndexed() = default;

  void Init(const LayoutDrawIndexed& ldraw_indexed);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  DrawIndexedInfo info_ = {};

  friend class Renderer;
};

class CommandDrawIndexedIndirect : public CommandBase {
 public:
  virtual ~CommandDrawIndexedIndirect() = default;

  void Init(const LayoutDrawIndexedIndirect& ldraw_indexed_indirect);
  void Build(const CommandInfo& cmd_info) const override;
  DrawIndexedIndirectInfo* GetInfo() { return &info_; };

 protected:
  DrawIndexedIndirectInfo info_ = {};

  friend class Renderer;
};

struct CommandImageBlitOffsets {
  std::array<float, 2> src_offsets[2];
  std::array<float, 2> dst_offsets[2];
};

class CommandBlitImage : public CommandBase {
 public:
  virtual ~CommandBlitImage() = default;

  void Init(const LayoutBlitImage& lblit_image);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  BlitImageInfo info_ = {};
  std::vector<CommandImageBlitOffsets> offsets_;

  Swapchain* src_swapchain_ = nullptr;
  Swapchain* dst_swapchain_ = nullptr;

  friend class Renderer;
};

class CommandPushConstants : public CommandBase {
 public:
  virtual ~CommandPushConstants() = default;

  void Init(LayoutPushConstants* lpush_constants);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  PushConstantsInfo info_ = {};
  std::vector<uint8_t> data_;

  friend class Renderer;
};

class CommandResetQueryPool : public CommandBase {
 public:
  virtual ~CommandResetQueryPool() = default;

  void Init(const LayoutResetQueryPool& lreset_query_pool);
  void Build(const CommandInfo& cmd_info) const override;
  ResetQueryPoolInfo* GetInfo() { return &info_; };

 protected:
  ResetQueryPoolInfo info_ = {};
  std::shared_ptr<LayoutQueryPool> lquery_pool_;

  friend class Renderer;
};

class CommandSetEvent : public CommandBase {
 public:
  virtual ~CommandSetEvent() = default;

  void Init(const LayoutSetEvent& lset_event);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  EventInfo info_ = {};
  std::shared_ptr<std::vector<std::shared_ptr<Event>>> frame_events_;

  friend class Renderer;
};

class CommandResetEvent : public CommandBase {
 public:
  virtual ~CommandResetEvent() = default;

  void Init(const LayoutResetEvent& lreset_event);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  EventInfo info_ = {};
  std::shared_ptr<std::vector<std::shared_ptr<Event>>> frame_events_;

  friend class Renderer;
};

class CommandNextSubpass : public CommandBase {
 public:
  virtual ~CommandNextSubpass() = default;

  void Init(const LayoutNextSubpass& lnext_subpass);
  void Build(const CommandInfo& cmd_info) const override;

 protected:
  NextSubpassInfo info_ = {};

  friend class Renderer;
};

}  // namespace xg

#endif  // XG_COMMAND_H_
