// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_PIPELINE_LAYOUT_VK_H_
#define XG_VULKAN_PIPELINE_LAYOUT_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/pipeline_layout.h"

namespace xg {

class PipelineLayoutVK : public PipelineLayout {
 public:
  virtual ~PipelineLayoutVK();

 protected:
  vk::Device device_;
  vk::PipelineLayout pipeline_layout_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_PIPELINE_LAYOUT_VK_H_
