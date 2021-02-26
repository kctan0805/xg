// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "overlay.h"

#include <cassert>
#include <memory>

#include "glm/glm.hpp"
#include "imgui/imgui.h"
#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/types.h"
#include "xg/viewer.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("overlay.xml");

  return layout;
}

bool Application::Init(xg::Engine* engine) {
  if (!SimpleApplication::Init(engine)) return false;

  auto& viewer = engine->GetViewers()[0];

  draw_update_data_ = &viewer->GetUpdateData(0);

  return true;
}

xg::Result Application::OnUpdate(std::shared_ptr<xg::Viewer> viewer) {
  auto camera = viewer->GetCamera();

  // update common uniform buffer
  auto uniform_data = static_cast<glm::mat4*>(draw_update_data_->Map());
  assert(uniform_data);
  *uniform_data = camera->GetPerspectiveMatrix() * camera->GetViewMatrix();
  draw_update_data_->Unmap();

  return xg::Result::kSuccess;
}

void Application::OnDrawOverlay(std::shared_ptr<xg::Viewer> viewer) {
  ImGui::ShowDemoWindow();
}
