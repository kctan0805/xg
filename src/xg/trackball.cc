// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include "xg/trackball.h"

#include <cassert>
#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_interpolation.hpp"

namespace xg {

void Trackball::Init(const TrackballInfo& info) {
  if (!info.camera) return;
  camera_ = info.camera;
  width_ = camera_->GetWidth();
  height_ = camera_->GetHeight();
  camera_eye_ = glm::dvec3(camera_->GetPosition());
  camera_center_ = glm::dvec3(camera_->GetCenter());
  camera_up_ = glm::dvec3(camera_->GetUp());
}

void Trackball::ComputeCoordinates(int mouse_x, int mouse_y, glm::dvec2* ndc,
                                   glm::dvec3* tbc) const {
  assert(ndc);
  assert(tbc);

  *ndc = glm::dvec2((mouse_x / width_) * 2.0 - 1.0,
                    (mouse_y / height_) * 2.0 - 1.0);

  auto l = glm::length(*ndc);
  if (l < 1.0) {
    auto h = 0.5 + std::cos(l * M_PI) * 0.5;
    *tbc = glm::dvec3(ndc->x, -ndc->y, h);
  } else {
    *tbc = glm::dvec3(ndc->x, -ndc->y, 0.0);
  }
}

void Trackball::OnMouseDown(MouseButton button, int posx, int posy) {
  if (!camera_) return;

  ComputeCoordinates(posx, posy, &prev_ndc_, &prev_tbc_);

  switch (button) {
    case MouseButton::kLeft:
      mouse_pressed_.left = true;
      break;
    case MouseButton::kRight:
      mouse_pressed_.right = true;
      break;
    case MouseButton::kMiddle:
      mouse_pressed_.middle = true;
      break;
  }
}

void Trackball::OnMouseUp(MouseButton button, int posx, int posy) {
  if (!camera_) return;

  ComputeCoordinates(posx, posy, &prev_ndc_, &prev_tbc_);

  switch (button) {
    case MouseButton::kLeft:
      mouse_pressed_.left = false;
      break;
    case MouseButton::kRight:
      mouse_pressed_.right = false;
      break;
    case MouseButton::kMiddle:
      mouse_pressed_.middle = false;
      break;
  }
}

void Trackball::OnMouseMove(int posx, int posy) {
  if (!camera_) return;
  if (!mouse_pressed_.left && !mouse_pressed_.right && !mouse_pressed_.middle)
    return;

  glm::dvec2 new_ndc;
  glm::dvec3 new_tbc;
  ComputeCoordinates(posx, posy, &new_ndc, &new_tbc);

  if (mouse_pressed_.left) {
    auto xp = glm::cross(glm::normalize(new_tbc), glm::normalize(prev_tbc_));
    auto xp_len = glm::length(xp);
    if (xp_len > 0.0) {
      auto axis = xp / xp_len;
      auto angle = std::asin(xp_len);
      Rotate(angle, axis);
    }
  } else if (mouse_pressed_.right) {
    Pan(new_ndc - prev_ndc_);
  } else if (mouse_pressed_.middle) {
    Zoom((new_ndc - prev_ndc_).y);
  }

  prev_ndc_ = new_ndc;
  prev_tbc_ = new_tbc;
}

void Trackball::Rotate(double angle, const glm::dvec3& axis) {
  auto eye = glm::dvec4(camera_eye_, 0.0);
  auto center = glm::dvec4(camera_center_, 1.0);
  auto up = glm::dvec4(camera_up_, 0.0);

  auto rotation = glm::axisAngleMatrix(axis, angle);
  auto lv = glm::lookAt(camera_eye_, camera_center_, camera_up_);
  auto center_eye_space = glm::dvec3(lv * center);

  auto matrix = glm::inverse(lv) *
                glm::translate(glm::dmat4(1.0), center_eye_space) * rotation *
                glm::translate(glm::dmat4(1.0), -center_eye_space) * lv;

  up = glm::normalize(matrix * (eye + up) - matrix * eye);
  center = matrix * center;
  eye = matrix * eye;

  camera_eye_ = glm::dvec3(eye);
  camera_center_ = glm::dvec3(center);
  camera_up_ = glm::dvec3(up);

  assert(camera_);
  camera_->LookAt(glm::vec3(camera_eye_), glm::vec3(camera_center_),
                  glm::vec3(camera_up_));
}

void Trackball::Zoom(double ratio) {
  camera_eye_ += (camera_center_ - camera_eye_) * ratio;

  assert(camera_);
  camera_->LookAt(glm::vec3(camera_eye_), glm::vec3(camera_center_),
                  glm::vec3(camera_up_));
}

void Trackball::Pan(const glm::dvec2& delta) {
  auto v = camera_center_ - camera_eye_;
  auto look_normal = glm::normalize(v);
  auto side_normal = glm::cross(camera_up_, look_normal);
  auto dist = glm::length(v);
  auto translation =
      side_normal * (delta.x * dist) + camera_up_ * (delta.y * dist);

  camera_eye_ += translation;
  camera_center_ += translation;

  assert(camera_);
  camera_->LookAt(glm::vec3(camera_eye_), glm::vec3(camera_center_),
                  glm::vec3(camera_up_));
}

}  // namespace xg
