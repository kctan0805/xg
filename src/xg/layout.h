// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_LAYOUT_H_
#define XG_LAYOUT_H_

#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/utility.hpp"
#include "cereal/types/variant.hpp"
#include "cereal/types/vector.hpp"
#include "glm/glm.hpp"
#include "xg/logger.h"
#include "xg/types.h"

namespace glm {

template <class Archive>
void serialize(Archive& archive, glm::vec3& v) {
  archive(v.x, v.y, v.z);
}

template <class Archive>
void serialize(Archive& archive, glm::vec4& v) {
  archive(v.x, v.y, v.z, v.w);
}

template <class Archive>
void serialize(Archive& archive, glm::mat4& m) {
  archive(m[0], m[1], m[2], m[3]);
}

}  // namespace glm

namespace xg {

template <class Archive>
void serialize(Archive& archive, Extent3D& extent) {
  archive(extent.width, extent.height, extent.depth);
}

template <class Archive>
void serialize(Archive& archive,
               ImageSubresourceRange& image_subresource_range) {
  archive(image_subresource_range.aspect_mask,
          image_subresource_range.base_mip_level,
          image_subresource_range.level_count,
          image_subresource_range.base_array_layer,
          image_subresource_range.layer_count);
}

template <class Archive>
void serialize(Archive& archive, PushConstantRange& push_constant_range) {
  archive(push_constant_range.stage_flags, push_constant_range.offset,
          push_constant_range.size);
}

template <class Archive>
void serialize(Archive& archive, MapEntry& map_entry) {
  archive(map_entry.constant_id, map_entry.offset, map_entry.size);
}

template <class Archive>
void serialize(Archive& archive, Viewport& viewport) {
  archive(viewport.x, viewport.y, viewport.width, viewport.height,
          viewport.min_depth, viewport.max_depth);
}

template <class Archive>
void serialize(Archive& archive, Rect2D& rect) {
  archive(rect.x, rect.y, rect.width, rect.height);
}

template <class Archive>
void serialize(Archive& archive, BufferCopy& region) {
  archive(region.src_offset, region.dst_offset, region.size);
}

template <class Archive>
void serialize(Archive& archive, ClearColorValue& value) {
  archive(value.value);
}

template <class Archive>
void serialize(Archive& archive, ClearDepthStencilValue& value) {
  archive(value.depth, value.stencil);
}

template <class Archive>
void serialize(Archive& archive, ImageSubresourceLayers& subresource) {
  archive(subresource.aspect_mask, subresource.mip_level,
          subresource.base_array_layer, subresource.layer_count);
}

template <class Archive>
void serialize(Archive& archive, Offset3D& offset) {
  archive(offset.x, offset.y, offset.z);
}

template <class Archive>
void serialize(Archive& archive, ImageBlit& region) {
  archive(region.src_subresource, region.src_offsets, region.dst_subresource,
          region.dst_offsets);
}

enum class LayoutType {
  kUndefined,
  kEngine,
  kConstant,
  kData,
  kRenderer,
  kWindow,
  kDevice,
  kQueue,
  kCommandPool,
  kCommandBuffer,
  kFence,
  kBuffer,
  kBufferLoader,
  kImage,
  kImageLoader,
  kImageView,
  kSwapchain,
  kRenderPass,
  kMultiview,
  kAttachment,
  kSubpass,
  kColorAttachment,
  kDepthStencilAttachment,
  kDependency,
  kShaderModule,
  kDescriptorSetLayout,
  kDescriptorSetLayoutBinding,
  kPipelineLayout,
  kComputePipeline,
  kGraphicsPipeline,
  kStage,
  kSpecializationInfo,
  kVertexInputState,
  kVertexBindingDescription,
  kVertexAttributeDescription,
  kInputAssemblyState,
  kViewportState,
  kViewport,
  kScissor,
  kRasterizationState,
  kMultisampleState,
  kDepthStencilState,
  kColorBlendState,
  kColorBlendAttachmentState,
  kDynamicState,
  kDescriptorPool,
  kDescriptorSet,
  kDescriptor,
  kFrame,
  kFramebuffer,
  kFramebufferAttachment,
  kSemaphore,
  kSampler,
  kQueryPool,
  kEvent,
  kCamera,
  kCommandGroup,
  kCommandList,
  kCommandContext,
  kFunction,
  kPipelineBarrier,
  kBufferMemoryBarrier,
  kImageMemoryBarrier,
  kCopyBuffer,
  kDispatch,
  kBeginRenderPass,
  kEndRenderPass,
  kSetViewport,
  kSetScissor,
  kBindDescriptorSets,
  kBindPipeline,
  kBindVertexBuffers,
  kBindIndexBuffer,
  kDrawIndexed,
  kDrawIndexedIndirect,
  kBlitImage,
  kPushConstants,
  kResetQueryPool,
  kSetEvent,
  kResetEvent,
  kViewer,
  kAcquireNextImage,
  kQueueSubmit,
  kQueuePresent,
  kSubmit,
  kResizer,
  kUpdater
};

struct LayoutBase {
  LayoutBase() = default;
  LayoutBase(LayoutType ltype) : layout_type(ltype) {}
  virtual ~LayoutBase() = default;

  LayoutType layout_type = LayoutType::kUndefined;
  std::string id;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(layout_type, id);
  }

  std::shared_ptr<void> instance;
};

struct LayoutEngine : LayoutBase {
  LayoutEngine() : LayoutBase{LayoutType::kEngine} {}

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this));
  }
};

struct LayoutConstant : LayoutBase {
  LayoutConstant() : LayoutBase{LayoutType::kConstant} {}

  float value = 0.0f;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), value);
  }
};

struct LayoutData : LayoutBase {
  LayoutData() : LayoutBase{LayoutType::kData} {}

  std::vector<uint8_t> data;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), data);
  }
};

struct LayoutRenderer : LayoutBase {
  LayoutRenderer() : LayoutBase{LayoutType::kRenderer} {}

  std::string app_name;
  bool debug = false;
  bool validation = false;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), app_name, debug, validation);
  }
};

struct LayoutWindow : LayoutBase {
  LayoutWindow() : LayoutBase{LayoutType::kWindow} {}

  std::shared_ptr<LayoutRenderer> lrenderer;
  int xpos = -1;
  int ypos = -1;
  int width = 0;
  int height = 0;
  std::string title;
  bool resizable = false;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lrenderer, xpos, ypos, width,
            height, title, resizable);
  }
};

struct LayoutQueue;

struct LayoutDevice : LayoutBase {
  LayoutDevice() : LayoutBase{LayoutType::kDevice} {}

  std::shared_ptr<LayoutRenderer> lrenderer;
  std::vector<std::shared_ptr<LayoutWindow>> lwindows;
  std::vector<std::shared_ptr<LayoutQueue>> lqueues;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lrenderer, lwindows, lqueues);
  }
};

