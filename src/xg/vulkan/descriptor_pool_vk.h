// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_DESCRIPTOR_POOL_VK_H_
#define XG_VULKAN_DESCRIPTOR_POOL_VK_H_

#include <memory>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "xg/descriptor_pool.h"
#include "xg/layout.h"

namespace xg {

class DescriptorPoolVK : public DescriptorPool {
 public:
  virtual ~DescriptorPoolVK();

  virtual bool AllocateDescriptorSets(
      const std::vector<std::shared_ptr<LayoutDescriptorSet>>& ldesc_sets,
      std::vector<std::shared_ptr<DescriptorSet>>* desc_sets) override;

 protected:
  vk::Device device_;
  vk::DescriptorPool desc_pool_;

  friend class RendererVK;
  friend class DeviceVK;
};

}  // namespace xg

#endif  // XG_VULKAN_DESCRIPTOR_POOL_VK_H_
