// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "serialize_layout.h"

#include <cassert>
#include <memory>
#include <vector>

#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/types.h"
#include "xg/viewer.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("serialize_layout.xml");
  return layout;
}

bool Application::Init(xg::Engine* engine) {
  if (!SimpleApplication::Init(engine)) return false;

  auto& viewer = engine->GetViewers()[0];
  auto& camera = viewer->GetCamera();

  // update common uniform buffer
  auto& draw_common_update_data = viewer->GetUpdateData(0);
  auto uniform_data = static_cast<glm::mat4*>(draw_common_update_data.Map());
  assert(uniform_data);
  *uniform_data = camera->GetPerspectiveMatrix();
  draw_common_update_data.Unmap();

  draw_update_data_ = &viewer->GetUpdateData(1);

  return true;
}

xg::Result Application::OnUpdate(std::shared_ptr<xg::Viewer> viewer) {
  auto& camera = viewer->GetCamera();

  // update instance uniform buffer
  auto uniform_data = static_cast<glm::mat4*>(draw_update_data_->Map());
  assert(uniform_data);
  *uniform_data = camera->GetViewMatrix();
  draw_update_data_->Unmap();

  return xg::Result::kSuccess;
}
