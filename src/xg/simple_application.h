// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SIMPLE_APPLICATION_H_
#define XG_SIMPLE_APPLICATION_H_

#include <memory>
#include <vector>

#include "glm/glm.hpp"
#include "xg/camera.h"
#include "xg/engine.h"
#include "xg/trackball.h"
#include "xg/types.h"
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

class SimpleApplication {
 public:
  SimpleApplication() = default;
  SimpleApplication(const SimpleApplication&) = delete;
  SimpleApplication& operator=(const SimpleApplication&) = delete;
  SimpleApplication(SimpleApplication&&) = delete;
  SimpleApplication& operator=(SimpleApplication&&) = delete;
  virtual ~SimpleApplication() = default;

  virtual bool Init(xg::Engine* engine);

 protected:
  virtual void OnMouseDown(std::shared_ptr<Viewer> viewer, MouseButton button, int posx, int posy);
  virtual void OnMouseUp(std::shared_ptr<Viewer> viewer, MouseButton button, int posx, int posy);
  virtual void OnMouseMove(std::shared_ptr<Viewer> viewer, int posx, int posy);
  virtual Result OnUpdate(View* view) {
    return Result::kSuccess;
  }
  virtual bool ShouldExit(std::shared_ptr<Viewer> viewer) {
    return viewer->ShouldClose();
  }
  virtual void OnDrawOverlay(std::shared_ptr<Viewer> viewer) {}

  std::vector<Trackball> trackballs_;
};

}  // namespace xg

#endif  // XG_SIMPLE_APPLICATION_H_
