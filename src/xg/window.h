// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_WINDOW_H_
#define XG_WINDOW_H_

#include <functional>
#include <memory>

#include "xg/layout.h"

namespace xg {

enum class MouseButton { kLeft, kRight, kMiddle };
enum class ButtonAction { kRelease, kPress, kRepeat };
enum class ModifierKey { kShift = 0x1, kControl = 0x2, kAlt = 0x4 };

enum class WindowAttrib {
  kFocused = 0x00020001,
  kIconified = 0x00020002,
  kResizable = 0x00020003,
  kVisible = 0x00020004
};

class Window {
 public:
  Window() = default;
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;
  virtual ~Window() = default;

  virtual const void* GetHandle() const = 0;
  virtual void GetFramebufferSize(int* width, int* height) const = 0;
  virtual int GetAttrib(WindowAttrib attrib) const = 0;
  virtual bool ShouldClose() const = 0;
  virtual void PollEvents() = 0;
  virtual void Iconify() = 0;
  virtual void Restore() = 0;

  using ResizeHandlerType = void(int, int);

  void SetResizeHandler(std::function<ResizeHandlerType> handler) {
    resize_handler_ = handler;
  }

  using MouseButtonHandlerType = void(MouseButton, ButtonAction, ModifierKey);

  void SetMouseButtonHandler(std::function<MouseButtonHandlerType> handler) {
    mouse_button_handler_ = handler;
  }

  using MouseMoveHandlerType = void(double, double);

  void SetMouseMoveHandler(std::function<MouseMoveHandlerType> handler) {
    mouse_move_handler_ = handler;
  }

 protected:
  virtual bool Init(const LayoutWindow& lwin) = 0;

  std::function<ResizeHandlerType> resize_handler_ = [](int, int) {};
  std::function<MouseButtonHandlerType> mouse_button_handler_ =
      [](MouseButton, ButtonAction, ModifierKey) {};
  std::function<MouseMoveHandlerType> mouse_move_handler_ = [](double, double) {
  };
};

}  // namespace xg

#endif  // XG_WINDOW_H_
