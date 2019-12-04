// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/simple_application.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "glm/glm.hpp"
#include "xg/camera.h"
#include "xg/engine.h"
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

bool SimpleApplication::Init(xg::Engine* engine) {
  auto& viewers = engine->GetViewers();

  int i = 0;
  for (const auto& viewer : viewers) {
    if (viewer->BuildCommandBuffers() != Result::kSuccess) return false;

    const auto& win = viewer->GetWindow();

    win->SetMouseButtonHandler([this, &viewer](MouseButton button,
                                               ButtonAction action,
                                               ModifierKey mods) {
      this->OnMouseButton(viewer, button, action, mods);
    });

    win->SetMouseMoveHandler([this, &viewer](double posx, double posy) {
      this->OnMouseMove(viewer, posx, posy);
    });

    viewer->SetUpdateHandler(
        [this, &viewer]() -> xg::Result { return this->OnUpdate(viewer); });

    viewer->SetShouldExitHandler(
        [this, &viewer]() -> bool { return this->ShouldExit(viewer); });

    ViewerData viewer_data = {};
    viewer_data.viewer_index = i;
    viewer_data_map_.insert(std::make_pair(viewer, viewer_data));
    ++i;
  }
  return true;
}

void SimpleApplication::OnMouseButton(std::shared_ptr<Viewer> viewer,
                                      MouseButton button, ButtonAction action,
                                      ModifierKey mods) {
  auto& viewer_data = viewer_data_map_[viewer];

  switch (action) {
    case ButtonAction::kPress:
      switch (button) {
        case MouseButton::kLeft:
          viewer_data.mouse_pressed.left = true;
          break;
        case MouseButton::kRight:
          viewer_data.mouse_pressed.right = true;
          break;
        case MouseButton::kMiddle:
          viewer_data.mouse_pressed.middle = true;
          break;
      }
      break;

    case ButtonAction::kRelease:
      switch (button) {
        case MouseButton::kLeft:
          viewer_data.mouse_pressed.left = false;
          break;
        case MouseButton::kRight:
          viewer_data.mouse_pressed.right = false;
          break;
        case MouseButton::kMiddle:
          viewer_data.mouse_pressed.middle = false;
          break;
      }
      break;
  }
}

void SimpleApplication::OnMouseMove(std::shared_ptr<Viewer> viewer, double posx,
                                    double posy) {
  auto& viewer_data = viewer_data_map_[viewer];
  auto& camera = viewer->GetCamera();
  const auto& new_pos = glm::vec2(posx, posy);
  const auto& delta = viewer_data.mouse_pos - new_pos;
  if (delta == glm::vec2()) return;

  if (viewer_data.mouse_pressed.left) {
    camera->Rotate({-delta.x, -delta.y});
  }
  if (viewer_data.mouse_pressed.right) {
    camera->Pan({-delta.x * viewer_data.camera_factor,
                 delta.y * viewer_data.camera_factor});
  }
  if (viewer_data.mouse_pressed.middle) {
    camera->Zoom(delta.y * viewer_data.camera_factor);
  }
  viewer_data.mouse_pos = new_pos;
}

}  // namespace xg
