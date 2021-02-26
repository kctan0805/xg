// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_WINDOW_SDL_H_
#define XG_WINDOW_SDL_H_

#include "SDL2/SDL.h"
#include "xg/window.h"

namespace xg {

class WindowSDL : public Window {
 public:
  static bool Initialize();
  static void Terminate() { SDL_Quit(); }

  WindowSDL() = default;
  ~WindowSDL() { SDL_DestroyWindow(window_); }

  void* GetHandle() const override { return window_; }

  WindowFlags GetWindowFlags() const override {
    return static_cast<WindowFlags>(SDL_GetWindowFlags(window_));
  }

  bool ShouldClose() const override { return closing_; }

  void PollEvents() override;

 protected:
  SDL_Window* window_ = nullptr;
  bool closing_ = false;
};

}  // namespace xg

#endif  // XG_WINDOW_SDL_H_
