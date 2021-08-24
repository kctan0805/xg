// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "hello_world.h"

#include <cassert>
#include <memory>

#include "glm/glm.hpp"
#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/types.h"
#include "xg/window_viewer.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("hello_world.xml");

  return layout;
}

bool Application::Init(xg::Engine* engine) {
  if (!SimpleApplication::Init(engine)) return false;

  return true;
}

xg::Result Application::OnUpdate(xg::View* view) {
  const auto& camera = view->GetCamera();
  auto update_data = view->GetUpdateData(0);

  // update common uniform buffer
  auto uniform_data = static_cast<glm::mat4*>(update_data->Map());
  assert(uniform_data);
  *uniform_data = camera->GetProjectionMatrix() * camera->GetViewMatrix();
  update_data->Unmap();

  return xg::Result::kSuccess;
}
