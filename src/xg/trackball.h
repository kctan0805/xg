// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_TRACKBALL_H_
#define XG_TRACKBALL_H_

#include <memory>

#include "glm/glm.hpp"
#include "xg/camera.h"
#include "xg/window.h"

namespace xg {

struct TrackballInfo {
  std::shared_ptr<Camera> camera;
};

class Trackball {
 public:
  void Init(const TrackballInfo& info);
  void OnMouseButton(MouseButton button, ButtonAction action, double posx,
                     double posy);
  void OnMouseMove(double posx, double posy);

 protected:
  void Rotate(double angle, const glm::dvec3& axis);
  void Zoom(double ratio);
  void Pan(const glm::dvec2& delta);
  void ComputeCoordinates(double mouse_x, double mouse_y, glm::dvec2* ndc,
                          glm::dvec3* tbc) const;

  double width_ = 0.0;
  double height_ = 0.0;
  glm::dvec2 prev_ndc_;
  glm::dvec3 prev_tbc_;
  glm::dvec3 camera_eye_;
  glm::dvec3 camera_center_;
  glm::dvec3 camera_up_;

  struct {
    bool left = false;
    bool right = false;
    bool middle = false;
  } mouse_pressed_;

  std::shared_ptr<Camera> camera_;
};

}  // namespace xg

#endif  // XG_TRACKBALL_H_