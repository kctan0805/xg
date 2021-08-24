// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserUpdater>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutUpdater>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kWindowViewer) {
    auto lwin_viewer = static_cast<LayoutWindowViewer*>(status->parent.get());
    lwin_viewer->lupdater = node;
  }
#ifdef XG_ENABLE_REALITY
  else if (status->parent->layout_type == LayoutType::kView) {
    auto lview = static_cast<LayoutView*>(status->parent.get());
    lview->lupdater = node;
  }
#endif  // XG_ENABLE_REALITY

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Buffer") == 0) {
      const char* value = child->Attribute("buffer");
      if (value) node->lbuffer_ids.emplace_back(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
