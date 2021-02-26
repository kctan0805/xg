// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_OVERLAY_H_
#define XG_OVERLAY_H_

#include <cstdint>
#include <functional>
#include <vector>

#include "xg/command_buffer.h"
#include "xg/layout.h"

namespace xg {

class Overlay {
 public:
  Overlay() = default;
  Overlay(const Overlay&) = delete;
  Overlay& operator=(const Overlay&) = delete;
  Overlay(Overlay&&) = delete;
  Overlay& operator=(Overlay&&) = delete;
  virtual ~Overlay() = default;

  virtual bool AddFont(const std::vector<uint8_t>& data, float pixel_size) = 0;
  virtual bool CreateFontsTexture(const CommandBuffer* cmd) = 0;
  virtual void DestroyFontUploadObjects() = 0;
  virtual void Draw(const CommandBuffer* cmd) = 0;
  virtual void Resize() = 0;

  using DrawHandlerType = void();

  void SetDrawHandler(std::function<DrawHandlerType> handler) {
    draw_handler_ = handler;
  }

 protected:
  virtual bool Init(const LayoutOverlay& loverlay) = 0;

  std::function<DrawHandlerType> draw_handler_ = []() {};
};

}  // namespace xg

#endif  // XG_OVERLAY_H_
