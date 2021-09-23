// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/openxr/reality_xr.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

// clang-format off
#include "vulkan/vulkan.hpp"
#include "openxr/openxr_platform.h"
#include "openxr/openxr.h"
// clang-format on
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/openxr/composition_layer_projection_xr.h"
#include "xg/openxr/reality_viewer_xr.h"
#include "xg/openxr/reference_space_xr.h"
#include "xg/openxr/session_xr.h"
#include "xg/openxr/swapchain_xr.h"
#include "xg/session.h"
#include "xg/utility.h"
#include "xg/viewer.h"
#include "xg/vulkan/device_vk.h"
#include "xg/vulkan/image_view_vk.h"
#include "xg/vulkan/image_vk.h"
#include "xg/vulkan/queue_vk.h"
#include "xg/vulkan/renderer_vk.h"

namespace xg {

RealityXR::~RealityXR() {
  if (instance_) {
    XG_TRACE("destroy: {}", (void*)instance_);
    const auto result = xrDestroyInstance(instance_);
    if (result != XR_SUCCESS) {
      XG_WARN(RealityResultString(static_cast<Result>(result)));
    }
  }
}

bool RealityXR::Init(const LayoutReality& lreality) {
  const auto& lrenderer = *lreality.lrenderer;

  if (!CreateInstance(lreality)) return false;
  if (lrenderer.validation && !CreateDebugMessenger()) return false;
  if (!InitSystem(lreality)) return false;
  if (!CreateSession(lreality)) return false;

  return true;
}

bool xg::RealityXR::CreateInstance(const LayoutReality& lreality) {
  const auto& lrenderer = *lreality.lrenderer;

  uint32_t count = 0;
  auto result =
      xrEnumerateInstanceExtensionProperties(nullptr, 0, &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  std::vector<XrExtensionProperties> found_extensions(
      count, XrExtensionProperties{XR_TYPE_EXTENSION_PROPERTIES});
  result = xrEnumerateInstanceExtensionProperties(nullptr, count, &count,
                                                  found_extensions.data());
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  std::vector<const char*> wanted_extensions{
      XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME};
  std::vector<const char*> wanted_layers;

  if (lrenderer.validation) {
    wanted_layers.emplace_back("XR_APILAYER_LUNARG_core_validation");
    wanted_extensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  std::vector<const char*> extensions;

  for (const char* wanted : wanted_extensions) {
    for (const auto& found : found_extensions) {
      if (std::strcmp(found.extensionName, wanted) == 0) {
        extensions.emplace_back(wanted);

        if (std::strcmp(found.extensionName,
                        XR_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
          debug_msg_available_ = true;
        }
        break;
      }
    }
  }

  result = xrEnumerateApiLayerProperties(0, &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  std::vector<XrApiLayerProperties> found_layers(
      count, XrApiLayerProperties{XR_TYPE_API_LAYER_PROPERTIES});
  result = xrEnumerateApiLayerProperties(count, &count, found_layers.data());
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  std::vector<const char*> layers;
  for (auto i = 0; i < wanted_layers.size(); ++i) {
    for (const auto& found : found_layers) {
      if (std::strcmp(found.layerName, wanted_layers[i]) == 0) {
        layers.emplace_back(wanted_layers[i]);
        break;
      }
    }
  }

  XrInstanceCreateInfo create_info = {XR_TYPE_INSTANCE_CREATE_INFO};
  auto& app_info = create_info.applicationInfo;

  std::strncpy(app_info.applicationName, lrenderer.app_name.c_str(),
               XR_MAX_APPLICATION_NAME_SIZE);
  std::strncpy(app_info.engineName, "xg", XR_MAX_ENGINE_NAME_SIZE);
  app_info.apiVersion = XR_CURRENT_API_VERSION;

  create_info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
  create_info.enabledApiLayerNames = layers.data();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.enabledExtensionNames = extensions.data();

  result = xrCreateInstance(&create_info, &instance_);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }
  XG_TRACE("createInstance: {} {}", (void*)instance_, lrenderer.app_name);

  const auto renderer_vk = static_cast<RendererVK*>(lrenderer.instance.get());
  vk_instance_ = renderer_vk->GetVkInstance();

  const auto& device = renderer_vk->GetDevice();
  const auto device_vk = static_cast<DeviceVK*>(device.get());
  vk_device_ = device_vk->GetVkDevice();

  return true;
}

static XRAPI_ATTR XrBool32 XRAPI_CALL DebugMessengerCallback(
    XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT messageType,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
  if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    XG_DEBUG(callbackData->message);
  } else if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    XG_INFO(callbackData->message);
  } else if (messageSeverity &
             XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    XG_WARN(callbackData->message);
  } else if (messageSeverity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    XG_ERROR(callbackData->message);
  }
  return XR_FALSE;
}

bool RealityXR::CreateDebugMessenger() {
  if (debug_msg_available_) {
    PFN_xrCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger = nullptr;
    auto result = xrGetInstanceProcAddr(
        instance_, "xrCreateDebugUtilsMessengerEXT",
        reinterpret_cast<PFN_xrVoidFunction*>(&CreateDebugUtilsMessenger));
    if (result != XR_SUCCESS) {
      XG_ERROR(RealityResultString(static_cast<Result>(result)));
      return false;
    }

    XrDebugUtilsMessengerCreateInfoEXT create_info = {
        XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    create_info.messageSeverities =
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                               XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
    create_info.userCallback = DebugMessengerCallback;
    result = CreateDebugUtilsMessenger(instance_, &create_info, &debug_msg_);
    if (result != XR_SUCCESS) {
      XG_ERROR(RealityResultString(static_cast<Result>(result)));
      return false;
    }
  }

  return true;
}

bool RealityXR::InitSystem(const LayoutReality& lreality) {
  XrSystemGetInfo sys_info = {XR_TYPE_SYSTEM_GET_INFO};
  sys_info.formFactor = static_cast<XrFormFactor>(lreality.form_factor);
  auto result = xrGetSystem(instance_, &sys_info, &system_id_);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  PFN_xrGetVulkanGraphicsRequirements2KHR GetVulkanGraphicsRequirements =
      nullptr;
  result = xrGetInstanceProcAddr(
      instance_, "xrGetVulkanGraphicsRequirements2KHR",
      reinterpret_cast<PFN_xrVoidFunction*>(&GetVulkanGraphicsRequirements));
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  XrGraphicsRequirementsVulkan2KHR req = {
      XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR};
  result = GetVulkanGraphicsRequirements(instance_, system_id_, &req);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  PFN_xrGetVulkanGraphicsDevice2KHR GetVulkanGraphicsDevice = nullptr;
  result = xrGetInstanceProcAddr(
      instance_, "xrGetVulkanGraphicsDevice2KHR",
      reinterpret_cast<PFN_xrVoidFunction*>(&GetVulkanGraphicsDevice));
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  XrVulkanGraphicsDeviceGetInfoKHR device_info = {
      XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR};
  device_info.systemId = system_id_;
  device_info.vulkanInstance = (VkInstance)vk_instance_;

  result =
      GetVulkanGraphicsDevice(instance_, &device_info, &vk_physical_device_);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  return true;
}

bool RealityXR::CreateSession(const LayoutReality& lreality) {
  auto session = std::make_shared<SessionXR>();
  if (!session) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }

  const auto queue_vk =
      static_cast<QueueVK*>(lreality.lsession->lqueue->instance.get());
  XrGraphicsBindingVulkan2KHR binding = {XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR};
  binding.instance = (VkInstance)vk_instance_;
  binding.physicalDevice = vk_physical_device_;
  binding.device = (VkDevice)vk_device_;
  binding.queueFamilyIndex = queue_vk->GetQueueFamilyIndex();
  binding.queueIndex = static_cast<uint32_t>(queue_vk->GetQueueIndex());

  XrSessionCreateInfo info = {XR_TYPE_SESSION_CREATE_INFO};
  info.next = &binding;
  info.systemId = system_id_;

  const auto result = xrCreateSession(instance_, &info, &session->session_);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  session_ = session;

  return true;
}

std::shared_ptr<Swapchain> RealityXR::CreateSwapchain(
    LayoutSwapchain* lswapchain) const {
  const auto& xr_session = static_cast<SessionXR*>(session_.get())->session_;
  const auto view_config_type =
      static_cast<XrViewConfigurationType>(lswapchain->view_config_type);

  uint32_t count = 0;
  auto result = xrEnumerateViewConfigurationViews(
      instance_, system_id_, view_config_type, 0, &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }
  if (count == 0) return nullptr;

  std::vector<XrViewConfigurationView> views(
      count, XrViewConfigurationView{XR_TYPE_VIEW_CONFIGURATION_VIEW});
  result = xrEnumerateViewConfigurationViews(
      instance_, system_id_, view_config_type, count, &count, views.data());
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  result = xrEnumerateSwapchainFormats(xr_session, 0, &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }
  assert(count > 0);

  std::vector<int64_t> swapchain_formats(count);
  result = xrEnumerateSwapchainFormats(xr_session, count, &count,
                                       swapchain_formats.data());
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }

  auto swapchain = std::make_shared<SwapchainXR>();
  if (!swapchain) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  if (lswapchain->view_index >= views.size()) {
    XG_ERROR("out of view: {} >= {}", lswapchain->view_index, views.size());
    return nullptr;
  }

  const auto& view = views[lswapchain->view_index];

  if (lswapchain->image_format == Format::kUndefined) {
    lswapchain->image_format = static_cast<xg::Format>(swapchain_formats[0]);
  } else {
    const auto it =
        std::find(swapchain_formats.begin(), swapchain_formats.end(),
                  static_cast<int64_t>(lswapchain->image_format));
    if (it == swapchain_formats.end())
      lswapchain->image_format = static_cast<xg::Format>(swapchain_formats[0]);
  }

  if (lswapchain->sample_count == 0)
    lswapchain->sample_count = view.recommendedSwapchainSampleCount;

  if (lswapchain->width == 0)
    lswapchain->width = view.recommendedImageRectWidth;

  if (lswapchain->height == 0)
    lswapchain->height = view.recommendedImageRectHeight;

  XrSwapchainCreateInfo info = {XR_TYPE_SWAPCHAIN_CREATE_INFO};
  info.usageFlags = static_cast<XrSwapchainUsageFlags>(lswapchain->usage);
  info.format = static_cast<int64_t>(lswapchain->image_format);
  info.sampleCount = lswapchain->sample_count;
  info.width = lswapchain->width;
  info.height = lswapchain->height;
  info.faceCount = lswapchain->face_count;
  info.arraySize = lswapchain->array_size;
  info.mipCount = lswapchain->mip_count;

  result = xrCreateSwapchain(xr_session, &info, &swapchain->swapchain_);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  result =
      xrEnumerateSwapchainImages(swapchain->swapchain_, 0, &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  std::vector<XrSwapchainImageVulkan2KHR> xr_images(
      count, XrSwapchainImageVulkan2KHR{XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR});
  result = xrEnumerateSwapchainImages(
      swapchain->swapchain_, count, &count,
      reinterpret_cast<XrSwapchainImageBaseHeader*>(xr_images.data()));
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  swapchain->images_.reserve(xr_images.size());
  swapchain->image_views_.reserve(xr_images.size());

  for (const auto& xr_image : xr_images) {
    auto image = std::make_shared<ImageVK>();
    if (!image) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return nullptr;
    }

    swapchain->images_.emplace_back(image);

    auto image_view = std::make_shared<ImageViewVK>();
    if (!image_view) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return nullptr;
    }

    image_view->device_ = vk_device_;
    swapchain->image_views_.emplace_back(image_view);
  }

