// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/camera.h"

#include <array>
#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace xg {

Frustum::Frustum(const glm::mat4& matrix) {
  // left
  planes_[0].x = matrix[0].w + matrix[0].x;
  planes_[0].y = matrix[1].w + matrix[1].x;
  planes_[0].z = matrix[2].w + matrix[2].x;
  planes_[0].w = matrix[3].w + matrix[3].x;

  // right
  planes_[1].x = matrix[0].w - matrix[0].x;
  planes_[1].y = matrix[1].w - matrix[1].x;
  planes_[1].z = matrix[2].w - matrix[2].x;
  planes_[1].w = matrix[3].w - matrix[3].x;

  // top
  planes_[2].x = matrix[0].w - matrix[0].y;
  planes_[2].y = matrix[1].w - matrix[1].y;
  planes_[2].z = matrix[2].w - matrix[2].y;
  planes_[2].w = matrix[3].w - matrix[3].y;

  // bottom
  planes_[3].x = matrix[0].w + matrix[0].y;
  planes_[3].y = matrix[1].w + matrix[1].y;
  planes_[3].z = matrix[2].w + matrix[2].y;
  planes_[3].w = matrix[3].w + matrix[3].y;

  // far
  planes_[4].x = matrix[0].w - matrix[0].z;
  planes_[4].y = matrix[1].w - matrix[1].z;
  planes_[4].z = matrix[2].w - matrix[2].z;
  planes_[4].w = matrix[3].w - matrix[3].z;

  // near
  planes_[5].x = matrix[0].w + matrix[0].z;
  planes_[5].y = matrix[1].w + matrix[1].z;
  planes_[5].z = matrix[2].w + matrix[2].z;
  planes_[5].w = matrix[3].w + matrix[3].z;

  for (auto i = 0; i < planes_.size(); i++) {
    float length =
        sqrtf(planes_[i].x * planes_[i].x + planes_[i].y * planes_[i].y +
              planes_[i].z * planes_[i].z);
    planes_[i] /= length;
  }
}

void Camera::Perspective(float fovy, float aspect, float z_near, float z_far) {
  perspective_ = glm::perspective(glm::radians(fovy), aspect, z_near, z_far);
  perspective_[1][1] *= -1;
}

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& center,
                    const glm::vec3& up) {
  eye_ = eye;
  center_ = center;
  up_ = up;
  Update();
}

void Camera::Zoom(float factor) {
  const auto& zoom = front_ * factor;
  eye_ += zoom;
  center_ += zoom;
  Update();
}

void Camera::Pan(const glm::vec2& factor) {
  const auto& x = right_ * factor.x;
  const auto& y = up_ * factor.y;
  eye_ += x + y;
  center_ += x + y;
  Update();
}

void Camera::Rotate(const glm::vec2& angle) {
  const auto& rot_x =
      glm::rotate(glm::mat4(1.0f), glm::radians(-angle.x), glm::normalize(up_));

  const auto& rot_y = glm::rotate(glm::mat4(1.0f), glm::radians(angle.y),
                                  glm::normalize(right_));

  const auto& trans = glm::translate(glm::mat4(1.0f), center_) * rot_y * rot_x *
                      glm::translate(glm::mat4(1.0f), -center_);

  eye_ = glm::vec3(trans * glm::vec4(eye_, 1));
  center_ = glm::vec3(trans * glm::vec4(center_, 1));

  Update();
}

void Camera::Update() {
  front_ = glm::normalize(center_ - eye_);
  right_ = glm::normalize(glm::cross(up_, front_));
  up_ = glm::normalize(glm::cross(front_, right_));
  view_ = glm::lookAt(eye_, center_, up_);
}

}  // namespace xg
