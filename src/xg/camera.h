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
  void Perspective(float fovy, float aspect, float z_near, float z_far);
  void LookAt(const glm::vec3& eye, const glm::vec3& center,
              const glm::vec3& up);

  void Zoom(float factor);
  void Pan(const glm::vec2& factor);
  void Rotate(const glm::vec2& angle);

  const glm::mat4& GetPerspectiveMatrix() const { return perspective_; }
  const glm::mat4& GetViewMatrix() const { return view_; }
  const glm::vec3& GetPosition() const { return eye_; }

 protected:
  void Update();

  glm::vec3 eye_ = glm::vec3();
  glm::vec3 center_ = glm::vec3();
  glm::vec3 up_ = glm::vec3();
  glm::vec3 front_ = glm::vec3();
  glm::vec3 right_ = glm::vec3();
  glm::mat4 perspective_;
  glm::mat4 view_;
};

}  // namespace xg

#endif  // XG_CAMERA_H_
