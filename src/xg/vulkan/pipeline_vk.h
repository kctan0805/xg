// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_PIPELINE_VK_H_
#define XG_VULKAN_PIPELINE_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/pipeline.h"

namespace xg {

class PipelineVK : public Pipeline {
 public:
  virtual ~PipelineVK();

  virtual void Exit() override;

 protected:
  vk::Device device_;
  vk::Pipeline pipeline_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_PIPELINE_VK_H_
