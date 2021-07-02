// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/device_vk.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/renderer.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/buffer_vk.h"
#include "xg/vulkan/descriptor_pool_vk.h"
#include "xg/vulkan/descriptor_set_layout_vk.h"
#include "xg/vulkan/descriptor_set_vk.h"
#include "xg/vulkan/event_vk.h"
#include "xg/vulkan/fence_vk.h"
#include "xg/vulkan/framebuffer_vk.h"
#include "xg/vulkan/image_view_vk.h"
#include "xg/vulkan/image_vk.h"
#include "xg/vulkan/pipeline_layout_vk.h"
#include "xg/vulkan/pipeline_vk.h"
#include "xg/vulkan/query_pool_vk.h"
#include "xg/vulkan/queue_vk.h"
#include "xg/vulkan/render_pass_vk.h"
#include "xg/vulkan/renderer_vk.h"
#include "xg/vulkan/sampler_vk.h"
#include "xg/vulkan/semaphore_vk.h"
#include "xg/vulkan/shader_module_vk.h"
#include "xg/vulkan/swapchain_vk.h"
#include "xg/vulkan/window_vk.h"
#include "xg/window.h"

namespace xg {

DeviceVK::~DeviceVK() {
  if (device_) {
    if (pipeline_cache_) {
      XG_TRACE("destroyPipelineCache: {}",
               (void*)(VkPipelineCache)pipeline_cache_);
      device_.destroyPipelineCache(pipeline_cache_);
    }

    if (vma_allocator_ != VK_NULL_HANDLE) {
      vmaDestroyAllocator(vma_allocator_);
    }
    device_.destroy();
  }
}

bool DeviceVK::Init(const LayoutDevice& ldevice) {
  if (!FindPhysicalDevice(ldevice)) return false;
  if (!CreateDevice(ldevice)) return false;
  if (!CreatePipelineCache()) return false;

  return true;
}

bool DeviceVK::FindPhysicalDevice(const LayoutDevice& ldevice) {
  const auto renderer =
      std::static_pointer_cast<RendererVK>(ldevice.lrenderer->instance);
  const auto& physical_devices =
      renderer->GetVkInstance().enumeratePhysicalDevices();

  for (const auto& phy : physical_devices) {
    auto queue_families = phy.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_families.size(); ++i) {
      const auto& queue_family = queue_families[i];
      if (queue_family.queueCount == 0) continue;

      bool result = true;
      for (const auto& lqueue : ldevice.lqueues) {
        vk::QueueFlags flags =
            static_cast<vk::QueueFlagBits>(lqueue->queue_family);

        if ((queue_family.queueFlags & flags) == flags) {
          if (lqueue->presentable && ldevice.lwindows.size() > 0) {
            bool support = true;
            for (const auto lwin : ldevice.lwindows) {
              const auto win =
                  std::static_pointer_cast<WindowVK>(lwin->instance);
              vk::Bool32 present_support = VK_FALSE;
              phy.getSurfaceSupportKHR(i, win->surface_, &present_support);
              if (present_support == VK_FALSE) {
                support = false;
                break;
              }
            }
            if (!support) {
              result = false;
              break;
            }
          }
        } else {
          result = false;
          break;
        }
      }
      if (result) {
        physical_device_ = phy;
        return true;
      }
    }
  }
  XG_ERROR("cannot find supported physhical device");
  return false;
}

