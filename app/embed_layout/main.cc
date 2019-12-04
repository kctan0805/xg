// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
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

#include "embed_layout.h"
#include "embed/embed.h"
#include "xg/engine.h"
#include "xg/layout.h"

#ifndef LAYOUT_PARSER
EMBED_DECL(LAYOUT);
#endif

int main() {
  Application app;

#ifdef LAYOUT_PARSER
  auto layout = app.CreateLayout();
  if (!layout) return EXIT_FAILURE;

  if (!layout->Serialize("embed_layout.bin")) return EXIT_FAILURE;

#else
  const auto& resource = LAYOUT();

  auto layout = xg::Layout::Deserialize(reinterpret_cast<const uint8_t*>(resource.data), resource.size);
  if (!layout) return EXIT_FAILURE;

  auto& engine = xg::Engine::Get();

  if (!engine.Init(std::move(layout))) return EXIT_FAILURE;
  if (!app.Init(&engine)) return EXIT_FAILURE;

  engine.Run();
#endif // LAYOUT_PARSER

  return 0;
}
