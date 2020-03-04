// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_DEVICE_H_
#define XG_DEVICE_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "xg/buffer.h"
#include "xg/descriptor_pool.h"
#include "xg/descriptor_set.h"
#include "xg/descriptor_set_layout.h"
#include "xg/event.h"
#include "xg/fence.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/image_view.h"
#include "xg/layout.h"
#include "xg/pipeline.h"
#include "xg/pipeline_layout.h"
#include "xg/query_pool.h"
#include "xg/queue.h"
#include "xg/render_pass.h"
#include "xg/sampler.h"
#include "xg/semaphore.h"
#include "xg/shader_module.h"
#include "xg/swapchain.h"

#ifdef _WIN32  // workaround build fail bug
#undef CreateEvent
#undef CreateSemaphore
#endif

namespace xg {

class Device {
 public:
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;
  virtual ~Device() = default;

  virtual bool CreateQueues(
      const LayoutDevice& ldevice,
      std::vector<std::shared_ptr<Queue>>* queues) const = 0;
  virtual std::shared_ptr<Fence> CreateFence(
      const LayoutFence& lfence) const = 0;
  virtual std::shared_ptr<Semaphore> CreateSemaphore(
      const LayoutSemaphore& lsemaphore) const = 0;
  virtual std::shared_ptr<Buffer> CreateBuffer(
      const LayoutBuffer& lbuffer) const = 0;
  virtual std::shared_ptr<Image> CreateImage(
      const LayoutImage& limage) const = 0;
  virtual std::shared_ptr<Swapchain> CreateSwapchain(
      LayoutSwapchain* lswapchain) const = 0;
  virtual std::shared_ptr<RenderPass> CreateRenderPass(
      const LayoutRenderPass& lrender_pass) const = 0;
  virtual std::shared_ptr<ShaderModule> CreateShaderModule(
      const LayoutShaderModule& lshader_module) const = 0;
  virtual std::shared_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(
      const LayoutDescriptorSetLayout& ldesc_set_layout) const = 0;
  virtual std::shared_ptr<PipelineLayout> CreatePipelineLayout(
      const LayoutPipelineLayout& lpipeline_layout) const = 0;
  virtual Result InitComputePipelines(
      const std::vector<std::shared_ptr<LayoutComputePipeline>>&
          lcompute_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) const = 0;
  virtual Result InitGraphicsPipelines(
      const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
          lgraphics_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) const = 0;
  virtual std::shared_ptr<ImageView> CreateImageView(
      const LayoutImageView& limage_view) const = 0;
  virtual std::shared_ptr<DescriptorPool> CreateDescriptorPool(
      const LayoutDescriptorPool& ldesc_pool) const = 0;
  virtual std::shared_ptr<Framebuffer> CreateFramebuffer(
      const LayoutFramebuffer& lframebuffer) const = 0;
  virtual std::shared_ptr<Sampler> CreateSampler(
      const LayoutSampler& lsampler) const = 0;
  virtual std::shared_ptr<QueryPool> CreateQueryPool(
      const LayoutQueryPool& lquery_pool) const = 0;
  virtual Result UpdateDescriptorSets(
      const std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets)
      const = 0;
  virtual void WaitIdle() const = 0;
  virtual void ResetFences(
      const std::vector<std::shared_ptr<Fence>>& fences) const = 0;
  virtual Result WaitForFences(
      const std::vector<std::shared_ptr<Fence>>& fences, bool wait_all,
      uint64_t timeout) const = 0;
  virtual std::shared_ptr<Event> CreateEvent(
      const LayoutEvent& levent) const = 0;

  int GetMinUniformBufferOffsetAlignment() const {
    return min_uniform_buffer_offset_align_;
  }

 protected:
  Device() = default;

  int min_uniform_buffer_offset_align_ = 0;
};

}  // namespace xg

#endif  // XG_DEVICE_H_
