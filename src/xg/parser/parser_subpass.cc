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

bool ParserSingleton<ParserSubpass>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutSubpass>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kRenderPass);
  auto lrender_pass =
      std::static_pointer_cast<LayoutRenderPass>(status->parent);
  node->lrender_pass = lrender_pass;
  lrender_pass->lsubpasses.emplace_back(node);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