  auto& image_view_create_info =
      vk::ImageViewCreateInfo()
          .setViewType(vk::ImageViewType::e2D)
          .setFormat(static_cast<vk::Format>(lswapchain->image_format))
          .setSubresourceRange(
              vk::ImageSubresourceRange()
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setLevelCount(1)
                  .setLayerCount(1));

  assert(xr_images.size() == swapchain->images_.size());
  assert(xr_images.size() == swapchain->image_views_.size());

  for (int i = 0; i < xr_images.size(); ++i) {
    auto& xr_image = xr_images[i];
    auto image = static_cast<ImageVK*>(swapchain->images_[i].get());

    XG_TRACE("enumerateSwapchainImagesToVector: {}", (void*)xr_image.image);

    image->width_ = lswapchain->width;
    image->height_ = lswapchain->height;
    image->format_ = static_cast<xg::Format>(lswapchain->image_format);
    image->image_ = xr_image.image;

    auto image_view =
        static_cast<ImageViewVK*>(swapchain->image_views_[i].get());

    image_view_create_info.setImage(xr_image.image);

    auto vk_result = vk_device_.createImageView(
        &image_view_create_info, nullptr, &image_view->image_view_);
    if (vk_result != vk::Result::eSuccess) {
      XG_ERROR(ResultString(static_cast<Result>(result)));
      return nullptr;
    }

    XG_TRACE("createImageView: {}",
             (void*)(VkImageView)image_view->image_view_);
  }

