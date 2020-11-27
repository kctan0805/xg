// road_runner - Road Runner
// Copyright (c) Jim Tan
//
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential

#version 450

layout(location = 0) in vec3 frag_color_;
layout(location = 0) out vec4 out_color_;

void main() {
	out_color_ = vec4(frag_color_, 1.0);
}
