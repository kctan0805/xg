// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef OVERLAY_H_
#define OVERLAY_H_

#include <memory>

#include "xg/engine.h"
#include "xg/layout.h"
#include "xg/simple_application.h"
#include "xg/viewer.h"

class Application : xg::SimpleApplication {
 public:
  std::shared_ptr<xg::Layout> CreateLayout() const;
  bool Init(xg::Engine* engine) override;
  xg::Result OnUpdate(std::shared_ptr<xg::Viewer> viewer) override;
  void OnDrawOverlay(std::shared_ptr<xg::Viewer> viewer) override;

 private:
  xg::UpdateData* draw_update_data_ = nullptr;
};

#endif  // OVERLAY_H_
