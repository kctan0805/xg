// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/descriptor_pool_vk.h"

#include <cassert>
#include <memory>

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/descriptor_set_layout_vk.h"
#include "xg/vulkan/descriptor_set_vk.h"

namespace xg {

DescriptorPoolVK::~DescriptorPoolVK() {
  if (desc_pool_ && device_) {
    XG_TRACE("destroyDescriptorPool: {}", (void*)(VkDescriptorPool)desc_pool_);

    device_.destroyDescriptorPool(desc_pool_);
  }
}

bool DescriptorPoolVK::AllocateDescriptorSets(
    const std::vector<std::shared_ptr<LayoutDescriptorSet>>& ldesc_sets,
    std::vector<std::shared_ptr<DescriptorSet>>* desc_sets) {
  std::vector<vk::DescriptorSetLayout> set_layouts;

  XG_TRACE("allocateDescriptorSets: {}", (void*)(VkDescriptorPool)desc_pool_);

  for (const auto& ldesc_set : ldesc_sets) {
    const auto& set_layout = std::static_pointer_cast<DescriptorSetLayoutVK>(
        ldesc_set->lset_layout->instance);
    set_layouts.emplace_back(set_layout->desc_set_layout_);

    XG_TRACE("  SetLayout: {} {}",
             (void*)(VkDescriptorSetLayout)set_layout->desc_set_layout_,
             ldesc_set->lset_layout->id);
  }

  auto alloc_info =
      vk::DescriptorSetAllocateInfo()
          .setDescriptorPool(desc_pool_)
          .setDescriptorSetCount(static_cast<uint32_t>(set_layouts.size()))
          .setPSetLayouts(set_layouts.data());

  std::vector<vk::DescriptorSet> vk_desc_sets(set_layouts.size());

  const auto& result =
      device_.allocateDescriptorSets(&alloc_info, vk_desc_sets.data());
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return false;
  }

  int i = 0;
  for (const auto& vk_desc_set : vk_desc_sets) {
    auto desc_set = std::make_shared<DescriptorSetVK>();
    if (!desc_set) {
      XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
      return false;
    }

    desc_set->desc_set_ = vk_desc_set;
    desc_sets->emplace_back(desc_set);

    XG_TRACE("  DescriptorSet: {} {}", (void*)(VkDescriptorSet)vk_desc_set,
             ldesc_sets[i]->id);
    ++i;
  }

  return true;
}

}  // namespace xg
