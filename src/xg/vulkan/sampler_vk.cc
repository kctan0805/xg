// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/sampler_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

SamplerVK::~SamplerVK() {
  if (sampler_ && device_) {
    XG_TRACE("destroySampler: {}", static_cast<void*>((VkSampler)sampler_));

    device_.destroySampler(sampler_);
  }
}

}  // namespace xg