  if (swapchain->Init(*lswapchain) != Result::kSuccess) return nullptr;

  return swapchain;
}

std::shared_ptr<CompositionLayerProjection>
RealityXR::CreateCompositionLayerProjection(
    const LayoutCompositionLayerProjection& lprojection) {
  auto projection = std::make_shared<CompositionLayerProjectionXR>();
  if (!projection) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  projection->composition_layer_projection_.layerFlags =
      static_cast<XrCompositionLayerFlags>(lprojection.layer_flags);

  const auto space_xr =
      static_cast<ReferenceSpaceXR*>(lprojection.lspace->instance.get());
  projection->composition_layer_projection_.space = space_xr->space_;

  for (const auto& lview : lprojection.lviews) {
    XrCompositionLayerProjectionView view = {
        XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
    const auto swapchain =
        static_cast<SwapchainXR*>(lview.lswapchain->instance.get());

    view.subImage.swapchain = swapchain->swapchain_;
    view.subImage.imageRect.extent.width = swapchain->GetWidth();
    view.subImage.imageRect.extent.height = swapchain->GetHeight();

    projection->composition_layer_projection_views_.emplace_back(view);
  }

  projection->composition_layer_projection_.viewCount = static_cast<uint32_t>(
      projection->composition_layer_projection_views_.size());

  projection->composition_layer_projection_.views =
      projection->composition_layer_projection_views_.data();

  return projection;
}

std::shared_ptr<Viewer> RealityXR::CreateRealityViewer(
    const LayoutRealityViewer& lreality_viewer) {
  auto viewer = std::make_shared<RealityViewerXR>();
  if (!viewer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  uint32_t count = 0;
  const auto result = xrEnumerateViewConfigurationViews(
      instance_, system_id_,
      static_cast<XrViewConfigurationType>(lreality_viewer.view_config_type), 0,
      &count, nullptr);
  if (result != XR_SUCCESS) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  viewer->views_.resize(count);
  viewer->instance_ = instance_;
  viewer->session_ = static_cast<SessionXR*>(session_.get())->session_;

  if (!viewer->Init(lreality_viewer)) return nullptr;

  return viewer;
}

}  // namespace xg
