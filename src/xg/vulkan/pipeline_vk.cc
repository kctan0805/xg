// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/pipeline_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

PipelineVK::~PipelineVK() { Exit(); }

void PipelineVK::Exit() {
  if (pipeline_ && device_) {
    XG_TRACE("destroyPipeline: {}", (void*)(VkPipeline)pipeline_);

    device_.destroyPipeline(pipeline_);
    pipeline_ = nullptr;
  }
}

}  // namespace xg
