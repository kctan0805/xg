// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_SHADER_MODULE_VK_H_
#define XG_VULKAN_SHADER_MODULE_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/shader_module.h"

namespace xg {

class ShaderModuleVK : public ShaderModule {
 public:
  virtual ~ShaderModuleVK();

 protected:
  vk::Device device_;
  vk::ShaderModule shader_module_;

  friend class RendererVK;
  friend class DeviceVK;
};

}  // namespace xg

#endif  // XG_VULKAN_SHADER_MODULE_VK_H_