struct LayoutQueue : LayoutBase {
  LayoutQueue() : LayoutBase{LayoutType::kQueue} {}

  QueueFamily queue_family = QueueFamily::kGraphics;
  float queue_priority = 0.0f;
  bool presentable = true;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), queue_family, queue_priority,
            presentable);
  }
};

struct LayoutCommandBuffer;

struct LayoutCommandPool : LayoutBase {
  LayoutCommandPool() : LayoutBase{LayoutType::kCommandPool} {}

  std::shared_ptr<LayoutQueue> lqueue;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lqueue);
  }

  const char* lqueue_id = nullptr;
};

struct LayoutFrame;

struct LayoutCommandBuffer : LayoutBase {
  LayoutCommandBuffer() : LayoutBase{LayoutType::kCommandBuffer} {}

  std::shared_ptr<LayoutCommandPool> lcmd_pool;
  std::shared_ptr<LayoutFrame> lframe;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lcmd_pool, lframe);
  }

  const char* lcmd_pool_id = nullptr;
};

struct LayoutFence : LayoutBase {
  LayoutFence() : LayoutBase{LayoutType::kFence} {}

  std::shared_ptr<LayoutFrame> lframe;
  bool signaled = false;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe, signaled);
  }
};

struct LayoutBuffer : LayoutBase {
  LayoutBuffer() : LayoutBase{LayoutType::kBuffer} {}

  std::shared_ptr<LayoutFrame> lframe;
  size_t size = 0;
  BufferUsage usage = BufferUsage::kUndefined;
  MemoryAllocFlags alloc_flags = MemoryAllocFlags::kDedicatedMemory;
  MemoryUsage mem_usage = MemoryUsage::kGpuOnly;
  int unit = 0;
  int unit_size = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe, size, usage,
            alloc_flags, mem_usage, unit, unit_size);
  }
};

struct LayoutBufferLoader : LayoutBase {
  LayoutBufferLoader() : LayoutBase{LayoutType::kBufferLoader} {}

  std::shared_ptr<LayoutBuffer> lbuffer;
  std::shared_ptr<LayoutQueue> lqueue;
  std::string file;
  size_t src_offset = 0;
  size_t dst_offset = 0;
  size_t size = static_cast<size_t>(-1);
  AccessFlags access_mask = AccessFlags::kMemoryRead;
  PipelineStageFlags stage_mask = PipelineStageFlags::kAllCommands;
  std::shared_ptr<LayoutData> ldata;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lbuffer, lqueue, file,
            src_offset, dst_offset, size, access_mask, stage_mask, ldata);
  }

  const void* data = nullptr;
  const char* lbuffer_id = nullptr;
  const char* lqueue_id = nullptr;
  const char* ldata_id = nullptr;
};

struct LayoutImage : LayoutBase {
  LayoutImage() : LayoutBase{LayoutType::kImage} {}

  ImageType image_type = ImageType::k2D;
  Format format = Format::kUndefined;
  Extent3D extent = {0, 0, 1};
  int mip_levels = 1;
  int array_layers = 1;
  ImageTiling tiling = ImageTiling::kOptimal;
  ImageUsage usage = ImageUsage::kTransferSrc;
  MemoryAllocFlags alloc_flags = MemoryAllocFlags::kDedicatedMemory;
  MemoryUsage mem_usage = MemoryUsage::kGpuOnly;
  ImageLayout initial_layout = ImageLayout::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), image_type, format, extent,
            mip_levels, array_layers, tiling, usage, alloc_flags, mem_usage,
            initial_layout);
  }
};

struct LayoutImageLoader : LayoutBase {
  LayoutImageLoader() : LayoutBase{LayoutType::kImageLoader} {}

  std::shared_ptr<LayoutImage> limage;
  std::shared_ptr<LayoutQueue> lqueue;
  std::string file;
  AccessFlags access_mask = AccessFlags::kShaderRead;
  ImageLayout layout = ImageLayout::kShaderReadOnlyOptimal;
  PipelineStageFlags stage_mask = PipelineStageFlags::kFragmentShader;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), limage, lqueue, file,
            access_mask, layout, stage_mask);
  }

  const char* limage_id = nullptr;
  const char* lqueue_id = nullptr;
};

struct LayoutImageView : LayoutBase {
  LayoutImageView() : LayoutBase{LayoutType::kImageView} {}

  std::shared_ptr<LayoutImage> limage;
  ImageViewType view_type = ImageViewType::k2D;
  Format format = Format::kUndefined;
  ImageSubresourceRange image_subresource_range = {ImageAspectFlags::kColor, 0,
                                                   1, 0, 1};
  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), limage, view_type, format,
            image_subresource_range);
  }

  const char* limage_id = nullptr;
};

struct LayoutSwapchain : LayoutBase {
  LayoutSwapchain() : LayoutBase{LayoutType::kSwapchain} {}

  std::shared_ptr<LayoutWindow> lwin;
  int min_image_count = 3;
  Format image_format = Format::kB8G8R8A8Unorm;
  ColorSpace image_color_space = ColorSpace::kSrgbNonlinear;
  int width = 0;
  int height = 0;
  int image_array_layers = 1;
  ImageUsage image_usage = ImageUsage::kColorAttachment;
  SurfaceTransformFlags pre_transform = SurfaceTransformFlags::kIdentity;
  CompositeAlpha composite_alpha = CompositeAlpha::kOpaque;
  PresentMode present_mode = PresentMode::kImmediate;
  bool clipped = true;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lwin, min_image_count,
            image_format, image_color_space, width, height, image_array_layers,
            image_usage, pre_transform, composite_alpha, present_mode, clipped);
  }

  const char* lwin_id = nullptr;
};

struct LayoutMultiview;
struct LayoutAttachment;
struct LayoutSubpass;
struct LayoutDependency;

struct LayoutRenderPass : LayoutBase {
  LayoutRenderPass() : LayoutBase{LayoutType::kRenderPass} {}

  std::shared_ptr<LayoutMultiview> lmultiview;
  std::vector<std::shared_ptr<LayoutAttachment>> lattachments;
  std::vector<std::shared_ptr<LayoutSubpass>> lsubpasses;
  std::vector<std::shared_ptr<LayoutDependency>> ldependencies;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lmultiview, lattachments,
            lsubpasses, ldependencies);
  }
};

struct LayoutMultiview : LayoutBase {
  LayoutMultiview() : LayoutBase{LayoutType::kMultiview} {}

  std::vector<int> view_masks;
  std::vector<int> view_offsets;
  std::vector<int> correlation_masks;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), view_masks, view_offsets,
            correlation_masks);
  }
};

struct LayoutAttachment : LayoutBase {
  LayoutAttachment() : LayoutBase{LayoutType::kAttachment} {}

