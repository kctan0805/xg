// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_DESCRIPTOR_SET_VK_H_
#define XG_VULKAN_DESCRIPTOR_SET_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/descriptor_pool.h"
#include "xg/descriptor_set.h"

namespace xg {

class DescriptorSetVK : public DescriptorSet {
 public:
  virtual ~DescriptorSetVK();

 protected:
  vk::Device device_;
  vk::DescriptorPool desc_pool_;
  vk::DescriptorSet desc_set_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class DescriptorPoolVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_DESCRIPTOR_SET_VK_H_
