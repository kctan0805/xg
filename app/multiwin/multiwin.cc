// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "multiwin.h"

#include <cassert>
#include <memory>

#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/types.h"
#include "xg/viewer.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("multiwin.xml");

  return layout;
}

bool Application::Init(xg::Engine* engine) {
  if (!SimpleApplication::Init(engine)) return false;

  auto& viewers = engine->GetViewers();

  for (const auto& viewer : viewers) {
    view_update_data_[viewer.get()] = &viewer->GetUpdateData(0);
  }
  return true;
}

xg::Result Application::OnUpdate(std::shared_ptr<xg::Viewer> viewer) {
  const auto& camera = viewer->GetCamera();

  // update common uniform buffer
  auto draw_update_data = view_update_data_[viewer.get()];
  assert(draw_update_data);
  auto uniform_data = static_cast<glm::mat4*>(draw_update_data->Map());
  assert(uniform_data);
  *uniform_data = camera->GetPerspectiveMatrix() * camera->GetViewMatrix();
  draw_update_data->Unmap();

  return xg::Result::kSuccess;
}
