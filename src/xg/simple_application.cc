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
#include "xg/window_viewer.h"

namespace xg {

bool SimpleApplication::Init(xg::Engine* engine) {
  auto& viewers = engine->GetViewers();

  TrackballInfo trackball_info = {};
  trackballs_.resize(viewers.size());
  int i = 0;
  for (const auto& viewer : viewers) {
    auto* win_viewer = dynamic_cast<WindowViewer*>(viewer.get());
    if (win_viewer) {
      const auto& win = win_viewer->GetWindow();

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

      win_viewer->SetDrawOverlayHandler(
          [this, &viewer]() { this->OnDrawOverlay(viewer); });

      viewer->SetUpdateHandler([this, win_viewer]() -> xg::Result {
        return this->OnUpdate(win_viewer->GetView());
      });

      auto* trackball = &trackballs_[i];
      trackball_info.camera = win_viewer->GetCamera();
      trackball->Init(trackball_info);

      win_viewer->SetCustomData(trackball);
    }

    if (viewer->BuildCommandBuffers() != Result::kSuccess) return false;

    viewer->SetShouldExitHandler(
        [this, &viewer]() -> bool { return this->ShouldExit(viewer); });

    ++i;
  }
  return true;
}

void SimpleApplication::OnMouseDown(std::shared_ptr<Viewer> viewer,
                                    MouseButton button, int posx, int posy) {
  auto* win_viewer = static_cast<WindowViewer*>(viewer.get());
  auto* trackball = static_cast<Trackball*>(win_viewer->GetCustomData());
  trackball->OnMouseDown(button, posx, posy);
}

void SimpleApplication::OnMouseUp(std::shared_ptr<Viewer> viewer,
                                  MouseButton button, int posx, int posy) {
  auto* win_viewer = static_cast<WindowViewer*>(viewer.get());
  auto* trackball = static_cast<Trackball*>(win_viewer->GetCustomData());
  trackball->OnMouseUp(button, posx, posy);
}

void SimpleApplication::OnMouseMove(std::shared_ptr<Viewer> viewer, int posx,
                                    int posy) {
  auto* win_viewer = static_cast<WindowViewer*>(viewer.get());
  auto* trackball = static_cast<Trackball*>(win_viewer->GetCustomData());
  trackball->OnMouseMove(posx, posy);
}

}  // namespace xg
