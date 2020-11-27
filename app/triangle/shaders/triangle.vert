// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#version 450

layout(location = 0) in vec2 position_;
layout(location = 1) in vec3 color_;

layout(location = 0) out vec3 frag_color_;

void main() {
  gl_Position = vec4(position_, 0.0, 1.0);
  frag_color_ = color_;
}
