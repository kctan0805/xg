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

enum class MouseButton { kLeft = 1, kMiddle = 2, kRight = 3 };

enum class WindowFlags : unsigned int {
  kVisible = 0x00000004,
  kResizable = 0x00000020,
  kMinimized = 0x00000040,
  kFocused = 0x00000400
};
inline WindowFlags operator|(WindowFlags lhs, WindowFlags rhs) {
  return static_cast<WindowFlags>(
      static_cast<std::underlying_type_t<WindowFlags>>(lhs) |
      static_cast<std::underlying_type_t<WindowFlags>>(rhs));
}
inline WindowFlags operator&(WindowFlags lhs, WindowFlags rhs) {
  return static_cast<WindowFlags>(
      static_cast<std::underlying_type_t<WindowFlags>>(lhs) &
      static_cast<std::underlying_type_t<WindowFlags>>(rhs));
}

class Window {
 public:
  Window() = default;
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;
  virtual ~Window() = default;

  virtual const void* GetHandle() const = 0;
  virtual void GetDrawableSize(int* width, int* height) const = 0;
  virtual WindowFlags GetWindowFlags() const = 0;
  virtual bool ShouldClose() const = 0;
  virtual void PollEvents() = 0;

  using ResizeHandlerType = void(int, int);

  void SetResizeHandler(std::function<ResizeHandlerType> handler) {
    resize_handler_ = handler;
  }

  using MouseButtonHandlerType = void(MouseButton, int, int);

  void SetMouseDownHandler(std::function<MouseButtonHandlerType> handler) {
    mouse_down_handler_ = handler;
  }

  void SetMouseUpHandler(std::function<MouseButtonHandlerType> handler) {
    mouse_up_handler_ = handler;
  }

  using MouseMoveHandlerType = void(int, int);

  void SetMouseMoveHandler(std::function<MouseMoveHandlerType> handler) {
    mouse_move_handler_ = handler;
  }

 protected:
  virtual bool Init(const LayoutWindow& lwin) = 0;

  std::function<ResizeHandlerType> resize_handler_ = [](int, int) {};
  std::function<MouseButtonHandlerType> mouse_down_handler_ = [](MouseButton,
                                                                 int, int) {};
  std::function<MouseButtonHandlerType> mouse_up_handler_ = [](MouseButton, int,
                                                               int) {};
  std::function<MouseMoveHandlerType> mouse_move_handler_ = [](int, int) {};
};

}  // namespace xg

#endif  // XG_WINDOW_H_
