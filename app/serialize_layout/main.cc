// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <fstream>
#include <memory>
#include <utility>

#if defined(_MSC_VER) && !defined(NDEBUG)
#include <vld.h>  // Visual Leak Detector
#endif

#include "serialize_layout.h"
#include "xg/engine.h"
#include "xg/layout.h"

int main() {
  Application app;
  auto layout = app.CreateLayout();
  if (!layout) return EXIT_FAILURE;

  if (!layout->Serialize("serialize_layout.bin")) return EXIT_FAILURE;

  layout = xg::Layout::Deserialize("serialize_layout.bin");
  if (!layout) return EXIT_FAILURE;

  auto& engine = xg::Engine::Get();

  if (!engine.Init(std::move(layout))) return EXIT_FAILURE;
  if (!app.Init(&engine)) return EXIT_FAILURE;

  engine.Run();

  return 0;
}
