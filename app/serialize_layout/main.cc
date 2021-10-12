// xg - XML Graphics Engine
// Copyright (c) Jim Tan <kctan.tw@gmail.com>
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>
#include <utility>

#include "SDL_main.h"
#include "serialize_layout.h"
#include "xg/engine.h"
#include "xg/layout.h"

int SDL_main(int argc, char* argv[]) {
  Application app;

#ifdef UNSERIALIZE_LAYOUT
  auto layout = xg::Layout::Deserialize("serialize_layout.bin");
  if (!layout) return EXIT_FAILURE;
#else
  auto layout = app.CreateLayout();
  if (!layout) return EXIT_FAILURE;

  if (!layout->Serialize("serialize_layout.bin")) return EXIT_FAILURE;
#endif // UNSERIALIZE_LAYOUT

  auto& engine = xg::Engine::Get();

  if (!engine.Init(std::move(layout))) return EXIT_FAILURE;
  if (!app.Init(&engine)) return EXIT_FAILURE;

  engine.Run();

  return 0;
}
