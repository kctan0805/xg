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
bool ParserSingleton<ParserDependency>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDependency>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kRenderPass);
  auto lrender_pass =
      std::static_pointer_cast<LayoutRenderPass>(status->parent);
  node->lrender_pass = lrender_pass;
  lrender_pass->ldependencies.emplace_back(node);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Source") == 0) {
      node->lsrc_subpass_id = child->Attribute("subpass");

      const char* value = child->Attribute("stageMask");
      if (value) node->src_stage_mask = StringToPipelineStageFlags(value);

      value = child->Attribute("accessMask");
      if (value) node->src_access_mask = StringToAccessFlags(value);

    } else if (strcmp(name, "Destination") == 0) {
      node->ldst_subpass_id = child->Attribute("subpass");

      const char* value = child->Attribute("stageMask");
      if (value) node->dst_stage_mask = StringToPipelineStageFlags(value);

      value = child->Attribute("accessMask");
      if (value) node->dst_access_mask = StringToAccessFlags(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
