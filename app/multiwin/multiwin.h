// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef MULTIWIN_H_
#define MULTIWIN_H_

#include <memory>
#include <unordered_map>

#include "xg/camera.h"
#include "xg/engine.h"
#include "xg/layout.h"
#include "xg/simple_application.h"
#include "xg/viewer.h"

class Application : xg::SimpleApplication {
 public:
  std::shared_ptr<xg::Layout> CreateLayout() const;
  bool Init(xg::Engine* engine) override;
  xg::Result OnUpdate(std::shared_ptr<xg::Viewer> viewer) override;

 private:
  xg::UpdateData* draw_update_data_[2];
  std::unordered_map<xg::Viewer*, xg::UpdateData*> view_update_data_;

};

#endif  // MULTIWIN_H_
