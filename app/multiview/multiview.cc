// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "multiview.h"

#include <cassert>
#include <memory>

#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/types.h"
#include "xg/window_viewer.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("multiview.xml");

  return layout;
}

bool Application::Init(xg::Engine *engine) {
  if (!SimpleApplication::Init(engine)) return false;

  auto *viewer = static_cast<xg::WindowViewer *>(engine->GetViewers()[0].get());
  const auto &camera = viewer->GetCamera();

  sub_camera_ = std::static_pointer_cast<xg::Camera>(engine->Find("subCamera"));
  if (!sub_camera_) return false;

  // update common uniform buffer
  auto update_data = viewer->GetUpdateData(0);
  auto common_data = static_cast<glm::mat4 *>(update_data->Map());
  assert(common_data);
  common_data[0] = camera->GetProjectionMatrix();
  common_data[1] = sub_camera_->GetProjectionMatrix();
  update_data->Unmap();

  return true;
}

xg::Result Application::OnUpdate(xg::View *view) {
  const auto &camera = view->GetCamera();
  auto update_data = view->GetUpdateData(1);

  // update instance uniform buffer
  auto instance_data = static_cast<glm::mat4 *>(update_data->Map());
  assert(instance_data);
  instance_data[0] = camera->GetViewMatrix();
  instance_data[1] = sub_camera_->GetViewMatrix();
  update_data->Unmap();

  return xg::Result::kSuccess;
}
