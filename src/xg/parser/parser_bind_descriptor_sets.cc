// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cassert>
#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

bool ParserSingleton<ParserBindDescriptorSets>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBindDescriptorSets>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  const char* value = element->Attribute("pipelineBindPoint");
  if (value) node->pipeline_bind_point = StringToPipelineBindPoint(value);

  value = element->Attribute("layout");
  if (value) node->llayout_id = value;

  element->QueryIntAttribute("firstSet", &node->first_set);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "DescriptorSet") == 0) {
      value = child->Attribute("descriptorSet");
      if (value) node->ldesc_set_ids.emplace_back(value);
    } else if (strcmp(name, "DynamicOffset") == 0) {
      LayoutDynamicOffset ldynamic_offset;

      value = child->Attribute("unit");
      if (value) {
        ldynamic_offset.unit =
            static_cast<int>(Expression::Get().Evaluate(value));
      }

      value = child->Attribute("unitSize");
      if (value) {
        ldynamic_offset.unit_size =
            static_cast<int>(Expression::Get().Evaluate(value));
      }

      ldynamic_offset.lbuffer_id = child->Attribute("buffer");

      node->ldynamic_offsets.emplace_back(ldynamic_offset);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
