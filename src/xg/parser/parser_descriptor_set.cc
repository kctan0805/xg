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

bool ParserSingleton<ParserDescriptorSet>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorSet>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kDescriptorPool) {
    node->ldesc_pool = std::static_pointer_cast<LayoutDescriptorPool>(status->parent);
  } else {
    if (status->parent->layout_type == LayoutType::kFrame) {
      node->lframe = std::static_pointer_cast<LayoutFrame>(status->parent);
    }
    node->ldesc_pool_id = element->Attribute("descriptorPool");
  }

  node->lset_layout_id = element->Attribute("setLayout");

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
