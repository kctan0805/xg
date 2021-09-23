// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OPENXR_REALITY_XR_H_
#define XG_OPENXR_REALITY_XR_H_

#include <memory>

// clang-format off
#include "vulkan/vulkan.hpp"
#include "openxr/openxr_platform.h"
#include "openxr/openxr.h"
// clang-format on
#include "xg/composition_layer_projection.h"
#include "xg/layout.h"
#include "xg/reality.h"
#include "xg/session.h"
#include "xg/viewer.h"

namespace xg {

class RealityXR : public Reality {
 public:
  RealityXR() = default;
  virtual ~RealityXR();

  const XrInstance& GetXrInstance() const { return instance_; }

  std::shared_ptr<Swapchain> CreateSwapchain(
      LayoutSwapchain* lswapchain) const override;
  std::shared_ptr<CompositionLayerProjection> CreateCompositionLayerProjection(
      const LayoutCompositionLayerProjection& lprojection) override;
  std::shared_ptr<Viewer> CreateRealityViewer(
      const LayoutRealityViewer& lreality_viewer) override;

 protected:
  bool Init(const LayoutReality& lreality);

  bool CreateInstance(const LayoutReality& lreality);
  bool CreateDebugMessenger();
  bool InitSystem(const LayoutReality& lreality);
  bool CreateSession(const LayoutReality& lreality);

  XrInstance instance_ = nullptr;
  XrDebugUtilsMessengerEXT debug_msg_ = nullptr;
  XrSystemId system_id_;
  vk::Instance vk_instance_;
  VkPhysicalDevice vk_physical_device_;
  vk::Device vk_device_;
  bool debug_msg_available_ = false;

  friend class Factory;
};

}  // namespace xg

#endif  // XG_OPENXR_REALITY_XR_H_