  Format format = Format::kUndefined;
  SampleCountFlags samples = SampleCountFlags::k1;
  AttachmentLoadOp load_op = AttachmentLoadOp::kDontCare;
  AttachmentStoreOp store_op = AttachmentStoreOp::kDontCare;
  AttachmentLoadOp stencil_load_op = AttachmentLoadOp::kDontCare;
  AttachmentStoreOp stencil_store_op = AttachmentStoreOp::kDontCare;
  ImageLayout initial_layout = ImageLayout::kUndefined;
  ImageLayout final_layout = ImageLayout::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), format, samples, load_op,
            store_op, stencil_load_op, stencil_store_op, initial_layout,
            final_layout);
  }
};

struct LayoutColorAttachment;
struct LayoutDepthStencilAttachment;

struct LayoutSubpass : LayoutBase {
  LayoutSubpass() : LayoutBase{LayoutType::kSubpass} {}

  std::weak_ptr<LayoutRenderPass> lrender_pass;
  std::vector<std::shared_ptr<LayoutColorAttachment>> lcolor_attachments;
  std::shared_ptr<LayoutDepthStencilAttachment> ldepth_stencil_attachment;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lrender_pass,
            lcolor_attachments, ldepth_stencil_attachment);
  }
};

struct LayoutColorAttachment : LayoutBase {
  LayoutColorAttachment() : LayoutBase{LayoutType::kColorAttachment} {}

  std::weak_ptr<LayoutSubpass> lsubpass;
  int attachment = -1;
  ImageLayout layout = ImageLayout::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lsubpass, attachment, layout);
  }

  const char* lattachment_id = nullptr;
};

struct LayoutDepthStencilAttachment : LayoutBase {
  LayoutDepthStencilAttachment()
      : LayoutBase{LayoutType::kDepthStencilAttachment} {}

  std::weak_ptr<LayoutSubpass> lsubpass;
  int attachment = -1;
  ImageLayout layout = ImageLayout::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lsubpass, attachment, layout);
  }

  const char* lattachment_id = nullptr;
};

struct LayoutDependency : LayoutBase {
  LayoutDependency() : LayoutBase{LayoutType::kDependency} {}

  std::weak_ptr<LayoutRenderPass> lrender_pass;
  int src_subpass = -1;
  PipelineStageFlags src_stage_mask = PipelineStageFlags::kUndefined;
  AccessFlags src_access_mask = AccessFlags::kUndefined;
  int dst_subpass = -1;
  PipelineStageFlags dst_stage_mask = PipelineStageFlags::kUndefined;
  AccessFlags dst_access_mask = AccessFlags::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lrender_pass, src_subpass,
            src_stage_mask, src_access_mask, dst_subpass, dst_stage_mask,
            dst_access_mask);
  }

  const char* lsrc_subpass_id = nullptr;
  const char* ldst_subpass_id = nullptr;
};

struct LayoutShaderModule : LayoutBase {
  LayoutShaderModule() : LayoutBase{LayoutType::kShaderModule} {}

  std::vector<uint8_t> code;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), code);
  }
};

struct LayoutDescriptorSetLayoutBinding;

struct LayoutDescriptorSetLayout : LayoutBase {
  LayoutDescriptorSetLayout() : LayoutBase{LayoutType::kDescriptorSetLayout} {}

  std::vector<std::shared_ptr<LayoutDescriptorSetLayoutBinding>>
      ldesc_set_layout_bindings;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), ldesc_set_layout_bindings);
  }
};

struct LayoutDescriptorSetLayoutBinding : LayoutBase {
  LayoutDescriptorSetLayoutBinding()
      : LayoutBase{LayoutType::kDescriptorSetLayoutBinding} {}

  int binding = 0;
  DescriptorType desc_type = DescriptorType::kSampler;
  int desc_count = 1;
  ShaderStageFlags stage_flags = ShaderStageFlags::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), binding, desc_type,
            desc_count, stage_flags);
  }
};

struct LayoutPipelineLayout : LayoutBase {
  LayoutPipelineLayout() : LayoutBase{LayoutType::kPipelineLayout} {}

  std::vector<std::shared_ptr<LayoutDescriptorSetLayout>> ldesc_set_layouts;
  std::vector<PushConstantRange> push_constant_ranges = {};

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), ldesc_set_layouts,
            push_constant_ranges);
  }

  std::vector<const char*> ldesc_set_layout_ids;
};

struct LayoutStage;

struct LayoutComputePipeline : LayoutBase {
  LayoutComputePipeline() : LayoutBase{LayoutType::kComputePipeline} {}

  std::shared_ptr<LayoutStage> lstage;
  std::shared_ptr<LayoutPipelineLayout> llayout;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lstage, llayout);
  }

  const char* llayout_id = nullptr;
};

struct LayoutVertexInputState;
struct LayoutInputAssemblyState;
struct LayoutViewportState;
struct LayoutRasterizationState;
struct LayoutMultisampleState;
struct LayoutDepthStencilState;
struct LayoutColorBlendState;
struct LayoutDynamicState;

struct LayoutGraphicsPipeline : LayoutBase {
  LayoutGraphicsPipeline() : LayoutBase{LayoutType::kGraphicsPipeline} {}

  std::vector<std::shared_ptr<LayoutStage>> lstages;
  std::shared_ptr<LayoutVertexInputState> lvertex_input_state;
  std::shared_ptr<LayoutInputAssemblyState> linput_assembly_state;
  std::shared_ptr<LayoutViewportState> lviewport_state;
  std::shared_ptr<LayoutRasterizationState> lrasterization_state;
  std::shared_ptr<LayoutMultisampleState> lmultisample_state;
  std::shared_ptr<LayoutDepthStencilState> ldepth_stencil_state;
  std::shared_ptr<LayoutColorBlendState> lcolor_blend_state;
  std::vector<std::shared_ptr<LayoutDynamicState>> ldynamic_states;
  std::shared_ptr<LayoutPipelineLayout> llayout;
  std::shared_ptr<LayoutRenderPass> lrender_pass;
  int subpass = -1;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lstages, lvertex_input_state,
            linput_assembly_state, lviewport_state, lrasterization_state,
            lmultisample_state, ldepth_stencil_state, lcolor_blend_state,
            ldynamic_states, llayout, lrender_pass, subpass);
  }

  const char* llayout_id = nullptr;
  const char* lrender_pass_id = nullptr;
  const char* lsubpass_id = nullptr;
};

struct LayoutSpecializationInfo;

struct LayoutStage : LayoutBase {
  LayoutStage() : LayoutBase{LayoutType::kStage} {}

