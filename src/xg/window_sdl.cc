// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/window_sdl.h"

#include <functional>

#include "SDL2/SDL.h"
#include "xg/logger.h"

namespace xg {

static void LogOutputFunction(void* userdata, int category,
                              SDL_LogPriority priority, const char* message) {
  if (priority & SDL_LOG_PRIORITY_VERBOSE) {
    XG_TRACE(message);
  } else if (priority & SDL_LOG_PRIORITY_DEBUG) {
    XG_DEBUG(message);
  } else if (priority & SDL_LOG_PRIORITY_INFO) {
    XG_INFO(message);
  } else if (priority & SDL_LOG_PRIORITY_WARN) {
    XG_WARN(message);
  } else if (priority & SDL_LOG_PRIORITY_ERROR) {
    XG_ERROR(message);
  } else if (priority & SDL_LOG_PRIORITY_CRITICAL) {
    XG_CRITICAL(message);
  }
}

bool WindowSDL::Initialize() {
  SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    XG_ERROR(SDL_GetError());
    return false;
  }

  return true;
}

void WindowSDL::PollEvents() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_MOUSEMOTION: {
        if (SDL_GetWindowID(window_) == e.button.windowID) {
          mouse_move_handler_(e.button.x, e.button.y);
        }
      } break;

      case SDL_MOUSEBUTTONDOWN: {
        if (SDL_GetWindowID(window_) == e.button.windowID) {
          mouse_down_handler_(static_cast<MouseButton>(e.button.button),
                              e.button.x, e.button.y);
        }
      } break;

      case SDL_MOUSEBUTTONUP: {
        if (SDL_GetWindowID(window_) == e.button.windowID) {
          mouse_up_handler_(static_cast<MouseButton>(e.button.button),
                            e.button.x, e.button.y);
        }
      } break;

      case SDL_WINDOWEVENT: {
        if (SDL_GetWindowID(window_) == e.window.windowID) {
          switch (e.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              resize_handler_(e.window.data1, e.window.data2);
              break;

            case SDL_WINDOWEVENT_RESTORED: {
              int w, h;
              GetDrawableSize(&w, &h);
              resize_handler_(w, h);
            } break;

            case SDL_WINDOWEVENT_CLOSE:
              closing_ = true;
              break;
          }
        }
      } break;

      case SDL_QUIT: {
        closing_ = true;
      } break;
    }
  }
}

}  // namespace xg
