// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_FRUSTUM_H_
#define XG_FRUSTUM_H_

#include <array>

#include "glm/glm.hpp"

namespace xg {

class Frustum {
 public:
  Frustum() = default;
  Frustum(const glm::mat4& matrix);

  const std::array<glm::vec4, 6>& GetPlanes() const { return planes_; }
  bool IsBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) const;

 protected:
  enum Planes { kLeft, kRight, kBottom, kTop, kNear, kFar };

  template <Planes i, Planes j>
  constexpr int ij2k() const {
    return i * (9 - i) / 2 + j - 1;
  }

  template <Planes a, Planes b, Planes c>
  glm::vec3 Intersection(const glm::vec3* crosses) const;

  std::array<glm::vec4, 6> planes_;
  std::array<glm::vec3, 8> points_;
};

}  // namespace xg

#endif  // XG_FRUSTUM_H_
