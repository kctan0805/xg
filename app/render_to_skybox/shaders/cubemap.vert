// road_runner - Road Runner
// Copyright (c) Jim Tan
//
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential

#version 450

layout(location = 0) in vec2 position_;
layout(location = 1) in vec3 color_;

layout(location = 0) out vec3 frag_color_;

void main() {
  gl_Position = vec4(position_, 0.0, 1.0);
  frag_color_ = color_;
}
