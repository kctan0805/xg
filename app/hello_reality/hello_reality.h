// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef HELLO_REALITY_H_
#define HELLO_REALITY_H_

#include <memory>

#include "glm/glm.hpp"
#include "xg/command.h"
#include "xg/engine.h"
#include "xg/layout.h"
#include "xg/simple_application.h"
#include "xg/viewer.h"

class Application : xg::SimpleApplication {
 public:
  std::shared_ptr<xg::Layout> CreateLayout() const;
  bool Init(xg::Engine* engine) override;
  xg::Result OnUpdate(xg::View* view) override;

 private:
  glm::mat4 space_location_ = glm::mat4(1.0f);
  glm::mat4 model_matrix_;
  std::shared_ptr<xg::CommandPushConstants> push_model_matrix_cmd_;
};

#endif  // HELLO_REALITY_H_
