// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_CAMERA_H_
#define XG_CAMERA_H_

#include <array>

#include "glm/glm.hpp"

namespace xg {

class Frustum {
 public:
  Frustum(const glm::mat4& matrix);
  const std::array<glm::vec4, 6>& GetPlanes() const { return planes_; }

 protected:
  std::array<glm::vec4, 6> planes_;
};

class Camera {
 public:
  void Perspective(float fov, float width, float height, float z_near,
                   float z_far);
  void LookAt(const glm::vec3& eye, const glm::vec3& center,
              const glm::vec3& up);

  const glm::mat4& GetPerspectiveMatrix() const { return perspective_; }
  const glm::mat4& GetViewMatrix() const { return view_; }
  const glm::vec3& GetPosition() const { return eye_; }
  const glm::vec3& GetCenter() const { return center_; }
  const glm::vec3& GetUp() const { return up_; }
  float GetWidth() const { return width_; }
  float GetHeight() const { return height_; }

 protected:
  glm::vec3 eye_ = glm::vec3();
  glm::vec3 center_ = glm::vec3();
  glm::vec3 up_ = glm::vec3();
  glm::mat4 perspective_;
  glm::mat4 view_;
  float width_ = 0.0f;
  float height_ = 0.0f;
};

}  // namespace xg

#endif  // XG_CAMERA_H_
