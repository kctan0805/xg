// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SHADER_MODULE_H_
#define XG_SHADER_MODULE_H_

namespace xg {

class ShaderModule {
 public:
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ShaderModule(ShaderModule&&) = delete;
  ShaderModule& operator=(ShaderModule&&) = delete;
  virtual ~ShaderModule() = default;

protected:
  ShaderModule() = default;
};

}  // namespace xg

#endif  // XG_SHADER_MODULE_H_
