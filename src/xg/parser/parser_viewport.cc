// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

bool ParserSingleton<ParserViewport>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutViewport>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kViewportState) {
    auto lviewport_state =
        std::static_pointer_cast<LayoutViewportState>(status->parent);
    lviewport_state->lviewports.emplace_back(node);
  } else {
    assert(status->parent->layout_type == LayoutType::kSetViewport);
    auto lset_viewport =
        std::static_pointer_cast<LayoutSetViewport>(status->parent);
    lset_viewport->lviewports.emplace_back(node);
  }

  const char* value = element->Attribute("x");
  if (value) node->viewport.x = Expression::Get().Evaluate(value);

  value = element->Attribute("y");
  if (value) node->viewport.y = Expression::Get().Evaluate(value);

  value = element->Attribute("width");
  if (value) node->viewport.width = Expression::Get().Evaluate(value);

  value = element->Attribute("height");
  if (value) node->viewport.height = Expression::Get().Evaluate(value);

  element->QueryFloatAttribute("minDepth", &node->viewport.min_depth);
  element->QueryFloatAttribute("maxDepth", &node->viewport.max_depth);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
