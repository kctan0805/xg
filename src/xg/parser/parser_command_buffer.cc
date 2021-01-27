// xg - XML Graphics Engine
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
bool ParserSingleton<ParserCommandBuffer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCommandBuffer>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kCommandPool) {
    node->lcmd_pool =
        std::static_pointer_cast<LayoutCommandPool>(status->parent);
  } else {
    if (status->parent->layout_type == LayoutType::kFrame) {
      node->lframe = std::static_pointer_cast<LayoutFrame>(status->parent);
    }
    node->lcmd_pool_id = element->Attribute("commandPool");
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
