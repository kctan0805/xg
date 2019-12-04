// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_DESCRIPTOR_POOL_H_
#define XG_DESCRIPTOR_POOL_H_

#include <memory>
#include <vector>

#include "xg/descriptor_set.h"
#include "xg/layout.h"

namespace xg {

class DescriptorPool {
 public:
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool& operator=(const DescriptorPool&) = delete;
  DescriptorPool(DescriptorPool&&) = delete;
  DescriptorPool& operator=(DescriptorPool&&) = delete;
  virtual ~DescriptorPool() = default;

  virtual bool AllocateDescriptorSets(
      const std::vector<std::shared_ptr<LayoutDescriptorSet>>& ldesc_sets,
      std::vector<std::shared_ptr<DescriptorSet>>* desc_sets) = 0;

 protected:
  DescriptorPool() = default;
};

}  // namespace xg

#endif  // XG_DESCRIPTOR_POOL_H_
