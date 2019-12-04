// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>
#include <utility>

#if defined(_MSC_VER) && !defined(NDEBUG)
#include <vld.h>  // Visual Leak Detector
#endif

#include "change_layout.h"
#include "xg/engine.h"
#include "xg/layout.h"

int main() {
  auto& engine = xg::Engine::Get();
  int index = 0;

  for (;;) {
    auto app = std::make_unique<Application>();
    if (!app) return EXIT_FAILURE;

    auto layout = app->CreateLayout(index);
    if (!layout) return EXIT_FAILURE;

    if (index == 0) {
      if (!engine.Init(std::move(layout))) return EXIT_FAILURE;
    } else {
      if (!engine.Load(std::move(layout))) return EXIT_FAILURE;
    }
    if (!app->Init(&engine)) return EXIT_FAILURE;

    if (engine.Run() != xg::Result::kTimeout) break;
    ++index;
  }
  return 0;
}