  ShaderStageFlags stage = ShaderStageFlags::kUndefined;
  std::shared_ptr<LayoutShaderModule> lshader_module;
  std::string name = "main";
  std::shared_ptr<LayoutSpecializationInfo> lspec_info;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), stage, lshader_module, name,
            lspec_info);
  }

  const char* lshader_module_id = nullptr;
};

struct LayoutSpecializationInfo : LayoutBase {
  LayoutSpecializationInfo() : LayoutBase{LayoutType::kSpecializationInfo} {}

  std::vector<MapEntry> map_entries = {};
  size_t data_size = 0;
  std::shared_ptr<LayoutData> ldata;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), map_entries, data_size,
            ldata);
  }

  const void* data = nullptr;
  const char* ldata_id = nullptr;
};

struct LayoutVertexBindingDescription;
struct LayoutVertexAttributeDescription;

struct LayoutVertexInputState : LayoutBase {
  LayoutVertexInputState() : LayoutBase{LayoutType::kVertexInputState} {}

  std::vector<std::shared_ptr<LayoutVertexBindingDescription>>
      lvertex_binding_descs;
  std::vector<std::shared_ptr<LayoutVertexAttributeDescription>>
      lvertex_attr_descs;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lvertex_binding_descs,
            lvertex_attr_descs);
  }
};

struct LayoutVertexBindingDescription : LayoutBase {
  LayoutVertexBindingDescription()
      : LayoutBase{LayoutType::kVertexBindingDescription} {}

  int binding = 0;
  int stride = 0;
  VertexInputRate input_rate = VertexInputRate::kVertex;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), binding, stride, input_rate);
  }
};

struct LayoutVertexAttributeDescription : LayoutBase {
  LayoutVertexAttributeDescription()
      : LayoutBase{LayoutType::kVertexAttributeDescription} {}

  int location = 0;
  int binding = 0;
  Format format = Format::kUndefined;
  int offset = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), location, binding, format,
            offset);
  }
};

struct LayoutInputAssemblyState : LayoutBase {
  LayoutInputAssemblyState() : LayoutBase{LayoutType::kInputAssemblyState} {}

  PrimitiveTopology topology = PrimitiveTopology::kTriangleList;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), topology);
  }
};

struct LayoutViewport;
struct LayoutScissor;

struct LayoutViewportState : LayoutBase {
  LayoutViewportState() : LayoutBase{LayoutType::kViewportState} {}

  std::vector<std::shared_ptr<LayoutViewport>> lviewports;
  std::vector<std::shared_ptr<LayoutScissor>> lscissors;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lviewports, lscissors);
  }
};

struct LayoutViewport : LayoutBase {
  LayoutViewport() : LayoutBase{LayoutType::kViewport} {}

  Viewport viewport = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), viewport);
  }
};

struct LayoutScissor : LayoutBase {
  LayoutScissor() : LayoutBase{LayoutType::kScissor} {}

  Rect2D rect = {};

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), rect);
  }
};

struct LayoutRasterizationState : LayoutBase {
  LayoutRasterizationState() : LayoutBase{LayoutType::kRasterizationState} {}

  bool depth_clamp_enable = false;
  bool rasterizer_discard_enable = false;
  PolygonMode polygon_mode = PolygonMode::kFill;
  CullMode cull_mode = CullMode::kNone;
  FrontFace front_face = FrontFace::kClockwise;
  bool depth_bias_enable = false;
  float line_width = 1.0f;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), depth_clamp_enable,
            rasterizer_discard_enable, polygon_mode, cull_mode, front_face,
            depth_bias_enable, line_width);
  }
};

struct LayoutMultisampleState : LayoutBase {
  LayoutMultisampleState() : LayoutBase{LayoutType::kMultisampleState} {}

  SampleCountFlags rasterization_samples = SampleCountFlags::k1;
  bool sample_shading_enable = false;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), rasterization_samples,
            sample_shading_enable);
  }
};

struct LayoutDepthStencilState : LayoutBase {
  LayoutDepthStencilState() : LayoutBase{LayoutType::kDepthStencilState} {}

  bool depth_test_enable = false;
  bool depth_write_enable = false;
  CompareOp depth_compare_op = CompareOp::kLessOrEqual;
  bool depth_bounds_test_enable = false;
  bool stencil_test_enable = false;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), depth_test_enable,
            depth_write_enable, depth_compare_op, depth_bounds_test_enable,
            stencil_test_enable);
  }
};

struct LayoutColorBlendAttachmentState;

struct LayoutColorBlendState : LayoutBase {
  LayoutColorBlendState() : LayoutBase{LayoutType::kColorBlendState} {}

  std::vector<LayoutColorBlendAttachmentState> lcolor_blend_attachments;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lcolor_blend_attachments);
  }
};

struct LayoutColorBlendAttachmentState : LayoutBase {
  LayoutColorBlendAttachmentState()
      : LayoutBase{LayoutType::kColorBlendAttachmentState} {}

  bool blend_enable = false;
  ColorComponent color_write_mask = ColorComponent::kR | ColorComponent::kG |
                                    ColorComponent::kB | ColorComponent::kA;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), blend_enable,
            color_write_mask);
  }
};

struct LayoutDynamicState : LayoutBase {
  LayoutDynamicState() : LayoutBase{LayoutType::kDynamicState} {}

  DynamicState state = DynamicState::kViewport;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), state);
  }
};

struct LayoutDescriptorSet;

struct LayoutDescriptorPool : LayoutBase {
  LayoutDescriptorPool() : LayoutBase{LayoutType::kDescriptorPool} {}

  size_t max_sets = 0;
  std::vector<std::pair<DescriptorType, size_t>> pool_sizes;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), max_sets, pool_sizes);
  }
};

struct LayoutDescriptor;

struct LayoutDescriptorSet : LayoutBase {
  LayoutDescriptorSet() : LayoutBase{LayoutType::kDescriptorSet} {}

  std::shared_ptr<LayoutFrame> lframe;
  std::shared_ptr<LayoutDescriptorPool> ldesc_pool;
  std::shared_ptr<LayoutDescriptorSetLayout> lset_layout;
  std::vector<std::shared_ptr<LayoutDescriptor>> ldescriptors;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe, ldesc_pool,
            lset_layout, ldescriptors);
  }

  const char* ldesc_pool_id = nullptr;
  const char* lset_layout_id = nullptr;
};

struct LayoutSampler;

struct LayoutDescriptor : LayoutBase {
  LayoutDescriptor() : LayoutBase{LayoutType::kDescriptor} {}

  int binding = 0;
  DescriptorType desc_type = DescriptorType::kSampler;
  std::shared_ptr<LayoutBuffer> lbuffer;
  std::shared_ptr<LayoutSampler> lsampler;
  std::shared_ptr<LayoutImageView> limage_view;
  ImageLayout image_layout = ImageLayout::kUndefined;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), binding, desc_type, lbuffer,
            lsampler, limage_view, image_layout);
  }

  const char* lbuffer_id = nullptr;
  const char* lsampler_id = nullptr;
  const char* limage_view_id = nullptr;
};

