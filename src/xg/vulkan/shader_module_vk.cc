// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/shader_module_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

ShaderModuleVK::~ShaderModuleVK() {
  if (shader_module_ && device_) {
    XG_TRACE("destroyShaderModule: {}", (void*)(VkShaderModule)shader_module_);

    device_.destroyShaderModule(shader_module_);
  }
}

}  // namespace xg
