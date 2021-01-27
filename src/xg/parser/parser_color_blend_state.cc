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
bool ParserSingleton<ParserColorBlendState>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutColorBlendState>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kGraphicsPipeline);
  auto lgraphics_pipeline =
      static_cast<LayoutGraphicsPipeline*>(status->parent.get());
  lgraphics_pipeline->lcolor_blend_state = node;

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Attachment") == 0) {
      LayoutColorBlendAttachmentState child_node;

      child->QueryBoolAttribute("blendEnable", &child_node.blend_enable);

      const char* value = child->Attribute("srcColorBlendFactor");
      if (value) child_node.src_color_blend_factor = StringToBlendFactor(value);

      value = child->Attribute("dstColorBlendFactor");
      if (value) child_node.dst_color_blend_factor = StringToBlendFactor(value);

      value = child->Attribute("colorBlendOp");
      if (value) child_node.color_blend_op = StringToBlendOp(value);

      value = child->Attribute("srcAlphaBlendFactor");
      if (value) child_node.src_alpha_blend_factor = StringToBlendFactor(value);

      value = child->Attribute("dstAlphaBlendFactor");
      if (value) child_node.dst_alpha_blend_factor = StringToBlendFactor(value);

      value = child->Attribute("alphaBlendOp");
      if (value) child_node.alpha_blend_op = StringToBlendOp(value);

      value = child->Attribute("colorWriteMask");
      if (value) child_node.color_write_mask = StringToColorComponent(value);

      node->lcolor_blend_attachments.emplace_back(child_node);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
