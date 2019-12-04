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

bool ParserSingleton<ParserFramebuffer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutFramebuffer>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kFrame) {
    node->lframe = std::static_pointer_cast<LayoutFrame>(status->parent);
  }

  node->lrender_pass_id = element->Attribute("renderPass");

  const char* value = element->Attribute("width");
  if (value) node->width = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("height");
  if (value) node->height = static_cast<int>(Expression::Get().Evaluate(value));

  element->QueryIntAttribute("layers", &node->layers);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Attachment") == 0) {
      LayoutFramebufferAttachment child_node;

      child_node.lswapchain_id = child->Attribute("swapchain");
      child_node.limage_view_id = child->Attribute("imageView");

      node->lattachments.emplace_back(child_node);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
