// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_DESCRIPTOR_SET_LAYOUT_VK_H_
#define XG_VULKAN_DESCRIPTOR_SET_LAYOUT_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/descriptor_set_layout.h"

namespace xg {

class DescriptorSetLayoutVK : public DescriptorSetLayout {
 public:
  virtual ~DescriptorSetLayoutVK();

 protected:
  vk::Device device_;
  vk::DescriptorSetLayout desc_set_layout_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class DescriptorPoolVK;
};

}  // namespace xg

#endif  // XG_VULKAN_DESCRIPTOR_SET_LAYOUT_VK_H_
