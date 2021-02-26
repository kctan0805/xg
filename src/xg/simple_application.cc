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
#include "xg/trackball.h"
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

bool SimpleApplication::Init(xg::Engine* engine) {
  auto& viewers = engine->GetViewers();

  int i = 0;
  for (const auto& viewer : viewers) {
    const auto& win = viewer->GetWindow();

    win->SetMouseDownHandler(
        [this, &viewer](MouseButton button, int posx, int posy) {
          this->OnMouseDown(viewer, button, posx, posy);
        });

    win->SetMouseUpHandler(
        [this, &viewer](MouseButton button, int posx, int posy) {
          this->OnMouseUp(viewer, button, posx, posy);
        });

    win->SetMouseMoveHandler([this, &viewer](int posx, int posy) {
      this->OnMouseMove(viewer, posx, posy);
    });

    viewer->SetUpdateHandler(
        [this, &viewer]() -> xg::Result { return this->OnUpdate(viewer); });

    viewer->SetShouldExitHandler(
        [this, &viewer]() -> bool { return this->ShouldExit(viewer); });

    viewer->SetDrawOverlayHandler(
        [this, &viewer]() { this->OnDrawOverlay(viewer); });

    ViewerData viewer_data = {};
    viewer_data.viewer_index = i;

    TrackballInfo trackball_info = {};
    trackball_info.camera = viewer->GetCamera();
    viewer_data.trackball.Init(trackball_info);

    viewer_data_map_.insert(std::make_pair(viewer, viewer_data));

    if (viewer->BuildCommandBuffers() != Result::kSuccess) return false;

    ++i;
  }
  return true;
}

void SimpleApplication::OnMouseDown(std::shared_ptr<Viewer> viewer,
                                    MouseButton button, int posx, int posy) {
  auto& viewer_data = viewer_data_map_[viewer];
  viewer_data.trackball.OnMouseDown(button, posx, posy);
}

void SimpleApplication::OnMouseUp(std::shared_ptr<Viewer> viewer,
                                  MouseButton button, int posx, int posy) {
  auto& viewer_data = viewer_data_map_[viewer];
  viewer_data.trackball.OnMouseUp(button, posx, posy);
}

void SimpleApplication::OnMouseMove(std::shared_ptr<Viewer> viewer, int posx,
                                    int posy) {
  auto& viewer_data = viewer_data_map_[viewer];
  viewer_data.trackball.OnMouseMove(posx, posy);
}

}  // namespace xg
