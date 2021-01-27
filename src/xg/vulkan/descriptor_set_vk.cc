// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/descriptor_set_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

DescriptorSetVK::~DescriptorSetVK() {
  if (desc_pool_ && device_) {
    XG_TRACE("freeDescriptorSets: {}", (void*)(VkDescriptorSet)desc_set_);

    device_.freeDescriptorSets(desc_pool_, 1, &desc_set_);
  }
}

}  // namespace xg
