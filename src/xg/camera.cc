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