bool DeviceVK::CreateDevice(const LayoutDevice& ldevice) {
  const auto renderer =
      std::static_pointer_cast<RendererVK>(ldevice.lrenderer->instance);
  const auto& queue_families = physical_device_.getQueueFamilyProperties();
  std::vector<uint32_t> queue_used_count(queue_families.size());
  std::unordered_map<uint32_t, std::vector<float>>
      queue_params_map;  // family, priorities
  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

  for (const auto& lqueue : ldevice.lqueues) {
    vk::QueueFlags flags = static_cast<vk::QueueFlagBits>(lqueue->queue_family);

    uint32_t family_candidate = static_cast<uint32_t>(queue_families.size());
    bool queue_over_used = false;
    for (uint32_t i = 0; i < queue_families.size(); ++i) {
      const auto& queue_family = queue_families[i];
      if (queue_family.queueCount == 0) continue;

      if (queue_family.queueFlags == flags) {
        if (family_candidate == queue_families.size()) family_candidate = i;
        if (queue_used_count[i] < queue_families[i].queueCount) {
          break;
        } else {
          queue_over_used = true;
        }
      }
    }

    if (family_candidate == static_cast<uint32_t>(queue_families.size()) ||
        queue_over_used) {
      uint32_t family_candidate2 = static_cast<uint32_t>(queue_families.size());
      bool queue_over_used2 = false;
      for (uint32_t i = 0; i < queue_families.size(); ++i) {
        const auto& queue_family = queue_families[i];
        if (queue_family.queueCount == 0) continue;

        if ((queue_family.queueFlags & flags) == flags) {
          if (family_candidate2 == queue_families.size()) family_candidate2 = i;
          if (queue_used_count[i] < queue_families[i].queueCount) {
            break;
          } else {
            queue_over_used2 = true;
          }
        }
      }

      if ((family_candidate2 != queue_families.size()) &&
          (family_candidate == queue_families.size() ||
           (queue_over_used && !queue_over_used2)))
        family_candidate = family_candidate2;
    }

    if (family_candidate == queue_families.size()) {
      XG_ERROR("cannot find queue family: {}", lqueue->id);
      return false;
    }

    auto it = queue_params_map.find(family_candidate);
    if (it == queue_params_map.end()) {
      auto ret =
          queue_params_map.emplace(family_candidate, std::vector<float>());
      ret.first->second.emplace_back(lqueue->queue_priority);
    } else {
      auto& priorities = it->second;
      if (priorities.size() < queue_families[family_candidate].queueCount) {
        priorities.emplace_back(lqueue->queue_priority);
      }
    }
    ++queue_used_count[family_candidate];
    queue_family_indices_.emplace_back(family_candidate);
  }

  for (const auto& param : queue_params_map) {
    queue_create_infos.emplace_back(
        vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(param.first)
            .setQueueCount(static_cast<uint32_t>(param.second.size()))
            .setPQueuePriorities(param.second.data()));

    XG_TRACE("QueueCreateInfo: {} {}", param.first, param.second.size());
  }

  const auto& found_extensions =
      physical_device_.enumerateDeviceExtensionProperties();
  std::vector<const char*> wanted_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
      VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
      VK_KHR_MULTIVIEW_EXTENSION_NAME};

  if (ldevice.lrenderer->debug) {
    wanted_extensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  }

  std::vector<const char*> extensions;

  for (const auto wanted : wanted_extensions) {
    for (const auto& found : found_extensions) {
      if (std::strcmp(found.extensionName, wanted) == 0) {
        if (std::strcmp(found.extensionName,
                        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0) {
          get_mem_req2_ext_enabled = true;
        } else if (std::strcmp(found.extensionName,
                               VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) ==
                   0) {
          dedicated_alloc_ext_enabled = true;
        }
        extensions.emplace_back(wanted);
        break;
      }
    }
  }

  physical_device_.getFeatures(&physical_device_features_);

  auto features2_index_type_uint8 =
      physical_device_
          .getFeatures2<vk::PhysicalDeviceFeatures2,
                        vk::PhysicalDeviceIndexTypeUint8FeaturesEXT>();

  auto index_type_uint8_features =
      features2_index_type_uint8
          .get<vk::PhysicalDeviceIndexTypeUint8FeaturesEXT>();

#ifdef VK_EXT_shader_atomic_float
  auto features2_atomic_float =
      physical_device_
          .getFeatures2<vk::PhysicalDeviceFeatures2,
                        vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT>();

  auto atomic_float_features =
      features2_atomic_float
          .get<vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT>();

  index_type_uint8_features.setPNext(&atomic_float_features);

#endif  // VK_EXT_shader_atomic_float

  const auto& create_info =
      vk::DeviceCreateInfo()
          .setPNext(&index_type_uint8_features)
          .setQueueCreateInfoCount(
              static_cast<uint32_t>(queue_create_infos.size()))
          .setPQueueCreateInfos(queue_create_infos.data())
          .setEnabledLayerCount(0)
          .setPpEnabledLayerNames(nullptr)
          .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
          .setPpEnabledExtensionNames(extensions.data())
          .setPEnabledFeatures(&physical_device_features_);

  const auto& result =
      physical_device_.createDevice(&create_info, nullptr, &device_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  auto properties = physical_device_.getProperties();
  physical_device_limits_ = properties.limits;
  min_uniform_buffer_offset_align_ =
      static_cast<int>(properties.limits.minUniformBufferOffsetAlignment);

  XG_TRACE("createDevice: {}", (void*)(VkDevice)device_);

  return true;
}

bool DeviceVK::CreatePipelineCache() {
  const auto& create_info = vk::PipelineCacheCreateInfo();
  const auto& result =
      device_.createPipelineCache(&create_info, 0, &pipeline_cache_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  XG_TRACE("createPipelineCache: {}", (void*)(VkPipelineCache)pipeline_cache_);

  return true;
}

bool DeviceVK::CreateQueues(const LayoutDevice& ldevice,
                            std::vector<std::shared_ptr<Queue>>* queues) const {
  const auto& queue_families = physical_device_.getQueueFamilyProperties();
  std::vector<uint32_t> queue_used_count(queue_families.size());
  int i = 0;
  for (const auto& lqueue : ldevice.lqueues) {
    std::shared_ptr<Queue> queue;

    uint32_t queue_family_index = queue_family_indices_[i];
    if (queue_used_count[queue_family_index] <
        queue_families[queue_family_index].queueCount) {
      // create a new queue
      uint32_t queue_index = queue_used_count[queue_family_index];

      queue = std::make_shared<QueueVK>();
      if (!queue) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return false;
      }

      auto queue_vk = std::static_pointer_cast<QueueVK>(queue);

      queue_vk->queue_ = device_.getQueue(queue_family_index, queue_index);
      if (!queue_vk->queue_) {
        XG_ERROR("get queue fail: queue_family_index={}, queue_index={}",
                 queue_family_index, queue_index);
        return false;
      }
      queue_vk->device_ = device_;
      queue_vk->family_index_ = static_cast<int>(queue_family_index);
      queue_vk->queue_index_ = static_cast<int>(queue_index);
      queue_vk->use_count_ = 1;

      XG_TRACE("getQueue: {} {} {}", (void*)(VkQueue)queue_vk->queue_,
               lqueue->id, queue_family_index);

    } else {
      // try to find an appropriate queue to share use
      std::shared_ptr<QueueVK> q_candidate;
      float priority_diff = std::numeric_limits<float>::max();
      for (const auto& q : *queues) {
        const auto& q_vk = std::static_pointer_cast<QueueVK>(q);
        if (static_cast<uint32_t>(q_vk->family_index_) == queue_family_index) {
          if (!q_candidate) {
            q_candidate = q_vk;
            priority_diff =
                std::abs(q_candidate->priority_ - lqueue->queue_priority);
          } else {
            // try to find minimum priority difference and used count
            float diff = std::abs(q_vk->priority_ - lqueue->queue_priority);
            if ((diff < priority_diff) ||
                ((diff == priority_diff) &&
                 q_vk->use_count_ < q_candidate->use_count_)) {
              q_candidate = q_vk;
              priority_diff = diff;
            }
          }
        }
      }
      assert(q_candidate);
      ++q_candidate->use_count_;
      queue = q_candidate;
    }
    ++queue_used_count[queue_family_index];
    ++i;
    queues->emplace_back(queue);
  }
  return true;
}

bool DeviceVK::CreateMemoryAllocator(const LayoutDevice& ldevice) {
  const auto renderer =
      std::static_pointer_cast<RendererVK>(ldevice.lrenderer->instance);
  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.instance = renderer->instance_;
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;

  if (get_mem_req2_ext_enabled && dedicated_alloc_ext_enabled)
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

  VmaVulkanFunctions functions = {};
  functions.vkAllocateMemory = vkAllocateMemory;
  functions.vkBindBufferMemory = vkBindBufferMemory;
  functions.vkBindImageMemory = vkBindImageMemory;
  functions.vkCreateBuffer = vkCreateBuffer;
  functions.vkCreateImage = vkCreateImage;
  functions.vkDestroyBuffer = vkDestroyBuffer;
  functions.vkDestroyImage = vkDestroyImage;
  functions.vkFreeMemory = vkFreeMemory;
  functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
  functions.vkGetBufferMemoryRequirements2KHR =
      renderer->dispatch_loader_dynamic_.vkGetBufferMemoryRequirements2KHR;
  functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
  functions.vkGetImageMemoryRequirements2KHR =
      renderer->dispatch_loader_dynamic_.vkGetImageMemoryRequirements2KHR;
  functions.vkGetPhysicalDeviceMemoryProperties =
      vkGetPhysicalDeviceMemoryProperties;
  functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
  functions.vkMapMemory = vkMapMemory;
  functions.vkUnmapMemory = vkUnmapMemory;
  functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
  functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
  functions.vkCmdCopyBuffer = vkCmdCopyBuffer;

  allocator_info.pVulkanFunctions = &functions;

  auto result = vmaCreateAllocator(&allocator_info, &vma_allocator_);
  if (result != VK_SUCCESS) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  return true;
}

std::shared_ptr<Fence> DeviceVK::CreateFence(const LayoutFence& lfence) const {
  auto fence = std::make_shared<FenceVK>();
  if (!fence) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  vk::FenceCreateInfo create_info;
  if (lfence.signaled) create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

  const auto& result =
      device_.createFence(&create_info, nullptr, &fence->fence_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  fence->device_ = device_;

  XG_TRACE("createFence: {} {} {}", (void*)(VkFence)fence->fence_, lfence.id,
           lfence.signaled);

  return fence;
}

std::shared_ptr<Semaphore> DeviceVK::CreateSemaphore(
    const LayoutSemaphore& lsemaphore) const {
  auto semaphore = std::make_shared<SemaphoreVK>();
  if (!semaphore) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const vk::SemaphoreCreateInfo create_info;

  const auto& result =
      device_.createSemaphore(&create_info, nullptr, &semaphore->semaphore_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  semaphore->device_ = device_;

  XG_TRACE("createSemaphore: {} {}", (void*)(VkSemaphore)semaphore->semaphore_,
           lsemaphore.id);

  return semaphore;
}

std::shared_ptr<Buffer> DeviceVK::CreateBuffer(
    const LayoutBuffer& lbuffer) const {
  auto buffer = std::make_shared<BufferVK>();
  if (!buffer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  buffer->vma_allocator_ = vma_allocator_;
  buffer->min_uniform_buffer_offset_align_ = min_uniform_buffer_offset_align_;

  auto result = buffer->Init(lbuffer);
  if (result != Result::kSuccess) return nullptr;

  return buffer;
}

std::shared_ptr<Image> DeviceVK::CreateImage(const LayoutImage& limage) const {
  auto image = std::make_shared<ImageVK>();
  if (!image) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  image->vma_allocator_ = vma_allocator_;

  auto result = image->Init(limage);
  if (result != Result::kSuccess) return nullptr;

  return image;
}

std::shared_ptr<Swapchain> DeviceVK::CreateSwapchain(
    LayoutSwapchain* lswapchain) const {
  auto swapchain = std::make_shared<SwapchainVK>();
  if (!swapchain) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  auto win = std::static_pointer_cast<WindowVK>(lswapchain->lwin->instance);

  const auto& surface_capabilities =
      physical_device_.getSurfaceCapabilitiesKHR(win->surface_);
  uint32_t min_image_count = static_cast<uint32_t>(lswapchain->min_image_count);

  if (min_image_count < surface_capabilities.minImageCount) {
    min_image_count = surface_capabilities.minImageCount;
  } else if ((surface_capabilities.maxImageCount > 0) &&
             (min_image_count > surface_capabilities.maxImageCount)) {
    min_image_count = surface_capabilities.maxImageCount;
  }
  lswapchain->min_image_count = static_cast<int>(min_image_count);

  const auto& found_surface_formats =
      physical_device_.getSurfaceFormatsKHR(win->surface_);

  auto surface_format = vk::Format::eR8G8B8A8Unorm;
  auto surface_colorspace = vk::ColorSpaceKHR::eSrgbNonlinear;

  if (found_surface_formats.size() > 1 ||
      found_surface_formats[0].format != vk::Format::eUndefined) {
    surface_format = found_surface_formats[0].format;
    surface_colorspace = found_surface_formats[0].colorSpace;
    for (const auto& format : found_surface_formats) {
      if (format.format == static_cast<vk::Format>(lswapchain->image_format) &&
          format.colorSpace ==
              static_cast<vk::ColorSpaceKHR>(lswapchain->image_color_space)) {
        surface_format = format.format;
        surface_colorspace = format.colorSpace;
        break;
      }
    }
  }
  lswapchain->image_format = static_cast<Format>(surface_format);
  lswapchain->image_color_space = static_cast<ColorSpace>(surface_colorspace);

  if (surface_capabilities.currentExtent.width == UINT32_MAX) {
    int width = lswapchain->width;
    int height = lswapchain->height;
    if (!width || !height) win->GetDrawableSize(&width, &height);

    width = std::min(
        std::max(width,
                 static_cast<int>(surface_capabilities.minImageExtent.width)),
        static_cast<int>(surface_capabilities.maxImageExtent.width));
    height = std::min(
        std::max(height,
                 static_cast<int>(surface_capabilities.minImageExtent.height)),
        static_cast<int>(surface_capabilities.maxImageExtent.height));

    lswapchain->width = width;
    lswapchain->height = height;
  } else {
    lswapchain->width =
        static_cast<int>(surface_capabilities.currentExtent.width);
    lswapchain->height =
        static_cast<int>(surface_capabilities.currentExtent.height);
  }

  vk::SurfaceTransformFlagBitsKHR pre_transform =
      surface_capabilities.currentTransform;

  if (surface_capabilities.supportedTransforms &
      static_cast<vk::SurfaceTransformFlagBitsKHR>(lswapchain->pre_transform)) {
    pre_transform =
        static_cast<vk::SurfaceTransformFlagBitsKHR>(lswapchain->pre_transform);
  }
  lswapchain->pre_transform = static_cast<SurfaceTransformFlags>(pre_transform);

  if (surface_capabilities.supportedCompositeAlpha &
      vk::CompositeAlphaFlagBitsKHR::eInherit) {
    lswapchain->composite_alpha = CompositeAlpha::kInherit;
  }

  const auto& found_present_modes =
      physical_device_.getSurfacePresentModesKHR(win->surface_);
  auto present_mode = vk::PresentModeKHR::eImmediate;
  if (found_present_modes.size() > 0) present_mode = found_present_modes[0];

  for (const auto& mode : found_present_modes) {
    if (mode == static_cast<vk::PresentModeKHR>(lswapchain->present_mode)) {
      present_mode = mode;
      break;
    }
  }
  lswapchain->present_mode = static_cast<PresentMode>(present_mode);

  swapchain->device_ = device_;

  auto result = swapchain->Init(*lswapchain);
  if (result != Result::kSuccess) return nullptr;

  return swapchain;
}

std::shared_ptr<RenderPass> DeviceVK::CreateRenderPass(
    const LayoutRenderPass& lrender_pass) const {
  auto render_pass = std::make_shared<RenderPassVK>();
  if (!render_pass) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  render_pass->device_ = device_;

  auto result = render_pass->Init(lrender_pass);
  if (result != Result::kSuccess) return nullptr;

  return render_pass;
}

std::shared_ptr<ShaderModule> DeviceVK::CreateShaderModule(
    const LayoutShaderModule& lshader_module) const {
  auto shader_module = std::make_shared<ShaderModuleVK>();
  if (!shader_module) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  const auto& create_info = vk::ShaderModuleCreateInfo()
                                .setCodeSize(lshader_module.code.size())
                                .setPCode(reinterpret_cast<const uint32_t*>(
                                    lshader_module.code.data()));

  const auto& result = device_.createShaderModule(
      &create_info, nullptr, &shader_module->shader_module_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  shader_module->device_ = device_;

  XG_TRACE("createShaderModule: {} {} {}",
           (void*)(VkShaderModule)shader_module->shader_module_,
           lshader_module.id, lshader_module.code.size());

  return shader_module;
}

std::shared_ptr<DescriptorSetLayout> DeviceVK::CreateDescriptorSetLayout(
    const LayoutDescriptorSetLayout& ldesc_set_layout) const {
  auto desc_set_layout = std::make_shared<DescriptorSetLayoutVK>();
  if (!desc_set_layout) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  std::vector<vk::DescriptorSetLayoutBinding> desc_set_layout_bindings;

  XG_TRACE("createDescriptorSetLayout: {}", ldesc_set_layout.id);

  for (const auto& lbinding : ldesc_set_layout.ldesc_set_layout_bindings) {
    const auto desc_type = static_cast<vk::DescriptorType>(lbinding->desc_type);
    const auto stage_flags =
        static_cast<vk::ShaderStageFlagBits>(lbinding->stage_flags);

    desc_set_layout_bindings.emplace_back(
        vk::DescriptorSetLayoutBinding()
            .setBinding(static_cast<uint32_t>(lbinding->binding))
            .setDescriptorType(desc_type)
            .setDescriptorCount(static_cast<uint32_t>(lbinding->desc_count))
            .setStageFlags(stage_flags));

    XG_TRACE("  Binding: {} {} {} {} {}", lbinding->id, lbinding->binding,
             vk::to_string(desc_type), lbinding->desc_count,
             vk::to_string(stage_flags));
  }

  const auto& create_info = vk::DescriptorSetLayoutCreateInfo()
                                .setBindingCount(static_cast<uint32_t>(
                                    desc_set_layout_bindings.size()))
                                .setPBindings(desc_set_layout_bindings.data());

  const auto& result = device_.createDescriptorSetLayout(
      &create_info, nullptr, &desc_set_layout->desc_set_layout_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  desc_set_layout->device_ = device_;

  XG_TRACE("  SetLayout: {} {}",
           (void*)(VkDescriptorSetLayout)desc_set_layout->desc_set_layout_,
           ldesc_set_layout.id);

  return desc_set_layout;
}

std::shared_ptr<PipelineLayout> DeviceVK::CreatePipelineLayout(
    const LayoutPipelineLayout& lpipeline_layout) const {
  auto pipeline_layout = std::make_shared<PipelineLayoutVK>();
  if (!pipeline_layout) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  std::vector<vk::DescriptorSetLayout> desc_set_layouts;
  std::vector<vk::PushConstantRange> push_constant_ranges;

  XG_TRACE("createPipelineLayout: {}", lpipeline_layout.id);

  for (const auto& llayout : lpipeline_layout.ldesc_set_layouts) {
    const auto& layout =
        std::static_pointer_cast<DescriptorSetLayoutVK>(llayout->instance);
    desc_set_layouts.emplace_back(layout->desc_set_layout_);

    XG_TRACE("  SetLayout: {} {}",
             (void*)(VkDescriptorSetLayout)layout->desc_set_layout_,
             llayout->id);
  }

  for (const auto& range : lpipeline_layout.push_constant_ranges) {
    const auto stage_flags =
        static_cast<vk::ShaderStageFlagBits>(range.stage_flags);

    push_constant_ranges.emplace_back(
        vk::PushConstantRange()
            .setStageFlags(stage_flags)
            .setOffset(static_cast<uint32_t>(range.offset))
            .setSize(static_cast<uint32_t>(range.size)));

    XG_TRACE("  PushConstantRange: {} {} {}", vk::to_string(stage_flags),
             range.offset, range.size);
  }

  const auto& create_info =
      vk::PipelineLayoutCreateInfo()
          .setSetLayoutCount(static_cast<uint32_t>(desc_set_layouts.size()))
          .setPSetLayouts(desc_set_layouts.data())
          .setPushConstantRangeCount(
              static_cast<uint32_t>(push_constant_ranges.size()))
          .setPPushConstantRanges(push_constant_ranges.data());

  const auto& result = device_.createPipelineLayout(
      &create_info, nullptr, &pipeline_layout->pipeline_layout_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  pipeline_layout->device_ = device_;

  XG_TRACE("  PipelineLayout: {} {}",
           (void*)(VkPipelineLayout)pipeline_layout->pipeline_layout_,
           lpipeline_layout.id);

  return pipeline_layout;
}

Result DeviceVK::InitComputePipelines(
    const std::vector<std::shared_ptr<LayoutComputePipeline>>&
        lcompute_pipelines,
    std::vector<std::shared_ptr<Pipeline>>* pipelines) const {
  assert(pipelines);
  assert(lcompute_pipelines.size() == pipelines->size());

  std::vector<vk::ComputePipelineCreateInfo> create_infos;
  std::vector<std::shared_ptr<vk::SpecializationInfo>> spec_infos;
  std::vector<std::shared_ptr<std::vector<vk::SpecializationMapEntry>>>
      spec_map_entries;
  std::vector<vk::Pipeline> vk_pipelines(lcompute_pipelines.size());

  XG_TRACE("createComputePipelines: {}",
           (void*)(VkPipelineCache)pipeline_cache_);

  for (const auto& lcompute_pipeline : lcompute_pipelines) {
    // stage
    assert(lcompute_pipeline->lstage);
    const auto& lstage = lcompute_pipeline->lstage;
    const auto stage = static_cast<vk::ShaderStageFlagBits>(lstage->stage);
    auto shader_module = std::static_pointer_cast<ShaderModuleVK>(
        lstage->lshader_module->instance);
    auto shader_stage_create_info =
        vk::PipelineShaderStageCreateInfo()
            .setStage(stage)
            .setModule(shader_module->shader_module_)
            .setPName(lstage->name.c_str());

    XG_TRACE("  Stage: {} {} {}", vk::to_string(stage),
             (void*)(VkShaderModule)shader_module->shader_module_,
             lstage->name);

    if (lstage->lspec_info) {
      const auto& lspec_info = lstage->lspec_info;
      auto vk_map_entries =
          std::make_shared<std::vector<vk::SpecializationMapEntry>>();
      if (!vk_map_entries) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return Result::kErrorOutOfHostMemory;
      }
      vk_map_entries->reserve(lspec_info->map_entries.size());

      XG_TRACE("    SpecializationInfo: {}", lspec_info->data_size);

      for (const auto& map_entry : lspec_info->map_entries) {
        vk_map_entries->emplace_back(
            vk::SpecializationMapEntry()
                .setConstantID(static_cast<uint32_t>(map_entry.constant_id))
                .setOffset(static_cast<uint32_t>(map_entry.offset))
                .setSize(map_entry.size));

        XG_TRACE("      MapEntry: {} {} {}", map_entry.constant_id,
                 map_entry.offset, map_entry.size);
      }

      auto spec_info = std::make_shared<vk::SpecializationInfo>();
      if (!spec_info) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return Result::kErrorOutOfHostMemory;
      }

      spec_info->setMapEntryCount(
          static_cast<uint32_t>(vk_map_entries->size()));
      spec_info->setPMapEntries(vk_map_entries->data());
      spec_info->setDataSize(lspec_info->data_size);
      spec_info->setPData(lspec_info->data);

      shader_stage_create_info.setPSpecializationInfo(spec_info.get());

      spec_map_entries.emplace_back(vk_map_entries);
      spec_infos.emplace_back(spec_info);
    }

    // pipeline layout
    const auto& lpipeline_layout = lcompute_pipeline->llayout;
    assert(lpipeline_layout);
    const auto& pipeline_layout =
        std::static_pointer_cast<PipelineLayoutVK>(lpipeline_layout->instance);

    XG_TRACE("  Layout: {} {}",
             (void*)(VkPipelineLayout)pipeline_layout->pipeline_layout_,
             lpipeline_layout->id);

    auto create_info = vk::ComputePipelineCreateInfo()
                           .setStage(shader_stage_create_info)
                           .setLayout(pipeline_layout->pipeline_layout_);
    create_infos.emplace_back(create_info);
  }

  const auto& result = device_.createComputePipelines(
      pipeline_cache_, static_cast<uint32_t>(create_infos.size()),
      create_infos.data(), nullptr, vk_pipelines.data());
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  for (int i = 0; i < vk_pipelines.size(); ++i) {
    auto pipeline_vk = static_cast<PipelineVK*>((*pipelines)[i].get());
    auto vk_pipeline = vk_pipelines[i];

    pipeline_vk->bind_point = PipelineBindPoint::kCompute;
    pipeline_vk->device_ = device_;
    pipeline_vk->pipeline_ = vk_pipeline;

    XG_TRACE("  Pipeline: {} {}", (void*)(VkPipeline)vk_pipeline,
             lcompute_pipelines[i]->id);
  }
  return Result::kSuccess;
}

Result DeviceVK::InitGraphicsPipelines(
    const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
        lgraphics_pipelines,
    std::vector<std::shared_ptr<Pipeline>>* pipelines) const {
  assert(pipelines);
  assert(lgraphics_pipelines.size() == pipelines->size());

  std::vector<std::shared_ptr<std::vector<vk::PipelineShaderStageCreateInfo>>>
      pipeline_shader_stage_create_infos;
  std::vector<std::shared_ptr<vk::SpecializationInfo>> spec_infos;
  std::vector<std::shared_ptr<std::vector<vk::SpecializationMapEntry>>>
      spec_map_entries;
  std::vector<std::shared_ptr<vk::PipelineVertexInputStateCreateInfo>>
      pipeline_vertex_input_state_create_infos;
  std::vector<std::shared_ptr<std::vector<vk::VertexInputBindingDescription>>>
      vertex_input_binding_descs;
  std::vector<std::shared_ptr<std::vector<vk::VertexInputAttributeDescription>>>
      vertex_input_attr_descs;
  std::vector<std::shared_ptr<vk::PipelineInputAssemblyStateCreateInfo>>
      pipeline_input_assembly_state_create_infos;
  std::vector<std::shared_ptr<vk::PipelineViewportStateCreateInfo>>
      pipeline_viewport_state_create_infos;
  std::vector<std::shared_ptr<std::vector<vk::Viewport>>>
      viewport_state_viewports;
  std::vector<std::shared_ptr<std::vector<vk::Rect2D>>> viewport_state_scissors;
  std::vector<std::shared_ptr<vk::PipelineRasterizationStateCreateInfo>>
      pipeline_rasterization_state_create_infos;
  std::vector<std::shared_ptr<vk::PipelineMultisampleStateCreateInfo>>
      pipeline_multisample_state_create_infos;
  std::vector<std::shared_ptr<vk::PipelineDepthStencilStateCreateInfo>>
      pipeline_depth_stencil_state_create_infos;
  std::vector<std::shared_ptr<vk::PipelineColorBlendStateCreateInfo>>
      pipeline_color_blend_state_create_infos;
  std::vector<
      std::shared_ptr<std::vector<vk::PipelineColorBlendAttachmentState>>>
      pipeline_color_blend_attachment_states;
  std::vector<std::shared_ptr<vk::PipelineDynamicStateCreateInfo>>
      pipeline_dynamic_state_create_infos;
  std::vector<std::shared_ptr<std::vector<vk::DynamicState>>>
      pipeline_dynamic_states;
  std::vector<vk::GraphicsPipelineCreateInfo> create_infos;
  std::vector<vk::Pipeline> vk_pipelines(lgraphics_pipelines.size());

  XG_TRACE("createGraphicsPipelines: {}",
           (void*)(VkPipelineCache)pipeline_cache_);

  for (const auto& lgraphics_pipeline : lgraphics_pipelines) {
    auto shader_stage_create_infos =
        std::make_shared<std::vector<vk::PipelineShaderStageCreateInfo>>();
    if (!shader_stage_create_infos) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }

    // stages
    for (const auto& lstage : lgraphics_pipeline->lstages) {
      const auto stage = static_cast<vk::ShaderStageFlagBits>(lstage->stage);
      auto shader_module = std::static_pointer_cast<ShaderModuleVK>(
          lstage->lshader_module->instance);
      auto shader_stage_create_info =
          vk::PipelineShaderStageCreateInfo()
              .setStage(stage)
              .setModule(shader_module->shader_module_)
              .setPName(lstage->name.c_str());

      XG_TRACE("  Stage: {} {} {}", vk::to_string(stage),
               (void*)(VkShaderModule)shader_module->shader_module_,
               lstage->name);

      if (lstage->lspec_info) {
        const auto& lspec_info = lstage->lspec_info;
        auto vk_map_entries =
            std::make_shared<std::vector<vk::SpecializationMapEntry>>();
        if (!vk_map_entries) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return Result::kErrorOutOfHostMemory;
        }
        vk_map_entries->reserve(lspec_info->map_entries.size());

        XG_TRACE("    SpecializationInfo: {}", lspec_info->data_size);

        for (const auto& map_entry : lspec_info->map_entries) {
          vk_map_entries->emplace_back(
              vk::SpecializationMapEntry()
                  .setConstantID(static_cast<uint32_t>(map_entry.constant_id))
                  .setOffset(static_cast<uint32_t>(map_entry.offset))
                  .setSize(map_entry.size));

          XG_TRACE("      MapEntry: {} {} {}", map_entry.constant_id,
                   map_entry.offset, map_entry.size);
        }

        auto spec_info = std::make_shared<vk::SpecializationInfo>();
        if (!spec_info) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return Result::kErrorOutOfHostMemory;
        }

        spec_info->setMapEntryCount(
            static_cast<uint32_t>(vk_map_entries->size()));
        spec_info->setPMapEntries(vk_map_entries->data());
        spec_info->setDataSize(lspec_info->data_size);
        spec_info->setPData(lspec_info->data);

        shader_stage_create_info.setPSpecializationInfo(spec_info.get());

        spec_map_entries.emplace_back(vk_map_entries);
        spec_infos.emplace_back(spec_info);
      }
      shader_stage_create_infos->emplace_back(shader_stage_create_info);
    }
    pipeline_shader_stage_create_infos.emplace_back(shader_stage_create_infos);

    // vertex input state
    auto vertex_input_bindings =
        std::make_shared<std::vector<vk::VertexInputBindingDescription>>();
    if (!vertex_input_bindings) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    const auto& lvertex_input_state = lgraphics_pipeline->lvertex_input_state;
    assert(lvertex_input_state);
    for (const auto& lvertex_binding :
         lvertex_input_state->lvertex_binding_descs) {
      const auto input_rate =
          static_cast<vk::VertexInputRate>(lvertex_binding->input_rate);
      vertex_input_bindings->emplace_back(
          vk::VertexInputBindingDescription()
              .setBinding(static_cast<uint32_t>(lvertex_binding->binding))
              .setStride(static_cast<uint32_t>(lvertex_binding->stride))
              .setInputRate(input_rate));

      XG_TRACE("  VertexInputState: VertexBindingDescription: {} {} {}",
               lvertex_binding->binding, lvertex_binding->stride,
               vk::to_string(input_rate));
    }
    vertex_input_binding_descs.emplace_back(vertex_input_bindings);

    auto vertex_input_attrs =
        std::make_shared<std::vector<vk::VertexInputAttributeDescription>>();
    if (!vertex_input_attrs) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    for (const auto& lvertex_attr : lvertex_input_state->lvertex_attr_descs) {
      const auto format = static_cast<vk::Format>(lvertex_attr->format);
      vertex_input_attrs->emplace_back(
          vk::VertexInputAttributeDescription()
              .setLocation(static_cast<uint32_t>(lvertex_attr->location))
              .setBinding(static_cast<uint32_t>(lvertex_attr->binding))
              .setFormat(format)
              .setOffset(static_cast<uint32_t>(lvertex_attr->offset)));

      XG_TRACE("  VertexInputState: VertexAttributeDescription: {} {} {} {}",
               lvertex_attr->location, lvertex_attr->binding,
               vk::to_string(format), lvertex_attr->offset);
    }
    vertex_input_attr_descs.emplace_back(vertex_input_attrs);

    auto vertex_input_state_create_info =
        std::make_shared<vk::PipelineVertexInputStateCreateInfo>();
    if (!vertex_input_state_create_info) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    vertex_input_state_create_info->setVertexBindingDescriptionCount(
        static_cast<uint32_t>(vertex_input_bindings->size()));
    vertex_input_state_create_info->setPVertexBindingDescriptions(
        vertex_input_bindings->data());
    vertex_input_state_create_info->setVertexAttributeDescriptionCount(
        static_cast<uint32_t>(vertex_input_attrs->size()));
    vertex_input_state_create_info->setPVertexAttributeDescriptions(
        vertex_input_attrs->data());

    pipeline_vertex_input_state_create_infos.emplace_back(
        vertex_input_state_create_info);

    // input assembly state
    auto input_assembly_state_create_info =
        std::make_shared<vk::PipelineInputAssemblyStateCreateInfo>();
    if (!input_assembly_state_create_info) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    const auto& linput_assembly_state =
        lgraphics_pipeline->linput_assembly_state;
    assert(linput_assembly_state);
    const auto topology =
        static_cast<vk::PrimitiveTopology>(linput_assembly_state->topology);
    input_assembly_state_create_info->setTopology(topology);

    pipeline_input_assembly_state_create_infos.emplace_back(
        input_assembly_state_create_info);

    XG_TRACE("  InputAssemblyState: {}", vk::to_string(topology));

    // viewport state
    auto viewports = std::make_shared<std::vector<vk::Viewport>>();
    if (!viewports) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    const auto& lviewport_state = lgraphics_pipeline->lviewport_state;
    assert(lviewport_state);
    for (const auto& lviewport : lviewport_state->lviewports) {
      viewports->emplace_back(vk::Viewport()
                                  .setX(lviewport->viewport.x)
                                  .setY(lviewport->viewport.y)
                                  .setWidth(lviewport->viewport.width)
                                  .setHeight(lviewport->viewport.height)
                                  .setMinDepth(lviewport->viewport.min_depth)
                                  .setMaxDepth(lviewport->viewport.max_depth));

      XG_TRACE("  ViewportState: Viewport: {} {} {} {} {} {}",
               lviewport->viewport.x, lviewport->viewport.y,
               lviewport->viewport.width, lviewport->viewport.height,
               lviewport->viewport.min_depth, lviewport->viewport.max_depth);
    }
    viewport_state_viewports.emplace_back(viewports);

    auto scissors = std::make_shared<std::vector<vk::Rect2D>>();
    if (!scissors) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    for (const auto& lscissor : lviewport_state->lscissors) {
      scissors->emplace_back(
          vk::Rect2D()
              .setOffset(vk::Offset2D()
                             .setX(static_cast<int32_t>(lscissor->x))
                             .setY(static_cast<int32_t>(lscissor->y)))
              .setExtent(
                  vk::Extent2D()
                      .setWidth(static_cast<uint32_t>(lscissor->width))
                      .setHeight(static_cast<uint32_t>(lscissor->height))));

      XG_TRACE("  ViewportState: Scissor: {} {} {} {}", lscissor->x,
               lscissor->y, lscissor->width, lscissor->height);
    }
    viewport_state_scissors.emplace_back(scissors);

    auto viewport_state_create_info =
        std::make_shared<vk::PipelineViewportStateCreateInfo>();
    if (!viewport_state_create_info) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    viewport_state_create_info->setViewportCount(
        static_cast<uint32_t>(viewports->size()));
    viewport_state_create_info->setPViewports(viewports->data());
    viewport_state_create_info->setScissorCount(
        static_cast<uint32_t>(scissors->size()));
    viewport_state_create_info->setPScissors(scissors->data());

    pipeline_viewport_state_create_infos.emplace_back(
        viewport_state_create_info);

    // rasterization state
    auto rasterization_state_create_info =
        std::make_shared<vk::PipelineRasterizationStateCreateInfo>();
    if (!rasterization_state_create_info) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }
    const auto& lrasterization_state = lgraphics_pipeline->lrasterization_state;
    assert(lrasterization_state);
    const auto polygon_mode =
        static_cast<vk::PolygonMode>(lrasterization_state->polygon_mode);
    const auto cull_mode =
        static_cast<vk::CullModeFlagBits>(lrasterization_state->cull_mode);
    const auto front_face =
        static_cast<vk::FrontFace>(lrasterization_state->front_face);
    rasterization_state_create_info->setDepthClampEnable(
        lrasterization_state->depth_clamp_enable ? VK_TRUE : VK_FALSE);
    rasterization_state_create_info->setRasterizerDiscardEnable(
        lrasterization_state->rasterizer_discard_enable ? VK_TRUE : VK_FALSE);
    rasterization_state_create_info->setPolygonMode(polygon_mode);
    rasterization_state_create_info->setCullMode(cull_mode);
    rasterization_state_create_info->setFrontFace(front_face);
    rasterization_state_create_info->setDepthBiasEnable(
        lrasterization_state->depth_bias_enable ? VK_TRUE : VK_FALSE);
    rasterization_state_create_info->setLineWidth(
        lrasterization_state->line_width);

    pipeline_rasterization_state_create_infos.emplace_back(
        rasterization_state_create_info);

    XG_TRACE("  RasterizationState: {} {} {} {} {} {} {}",
             lrasterization_state->depth_clamp_enable,
             lrasterization_state->rasterizer_discard_enable,
             vk::to_string(polygon_mode), vk::to_string(cull_mode),
             vk::to_string(front_face), lrasterization_state->depth_bias_enable,
             lrasterization_state->line_width);

    // multisample state
    auto multisample_state_create_info =
        std::make_shared<vk::PipelineMultisampleStateCreateInfo>();
    if (!multisample_state_create_info) return Result::kErrorOutOfHostMemory;

    const auto& lmultisample_state = lgraphics_pipeline->lmultisample_state;
    assert(lmultisample_state);
    const auto rasterization_samples = static_cast<vk::SampleCountFlagBits>(
        lmultisample_state->rasterization_samples);
    multisample_state_create_info->setRasterizationSamples(
        rasterization_samples);
    multisample_state_create_info->setSampleShadingEnable(
        lmultisample_state->sample_shading_enable ? VK_TRUE : VK_FALSE);

    pipeline_multisample_state_create_infos.emplace_back(
        multisample_state_create_info);

    XG_TRACE("  MultisampleState: {} {}", vk::to_string(rasterization_samples),
             lmultisample_state->sample_shading_enable);

    // depth stencil state
    auto depth_stencil_state_create_info =
        std::make_shared<vk::PipelineDepthStencilStateCreateInfo>();
    if (!depth_stencil_state_create_info) return Result::kErrorOutOfHostMemory;

    const auto& ldepth_stencil_state = lgraphics_pipeline->ldepth_stencil_state;
    assert(ldepth_stencil_state);
    const auto depth_compare_op =
        static_cast<vk::CompareOp>(ldepth_stencil_state->depth_compare_op);
    depth_stencil_state_create_info->setDepthTestEnable(
        ldepth_stencil_state->depth_test_enable ? VK_TRUE : VK_FALSE);
    depth_stencil_state_create_info->setDepthWriteEnable(
        ldepth_stencil_state->depth_write_enable ? VK_TRUE : VK_FALSE);
    depth_stencil_state_create_info->setDepthCompareOp(depth_compare_op);
    depth_stencil_state_create_info->setDepthBoundsTestEnable(
        ldepth_stencil_state->depth_bounds_test_enable ? VK_TRUE : VK_FALSE);
    depth_stencil_state_create_info->setStencilTestEnable(
        ldepth_stencil_state->stencil_test_enable ? VK_TRUE : VK_FALSE);
    depth_stencil_state_create_info->setFront(vk::StencilOpState());
    depth_stencil_state_create_info->setBack(vk::StencilOpState());
    depth_stencil_state_create_info->setMinDepthBounds(
        ldepth_stencil_state->min_depth_bounds);
    depth_stencil_state_create_info->setMaxDepthBounds(
        ldepth_stencil_state->max_depth_bounds);

    pipeline_depth_stencil_state_create_infos.emplace_back(
        depth_stencil_state_create_info);

    XG_TRACE("  DepthStencilState: {} {} {} {} {} {} {}",
             ldepth_stencil_state->depth_test_enable,
             ldepth_stencil_state->depth_write_enable,
             vk::to_string(depth_compare_op),
             ldepth_stencil_state->depth_bounds_test_enable,
             ldepth_stencil_state->stencil_test_enable,
             ldepth_stencil_state->min_depth_bounds,
             ldepth_stencil_state->max_depth_bounds);

    // color blend state
    auto color_blend_attachments =
        std::make_shared<std::vector<vk::PipelineColorBlendAttachmentState>>();
    if (!color_blend_attachments) return Result::kErrorOutOfHostMemory;

    const auto& lcolor_blend_state = lgraphics_pipeline->lcolor_blend_state;
    assert(lcolor_blend_state);
    for (const auto& lcolor_blend_attachment :
         lcolor_blend_state->lcolor_blend_attachments) {
      const auto src_color_blend_factor = static_cast<vk::BlendFactor>(
          lcolor_blend_attachment.src_color_blend_factor);
      const auto dst_color_blend_factor = static_cast<vk::BlendFactor>(
          lcolor_blend_attachment.dst_color_blend_factor);
      const auto color_blend_op =
          static_cast<vk::BlendOp>(lcolor_blend_attachment.color_blend_op);

      const auto src_alpha_blend_factor = static_cast<vk::BlendFactor>(
          lcolor_blend_attachment.src_alpha_blend_factor);
      const auto dst_alpha_blend_factor = static_cast<vk::BlendFactor>(
          lcolor_blend_attachment.dst_alpha_blend_factor);
      const auto alpha_blend_op =
          static_cast<vk::BlendOp>(lcolor_blend_attachment.alpha_blend_op);

      const auto color_write_mask = static_cast<vk::ColorComponentFlagBits>(
          lcolor_blend_attachment.color_write_mask);
      color_blend_attachments->emplace_back(
          vk::PipelineColorBlendAttachmentState()
              .setSrcColorBlendFactor(src_color_blend_factor)
              .setDstColorBlendFactor(dst_color_blend_factor)
              .setColorBlendOp(color_blend_op)
              .setSrcAlphaBlendFactor(src_alpha_blend_factor)
              .setDstAlphaBlendFactor(dst_alpha_blend_factor)
              .setAlphaBlendOp(alpha_blend_op)
              .setColorWriteMask(color_write_mask)
              .setBlendEnable(lcolor_blend_attachment.blend_enable ? VK_TRUE
                                                                   : VK_FALSE));

      XG_TRACE("  ColorBlendState: Attachment: {} {} {} {} {} {} {} {}",
               vk::to_string(src_color_blend_factor),
               vk::to_string(dst_color_blend_factor),
               vk::to_string(color_blend_op),
               vk::to_string(src_alpha_blend_factor),
               vk::to_string(dst_alpha_blend_factor),
               vk::to_string(alpha_blend_op), vk::to_string(color_write_mask),
               lcolor_blend_attachment.blend_enable);
    }
    pipeline_color_blend_attachment_states.emplace_back(
        color_blend_attachments);

    auto color_blend_state_create_info =
        std::make_shared<vk::PipelineColorBlendStateCreateInfo>();
    if (!color_blend_state_create_info) return Result::kErrorOutOfHostMemory;

    color_blend_state_create_info->setAttachmentCount(
        static_cast<uint32_t>(color_blend_attachments->size()));
    color_blend_state_create_info->setPAttachments(
        color_blend_attachments->data());

    pipeline_color_blend_state_create_infos.emplace_back(
        color_blend_state_create_info);

    // dynamic states
    auto dynamic_states = std::make_shared<std::vector<vk::DynamicState>>();
    if (!dynamic_states) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }

    for (const auto& ldynamic_state : lgraphics_pipeline->ldynamic_states) {
      const auto state = static_cast<vk::DynamicState>(ldynamic_state->state);
      dynamic_states->emplace_back(state);

      XG_TRACE("  DynamicState: {}", vk::to_string(state));
    }
    pipeline_dynamic_states.emplace_back(dynamic_states);

    auto dynamic_state_create_info =
        std::make_shared<vk::PipelineDynamicStateCreateInfo>();
    if (!dynamic_state_create_info) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return Result::kErrorOutOfHostMemory;
    }

    dynamic_state_create_info->setDynamicStateCount(
        static_cast<uint32_t>(dynamic_states->size()));
    dynamic_state_create_info->setPDynamicStates(dynamic_states->data());

    pipeline_dynamic_state_create_infos.emplace_back(dynamic_state_create_info);

    // pipeline layout
    const auto& lpipeline_layout = lgraphics_pipeline->llayout;
    assert(lpipeline_layout);
    const auto& pipeline_layout =
        std::static_pointer_cast<PipelineLayoutVK>(lpipeline_layout->instance);

    XG_TRACE("  Layout: {} {}",
             (void*)(VkPipelineLayout)pipeline_layout->pipeline_layout_,
             lpipeline_layout->id);

    // render pass
    const auto& lrender_pass = lgraphics_pipeline->lrender_pass;
    assert(lrender_pass);
    const auto& render_pass =
        std::static_pointer_cast<RenderPassVK>(lrender_pass->instance);

    XG_TRACE("  RenderPass: {} {}",
             (void*)(VkRenderPass)render_pass->render_pass_, lrender_pass->id);

    auto create_info =
        vk::GraphicsPipelineCreateInfo()
            .setStageCount(
                static_cast<uint32_t>(shader_stage_create_infos->size()))
            .setPStages(shader_stage_create_infos->data())
            .setPVertexInputState(vertex_input_state_create_info.get())
            .setPInputAssemblyState(input_assembly_state_create_info.get())
            .setPViewportState(viewport_state_create_info.get())
            .setPRasterizationState(rasterization_state_create_info.get())
            .setPMultisampleState(multisample_state_create_info.get())
            .setPDepthStencilState(depth_stencil_state_create_info.get())
            .setPColorBlendState(color_blend_state_create_info.get())
            .setPDynamicState(dynamic_state_create_info.get())
            .setLayout(pipeline_layout->pipeline_layout_)
            .setRenderPass(render_pass->render_pass_)
            .setSubpass(static_cast<uint32_t>(lgraphics_pipeline->subpass));
    create_infos.emplace_back(create_info);
  }

  const auto& result = device_.createGraphicsPipelines(
      pipeline_cache_, static_cast<uint32_t>(create_infos.size()),
      create_infos.data(), nullptr, vk_pipelines.data());
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  for (int i = 0; i < vk_pipelines.size(); ++i) {
    auto pipeline_vk = static_cast<PipelineVK*>((*pipelines)[i].get());
    auto vk_pipeline = vk_pipelines[i];

    pipeline_vk->bind_point = PipelineBindPoint::kGraphics;
    pipeline_vk->device_ = device_;
    pipeline_vk->pipeline_ = vk_pipeline;

    XG_TRACE("  Pipeline: {} {}", (void*)(VkPipeline)vk_pipeline,
             lgraphics_pipelines[i]->id);
  }
  return Result::kSuccess;
}

