// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#version 450

layout(set = 0, binding = 0) uniform DrawCommonUniform {
  mat4 proj_view_;
};

layout(push_constant) uniform PushConstants {
	mat4 model_;
};

layout(location = 0) in vec3 position_;
layout(location = 1) in vec3 color_;
layout(location = 2) in vec2 tex_coord_;

layout(location = 0) out vec3 frag_color_;
layout(location = 1) out vec2 frag_tex_coord_;

void main() {
  gl_Position = proj_view_ * model_ * vec4(position_, 1.0);
  frag_color_ = vec3(1.0);
  frag_tex_coord_ = tex_coord_;
}
