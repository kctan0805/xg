// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_WINDOW_GLFW_H_
#define XG_WINDOW_GLFW_H_

#include "xg/window.h"
#pragma warning(push)
#pragma warning(disable : 4819)
#include "GLFW/glfw3.h"
#pragma warning(pop)

namespace xg {

class WindowGLFW : public Window {
 public:
  WindowGLFW() = default;
  ~WindowGLFW() { glfwDestroyWindow(window_); }

  static bool Initialize();
  static void Terminate() { glfwTerminate(); }

  bool Init(const LayoutWindow& lwin) override;

  const void* GetHandle() const override { return window_; }

  void GetFramebufferSize(int* width, int* height) const override {
    glfwGetFramebufferSize(window_, width, height);
  }

  void GetCursorPos(double* xpos, double* ypos) const override {
    glfwGetCursorPos(window_, xpos, ypos);
  }

  int GetAttrib(WindowAttrib attrib) const override {
    return glfwGetWindowAttrib(window_, static_cast<int>(attrib));
  }

  bool ShouldClose() const override {
    return static_cast<bool>(glfwWindowShouldClose(window_));
  }

  void PollEvents() override { glfwPollEvents(); }
  void Iconify() override { glfwIconifyWindow(window_); }
  void Restore() override { glfwRestoreWindow(window_); }

 protected:
  static void ResizeHandler(GLFWwindow* window, int width, int height);
  static void MouseButtonHandler(GLFWwindow* window, int button, int action,
                                 int mods);
  static void MouseMoveHandler(GLFWwindow* window, double posx, double posy);

  GLFWwindow* window_ = nullptr;
};

}  // namespace xg

#endif  // XG_WINDOW_GLFW_H_
