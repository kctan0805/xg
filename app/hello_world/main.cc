// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>
#include <utility>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED

#ifndef NDEBUG
#include <vld.h>  // Visual Leak Detector
#endif
#endif  // _MSC_VER

#include "SDL_main.h"
#include "hello_world.h"
#include "xg/engine.h"
#include "xg/layout.h"

int main() {
  Application app;

  auto layout = app.CreateLayout();
  if (!layout) return EXIT_FAILURE;

  auto& engine = xg::Engine::Get();

  if (!engine.Init(std::move(layout))) return EXIT_FAILURE;
  if (!app.Init(&engine)) return EXIT_FAILURE;

  engine.Run();

  return 0;
}
