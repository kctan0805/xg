// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/pipeline_layout_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

PipelineLayoutVK::~PipelineLayoutVK() {
  if (pipeline_layout_ && device_) {
    XG_TRACE("destroyPipelineLayout: {}",
             (void*)(VkPipelineLayout)pipeline_layout_);

    device_.destroyPipelineLayout(pipeline_layout_);
  }
}

}  // namespace xg
