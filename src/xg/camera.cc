// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/camera.h"

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

void Camera::Perspective(float fov, float width, float height, float z_near,
                         float z_far) {
  perspective_ =
      glm::perspectiveFov(glm::radians(fov), width, height, z_near, z_far);
  perspective_[1][1] *= -1;
  width_ = width;
  height_ = height;
}

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& center,
                    const glm::vec3& up) {
  eye_ = eye;
  center_ = center;
  up_ = up;
  view_ = glm::lookAt(eye_, center_, up_);
}

}  // namespace xg
