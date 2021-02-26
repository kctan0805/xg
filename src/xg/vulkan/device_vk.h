// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_DEVICE_VK_H_
#define XG_VULKAN_DEVICE_VK_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/buffer.h"
#include "xg/descriptor_pool.h"
#include "xg/descriptor_set.h"
#include "xg/descriptor_set_layout.h"
#include "xg/device.h"
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

class DeviceVK : public Device {
 public:
  virtual ~DeviceVK();

  const vk::PhysicalDevice& GetVkPhysicalDevice() const {
    return physical_device_;
  }
  const vk::Device& GetVkDevice() const { return device_; }
  const VmaAllocator& GetVmaAllocator() const { return vma_allocator_; }
  const vk::PipelineCache& GetVkPipelineCache() const {
    return pipeline_cache_;
  }

 protected:
  bool Init(const LayoutDevice& ldevice);
  bool FindPhysicalDevice(const LayoutDevice& ldevice);
  bool CreateDevice(const LayoutDevice& ldevice);
  bool CreatePipelineCache();
  bool CreateMemoryAllocator(const LayoutDevice& ldevice);

  bool CreateQueues(const LayoutDevice& ldevice,
                    std::vector<std::shared_ptr<Queue>>* queues) const override;
  std::shared_ptr<Fence> CreateFence(const LayoutFence& lfence) const override;
  std::shared_ptr<Semaphore> CreateSemaphore(
      const LayoutSemaphore& lsemaphore) const override;
  std::shared_ptr<Buffer> CreateBuffer(
      const LayoutBuffer& lbuffer) const override;
  std::shared_ptr<Image> CreateImage(const LayoutImage& limage) const override;
  std::shared_ptr<Swapchain> CreateSwapchain(
      LayoutSwapchain* lswapchain) const override;
  std::shared_ptr<RenderPass> CreateRenderPass(
      const LayoutRenderPass& lrender_pass) const override;
  std::shared_ptr<ShaderModule> CreateShaderModule(
      const LayoutShaderModule& lshader_module) const override;
  std::shared_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(
      const LayoutDescriptorSetLayout& ldesc_set_layout) const override;
  std::shared_ptr<PipelineLayout> CreatePipelineLayout(
      const LayoutPipelineLayout& lpipeline_layout) const override;
  Result InitComputePipelines(
      const std::vector<std::shared_ptr<LayoutComputePipeline>>&
          lcompute_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) const override;
  Result InitGraphicsPipelines(
      const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
          lgraphics_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) const override;
  std::shared_ptr<ImageView> CreateImageView(
      const LayoutImageView& limage_view) const override;
  std::shared_ptr<DescriptorPool> CreateDescriptorPool(
      const LayoutDescriptorPool& ldesc_pool) const override;
  std::shared_ptr<Framebuffer> CreateFramebuffer(
      const LayoutFramebuffer& lframebuffer) const override;
  std::shared_ptr<QueryPool> CreateQueryPool(
      const LayoutQueryPool& lquery_pool) const override;
  std::shared_ptr<Sampler> CreateSampler(
      const LayoutSampler& lsampler) const override;
  Result UpdateDescriptorSets(
      const std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets)
      const override;
  void WaitIdle() const override;
  void ResetFences(
      const std::vector<std::shared_ptr<Fence>>& fences) const override;
  Result WaitForFences(const std::vector<std::shared_ptr<Fence>>& fences,
                       bool wait_all, uint64_t timeout) const override;
  std::shared_ptr<Event> CreateEvent(const LayoutEvent& levent) const override;

  vk::PhysicalDevice physical_device_;
  bool get_mem_req2_ext_enabled = false;
  bool dedicated_alloc_ext_enabled = false;
  vk::PhysicalDeviceFeatures physical_device_features_;
  vk::Device device_;
  VmaAllocator vma_allocator_ = VK_NULL_HANDLE;
  std::vector<uint32_t> queue_family_indices_;
  vk::PhysicalDeviceLimits physical_device_limits_;
  vk::PipelineCache pipeline_cache_;

  friend class RendererVK;
};

}  // namespace xg

#endif  // XG_VULKAN_DEVICE_VK_H_
