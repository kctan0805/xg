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
bool ParserSingleton<ParserDepthStencilAttachment>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDepthStencilAttachment>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kSubpass);
  auto lsubpass = std::static_pointer_cast<LayoutSubpass>(status->parent);
  node->lsubpass = lsubpass;
  lsubpass->ldepth_stencil_attachment = node;

  node->lattachment_id = element->Attribute("attachment");

  const char* value = element->Attribute("layout");
  if (value) node->layout = StringToImageLayout(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
