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
#include "glm/gtc/quaternion.hpp"

namespace xg {

class Camera {
 public:
  void Perspective(float fov, float width, float height, float z_near,
                   float z_far);
  void LookAt(const glm::vec3& eye, const glm::vec3& center,
              const glm::vec3& up);
  void ComputeProjectionFromFov(float angle_left, float angle_right,
                                float angle_up, float angle_down);
  void ComputeViewFromPose(const glm::quat& orientation,
                           const glm::vec3& position);

  const glm::mat4& GetProjectionMatrix() const { return projection_; }
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
  glm::mat4 projection_;
  glm::mat4 view_;
  float width_ = 0.0f;
  float height_ = 0.0f;
  float z_near_ = 0.0f;
  float z_far_ = 0.0f;
};

}  // namespace xg

#endif  // XG_CAMERA_H_
