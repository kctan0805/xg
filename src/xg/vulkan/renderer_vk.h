// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_RENDERER_VK_H_
#define XG_VULKAN_RENDERER_VK_H_

#include <memory>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/device.h"
#include "xg/layout.h"
#include "xg/pipeline.h"
#include "xg/renderer.h"
#include "xg/vulkan/device_vk.h"
#include "xg/window.h"

#ifdef _WIN32  // workaround build fail bug
#undef CreateWindow
#endif

namespace xg {

class RendererVK : public Renderer {
 public:
  RendererVK() = default;
  virtual ~RendererVK();

  const vk::Instance& GetVkInstance() const { return instance_; }

  std::shared_ptr<Window> CreateWindow(const LayoutWindow& lwin) override;
  bool InitDevice(const LayoutDevice& ldevice) override;
  bool CreateComputePipelines(
      const std::vector<std::shared_ptr<LayoutComputePipeline>>&
          lcompute_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) override;
  bool CreateGraphicsPipelines(
      const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
          lgraphics_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) override;
  std::shared_ptr<Overlay> CreateOverlay(const LayoutOverlay& loverlay) override;

 protected:
  bool Init(const LayoutRenderer& lrenderer);

  bool CreateInstance(const LayoutRenderer& lrenderer);
  bool CreateDebugMessenger();
  void CreateDispatchLoader(DeviceVK* device);
  void DebugMarkerSetObjectName(const LayoutBase& lbase) const override;

  vk::DispatchLoaderDynamic dispatch_loader_dynamic_;
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT debug_msg_;
  vk::DebugReportCallbackEXT debug_report_;
  bool debug_msg_available_ = false;
  bool debug_report_available_ = false;

  friend class DeviceVK;
  friend class Factory;
};

}  // namespace xg

#endif  // XG_VULKAN_RENDERER_VK_H_
