// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "hello_reality.h"

#include <cassert>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "xg/camera.h"
#include "xg/layout.h"
#include "xg/parser.h"
#include "xg/reality_viewer.h"
#include "xg/types.h"

std::shared_ptr<xg::Layout> Application::CreateLayout() const {
  auto layout = xg::Parser::Get().ParseFile("hello_reality.xml");

  auto model_matrix_data =
      std::static_pointer_cast<xg::LayoutData>(layout->Find("mainModelMatrix"));
  if (!model_matrix_data) return nullptr;

  std::memcpy(const_cast<float*>(glm::value_ptr(model_matrix_)),
              model_matrix_data->data.data(), model_matrix_data->data.size());

  return layout;
}

bool Application::Init(xg::Engine* engine) {
  if (!SimpleApplication::Init(engine)) return false;

  push_model_matrix_cmd_ = std::static_pointer_cast<xg::CommandPushConstants>(
      engine->Find("mainPushModelMatrix"));
  if (!push_model_matrix_cmd_) return false;

  return true;
}

xg::Result Application::OnUpdate(xg::View* view) {
  auto camera = view->GetCamera();
  auto update_data = view->GetUpdateData(0);

  // update common uniform buffer
  auto uniform_data = static_cast<glm::mat4*>(update_data->Map());
  assert(uniform_data);
  *uniform_data = camera->GetProjectionMatrix() * camera->GetViewMatrix();
  update_data->Unmap();

  // update model matrix
  auto* reality_viewer =
      reinterpret_cast<xg::RealityViewer*>(view->GetViewer());
  const auto& space_location = reality_viewer->GetSpaceLocations()[0];
  if (space_location_ != space_location) {
    auto model_matrix = space_location * model_matrix_;
    push_model_matrix_cmd_->SetData(glm::value_ptr(model_matrix),
                                    sizeof(model_matrix));
    view->RebuildCommandBuffers();
    space_location_ = space_location;
  }

  return xg::Result::kSuccess;
}