struct LayoutFramebuffer;
struct LayoutSemaphore;

struct LayoutFrame : LayoutBase {
  LayoutFrame() : LayoutBase{LayoutType::kFrame} {}

  std::shared_ptr<LayoutSwapchain> lswapchain;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lswapchain);
  }

  const char* lswapchain_id = nullptr;
  int curr_frame = 0;
};

struct LayoutFramebufferAttachment;

struct LayoutFramebuffer : LayoutBase {
  LayoutFramebuffer() : LayoutBase{LayoutType::kFramebuffer} {}

  std::shared_ptr<LayoutFrame> lframe;
  std::shared_ptr<LayoutRenderPass> lrender_pass;
  int width = 0;
  int height = 0;
  int layers = 1;
  std::vector<LayoutFramebufferAttachment> lattachments;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe, lrender_pass, width,
            height, layers, lattachments);
  }

  const char* lrender_pass_id = nullptr;
  int frame = 0;
};

struct LayoutFramebufferAttachment : LayoutBase {
  LayoutFramebufferAttachment()
      : LayoutBase{LayoutType::kFramebufferAttachment} {}

  std::shared_ptr<LayoutSwapchain> lswapchain;
  std::shared_ptr<LayoutImageView> limage_view;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lswapchain, limage_view);
  }

  const char* lswapchain_id = nullptr;
  const char* limage_view_id = nullptr;
};

struct LayoutSemaphore : LayoutBase {
  LayoutSemaphore() : LayoutBase{LayoutType::kSemaphore} {}

  std::shared_ptr<LayoutFrame> lframe;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe);
  }
};

struct LayoutSampler : LayoutBase {
  LayoutSampler() : LayoutBase{LayoutType::kSampler} {}

  Filter mag_filter = Filter::kNearest;
  Filter min_filter = Filter::kNearest;
  SamplerMipmapMode mipmap_mode = SamplerMipmapMode::kNearest;
  bool anisotropy_enable = false;
  float max_anisotropy = std::numeric_limits<float>::max();

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), mag_filter, min_filter,
            mipmap_mode, anisotropy_enable, max_anisotropy);
  }
};

struct LayoutQueryPool : LayoutBase {
  LayoutQueryPool() : LayoutBase{LayoutType::kQueryPool} {}

  std::shared_ptr<LayoutFrame> lframe;
  QueryType query_type = QueryType::kOcclusion;
  int query_count = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe, query_type,
            query_count);
  }
};

struct LayoutEvent : LayoutBase {
  LayoutEvent() : LayoutBase{LayoutType::kEvent} {}

  std::shared_ptr<LayoutFrame> lframe;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lframe);
  }
};

struct LayoutCamera : LayoutBase {
  LayoutCamera() : LayoutBase{LayoutType::kCamera} {}

  float fovy = 0.0f;
  float aspect = 0.0f;
  float z_near = 0.0f;
  float z_far = 0.0f;
  glm::vec3 eye = glm::vec3(0.0f);
  glm::vec3 center = glm::vec3(0.0f);
  glm::vec3 up = glm::vec3(0.0f);

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), fovy, aspect, z_near, z_far,
            eye, center, up);
  }
};

struct LayoutCommandGroup : LayoutBase {
  LayoutCommandGroup() : LayoutBase{LayoutType::kCommandGroup} {}

  std::vector<std::shared_ptr<LayoutBase>> lcmd_nodes;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lcmd_nodes);
  }

  std::vector<const char*> lcmd_node_ids;
};

struct LayoutCommandList : LayoutBase {
  LayoutCommandList() : LayoutBase{LayoutType::kCommandList} {}

  std::weak_ptr<LayoutCommandGroup> lcmd_group;
  std::vector<std::shared_ptr<LayoutBase>> lcmds;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lcmd_group, lcmds);
  }
};

struct LayoutCommandContext : LayoutBase {
  LayoutCommandContext() : LayoutBase{LayoutType::kCommandContext} {}

  std::shared_ptr<LayoutCommandGroup> lcmd_group;
  std::shared_ptr<LayoutCommandBuffer> lcmd_buffer;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lcmd_group, lcmd_buffer);
  }

  const char* lcmd_group_id = nullptr;
  const char* lcmd_buffer_id = nullptr;
};

struct LayoutFunction : LayoutBase {
  LayoutFunction() : LayoutBase{LayoutType::kFunction} {}

  std::shared_ptr<LayoutData> ldata;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), ldata);
  }

  const char* ldata_id = nullptr;
};

struct LayoutCopyBuffer : LayoutBase {
  LayoutCopyBuffer() : LayoutBase{LayoutType::kCopyBuffer} {}

  std::shared_ptr<LayoutBuffer> lsrc_buffer;
  std::shared_ptr<LayoutBuffer> ldst_buffer;
  std::vector<BufferCopy> regions;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lsrc_buffer, ldst_buffer,
            regions);
  }

  const char* lsrc_buffer_id = nullptr;
  const char* ldst_buffer_id = nullptr;
};

struct LayoutDispatch : LayoutBase {
  LayoutDispatch() : LayoutBase{LayoutType::kDispatch} {}

  int group_count_x = 1;
  int group_count_y = 1;
  int group_count_z = 1;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), group_count_x, group_count_y,
            group_count_z);
  }
};

struct LayoutBeginRenderPass : LayoutBase {
  LayoutBeginRenderPass() : LayoutBase{LayoutType::kBeginRenderPass} {}

  std::shared_ptr<LayoutRenderPass> lrender_pass;
  std::shared_ptr<LayoutFramebuffer> lframebuffer;
  Rect2D rect = {};
  std::vector<std::variant<ClearColorValue, ClearDepthStencilValue>>
      clear_values;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lrender_pass, lframebuffer,
            rect, clear_values);
  }

  const char* lrender_pass_id = nullptr;
  const char* lframebuffer_id = nullptr;
};

struct LayoutEndRenderPass : LayoutBase {
  LayoutEndRenderPass() : LayoutBase{LayoutType::kEndRenderPass} {}

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this));
  }
};

struct LayoutSetViewport : LayoutBase {
  LayoutSetViewport() : LayoutBase{LayoutType::kSetViewport} {}

  int first_viewport = 0;
  std::vector<std::shared_ptr<LayoutViewport>> lviewports;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), first_viewport, lviewports);
  }
};

struct LayoutSetScissor : LayoutBase {
  LayoutSetScissor() : LayoutBase{LayoutType::kSetScissor} {}

  int first_scissor = 0;
  std::vector<std::shared_ptr<LayoutScissor>> lscissors;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), first_scissor, lscissors);
  }
};