std::shared_ptr<ImageView> DeviceVK::CreateImageView(
    const LayoutImageView& limage_view) const {
  auto image_view = std::make_shared<ImageViewVK>();
  if (!image_view) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  image_view->device_ = device_;

  auto result = image_view->Init(limage_view);
  if (result != Result::kSuccess) return nullptr;

  return image_view;
}

std::shared_ptr<DescriptorPool> DeviceVK::CreateDescriptorPool(
    const LayoutDescriptorPool& ldesc_pool) const {
  auto desc_pool = std::make_shared<DescriptorPoolVK>();
  if (!desc_pool) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  std::vector<vk::DescriptorPoolSize> pool_sizes;

  XG_TRACE("createDescriptorPool: {} {}", ldesc_pool.id, ldesc_pool.max_sets);

  for (const auto& pool_size : ldesc_pool.pool_sizes) {
    const auto type = static_cast<vk::DescriptorType>(pool_size.first);
    const auto& pool_size_vk =
        vk::DescriptorPoolSize().setType(type).setDescriptorCount(
            static_cast<uint32_t>(pool_size.second));
    pool_sizes.emplace_back(pool_size_vk);

    XG_TRACE("  PoolSize: {} {}", vk::to_string(type), pool_size.second);
  }

  const auto& create_info =
      vk::DescriptorPoolCreateInfo()
          .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
          .setMaxSets(static_cast<uint32_t>(ldesc_pool.max_sets))
          .setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
          .setPPoolSizes(pool_sizes.data());

  const auto& result = device_.createDescriptorPool(&create_info, nullptr,
                                                    &desc_pool->desc_pool_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  desc_pool->device_ = device_;

  XG_TRACE("  DescriptorPool: {} {}",
           (void*)(VkDescriptorPool)desc_pool->desc_pool_, ldesc_pool.id);

  return desc_pool;
}

std::shared_ptr<Framebuffer> DeviceVK::CreateFramebuffer(
    const LayoutFramebuffer& lframebuffer) const {
  auto framebuffer = std::make_shared<FramebufferVK>();
  if (!framebuffer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  framebuffer->device_ = device_;

  const auto& result = framebuffer->Init(lframebuffer);
  if (result != Result::kSuccess) return nullptr;

  return framebuffer;
}

std::shared_ptr<Sampler> DeviceVK::CreateSampler(
    const LayoutSampler& lsampler) const {
  auto sampler = std::make_shared<SamplerVK>();
  if (!sampler) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const auto mag_filter = static_cast<vk::Filter>(lsampler.mag_filter);
  const auto min_filter = static_cast<vk::Filter>(lsampler.min_filter);
  const auto mipmap_mode =
      static_cast<vk::SamplerMipmapMode>(lsampler.mipmap_mode);
  const auto address_mode_u =
      static_cast<vk::SamplerAddressMode>(lsampler.address_mode_u);
  const auto address_mode_v =
      static_cast<vk::SamplerAddressMode>(lsampler.address_mode_v);
  const auto address_mode_w =
      static_cast<vk::SamplerAddressMode>(lsampler.address_mode_w);
  auto& create_info = vk::SamplerCreateInfo()
                          .setMagFilter(mag_filter)
                          .setMinFilter(min_filter)
                          .setMipmapMode(mipmap_mode)
                          .setAddressModeU(address_mode_u)
                          .setAddressModeV(address_mode_v)
                          .setAddressModeW(address_mode_w);

  if (lsampler.anisotropy_enable &&
      physical_device_features_.samplerAnisotropy) {
    float value = std::min(physical_device_limits_.maxSamplerAnisotropy,
                           lsampler.max_anisotropy);
    create_info.setAnisotropyEnable(VK_TRUE).setMaxAnisotropy(value);
  }

  const auto& result =
      device_.createSampler(&create_info, nullptr, &sampler->sampler_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }

  sampler->device_ = device_;

  XG_TRACE("createSampler: {} {} {} {} {} {} {} {} {} {}",
           (void*)(VkSampler)sampler->sampler_, lsampler.id,
           vk::to_string(mag_filter), vk::to_string(min_filter),
           vk::to_string(mipmap_mode), vk::to_string(address_mode_u),
           vk::to_string(address_mode_v), vk::to_string(address_mode_w),
           create_info.anisotropyEnable, create_info.maxAnisotropy);

  return sampler;
}

std::shared_ptr<QueryPool> DeviceVK::CreateQueryPool(
    const LayoutQueryPool& lquery_pool) const {
  auto query_pool = std::make_shared<QueryPoolVK>();
  if (!query_pool) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const auto query_type = static_cast<vk::QueryType>(lquery_pool.query_type);
  const auto& create_info =
      vk::QueryPoolCreateInfo()
          .setQueryType(query_type)
          .setQueryCount(static_cast<uint32_t>(lquery_pool.query_count));

  const auto& result =
      device_.createQueryPool(&create_info, nullptr, &query_pool->query_pool_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }

  query_pool->device_ = device_;

  XG_TRACE("createQueryPool: {} {} {} {}",
           (void*)(VkQueryPool)query_pool->query_pool_, lquery_pool.id,
           vk::to_string(query_type), lquery_pool.query_count);

  return query_pool;
}

Result DeviceVK::UpdateDescriptorSets(
    const std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets) const {
  if (ldesc_sets.size() == 0) {
    XG_WARN("no descriptor sets to update");
    return Result::kSuccess;
  }
  std::vector<vk::WriteDescriptorSet> writes;
  std::vector<std::shared_ptr<std::vector<vk::DescriptorImageInfo>>>
      desc_image_infos;
  std::vector<std::shared_ptr<std::vector<vk::DescriptorBufferInfo>>>
      desc_buffer_infos;

  XG_TRACE("updateDescriptorSets");

  for (auto ldesc_set : ldesc_sets) {
    auto desc_set =
        std::static_pointer_cast<DescriptorSetVK>(ldesc_set->instance);

    for (auto ldesc : ldesc_set->ldescriptors) {
      const auto desc_type = static_cast<vk::DescriptorType>(ldesc->desc_type);
      auto write =
          vk::WriteDescriptorSet()
              .setDstSet(desc_set->desc_set_)
              .setDstBinding(static_cast<uint32_t>(ldesc->binding))
              .setDescriptorCount(static_cast<uint32_t>(ldesc->desc_count))
              .setDescriptorType(desc_type);

      XG_TRACE("  descriptorWrite: {} {} {} {} {}", ldesc_set->id,
               (void*)(VkDescriptorSet)desc_set->desc_set_, ldesc->binding,
               ldesc->desc_count, vk::to_string(desc_type));

      switch (ldesc->desc_type) {
        case DescriptorType::kSampler:
        case DescriptorType::kCombinedImageSampler:
        case DescriptorType::kSampledImage:
        case DescriptorType::kStorageImage:
        case DescriptorType::kInputAttachment: {
          const auto& vk_desc_image_infos =
              std::make_shared<std::vector<vk::DescriptorImageInfo>>();
          if (!vk_desc_image_infos) {
            XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
            return Result::kErrorOutOfHostMemory;
          }

          for (const auto& ldesc_image_info : ldesc->ldesc_image_infos) {
            const auto image_layout =
                static_cast<vk::ImageLayout>(ldesc_image_info->image_layout);
            auto vk_desc_image_info =
                vk::DescriptorImageInfo().setImageLayout(image_layout);

            if (ldesc_image_info->limage_view) {
              const auto& image_view = std::static_pointer_cast<ImageViewVK>(
                  ldesc_image_info->limage_view->instance);
              vk_desc_image_info.setImageView(image_view->image_view_);
            }

            if (ldesc_image_info->lsampler) {
              const auto& sampler = std::static_pointer_cast<SamplerVK>(
                  ldesc_image_info->lsampler->instance);
              vk_desc_image_info.setSampler(sampler->sampler_);
            }

            XG_TRACE("    ImageInfo: {} {} {}",
                     (void*)(VkSampler)vk_desc_image_info.sampler,
                     (void*)(VkImageView)vk_desc_image_info.imageView,
                     vk::to_string(image_layout));

            vk_desc_image_infos->emplace_back(std::move(vk_desc_image_info));
          }

          write.setPImageInfo(vk_desc_image_infos->data());
          desc_image_infos.emplace_back(std::move(vk_desc_image_infos));
          break;
        }
        case DescriptorType::kUniformBuffer:
        case DescriptorType::kStorageBuffer:
        case DescriptorType::kUniformBufferDynamic:
        case DescriptorType::kStorageBufferDynamic: {
          const auto& vk_desc_buffer_infos =
              std::make_shared<std::vector<vk::DescriptorBufferInfo>>();
          if (!vk_desc_buffer_infos) {
            XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
            return Result::kErrorOutOfHostMemory;
          }

          for (const auto& ldesc_buffer_info : ldesc->ldesc_buffer_infos) {
            const auto& buffer = std::static_pointer_cast<BufferVK>(
                ldesc_buffer_info->lbuffer->instance);

            vk::DeviceSize range;
            if (ldesc_buffer_info->range == -1)
              range = buffer->GetSize();
            else
              range = ldesc_buffer_info->range;

            auto vk_desc_buffer_info = vk::DescriptorBufferInfo()
                                           .setBuffer(buffer->buffer_)
                                           .setOffset(ldesc_buffer_info->offset)
                                           .setRange(range);

            vk_desc_buffer_infos->emplace_back(std::move(vk_desc_buffer_info));

            XG_TRACE("    BufferInfo: {} {} {}",
                     (void*)(VkBuffer)buffer->buffer_,
                     ldesc_buffer_info->offset, range);
          }
          write.setPBufferInfo(vk_desc_buffer_infos->data());
          desc_buffer_infos.emplace_back(std::move(vk_desc_buffer_infos));
          break;
        }
        case DescriptorType::kUniformTexelBuffer:
        case DescriptorType::kStorageTexelBuffer: {
          assert(0);  // TODO(kctan): IMPLEMENT
          return Result::kErrorFeatureNotPresent;
        }
        default:
          assert(0);
          return Result::kErrorFormatNotSupported;
      }
      writes.emplace_back(write);
    }
  }
  device_.updateDescriptorSets(writes, nullptr);
  return Result::kSuccess;
}

void DeviceVK::WaitIdle() const {
  XG_TRACE("waitIdle");
  device_.waitIdle();
}

void DeviceVK::ResetFences(
    const std::vector<std::shared_ptr<Fence>>& fences) const {
  std::vector<vk::Fence> vk_fences;
  vk_fences.reserve(fences.size());

  for (const auto& fence : fences) {
    const auto& fence_vk = static_cast<FenceVK*>(fence.get());
    vk_fences.emplace_back(fence_vk->fence_);
  }

  device_.resetFences(vk_fences);
}

Result DeviceVK::WaitForFences(
    const std::vector<std::shared_ptr<Fence>>& fences, bool wait_all,
    uint64_t timeout) const {
  std::vector<vk::Fence> vk_fences;
  vk_fences.reserve(fences.size());

  for (const auto& fence : fences) {
    const auto& fence_vk = static_cast<FenceVK*>(fence.get());
    vk_fences.emplace_back(fence_vk->fence_);
  }

  const auto& result =
      device_.waitForFences(vk_fences, wait_all ? VK_TRUE : VK_FALSE, timeout);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }
  return Result::kSuccess;
}

std::shared_ptr<Event> DeviceVK::CreateEvent(const LayoutEvent& levent) const {
  auto event = std::make_shared<EventVK>();
  if (!event) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const vk::EventCreateInfo create_info;

  const auto& result =
      device_.createEvent(&create_info, nullptr, &event->event_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  event->device_ = device_;

  XG_TRACE("createEvent: {} {}", (void*)(VkEvent)event->event_, levent.id);

  return event;
}

}  // namespace xg
