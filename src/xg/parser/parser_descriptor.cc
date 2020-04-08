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

bool ParserSingleton<ParserDescriptor>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptor>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kDescriptorSet);
  auto ldesc_set = static_cast<LayoutDescriptorSet*>(status->parent.get());
  ldesc_set->ldescriptors.emplace_back(node);

  element->QueryIntAttribute("binding", &node->binding);

  const char* value = element->Attribute("descriptorCount");
  if (value)
    node->desc_count = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("descriptorType");
  if (value) node->desc_type = StringToDescriptorType(value);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