struct LayoutDynamicOffset {
  int dynamic_offset = 0;
  int unit = 0;
  int unit_size = 0;
  std::shared_ptr<LayoutBuffer> lbuffer;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(dynamic_offset, unit, unit_size, lbuffer);
  }

  const char* lbuffer_id = nullptr;
};

struct LayoutBindDescriptorSets : LayoutBase {
  LayoutBindDescriptorSets() : LayoutBase{LayoutType::kBindDescriptorSets} {}

  PipelineBindPoint pipeline_bind_point = PipelineBindPoint::kGraphics;
  std::shared_ptr<LayoutPipelineLayout> layout;
  int first_set = 0;
  std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets;
  std::vector<LayoutDynamicOffset> ldynamic_offsets;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), pipeline_bind_point, layout,
            first_set, ldesc_sets, ldynamic_offsets);
  }

  const char* llayout_id = nullptr;
  std::vector<const char*> ldesc_set_ids;
};

struct LayoutBindPipeline : LayoutBase {
  LayoutBindPipeline() : LayoutBase{LayoutType::kBindPipeline} {}

  std::shared_ptr<LayoutBase> lpipeline;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lpipeline);
  }

  const char* lpipeline_id = nullptr;
};

struct LayoutBindVertexBuffers : LayoutBase {
  LayoutBindVertexBuffers() : LayoutBase{LayoutType::kBindVertexBuffers} {}

  int first_binding = 0;
  std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;
  std::vector<size_t> offsets;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), first_binding, lbuffers,
            offsets);
  }

  std::vector<const char*> lbuffer_ids;
};

struct LayoutBindIndexBuffer : LayoutBase {
  LayoutBindIndexBuffer() : LayoutBase{LayoutType::kBindIndexBuffer} {}

  size_t offset = 0;
  std::shared_ptr<LayoutBuffer> lbuffer;
  IndexType index_type = IndexType::kUint16;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), offset, lbuffer, index_type);
  }

  const char* lbuffer_id = nullptr;
};

struct LayoutDrawIndexed : LayoutBase {
  LayoutDrawIndexed() : LayoutBase{LayoutType::kDrawIndexed} {}

  int index_count = 0;
  int instance_count = 1;
  int first_index = 0;
  int vertex_offset = 0;
  int first_instance = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), index_count, instance_count,
            first_index, vertex_offset, first_instance);
  }
};

struct LayoutDrawIndexedIndirect : LayoutBase {
  LayoutDrawIndexedIndirect() : LayoutBase{LayoutType::kDrawIndexedIndirect} {}

  std::shared_ptr<LayoutBuffer> lbuffer;
  size_t offset = 0;
  int unit = 0;
  int unit_size = 0;
  int draw_count = 1;
  int stride = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lbuffer, offset, unit,
            unit_size, draw_count, stride);
  }

  const char* lbuffer_id = nullptr;
};

struct LayoutBufferMemoryBarrier;
struct LayoutImageMemoryBarrier;

struct LayoutPipelineBarrier : LayoutBase {
  LayoutPipelineBarrier() : LayoutBase{LayoutType::kPipelineBarrier} {}

  PipelineStageFlags src_stage_mask = PipelineStageFlags::kUndefined;
  PipelineStageFlags dst_stage_mask = PipelineStageFlags::kUndefined;
  std::vector<std::shared_ptr<LayoutBufferMemoryBarrier>>
      lbuffer_memory_barriers;
  std::vector<std::shared_ptr<LayoutImageMemoryBarrier>> limage_memory_barriers;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), src_stage_mask,
            dst_stage_mask, lbuffer_memory_barriers, limage_memory_barriers);
  }
};

struct LayoutBufferMemoryBarrier : LayoutBase {
  LayoutBufferMemoryBarrier() : LayoutBase{LayoutType::kBufferMemoryBarrier} {}

  AccessFlags src_access_mask = AccessFlags::kUndefined;
  AccessFlags dst_access_mask = AccessFlags::kUndefined;
  std::shared_ptr<LayoutQueue> lsrc_queue;
  std::shared_ptr<LayoutQueue> ldst_queue;
  std::shared_ptr<LayoutBuffer> lbuffer;
  size_t offset = 0;
  size_t size = static_cast<size_t>(-1);

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), src_access_mask,
            dst_access_mask, lsrc_queue, ldst_queue, lbuffer, offset, size);
  }

  const char* lsrc_queue_id = nullptr;
  const char* ldst_queue_id = nullptr;
  const char* lbuffer_id = nullptr;
};

struct LayoutImageMemoryBarrier : LayoutBase {
  LayoutImageMemoryBarrier() : LayoutBase{LayoutType::kImageMemoryBarrier} {}

  std::shared_ptr<LayoutImage> limage;
  std::shared_ptr<LayoutSwapchain> lswapchain;
  std::shared_ptr<LayoutQueue> lsrc_queue;
  std::shared_ptr<LayoutQueue> ldst_queue;
  AccessFlags src_access_mask = AccessFlags::kUndefined;
  AccessFlags dst_access_mask = AccessFlags::kUndefined;
  ImageLayout old_layout = ImageLayout::kUndefined;
  ImageLayout new_layout = ImageLayout::kUndefined;
  ImageSubresourceRange image_subresource_range = {ImageAspectFlags::kColor, 0,
                                                   1, 0, 1};

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), limage, lswapchain,
            lsrc_queue, ldst_queue, src_access_mask, dst_access_mask,
            old_layout, new_layout, image_subresource_range);
  }

  const char* limage_id = nullptr;
  const char* lswapchain_id = nullptr;
  const char* lsrc_queue_id = nullptr;
  const char* ldst_queue_id = nullptr;
};

struct LayoutBlitImage : LayoutBase {
  LayoutBlitImage() : LayoutBase{LayoutType::kBlitImage} {}

  std::shared_ptr<LayoutImage> lsrc_image;
  std::shared_ptr<LayoutSwapchain> lsrc_swapchain;
  std::shared_ptr<LayoutImage> ldst_image;
  std::shared_ptr<LayoutSwapchain> ldst_swapchain;
  ImageLayout src_image_layout = ImageLayout::kUndefined;
  ImageLayout dst_image_layout = ImageLayout::kUndefined;
  std::vector<ImageBlit> regions;
  Filter filter = Filter::kNearest;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lsrc_image, lsrc_swapchain,
            ldst_image, ldst_swapchain, src_image_layout, dst_image_layout,
            regions, filter);
  }

  const char* lsrc_image_id = nullptr;
  const char* lsrc_swapchain_id = nullptr;
  const char* ldst_image_id = nullptr;
  const char* ldst_swapchain_id = nullptr;
};

struct LayoutPushConstants : LayoutBase {
  LayoutPushConstants() : LayoutBase{LayoutType::kPushConstants} {}

