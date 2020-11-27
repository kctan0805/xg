// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#version 450

layout(location = 0) in vec3 frag_color_;

layout(location = 0) out vec4 out_color_;

void main() {
  out_color_ = vec4(frag_color_, 1.0);
}
