// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OVERLAY_IMGUI_H_
#define XG_OVERLAY_IMGUI_H_

#include <memory>

#include "imgui/imgui.h"
#include "xg/overlay.h"
#include "xg/window.h"

namespace xg {

class OverlayImGui : public Overlay {
 public:
  static bool Initialize();

  OverlayImGui() = default;

  bool AddFont(const std::vector<uint8_t>& data, float pixel_size) override;

 protected:
  std::shared_ptr<Window> win_;
  ImGuiContext* ctxt_ = nullptr;
};

}  // namespace xg

#endif  // XG_OVERLAY_IMGUI_H_