  std::shared_ptr<LayoutPipelineLayout> llayout;
  ShaderStageFlags stage_flags = ShaderStageFlags::kUndefined;
  size_t offset = 0;
  size_t size = 0;
  std::shared_ptr<LayoutData> ldata;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), llayout, stage_flags, offset,
            size, ldata);
  }

  const void* values = nullptr;
  const char* llayout_id = nullptr;
  const char* ldata_id = nullptr;
};

struct LayoutResetQueryPool : LayoutBase {
  LayoutResetQueryPool() : LayoutBase{LayoutType::kResetQueryPool} {}

  std::shared_ptr<LayoutQueryPool> lquery_pool;
  int first_query = 0;
  int query_count = 0;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lquery_pool, first_query,
            query_count);
  }

  const char* lquery_pool_id = nullptr;
};

struct LayoutSetEvent : LayoutBase {
  LayoutSetEvent() : LayoutBase{LayoutType::kSetEvent} {}

  std::shared_ptr<LayoutEvent> levent;
  PipelineStageFlags stage_mask = PipelineStageFlags::kAllCommands;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), levent, stage_mask);
  }

  const char* levent_id = nullptr;
};

struct LayoutResetEvent : LayoutBase {
  LayoutResetEvent() : LayoutBase{LayoutType::kResetEvent} {}

  std::shared_ptr<LayoutEvent> levent;
  PipelineStageFlags stage_mask = PipelineStageFlags::kAllCommands;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), levent, stage_mask);
  }

  const char* levent_id = nullptr;
};

struct LayoutAcquireNextImage;
struct LayoutQueueSubmit;
struct LayoutQueuePresent;
struct LayoutResizer;
struct LayoutUpdater;

struct LayoutViewer : LayoutBase {
  LayoutViewer() : LayoutBase{LayoutType::kViewer} {}

  std::shared_ptr<LayoutWindow> lwin;
  std::shared_ptr<LayoutFrame> lframe;
  std::shared_ptr<LayoutCamera> lcamera;
  std::vector<std::shared_ptr<LayoutCommandContext>> lcmd_contexts;
  std::shared_ptr<LayoutAcquireNextImage> lacquire_next_image;
  std::vector<std::shared_ptr<LayoutQueueSubmit>> lqueue_submits;
  std::shared_ptr<LayoutQueuePresent> lqueue_present;
  std::shared_ptr<LayoutResizer> lresizer;
  std::shared_ptr<LayoutUpdater> lupdater;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lwin, lframe, lcamera,
            lcmd_contexts, lacquire_next_image, lqueue_submits, lqueue_present,
            lresizer, lupdater);
  }

  const char* lwin_id = nullptr;
  const char* lframe_id = nullptr;
  const char* lcamera_id = nullptr;
  std::vector<const char*> lcmd_context_ids;
  std::vector<const char*> lqueue_submit_ids;
};

struct LayoutAcquireNextImage : LayoutBase {
  LayoutAcquireNextImage() : LayoutBase{LayoutType::kAcquireNextImage} {}

  std::shared_ptr<LayoutFence> lwait_fence;
  uint64_t timeout = std::numeric_limits<uint64_t>::max();
  std::shared_ptr<LayoutSemaphore> lsemaphore;
  std::shared_ptr<LayoutFence> lfence;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lwait_fence, timeout,
            lsemaphore, lfence);
  }

  const char* lwait_fence_id = nullptr;
  const char* lsemaphore_id = nullptr;
  const char* lfence_id = nullptr;
};

struct LayoutSubmit;

struct LayoutQueueSubmit : LayoutBase {
  LayoutQueueSubmit() : LayoutBase{LayoutType::kQueueSubmit} {}

  std::shared_ptr<LayoutQueue> lqueue;
  std::vector<std::shared_ptr<LayoutSubmit>> lsubmits;
  std::shared_ptr<LayoutFence> lfence;
  bool enabled = true;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lqueue, lsubmits, lfence,
            enabled);
  }

  const char* lqueue_id = nullptr;
  const char* lfence_id = nullptr;
};

struct LayoutSubmit : LayoutBase {
  LayoutSubmit() : LayoutBase{LayoutType::kSubmit} {}

  std::vector<std::shared_ptr<LayoutSemaphore>> lwait_semaphores;
  std::vector<PipelineStageFlags> wait_dst_stage_masks;
  std::vector<std::shared_ptr<LayoutCommandBuffer>> lcmd_buffers;
  std::vector<std::shared_ptr<LayoutSemaphore>> lsignal_semaphores;
  std::vector<int> wait_frame_offsets;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lwait_semaphores,
            wait_dst_stage_masks, lcmd_buffers, lsignal_semaphores,
            wait_frame_offsets);
  }

  std::vector<const char*> lwait_semaphore_ids;
  std::vector<const char*> lcmd_buffer_ids;
  std::vector<const char*> lsignal_semaphore_ids;
};

struct LayoutQueuePresent : LayoutBase {
  LayoutQueuePresent() : LayoutBase{LayoutType::kQueuePresent} {}

  std::shared_ptr<LayoutQueue> lqueue;
  std::vector<std::shared_ptr<LayoutSemaphore>> lwait_semaphores;
  std::vector<std::shared_ptr<LayoutSwapchain>> lswapchains;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lqueue, lwait_semaphores,
            lswapchains);
  }

  const char* lqueue_id = nullptr;
  std::vector<const char*> lwait_semaphore_ids;
  std::vector<const char*> lswapchain_ids;
};

struct LayoutResizer : LayoutBase {
  LayoutResizer() : LayoutBase{LayoutType::kResizer} {}

  std::vector<std::shared_ptr<LayoutImage>> limages;
  std::vector<std::shared_ptr<LayoutImageView>> limage_views;
  std::vector<std::shared_ptr<LayoutGraphicsPipeline>> lgraphics_pipelines;
  std::vector<std::shared_ptr<LayoutFramebuffer>> lframebuffers;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), limages, limage_views,
            lgraphics_pipelines, lframebuffers);
  }

  std::vector<const char*> limage_ids;
  std::vector<const char*> limage_view_ids;
  std::vector<const char*> lgraphics_pipeline_ids;
  std::vector<const char*> lframebuffer_ids;
};

struct LayoutUpdater : LayoutBase {
  LayoutUpdater() : LayoutBase{LayoutType::kUpdater} {}

  std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<LayoutBase>(this), lbuffers);
  }

  std::vector<const char*> lbuffer_ids;
};

