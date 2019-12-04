// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_DESCRIPTOR_SET_LAYOUT_H_
#define XG_DESCRIPTOR_SET_LAYOUT_H_

namespace xg {

class DescriptorSetLayout {
 public:
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout(DescriptorSetLayout&&) = delete;
  DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;
  virtual ~DescriptorSetLayout() = default;

protected:
  DescriptorSetLayout() = default;
};

}  // namespace xg

#endif  // XG_DESCRIPTOR_SET_LAYOUT_H_
