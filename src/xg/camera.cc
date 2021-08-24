// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/camera.h"

#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace xg {

void Camera::Perspective(float fov, float width, float height, float z_near,
                         float z_far) {
  projection_ =
      glm::perspectiveFov(glm::radians(fov), width, height, z_near, z_far);
  projection_[1][1] *= -1;
  width_ = width;
  height_ = height;
  z_near_ = z_near;
  z_far_ = z_far;
}

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& center,
                    const glm::vec3& up) {
  eye_ = eye;
  center_ = center;
  up_ = up;
  view_ = glm::lookAt(eye_, center_, up_);
}

void Camera::ComputeProjectionFromFov(float angle_left, float angle_right,
                                      float angle_up, float angle_down) {
  const auto tan_left = std::tan(angle_left);
  const auto tan_right = std::tan(angle_right);
  const auto tan_down = std::tan(angle_down);
  const auto tan_up = std::tan(angle_up);
  const auto tan_width = tan_right - tan_left;
  const auto tan_height = tan_up - tan_down;
  const auto a11 = 2.0f / tan_width;
  const auto a22 = 2.0f / tan_height;
  const auto a31 = (tan_right + tan_left) / tan_width;
  const auto a32 = (tan_up + tan_down) / tan_height;
  const auto a33 = -z_far_ / (z_far_ - z_near_);
  const auto a43 = -(z_far_ * z_near_) / (z_far_ - z_near_);

  const float mat[16] = {a11, 0.0f, 0.0f, 0.0f,  0.0f, a22,  0.0f, 0.0f,
                         a31, a32,  a33,  -1.0f, 0.0f, 0.0f, a43,  0.0f};

  projection_ = glm::make_mat4(mat);
}

void Camera::ComputeViewFromPose(const glm::quat& orientation,
                                 const glm::vec3& position) {
  glm::mat4 rotation = glm::mat4_cast(orientation);
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
  glm::mat4 view_glm = translation * rotation;

  view_ = glm::inverse(view_glm);
}

}  // namespace xg