struct Layout : std::enable_shared_from_this<Layout> {
  std::shared_ptr<LayoutRenderer> lrenderer;
  std::vector<std::shared_ptr<LayoutWindow>> lwindows;
  std::shared_ptr<LayoutDevice> ldevice;
  std::vector<std::shared_ptr<LayoutQueue>> lqueues;
  std::vector<std::shared_ptr<LayoutCommandPool>> lcmd_pools;
  std::vector<std::shared_ptr<LayoutCommandBuffer>> lcmd_buffers;
  std::vector<std::shared_ptr<LayoutFence>> lfences;
  std::vector<std::shared_ptr<LayoutBuffer>> lbuffers;
  std::vector<std::shared_ptr<LayoutBufferLoader>> lbuffer_loaders;
  std::vector<std::shared_ptr<LayoutImage>> limages;
  std::vector<std::shared_ptr<LayoutImageLoader>> limage_loaders;
  std::vector<std::shared_ptr<LayoutImageView>> limage_views;
  std::vector<std::shared_ptr<LayoutRenderPass>> lrender_passes;
  std::vector<std::shared_ptr<LayoutColorAttachment>> lcolor_attachments;
  std::vector<std::shared_ptr<LayoutDepthStencilAttachment>>
      ldepth_stencil_attachments;
  std::vector<std::shared_ptr<LayoutDependency>> ldependencies;
  std::vector<std::shared_ptr<LayoutShaderModule>> lshader_modules;
  std::vector<std::shared_ptr<LayoutDescriptorSetLayout>> ldesc_set_layouts;
  std::vector<std::shared_ptr<LayoutPipelineLayout>> lpipeline_layouts;
  std::vector<std::shared_ptr<LayoutComputePipeline>> lcompute_pipelines;
  std::vector<std::shared_ptr<LayoutGraphicsPipeline>> lgraphics_pipelines;
  std::vector<std::shared_ptr<LayoutSwapchain>> lswapchains;
  std::vector<std::shared_ptr<LayoutDescriptorPool>> ldesc_pools;
  std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets;
  std::vector<std::shared_ptr<LayoutDescriptor>> ldescriptors;
  std::vector<std::shared_ptr<LayoutFrame>> lframes;
  std::vector<std::shared_ptr<LayoutFramebuffer>> lframebuffers;
  std::vector<std::shared_ptr<LayoutSemaphore>> lsemaphores;
  std::vector<std::shared_ptr<LayoutSampler>> lsamplers;
  std::vector<std::shared_ptr<LayoutQueryPool>> lquery_pools;
  std::vector<std::shared_ptr<LayoutEvent>> levents;
  std::vector<std::shared_ptr<LayoutCamera>> lcameras;
  std::vector<std::shared_ptr<LayoutCommandGroup>> lcmd_groups;
  std::vector<std::shared_ptr<LayoutCommandList>> lcmd_lists;
  std::vector<std::shared_ptr<LayoutCommandContext>> lcmd_contexts;
  std::vector<std::shared_ptr<LayoutFunction>> lfunctions;
  std::vector<std::shared_ptr<LayoutBufferMemoryBarrier>>
      lbuffer_memory_barriers;
  std::vector<std::shared_ptr<LayoutImageMemoryBarrier>> limage_memory_barriers;
  std::vector<std::shared_ptr<LayoutCopyBuffer>> lcopy_buffers;
  std::vector<std::shared_ptr<LayoutBeginRenderPass>> lbegin_render_passes;
  std::vector<std::shared_ptr<LayoutBindDescriptorSets>> lbind_desc_sets;
  std::vector<std::shared_ptr<LayoutBindPipeline>> lbind_pipelines;
  std::vector<std::shared_ptr<LayoutBindVertexBuffers>> lbind_vertex_buffers;
  std::vector<std::shared_ptr<LayoutBindIndexBuffer>> lbind_index_buffers;
  std::vector<std::shared_ptr<LayoutDrawIndexedIndirect>>
      ldraw_indexed_indirects;
  std::vector<std::shared_ptr<LayoutBlitImage>> lblit_images;
  std::vector<std::shared_ptr<LayoutPushConstants>> lpush_constants;
  std::vector<std::shared_ptr<LayoutResetQueryPool>> lreset_query_pools;
  std::vector<std::shared_ptr<LayoutSetEvent>> lset_events;
  std::vector<std::shared_ptr<LayoutResetEvent>> lreset_events;
  std::vector<std::shared_ptr<LayoutViewer>> lviewers;
  std::vector<std::shared_ptr<LayoutQueueSubmit>> lqueue_submits;
  std::vector<std::shared_ptr<LayoutQueuePresent>> lqueue_presents;
  std::vector<std::shared_ptr<LayoutBase>> lnodes;
  std::unordered_map<std::string, std::shared_ptr<LayoutBase>> node_id_map;

  std::shared_ptr<LayoutBase> Find(const std::string& id) const {
    const auto it = node_id_map.find(id);
    if (it != node_id_map.end()) return it->second;
    XG_WARN("cannot find layout: {}", id);
    return nullptr;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(lrenderer);
    archive(lwindows);
    archive(ldevice);
    archive(lqueues);
    archive(lcmd_pools);
    archive(lcmd_buffers);
    archive(lfences);
    archive(lbuffers);
    archive(lbuffer_loaders);
    archive(limages);
    archive(limage_loaders);
    archive(limage_views);
    archive(lrender_passes);
    archive(lcolor_attachments);
    archive(ldepth_stencil_attachments);
    archive(ldependencies);
    archive(lshader_modules);
    archive(ldesc_set_layouts);
    archive(lpipeline_layouts);
    archive(lcompute_pipelines);
    archive(lgraphics_pipelines);
    archive(lswapchains);
    archive(ldesc_pools);
    archive(ldesc_sets);
    archive(ldescriptors);
    archive(lframes);
    archive(lframebuffers);
    archive(lsemaphores);
    archive(lsamplers);
    archive(lquery_pools);
    archive(levents);
    archive(lcameras);
    archive(lcmd_groups);
    archive(lcmd_lists);
    archive(lcmd_contexts);
    archive(lfunctions);
    archive(lbuffer_memory_barriers);
    archive(limage_memory_barriers);
    archive(lcopy_buffers);
    archive(lbegin_render_passes);
    archive(lbind_desc_sets);
    archive(lbind_pipelines);
    archive(lbind_vertex_buffers);
    archive(lbind_index_buffers);
    archive(ldraw_indexed_indirects);

    archive(lblit_images);
    archive(lpush_constants);
    archive(lreset_query_pools);
    archive(lset_events);
    archive(lreset_events);
    archive(lviewers);
    archive(lqueue_submits);
    archive(lqueue_presents);
    archive(lnodes);
    archive(node_id_map);
  }

  bool Serialize(const std::string& filepath);
  static std::shared_ptr<Layout> Deserialize(const std::string& filepath);
  static std::shared_ptr<Layout> Deserialize(const uint8_t* data, size_t length);
};

}  // namespace xg

#endif  // XG_LAYOUT_H_
