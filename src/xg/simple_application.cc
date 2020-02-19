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

    TrackballInfo trackball_info = {};
    trackball_info.camera = viewer->GetCamera();
    viewer_data.trackball.Init(trackball_info);

    viewer_data_map_.insert(std::make_pair(viewer, viewer_data));
    ++i;
  }
  return true;
}

void SimpleApplication::OnMouseButton(std::shared_ptr<Viewer> viewer,
                                      MouseButton button, ButtonAction action,
                                      ModifierKey mods) {
  auto& viewer_data = viewer_data_map_[viewer];
  double posx, posy;
  viewer->GetWindow()->GetCursorPos(&posx, &posy);
  viewer_data.trackball.OnMouseButton(button, action, posx, posy);
}

void SimpleApplication::OnMouseMove(std::shared_ptr<Viewer> viewer, double posx,
                                    double posy) {
  auto& viewer_data = viewer_data_map_[viewer];
  viewer_data.trackball.OnMouseMove(posx, posy);
}

}  // namespace xg
