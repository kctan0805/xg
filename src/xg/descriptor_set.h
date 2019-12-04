// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_DESCRIPTOR_SET_H_
#define XG_DESCRIPTOR_SET_H_

namespace xg {

class DescriptorSet {
 public:
  DescriptorSet(const DescriptorSet&) = delete;
  DescriptorSet& operator=(const DescriptorSet&) = delete;
  DescriptorSet(DescriptorSet&&) = delete;
  DescriptorSet& operator=(DescriptorSet&&) = delete;
  virtual ~DescriptorSet() = default;

protected:
  DescriptorSet() = default;
};

}  // namespace xg

#endif  // XG_DESCRIPTOR_SET_H_
