// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/descriptor_set_layout_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

DescriptorSetLayoutVK::~DescriptorSetLayoutVK() {
  if (desc_set_layout_ && device_) {
    XG_TRACE("destroyDescriptorSetLayout: {}",
             static_cast<void*>((VkDescriptorSetLayout)desc_set_layout_));

    device_.destroyDescriptorSetLayout(desc_set_layout_);
  }
}

}  // namespace xg
