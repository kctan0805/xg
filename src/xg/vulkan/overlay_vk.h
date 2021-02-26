// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_OVERLAY_VK_H_
#define XG_VULKAN_OVERLAY_VK_H_

#include "imgui/imgui.h"
#include "xg/command_buffer.h"
#include "xg/layout.h"
#include "xg/overlay.h"
#include "xg/overlay_imgui.h"
#include "xg/types.h"

namespace xg {

class OverlayVK : public OverlayImGui {
 public:
  static void Terminate();

  ~OverlayVK() { ImGui::DestroyContext(ctxt_); }

  bool CreateFontsTexture(const CommandBuffer* cmd) override;
  void DestroyFontUploadObjects() override;
  void Draw(const CommandBuffer* cmd) override;
  void Resize() override;

 protected:
  bool Init(const LayoutOverlay& loverlay) override;

  int min_image_count_ = 0;

  friend class RendererVK;
};

}  // namespace xg

#endif  // XG_VULKAN_OVERLAY_VK_H_
