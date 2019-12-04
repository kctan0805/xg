// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_SAMPLER_VK_H_
#define XG_VULKAN_SAMPLER_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/sampler.h"

namespace xg {

class SamplerVK : public Sampler {
 public:
  virtual ~SamplerVK();

 protected:
  vk::Device device_;
  vk::Sampler sampler_;

  friend class RendererVK;
  friend class DeviceVK;
};

}  // namespace xg

#endif  // XG_VULKAN_SAMPLER_VK_H_
