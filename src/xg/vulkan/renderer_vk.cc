// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/renderer_vk.h"

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/buffer_vk.h"
#include "xg/vulkan/command_buffer_vk.h"
#include "xg/vulkan/command_pool_vk.h"
#include "xg/vulkan/descriptor_pool_vk.h"
#include "xg/vulkan/descriptor_set_layout_vk.h"
#include "xg/vulkan/descriptor_set_vk.h"
#include "xg/vulkan/device_vk.h"
#include "xg/vulkan/fence_vk.h"
#include "xg/vulkan/framebuffer_vk.h"
#include "xg/vulkan/image_view_vk.h"
#include "xg/vulkan/image_vk.h"
#include "xg/vulkan/pipeline_layout_vk.h"
#include "xg/vulkan/pipeline_vk.h"
#include "xg/vulkan/query_pool_vk.h"
#include "xg/vulkan/queue_vk.h"
#include "xg/vulkan/render_pass_vk.h"
#include "xg/vulkan/sampler_vk.h"
#include "xg/vulkan/semaphore_vk.h"
#include "xg/vulkan/shader_module_vk.h"
#include "xg/vulkan/swapchain_vk.h"
#include "xg/vulkan/window_vk.h"
#include "xg/window_glfw.h"

namespace xg {

RendererVK::~RendererVK() {
  device_.reset();
  if (instance_) {
    if (debug_msg_)
      instance_.destroyDebugUtilsMessengerEXT(debug_msg_, nullptr,
                                              dispatch_loader_dynamic_);

    XG_TRACE("destroy: {}", static_cast<void*>((VkInstance)instance_));
    instance_.destroy();
  }
  WindowGLFW::Terminate();
}

bool RendererVK::Init(const LayoutRenderer& lrenderer) {
  if (!WindowGLFW::Initialize()) return false;
  if (!CreateInstance(lrenderer)) return false;
  CreateDispatchLoader(nullptr);
  if (lrenderer.debug && !CreateDebugMessenger()) return false;

  return true;
}

bool RendererVK::CreateInstance(const LayoutRenderer& lrenderer) {
  const auto& found_extensions = vk::enumerateInstanceExtensionProperties();
  int win_extension_count = 0;
  const char** win_extensions =
      WindowVK::GetRequiredExtensions(&win_extension_count);
  std::vector<const char*> wanted_extensions{
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
  std::vector<const char*> wanted_layers;

  if (lrenderer.debug) {
    wanted_extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    wanted_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    wanted_layers.emplace_back("VK_LAYER_LUNARG_monitor");
    wanted_layers.emplace_back("VK_LAYER_RENDERDOC_Capture");
  }

  if (lrenderer.validation) {
    wanted_layers.emplace_back("VK_LAYER_LUNARG_assistant_layer");
    wanted_layers.emplace_back("VK_LAYER_KHRONOS_validation");
  }

  std::vector<const char*> extensions;

  for (auto i = 0; i < win_extension_count; ++i) {
    for (const auto& found : found_extensions) {
      if (std::string(found.extensionName.data()) == win_extensions[i]) {
        extensions.emplace_back(win_extensions[i]);
        break;
      }
    }
  }

  for (const char* wanted : wanted_extensions) {
    for (const auto& found : found_extensions) {
      if (std::string(found.extensionName.data()) == wanted) {
        extensions.emplace_back(wanted);
        break;
      }
    }
  }

  std::vector<const char*> layers;
  const auto& found_layers = vk::enumerateInstanceLayerProperties();

  for (auto i = 0; i < wanted_layers.size(); ++i) {
    for (const auto& found : found_layers) {
      if (std::string(found.layerName.data()) == wanted_layers[i]) {
        layers.emplace_back(wanted_layers[i]);
        break;
      }
    }
  }

  const auto& app_info = vk::ApplicationInfo()
                             .setPApplicationName(lrenderer.app_name.c_str())
                             .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
                             .setPEngineName("xg")
                             .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                             .setApiVersion(VK_API_VERSION_1_1);

  const auto& create_info =
      vk::InstanceCreateInfo()
          .setPApplicationInfo(&app_info)
          .setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
          .setPpEnabledLayerNames(layers.data())
          .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
          .setPpEnabledExtensionNames(extensions.data());

  const auto& result = vk::createInstance(&create_info, nullptr, &instance_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }
  XG_TRACE("createInstance: {} {}", static_cast<void*>((VkInstance)instance_),
           lrenderer.app_name);

  return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    XG_DEBUG(callbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    XG_INFO(callbackData->pMessage);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    XG_WARN(callbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    XG_ERROR(callbackData->pMessage);
  }
  return VK_FALSE;
}

bool RendererVK::CreateDebugMessenger() {
  const auto& create_info =
      vk::DebugUtilsMessengerCreateInfoEXT()
          .setMessageSeverity(
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
          .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
          .setPfnUserCallback(DebugMessengerCallback);

  const auto& result = instance_.createDebugUtilsMessengerEXT(
      &create_info, nullptr, &debug_msg_, dispatch_loader_dynamic_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }
  return true;
}

void RendererVK::CreateDispatchLoader(DeviceVK* device) {
  PFN_vkGetInstanceProcAddr inst_proc_addr =
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(
          instance_.getProcAddr("vkGetInstanceProcAddr"));

  if (device == nullptr) {
    dispatch_loader_dynamic_.init(instance_, inst_proc_addr);
  } else {
    auto& vk_device = device->GetVkDevice();
    PFN_vkGetDeviceProcAddr device_proc_addr =
        reinterpret_cast<PFN_vkGetDeviceProcAddr>(
            vk_device.getProcAddr("vkGetDeviceProcAddr"));

    dispatch_loader_dynamic_.init(instance_, inst_proc_addr, vk_device,
                                  device_proc_addr);
  }
}

std::shared_ptr<Window> RendererVK::CreateWindow(const LayoutWindow& lwin) {
  auto window = std::make_shared<WindowVK>();
  if (!window) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  if (!window->Init(lwin)) return nullptr;

  return window;
}

bool RendererVK::InitDevice(const LayoutDevice& ldevice) {
  auto device = std::make_shared<DeviceVK>();
  if (!device) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }
  if (!device->Init(ldevice)) return false;
  CreateDispatchLoader(device.get());
  if (!device->CreateMemoryAllocator(ldevice)) return false;

  device_ = device;

  return true;
}

bool RendererVK::CreateGraphicsPipelines(
    const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
        lgraphics_pipelines,
    std::vector<std::shared_ptr<Pipeline>>* pipelines) {
  for (const auto& lgraphics_pipeline : lgraphics_pipelines) {
    for (auto& lstage : lgraphics_pipeline->lstages) {
      if (lstage && lstage->lspec_info) {
        auto& lspec_info = lstage->lspec_info;

        if (lspec_info->ldata) {
          auto& ldata = lspec_info->ldata;
          lspec_info->data = ldata->data.data();

          if (lspec_info->data_size < ldata->data.size()) {
            XG_WARN("specialization data size {} < {}", lspec_info->data_size,
                    ldata->data.size());
            lspec_info->data_size = ldata->data.size();
          }
        }
      }
    }

    auto pipeline = std::make_shared<PipelineVK>();
    if (!pipeline) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return false;
    }
    pipelines->emplace_back(pipeline);
  }

  auto device_vk = static_cast<DeviceVK*>(device_.get());
  auto result =
      device_vk->InitGraphicsPipelines(lgraphics_pipelines, pipelines);
  if (result != Result::kSuccess) {
    pipelines->clear();
    return false;
  }
  return true;
}

bool RendererVK::CreateComputePipelines(
    const std::vector<std::shared_ptr<LayoutComputePipeline>>&
        lcompute_pipelines,
    std::vector<std::shared_ptr<Pipeline>>* pipelines) {
  for (const auto& lcompute_pipeline : lcompute_pipelines) {
    if (lcompute_pipeline->lstage && lcompute_pipeline->lstage->lspec_info) {
      auto& lspec_info = lcompute_pipeline->lstage->lspec_info;

      if (lspec_info->ldata) {
        auto& ldata = lspec_info->ldata;
        lspec_info->data = ldata->data.data();

        if (lspec_info->data_size < ldata->data.size()) {
          XG_WARN("specialization data size {} < {}", lspec_info->data_size,
                  ldata->data.size());
          lspec_info->data_size = ldata->data.size();
        }
      }
    }

    auto pipeline = std::make_shared<PipelineVK>();
    if (!pipeline) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return false;
    }
    pipelines->emplace_back(pipeline);
  }

  auto device_vk = static_cast<DeviceVK*>(device_.get());
  auto result = device_vk->InitComputePipelines(lcompute_pipelines, pipelines);
  if (result != Result::kSuccess) {
    pipelines->clear();
    return false;
  }
  return true;
}

void RendererVK::DebugMarkerSetObjectName(const LayoutBase& lbase) const {
  if (!dispatch_loader_dynamic_.vkDebugMarkerSetObjectNameEXT) return;
  if (!lbase.realize) return;

  auto device_vk = static_cast<DeviceVK*>(device_.get());
  vk::DebugMarkerObjectNameInfoEXT name_info;
  switch (lbase.layout_type) {
    case LayoutType::kDevice: {
      const auto device = static_cast<DeviceVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eDevice)
          .setObject(reinterpret_cast<uint64_t>((VkDevice)device->device_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kSwapchain: {
      const auto swapchain = static_cast<SwapchainVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eSwapchainKHR)
          .setObject(
              reinterpret_cast<uint64_t>((VkSwapchainKHR)swapchain->swapchain_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kQueue: {
      const auto queue = static_cast<QueueVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eQueue)
          .setObject(reinterpret_cast<uint64_t>((VkQueue)queue->queue_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kCommandPool: {
      const auto cmd_pool = static_cast<CommandPoolVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eCommandPool)
          .setObject(
              reinterpret_cast<uint64_t>((VkCommandPool)cmd_pool->cmd_pool_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kCommandBuffer: {
      const auto lcmd_buffer = static_cast<const LayoutCommandBuffer*>(&lbase);
      if (lcmd_buffer->lframe) {
        const auto& cmd_buffers = std::static_pointer_cast<
            std::vector<std::shared_ptr<CommandBuffer>>>(lbase.instance);
        int i = 0;
        for (const auto& cmd_buffer : *cmd_buffers) {
          const auto cmd_buffer_vk =
              static_cast<CommandBufferVK*>(cmd_buffer.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eCommandBuffer)
              .setObject(reinterpret_cast<uint64_t>(
                  (VkCommandBuffer)cmd_buffer_vk->cmd_buffer_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto cmd_buffer =
            static_cast<CommandBufferVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eCommandBuffer)
            .setObject(reinterpret_cast<uint64_t>(
                (VkCommandBuffer)cmd_buffer->cmd_buffer_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kFence: {
      const auto lfence = static_cast<const LayoutFence*>(&lbase);
      if (lfence->lframe) {
        const auto& fences =
            std::static_pointer_cast<std::vector<std::shared_ptr<Fence>>>(
                lbase.instance);
        int i = 0;
        for (const auto& fence : *fences) {
          const auto fence_vk = static_cast<FenceVK*>(fence.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eFence)
              .setObject(reinterpret_cast<uint64_t>((VkFence)fence_vk->fence_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto fence = static_cast<FenceVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eFence)
            .setObject(reinterpret_cast<uint64_t>((VkFence)fence->fence_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kBuffer: {
      const auto lbuffer = static_cast<const LayoutBuffer*>(&lbase);
      if (lbuffer->lframe) {
        const auto& buffers =
            std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
                lbase.instance);
        int i = 0;
        for (const auto& buffer : *buffers) {
          const auto buffer_vk = static_cast<BufferVK*>(buffer.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer)
              .setObject(
                  reinterpret_cast<uint64_t>((VkBuffer)buffer_vk->buffer_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto buffer = static_cast<BufferVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer)
            .setObject(reinterpret_cast<uint64_t>((VkBuffer)buffer->buffer_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kImage: {
      const auto image = static_cast<ImageVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eImage)
          .setObject(reinterpret_cast<uint64_t>((VkImage)image->image_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kImageView: {
      const auto image_view = static_cast<ImageViewVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eImageView)
          .setObject(
              reinterpret_cast<uint64_t>((VkImageView)image_view->image_view_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kSampler: {
      const auto sampler = static_cast<SamplerVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler)
          .setObject(reinterpret_cast<uint64_t>((VkSampler)sampler->sampler_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kDescriptorSetLayout: {
      const auto desc_set_layout =
          static_cast<DescriptorSetLayoutVK*>(lbase.instance.get());
      name_info
          .setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSetLayout)
          .setObject(reinterpret_cast<uint64_t>(
              (VkDescriptorSetLayout)desc_set_layout->desc_set_layout_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kDescriptorPool: {
      const auto desc_pool =
          static_cast<DescriptorPoolVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorPool)
          .setObject(reinterpret_cast<uint64_t>(
              (VkDescriptorPool)desc_pool->desc_pool_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kDescriptorSet: {
      const auto ldesc_set = static_cast<const LayoutDescriptorSet*>(&lbase);
      if (ldesc_set->lframe) {
        const auto& desc_sets = std::static_pointer_cast<
            std::vector<std::shared_ptr<DescriptorSet>>>(lbase.instance);
        int i = 0;
        for (const auto& desc_set : *desc_sets) {
          const auto desc_set_vk =
              static_cast<DescriptorSetVK*>(desc_set.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSet)
              .setObject(reinterpret_cast<uint64_t>(
                  (VkDescriptorSet)desc_set_vk->desc_set_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto desc_set =
            static_cast<DescriptorSetVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eDescriptorSet)
            .setObject(reinterpret_cast<uint64_t>(
                (VkDescriptorSet)desc_set->desc_set_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kRenderPass: {
      const auto render_pass = static_cast<RenderPassVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eRenderPass)
          .setObject(reinterpret_cast<uint64_t>(
              (VkRenderPass)render_pass->render_pass_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kShaderModule: {
      const auto shader_module =
          static_cast<ShaderModuleVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::eShaderModule)
          .setObject(reinterpret_cast<uint64_t>(
              (VkShaderModule)shader_module->shader_module_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kPipelineLayout: {
      const auto pipeline_layout =
          static_cast<PipelineLayoutVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::ePipelineLayout)
          .setObject(reinterpret_cast<uint64_t>(
              (VkPipelineLayout)pipeline_layout->pipeline_layout_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kComputePipeline:
    case LayoutType::kGraphicsPipeline: {
      const auto pipeline = static_cast<PipelineVK*>(lbase.instance.get());
      name_info.setObjectType(vk::DebugReportObjectTypeEXT::ePipeline)
          .setObject(
              reinterpret_cast<uint64_t>((VkPipeline)pipeline->pipeline_))
          .setPObjectName(lbase.id.c_str());
      break;
    }
    case LayoutType::kSemaphore: {
      const auto lsemaphore = static_cast<const LayoutSemaphore*>(&lbase);
      if (lsemaphore->lframe) {
        const auto& semaphores =
            std::static_pointer_cast<std::vector<std::shared_ptr<Semaphore>>>(
                lbase.instance);
        int i = 0;
        for (const auto& semaphore : *semaphores) {
          const auto semaphore_vk = static_cast<SemaphoreVK*>(semaphore.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eSemaphore)
              .setObject(reinterpret_cast<uint64_t>(
                  (VkSemaphore)semaphore_vk->semaphore_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto semaphore = static_cast<SemaphoreVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eSemaphore)
            .setObject(
                reinterpret_cast<uint64_t>((VkSemaphore)semaphore->semaphore_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kFramebuffer: {
      const auto lframebuffer = static_cast<const LayoutFramebuffer*>(&lbase);
      if (lframebuffer->lframe) {
        const auto& framebuffers =
            std::static_pointer_cast<std::vector<std::shared_ptr<Framebuffer>>>(
                lbase.instance);
        int i = 0;
        for (const auto& framebuffer : *framebuffers) {
          const auto framebuffer_vk =
              static_cast<FramebufferVK*>(framebuffer.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eFramebuffer)
              .setObject(reinterpret_cast<uint64_t>(
                  (VkFramebuffer)framebuffer_vk->framebuffer_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto framebuffer =
            static_cast<FramebufferVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eFramebuffer)
            .setObject(reinterpret_cast<uint64_t>(
                (VkFramebuffer)framebuffer->framebuffer_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    case LayoutType::kQueryPool: {
      const auto lquery_pool = static_cast<const LayoutQueryPool*>(&lbase);
      if (lquery_pool->lframe) {
        const auto& query_pools =
            std::static_pointer_cast<std::vector<std::shared_ptr<QueryPool>>>(
                lbase.instance);
        int i = 0;
        for (const auto& query_pool : *query_pools) {
          const auto query_pool_vk =
              static_cast<QueryPoolVK*>(query_pool.get());
          const auto id = lbase.id + std::to_string(i);
          name_info.setObjectType(vk::DebugReportObjectTypeEXT::eQueryPool)
              .setObject(reinterpret_cast<uint64_t>(
                  (VkQueryPool)query_pool_vk->query_pool_))
              .setPObjectName(id.c_str());

          const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
              &name_info, dispatch_loader_dynamic_);
          if (result != vk::Result::eSuccess) {
            XG_ERROR(ResultString(static_cast<Result>(result)));
          }
          ++i;
        }
        return;
      } else {
        const auto query_pool = static_cast<QueryPoolVK*>(lbase.instance.get());
        name_info.setObjectType(vk::DebugReportObjectTypeEXT::eQueryPool)
            .setObject(reinterpret_cast<uint64_t>(
                (VkQueryPool)query_pool->query_pool_))
            .setPObjectName(lbase.id.c_str());
      }
      break;
    }
    default:
      return;
  }

  const auto& result = device_vk->device_.debugMarkerSetObjectNameEXT(
      &name_info, dispatch_loader_dynamic_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
  }
}

}  // namespace xg
