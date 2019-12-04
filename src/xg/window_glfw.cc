// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/window_glfw.h"

#include <functional>

#pragma warning(push)
#pragma warning(disable : 4819)
#include "GLFW/glfw3.h"
#pragma warning(pop)
#include "xg/logger.h"

namespace xg {

static void GlfwErrorCallback(int error, const char* description) {
  XG_ERROR(description);
}

bool WindowGLFW::Initialize() {
  glfwSetErrorCallback(GlfwErrorCallback);

  if (!glfwInit()) {
    XG_ERROR("glfw init fail");
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  return true;
}

bool WindowGLFW::Init(const LayoutWindow& lwin) {
  glfwWindowHint(GLFW_RESIZABLE, lwin.resizable ? GLFW_TRUE : GLFW_FALSE);
  window_ = glfwCreateWindow(lwin.width, lwin.height, lwin.title.c_str(),
                             nullptr, nullptr);
  if (!window_) {
    XG_ERROR("create glfw window fail");
    return false;
  }

  if (lwin.xpos != -1 && lwin.ypos != -1)
    glfwSetWindowPos(window_, lwin.xpos, lwin.ypos);

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, WindowGLFW::ResizeHandler);
  glfwSetMouseButtonCallback(window_, WindowGLFW::MouseButtonHandler);
  glfwSetCursorPosCallback(window_, WindowGLFW::MouseMoveHandler);

  return true;
}

void WindowGLFW::ResizeHandler(GLFWwindow* window, int width, int height) {
  const auto& self = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
  self->resize_handler_(width, height);
}

void WindowGLFW::MouseButtonHandler(GLFWwindow* window, int button, int action,
                                    int mods) {
  const auto& self = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
  self->mouse_button_handler_(static_cast<MouseButton>(button),
                              static_cast<ButtonAction>(action),
                              static_cast<ModifierKey>(mods));
}

void WindowGLFW::MouseMoveHandler(GLFWwindow* window, double posx,
                                  double posy) {
  const auto& self = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
  self->mouse_move_handler_(posx, posy);
}

}  // namespace xg
