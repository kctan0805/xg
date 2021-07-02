// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/openxr/reality_xr.h"

#include <cstdint>
#include <memory>
#include <vector>

// clang-format off
#include "vulkan/vulkan.hpp"
#include "openxr/openxr_platform.h"
#include "openxr/openxr.hpp"
// clang-format on
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/session.h"
#include "xg/system.h"
#include "xg/utility.h"
#include "xg/openxr/session_xr.h"
#include "xg/openxr/system_xr.h"
#include "xg/vulkan/device_vk.h"
#include "xg/vulkan/queue_vk.h"
#include "xg/vulkan/renderer_vk.h"

namespace xg {

RealityXR::~RealityXR() {
  if (instance_) {
    XG_TRACE("destroy: {}", (void*)(XrInstance)instance_);
    instance_.destroy();
  }
}

bool RealityXR::Init(const LayoutReality& lreality) {
  const auto& lrenderer = *lreality.lrenderer;

  if (!CreateInstance(lreality)) return false;
  CreateDispatchLoader();
  if (lrenderer.validation && !CreateDebugMessenger()) return false;

  return true;
}

bool xg::RealityXR::CreateInstance(const LayoutReality& lreality) {
  const auto& lrenderer = *lreality.lrenderer;
  const auto& found_extensions =
      xr::enumerateInstanceExtensionPropertiesToVector(nullptr);
  std::vector<const char*> wanted_extensions{
      XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};
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

  std::vector<const char*> layers;
  const auto& found_layers = xr::enumerateApiLayerPropertiesToVector();

  for (auto i = 0; i < wanted_layers.size(); ++i) {
    for (const auto& found : found_layers) {
      if (std::strcmp(found.layerName, wanted_layers[i]) == 0) {
        layers.emplace_back(wanted_layers[i]);
        break;
      }
    }
  }

  xr::InstanceCreateInfo create_info;
  xr::ApplicationInfo& app_info = create_info.applicationInfo;

  std::strncpy(app_info.applicationName, lrenderer.app_name.c_str(),
               XR_MAX_APPLICATION_NAME_SIZE);
  app_info.applicationVersion = static_cast<uint32_t>(XR_MAKE_VERSION(1, 0, 0));
  std::strncpy(app_info.engineName, "xg", XR_MAX_ENGINE_NAME_SIZE);
  app_info.engineVersion = static_cast<uint32_t>(XR_MAKE_VERSION(1, 0, 0));
  app_info.apiVersion = xr::Version(XR_CURRENT_API_VERSION);

  create_info.enabledApiLayerCount = static_cast<uint32_t>(layers.size());
  create_info.enabledApiLayerNames = layers.data();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.enabledExtensionNames = extensions.data();

  const auto& result = xr::createInstance(create_info, instance_);
  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }
  XG_TRACE("createInstance: {} {}", (void*)(XrInstance)instance_,
           lrenderer.app_name);

  const auto renderer_vk = static_cast<RendererVK*>(lrenderer.instance.get());
  vk_instance_ = renderer_vk->GetVkInstance();

  const auto& device = renderer_vk->GetDevice();
  const auto device_vk = static_cast<DeviceVK*>(device.get());
  vk_device_ = device_vk->GetVkDevice();

  return true;
}

void RealityXR::CreateDispatchLoader() {
  dispatch_loader_dynamic_ = xr::DispatchLoaderDynamic(instance_);
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
  xr::Result result = xr::Result::Success;

  if (debug_msg_available_) {
    xr::DebugUtilsMessengerCreateInfoEXT create_info;
    create_info.messageSeverities =
        xr::DebugUtilsMessageSeverityFlagBitsEXT::AllBits;
    create_info.messageTypes = xr::DebugUtilsMessageTypeFlagBitsEXT::AllBits;
    create_info.userCallback = DebugMessengerCallback;

    result = instance_.createDebugUtilsMessengerEXT(create_info, debug_msg_,
                                                    dispatch_loader_dynamic_);
  }

  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return false;
  }
  return true;
}

bool RealityXR::InitSystem(const LayoutSystem& lsystem) {
  auto system = std::make_shared<SystemXR>();
  if (!system) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }

  xr::SystemGetInfo info;
  info.formFactor = static_cast<xr::FormFactor>(lsystem.form_factor);

  // auto result = instance_.getSystem(info, system->system_id_);
  // if (result != xr::Result::Success) {
  //  XG_ERROR(RealityResultString(static_cast<Result>(result)));
  //  return nullptr;
  //}
  system->system_id_ = instance_.getSystem(info);

  xr::GraphicsRequirementsVulkanKHR req;
  auto result = instance_.getVulkanGraphicsRequirementsKHR(
      system->system_id_, req, dispatch_loader_dynamic_);
  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  result = instance_.getVulkanGraphicsDeviceKHR(
      system->system_id_, (VkInstance)vk_instance_, &vk_physical_device_,
      dispatch_loader_dynamic_);
  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  system_ = system;

  return true;
}

std::shared_ptr<Session> RealityXR::CreateSession(
    const LayoutSession& lsession) {
  auto session = std::make_shared<SessionXR>();
  if (!session) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  const auto queue_vk = static_cast<QueueVK*>(lsession.lqueue->instance.get());
  xr::GraphicsBindingVulkanKHR binding;
  binding.instance = (VkInstance)vk_instance_;
  binding.physicalDevice = vk_physical_device_;
  binding.device = (VkDevice)vk_device_;
  binding.queueFamilyIndex = queue_vk->GetQueueFamilyIndex();
  binding.queueIndex = static_cast<uint32_t>(queue_vk->GetQueueIndex());

  const auto system_xr = static_cast<SystemXR*>(system_.get());
  xr::SessionCreateInfo info;
  info.next = &binding;
  info.systemId = system_xr->system_id_;

  const auto result = instance_.createSession(info, session->session_);
  if (result != xr::Result::Success) {
    XG_ERROR(RealityResultString(static_cast<Result>(result)));
    return nullptr;
  }

  return session;
}

}  // namespace xg
