// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/frustum.h"

#include "glm/matrix.hpp"

namespace xg {

Frustum::Frustum(const glm::mat4& matrix) {
  const auto m = glm::transpose(matrix);

  planes_[kLeft] = m[3] + m[0];
  planes_[kRight] = m[3] - m[0];
  planes_[kBottom] = m[3] + m[1];
  planes_[kTop] = m[3] - m[1];
  planes_[kNear] = m[3] + m[2];
  planes_[kFar] = m[3] - m[2];

  const glm::vec3 crosses[] = {
      glm::cross(glm::vec3(planes_[kLeft]), glm::vec3(planes_[kRight])),
      glm::cross(glm::vec3(planes_[kLeft]), glm::vec3(planes_[kBottom])),
      glm::cross(glm::vec3(planes_[kLeft]), glm::vec3(planes_[kTop])),
      glm::cross(glm::vec3(planes_[kLeft]), glm::vec3(planes_[kNear])),
      glm::cross(glm::vec3(planes_[kLeft]), glm::vec3(planes_[kFar])),
      glm::cross(glm::vec3(planes_[kRight]), glm::vec3(planes_[kBottom])),
      glm::cross(glm::vec3(planes_[kRight]), glm::vec3(planes_[kTop])),
      glm::cross(glm::vec3(planes_[kRight]), glm::vec3(planes_[kNear])),
      glm::cross(glm::vec3(planes_[kRight]), glm::vec3(planes_[kFar])),
      glm::cross(glm::vec3(planes_[kBottom]), glm::vec3(planes_[kTop])),
      glm::cross(glm::vec3(planes_[kBottom]), glm::vec3(planes_[kNear])),
      glm::cross(glm::vec3(planes_[kBottom]), glm::vec3(planes_[kFar])),
      glm::cross(glm::vec3(planes_[kTop]), glm::vec3(planes_[kNear])),
      glm::cross(glm::vec3(planes_[kTop]), glm::vec3(planes_[kFar])),
      glm::cross(glm::vec3(planes_[kNear]), glm::vec3(planes_[kFar]))};

  points_[0] = Intersection<kLeft, kBottom, kNear>(crosses);
  points_[1] = Intersection<kLeft, kTop, kNear>(crosses);
  points_[2] = Intersection<kRight, kBottom, kNear>(crosses);
  points_[3] = Intersection<kRight, kTop, kNear>(crosses);
  points_[4] = Intersection<kLeft, kBottom, kFar>(crosses);
  points_[5] = Intersection<kLeft, kTop, kFar>(crosses);
  points_[6] = Intersection<kRight, kBottom, kFar>(crosses);
  points_[7] = Intersection<kRight, kTop, kFar>(crosses);
}

bool Frustum::IsBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) const {
  // check box outside/inside of frustum
  for (int i = 0; i < 6; ++i) {
    if ((glm::dot(planes_[i], glm::vec4(minp.x, minp.y, minp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(maxp.x, minp.y, minp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(minp.x, maxp.y, minp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(maxp.x, maxp.y, minp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(minp.x, minp.y, maxp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(maxp.x, minp.y, maxp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(minp.x, maxp.y, maxp.z, 1.0f)) <
         0.0f) &&
        (glm::dot(planes_[i], glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f)) <
         0.0f)) {
      return false;
    }
  }

  // check frustum outside/inside box
  int out;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].x > maxp.x) ? 1 : 0);
  if (out == 8) return false;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].x < minp.x) ? 1 : 0);
  if (out == 8) return false;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].y > maxp.y) ? 1 : 0);
  if (out == 8) return false;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].y < minp.y) ? 1 : 0);
  if (out == 8) return false;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].z > maxp.z) ? 1 : 0);
  if (out == 8) return false;
  out = 0;
  for (int i = 0; i < 8; i++) out += ((points_[i].z < minp.z) ? 1 : 0);
  if (out == 8) return false;

  return true;
}

template <Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
glm::vec3 Frustum::Intersection(const glm::vec3* crosses) const {
  float d = glm::dot(glm::vec3(planes_[a]), crosses[ij2k<b, c>()]);
  glm::vec3 res = glm::mat3(crosses[ij2k<b, c>()], -crosses[ij2k<a, c>()],
                            crosses[ij2k<a, b>()]) *
                  glm::vec3(planes_[a].w, planes_[b].w, planes_[c].w);
  return res * (-1.0f / d);
}

}  // namespace xg
