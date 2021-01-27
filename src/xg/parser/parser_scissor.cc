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

template <>
bool ParserSingleton<ParserScissor>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutScissor>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kViewportState) {
    auto lviewport_state =
        std::static_pointer_cast<LayoutViewportState>(status->parent);
    lviewport_state->lscissors.emplace_back(node);
  } else {
    assert(status->parent->layout_type == LayoutType::kSetScissor);
    auto lset_scissor =
        std::static_pointer_cast<LayoutSetScissor>(status->parent);
    lset_scissor->lscissors.emplace_back(node);
  }

  const char* value = element->Attribute("x");
  if (value) node->rect.x = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("y");
  if (value) node->rect.y = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("width");
  if (value)
    node->rect.width = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("height");
  if (value)
    node->rect.height = static_cast<int>(Expression::Get().Evaluate(value));

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
