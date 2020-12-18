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

bool ParserSingleton<ParserDescriptorSetLayoutBinding>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorSetLayoutBinding>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kDescriptorSetLayout);
  auto ldesc_set_layout =
      static_cast<LayoutDescriptorSetLayout*>(status->parent.get());
  ldesc_set_layout->ldesc_set_layout_bindings.emplace_back(node);

  element->QueryIntAttribute("binding", &node->binding);

  const char* value = element->Attribute("descriptorType");
  if (value) node->desc_type = StringToDescriptorType(value);

  value = element->Attribute("descriptorCount");
  if (value)
    node->desc_count = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("stageFlags");
  if (value) node->stage_flags = StringToShaderStageFlags(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
